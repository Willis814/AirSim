// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

using UnrealBuildTool;
using System.IO;

public class AirSim : ModuleRules
{
    private string ModulePath
    {
        get { return ModuleDirectory; }
    }

    private string AirLibPath
    {
        get { return Path.Combine(ModulePath, "AirLib"); }
    }
    private string AirSimPluginPath
    {
        get { return Directory.GetParent(ModulePath).FullName; }
    }
    private string ProjectBinariesPath
    {
        get
        {
            return Path.Combine(
              Directory.GetParent(AirSimPluginPath).Parent.FullName, "Binaries");
        }
    }
    private string AirSimPluginDependencyPath
    {
        get { return Path.Combine(AirSimPluginPath, "Dependencies"); }
    }

    private enum CompileMode
    {
        HeaderOnlyNoRpc,
        HeaderOnlyWithRpc,
        CppCompileNoRpc,
        CppCompileWithRpc
    }

    private void SetupCompileMode(CompileMode mode, ReadOnlyTargetRules Target)
    {
        if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            PublicAdditionalLibraries.Add(Path.Combine(AirLibPath, "deps", "MotionCore", "lib", "ExcelFormat" + ".a"));
        }

        LoadAirSimDependency(Target, "MavLinkCom", "MavLinkCom");
        LoadAirSimDependency(Target, "MotionCore", "MotionCore", "MotionCore");

