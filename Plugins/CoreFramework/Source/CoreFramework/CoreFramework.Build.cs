// Tencent is pleased to support the open source community by making UnLua available.
// 
// Copyright (C) 2019 THL A29 Limited, a Tencent company. All rights reserved.
//
// Licensed under the MIT License (the "License"); 
// you may not use this file except in compliance with the License. You may obtain a copy of the License at
//
// http://opensource.org/licenses/MIT
//
// Unless required by applicable law or agreed to in writing, 
// software distributed under the License is distributed on an "AS IS" BASIS, 
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
// See the License for the specific language governing permissions and limitations under the License.

using System;
using System.IO;
#if UE_5_0_OR_LATER
using EpicGames.Core;
#else
using Tools.DotNETCommon;
#endif
using UnrealBuildTool;

public class CoreFramework : ModuleRules
{
    public CoreFramework(ReadOnlyTargetRules Target) : base(Target)
    {
#if UE_5_2_OR_LATER
        IWYUSupport = IWYUSupport.None;
#else
        bEnforceIWYU = false;
#endif
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(
            new string[]
            {
            }
        );

        PrivateIncludePaths.AddRange(
            new[]
            {
                "CoreFramework/Private",
            }
        );

        PublicDependencyModuleNames.AddRange(
            new[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "Slate",
                "InputCore",
            }
        );

        if (Target.bBuildEditor)
        {
            OptimizeCode = CodeOptimization.Never;
            PrivateDependencyModuleNames.Add("UnrealEd");
        }
    }

    private bool IsPluginEnabled(string name)
    {
        var engineDir = DirectoryReference.FromString(EngineDirectory);
        var projectDir = Target.ProjectFile.Directory;
        var projectDesc = ProjectDescriptor.FromFile(Target.ProjectFile);
        
        foreach (var plugin in Plugins.ReadAvailablePlugins(engineDir, projectDir, null))
        {
            if (plugin.Name != name)
                continue;
            return Plugins.IsPluginEnabledForTarget(plugin, projectDesc, Target.Platform, Target.Configuration, Target.Type);
        }

        return false;
    }
}