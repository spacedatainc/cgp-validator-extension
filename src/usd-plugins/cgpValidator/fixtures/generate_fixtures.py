#!/usr/bin/env python3

from pxr import Sdf, Usd, UsdGeom, Vt


def write_stage(path, asset_path=None, rtx_ocio_cfg_file_path=None):
    stage = Usd.Stage.CreateNew(path)
    UsdGeom.SetStageUpAxis(stage, UsdGeom.Tokens.y)

    root = stage.DefinePrim("/Root", "Xform")
    if asset_path is not None:
        attr = root.CreateAttribute("assetPath", Sdf.ValueTypeNames.Asset)
        attr.Set(Sdf.AssetPath(asset_path))
    if rtx_ocio_cfg_file_path is not None:
        root_layer = stage.GetRootLayer()
        custom_layer_data = root_layer.customLayerData
        render_settings = custom_layer_data.get("renderSettings", {})
        render_settings["rtx:post:tonemap:ocio:cfgFilePath"] = Vt.Token(rtx_ocio_cfg_file_path)
        custom_layer_data["renderSettings"] = render_settings
        root_layer.customLayerData = custom_layer_data

    stage.GetRootLayer().Save()


if __name__ == "__main__":
    write_stage("fixtures/drive_letter_valid.usda", asset_path="/workspace/test_asset.usd")
    write_stage("fixtures/drive_letter_invalid.usda", asset_path="C:/workspace/test_asset.usd")
    write_stage("fixtures/drive_letter_nonstandard_ar.usda", asset_path="file:///C:/workspace/test_asset.usd")
    write_stage(
        "fixtures/rtx_ocio_config_path_valid.usda",
        rtx_ocio_cfg_file_path="/Omniverse/OCIO/config.ocio")
    write_stage(
        "fixtures/rtx_ocio_config_path_invalid.usda",
        rtx_ocio_cfg_file_path="/home/masahikokoyama/.cache/packman/chk/kit-kernel/rendering-data/runtime/cg-config-v1.0.0_aces-v1.3_ocio-v2.1.ocio")
