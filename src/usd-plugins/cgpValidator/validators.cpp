#include <pxr/base/vt/dictionary.h>
#include <pxr/base/vt/value.h>
#include <pxr/base/plug/plugin.h>
#include <pxr/base/plug/registry.h>
#include <pxr/usd/sdf/assetPath.h>
#include <pxr/usd/sdf/layer.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usdUtils/dependencies.h>

#include <pxr/usdValidation/usdValidation/error.h>
#include <pxr/usdValidation/usdValidation/registry.h>
#include <pxr/usdValidation/usdValidation/timeRange.h>
#include <pxr/usdValidation/usdValidation/validator.h>

#include <cctype>
#include <optional>
#include <string>
#include <vector>
#include <iostream>

PXR_NAMESPACE_OPEN_SCOPE

namespace {

// Returns true if `path` is qualified with a URI scheme handled by a
// non-default asset resolver (e.g. "file:", "omniverse:", "usdz:").
// Such paths are resolved by whatever resolver is registered for that
// scheme rather than by ArDefaultResolver's plain filesystem-path
// convention, so this validator only cares about bare paths and leaves
// scheme-qualified paths to whatever validation that resolver performs.
// A Windows drive letter ("C:") is distinguished from a URI scheme by
// length: RFC 3986 schemes are effectively always two or more characters
// (file, http, omniverse, usdz, ...), while a drive letter is exactly one.
static bool HasNonDefaultResolverScheme(const std::string &path)
{
    if (path.empty() || !std::isalpha(static_cast<unsigned char>(path[0]))) {
        return false;
    }
    size_t i = 1;
    while (i < path.size()) {
        const unsigned char ch = static_cast<unsigned char>(path[i]);
        if (std::isalnum(ch) || ch == '+' || ch == '-' || ch == '.') {
            ++i;
            continue;
        }
        break;
    }
    return i >= 2 && i < path.size() && path[i] == ':';
}

static bool HasWindowsDriveLetter(const std::string &path)
{
    if (HasNonDefaultResolverScheme(path)) {
        return false;
    }
    return path.size() >= 3
        && std::isalpha(static_cast<unsigned char>(path[0]))
        && path[1] == ':'
        && (path[2] == '/' || path[2] == '\\');
}

// The Omniverse RTX renderer stores its OCIO config file path as a plain
// filesystem path (not an asset reference) in customLayerData, e.g.:
//   dictionary renderSettings = {
//       token "rtx:post:tonemap:ocio:cfgFilePath" = "..."
//   }
// See ocio_config_path_sample.usda. Because it is never resolved as an
// asset dependency, UsdUtilsComputeAllDependencies never sees it, so it
// needs to be read directly out of customLayerData. Returns nullopt if
// the metadata is not authored at all, as opposed to being authored with
// an empty string.
static std::optional<std::string>
GetRtxOcioConfigFilePath(const UsdStagePtr &stage)
{
    if (!stage || !stage->GetRootLayer()) {
        return std::nullopt;
    }
    const VtDictionary customLayerData =
        stage->GetRootLayer()->GetCustomLayerData();
    const VtValue *value = customLayerData.GetValueAtPath(
        std::vector<std::string> {
            "renderSettings", "rtx:post:tonemap:ocio:cfgFilePath" });
    if (!value) {
        return std::nullopt;
    }
    if (value->IsHolding<TfToken>()) {
        return value->UncheckedGet<TfToken>().GetString();
    }
    if (value->IsHolding<std::string>()) {
        return value->UncheckedGet<std::string>();
    }
    return std::nullopt;
}

static constexpr const char *kExpectedRtxOcioConfigFilePath =
    "/Omniverse/OCIO/config.ocio";

static void DumpJsValue(const JsValue &value, int indent = 0)
{
    const std::string pad(indent, ' ');
    if (value.IsString()) {
        std::cout << pad << "string: " << value.GetString() << std::endl;
        return;
    }
    if (value.IsBool()) {
        std::cout << pad << "bool: " << (value.GetBool() ? "true" : "false")
                  << std::endl;
        return;
    }
    if (value.IsReal()) {
        std::cout << pad << "number: " << value.GetReal() << std::endl;
        return;
    }
    if (value.IsArray()) {
        const JsArray &arr = value.GetJsArray();
        std::cout << pad << "array: [" << arr.size() << "]" << std::endl;
        for (size_t i = 0; i < arr.size(); ++i) {
            std::cout << pad << "  [" << i << "] -> ";
            const JsValue &elem = arr[i];
            if (elem.IsString()) {
                std::cout << elem.GetString() << std::endl;
            } else {
                std::cout << "object" << std::endl;
                DumpJsValue(elem, indent + 4);
            }
        }
        return;
    }
    if (value.IsObject()) {
        std::cout << pad << "object: {" << std::endl;
        const JsObject &obj = value.GetJsObject();
        for (const auto &entry : obj) {
            std::cout << pad << "  key='" << entry.first << "'" << std::endl;
            DumpJsValue(entry.second, indent + 4);
        }
        std::cout << pad << "}" << std::endl;
        return;
    }
    std::cout << pad << "type: unknown" << std::endl;
}

static void AddDriveLetterError(const UsdStagePtr &stage,
                                UsdValidationErrorVector &errors,
                                const std::string &path)
{
    errors.emplace_back(
        TfToken("DriveLetterInAssetPath"),
        UsdValidationErrorType::Error,
        UsdValidationErrorSites { UsdValidationErrorSite(stage, SdfPath("/")) },
        TfStringPrintf("Asset path contains a Windows drive letter: %s",
                       path.c_str()));
}

static UsdValidationErrorVector DriveLetterValidator(
    const UsdStagePtr &stage,
    const UsdValidationTimeRange & /*timeRange*/)
{
    UsdValidationErrorVector errors;
    if (!stage || !stage->GetRootLayer()) {
        return errors;
    }

    std::vector<SdfLayerRefPtr> layers;
    std::vector<std::string> assetPaths;
    std::vector<std::string> unresolvedPaths;

    const std::string rootIdentifier = stage->GetRootLayer()->GetIdentifier();
    if (!rootIdentifier.empty()) {
        const SdfAssetPath rootAssetPath(rootIdentifier);
        UsdUtilsComputeAllDependencies(rootAssetPath,
                                      &layers,
                                      &assetPaths,
                                      &unresolvedPaths);
    }

    for (const std::string &assetPath : assetPaths) {
        if (HasWindowsDriveLetter(assetPath)) {
            AddDriveLetterError(stage, errors, assetPath);
        }
    }

    for (const std::string &assetPath : unresolvedPaths) {
        if (HasWindowsDriveLetter(assetPath)) {
            AddDriveLetterError(stage, errors, assetPath);
        }
    }

    return errors;
}

static UsdValidationErrorVector RtxOcioConfigPathValidator(
    const UsdStagePtr &stage,
    const UsdValidationTimeRange & /*timeRange*/)
{
    UsdValidationErrorVector errors;
    if (!stage || !stage->GetRootLayer()) {
        return errors;
    }

    const std::optional<std::string> actualConfigFilePath =
        GetRtxOcioConfigFilePath(stage);
    if (actualConfigFilePath
        && *actualConfigFilePath != kExpectedRtxOcioConfigFilePath) {
        errors.emplace_back(
            TfToken("RtxOcioConfigFilePathMismatch"),
            UsdValidationErrorType::Error,
            UsdValidationErrorSites { UsdValidationErrorSite(stage, SdfPath("/")) },
            TfStringPrintf(
                "rtx:post:tonemap:ocio:cfgFilePath mismatch. Expected <%s>, got <%s>.",
                kExpectedRtxOcioConfigFilePath,
                actualConfigFilePath->c_str()));
    }

    return errors;
}

} // namespace