        switch (mode)
        {
            case CompileMode.HeaderOnlyNoRpc:
                PublicDefinitions.Add("AIRLIB_HEADER_ONLY=1");
                PublicDefinitions.Add("AIRLIB_NO_RPC=1");
                AddLibDependency("AirLib", Path.Combine(AirLibPath, "lib"), "AirLib", Target, false);
                break;

            case CompileMode.HeaderOnlyWithRpc:
                PublicDefinitions.Add("AIRLIB_HEADER_ONLY=1");
                AddLibDependency("AirLib", Path.Combine(AirLibPath, "lib"), "AirLib", Target, false);
                LoadAirSimDependency(Target, "rpclib", "rpc");
                break;

            case CompileMode.CppCompileNoRpc:
                LoadAirSimDependency(Target, "MavLinkCom", "MavLinkCom");
                PublicDefinitions.Add("AIRLIB_NO_RPC=1");
                break;

            case CompileMode.CppCompileWithRpc:
                LoadAirSimDependency(Target, "rpclib", "rpc");
                break;

            default:
                throw new System.Exception("CompileMode specified in plugin's Build.cs file is not recognized");
        }

    }

    public AirSim(ReadOnlyTargetRules Target) : base(Target)
    {
        //bEnforceIWYU = true; //to support 4.16
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        bEnableExceptions = true;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "ImageWrapper", "RenderCore", "RHI", "AssetRegistry", "PhysicsCore", "PhysXVehicles", "PhysXVehicleLib", "PhysX", "APEX", "Landscape", "Projects", "CinematicCamera" });
        PrivateDependencyModuleNames.AddRange(new string[] { "UMG", "Slate", "SlateCore" });

        //suppress VC++ proprietary warnings
        PublicDefinitions.Add("_SCL_SECURE_NO_WARNINGS=1");
        PublicDefinitions.Add("_CRT_SECURE_NO_WARNINGS=1");
        PublicDefinitions.Add("HMD_MODULE_INCLUDED=0");

        PublicIncludePaths.Add(Path.Combine(AirLibPath, "include"));
        PublicIncludePaths.Add(Path.Combine(AirLibPath, "deps", "eigen3"));
        PublicIncludePaths.Add(Path.Combine(AirLibPath, "deps", "MotionCore", "include"));
        AddOSLibDependencies(Target);

        AddMotionCoreConfigFiles(Path.Combine(AirLibPath, "deps", "MotionCore"));

        SetupCompileMode(CompileMode.HeaderOnlyWithRpc, Target);

        bUseRTTI = true;
    }

    private void AddOSLibDependencies(ReadOnlyTargetRules Target)
    {
        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            // for SHGetFolderPath.
            PublicAdditionalLibraries.Add("Shell32.lib");

            //for joystick support
            PublicAdditionalLibraries.Add("dinput8.lib");
            PublicAdditionalLibraries.Add("dxguid.lib");
        }

        if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            // needed when packaging
            PublicAdditionalLibraries.Add("stdc++");
            PublicAdditionalLibraries.Add("supc++");
        }
    }

    static void CopyFileIfNewer(string srcFilePath, string destFolder)
    {
        FileInfo srcFile = new FileInfo(srcFilePath);
        FileInfo destFile = new FileInfo(Path.Combine(destFolder, srcFile.Name));
        if (!destFile.Exists || srcFile.LastWriteTime > destFile.LastWriteTime)
        {
            srcFile.CopyTo(destFile.FullName, true);
        }
        //else skip
    }

    private bool LoadAirSimDependency(ReadOnlyTargetRules Target, string LibName, string LibFileName, string DllFileName = null)
    {
        string LibrariesPath = Path.Combine(AirLibPath, "deps", LibName, "lib");
        return AddLibDependency(LibName, LibrariesPath, LibFileName, Target, true, DllFileName);
    }

    private bool AddLibDependency(string LibName, string LibPath, string LibFileName, ReadOnlyTargetRules Target, bool IsAddLibInclude, string DllFileName = null)
    {
        string PlatformString = (Target.Platform == UnrealTargetPlatform.Win64 || Target.Platform == UnrealTargetPlatform.Mac) ? "x64" : "x86";
        string ConfigurationString = (Target.Configuration == UnrealTargetConfiguration.Debug) ? "Debug" : "Release";
        bool isLibrarySupported = false;


        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            isLibrarySupported = true;

            PublicAdditionalLibraries.Add(Path.Combine(LibPath, PlatformString, ConfigurationString, LibFileName + ".lib"));

            if (DllFileName != null)
            {
                PublicDelayLoadDLLs.Add(DllFileName + ".dll");
                RuntimeDependencies.Add(Path.Combine(LibPath, PlatformString, ConfigurationString, DllFileName + ".dll"));
            }

        }
        else if (Target.Platform == UnrealTargetPlatform.Linux || Target.Platform == UnrealTargetPlatform.Mac)
        {
            isLibrarySupported = true;
            if (LibFileName != "MotionCore")
            {
                PublicAdditionalLibraries.Add(Path.Combine(LibPath, "lib" + LibFileName + ".a"));
            }
            if (DllFileName != null)
            {
                //PublicAdditionalLibraries.Add(DllFileName + ".so");
                PublicAdditionalLibraries.Add(Path.Combine(LibPath, "lib" + DllFileName + ".so"));
                //PublicDelayLoadDLLs.Add(Path.Combine(LibPath, "lib" + DllFileName + ".so"));
                RuntimeDependencies.Add(Path.Combine(LibPath, "lib" + DllFileName + ".so"));
            }
        }

        if (isLibrarySupported && IsAddLibInclude)
        {
            // Include path
            PublicIncludePaths.Add(Path.Combine(AirLibPath, "deps", LibName, "include"));
            PublicIncludePaths.Add(Path.Combine(AirLibPath, "deps", LibName, "include", LibName));
        }
        PublicDefinitions.Add(string.Format("WITH_" + LibName.ToUpper() + "_BINDING={0}", isLibrarySupported ? 1 : 0));

        return isLibrarySupported;
    }

    private void AddMotionCoreConfigFiles(string path)
    {
        string[] configFiles = { "Probot3DMulti.xls", "Rook3DMulti.xls" };

        for (int i = 0; i < configFiles.Length; i++)
        {
            RuntimeDependencies.Add(Path.Combine(path, configFiles[i]));
        }
    }

}
