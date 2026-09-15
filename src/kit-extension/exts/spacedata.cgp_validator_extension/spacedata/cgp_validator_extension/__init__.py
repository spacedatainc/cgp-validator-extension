# SPDX-FileCopyrightText: Copyright (c) 2024 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
# SPDX-License-Identifier: LicenseRef-NvidiaProprietary
#
# NVIDIA CORPORATION, its affiliates and licensors retain all intellectual
# property and proprietary rights in and to this material, related
# documentation and any modifications thereto. Any use, reproduction,
# disclosure or distribution of this material and related documentation
# without an express license agreement from NVIDIA CORPORATION or
# its affiliates is strictly prohibited.

import os
from pxr import Usd, Plug

pluginsRoot = os.path.join(os.path.dirname(__file__), "../../plugins")

cgpValidatorPluginPath = os.path.join(pluginsRoot, "cgpValidatorPlugin", "resources")
plugins = Plug.Registry().RegisterPlugins(cgpValidatorPluginPath)
if plugins and len(plugins) > 0:
    plg = plugins[0]
    plg.Load()
    print(f"CGPValidatorExtension: Registered plugin: {plg.name} from {plg.path}")
else:
    print(f"CGPValidatorExtension: ERROR: Failed to Register plugin from {cgpValidatorPluginPath}")

from omni.asset_validator.core import registerRule, UsdValidatorAdapter
@registerRule("cgpValidatorPlugin")
class DriveLetterValidator(UsdValidatorAdapter):
    @classmethod
    def validator_name(cls) -> str:
        return "cgpValidatorPlugin:DriveLetterValidator"

@registerRule("cgpValidatorPlugin")
class RtxOcioConfigPathValidator(UsdValidatorAdapter):
    @classmethod
    def validator_name(cls) -> str:
        return "cgpValidatorPlugin:RtxOcioConfigPathValidator"