TF_REGISTRY_FUNCTION(UsdValidationRegistry)
{
    UsdValidationRegistry &registry = UsdValidationRegistry::GetInstance();
    const TfToken driveLetterValidatorName("cgpValidatorPlugin:DriveLetterValidator");
    const TfToken rtxOcioConfigPathValidatorName("cgpValidatorPlugin:RtxOcioConfigPathValidator");
    
    // `RegisterPluginValidator` fails to register these validators in Omniverse
    // reporting "Validator metadata missing" even though the metadata is present
    // in plugInfo.json. As a workaround, register the validators directly
    // using `RegisterValidator`.

    UsdValidationValidatorMetadata metadata;
    metadata.pluginPtr = PlugRegistry::GetInstance().GetPluginWithName("cgpValidatorPlugin");
    metadata.isTimeDependent = false;
    metadata.isSuite = false;

    metadata.name = driveLetterValidatorName;
    metadata.doc = "Validates that asset paths do not contain Windows drive letters.";
    registry.RegisterValidator(metadata, &DriveLetterValidator);
    
    metadata.name = rtxOcioConfigPathValidatorName;
    metadata.doc = "Validator that tests the rtx:post:tonemap:ocio:cfgFilePath value.";
    registry.RegisterValidator(metadata, &RtxOcioConfigPathValidator);
}

PXR_NAMESPACE_CLOSE_SCOPE
