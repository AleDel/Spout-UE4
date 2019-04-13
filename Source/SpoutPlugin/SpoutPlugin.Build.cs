// Some copyright should be here...

using System.IO;

namespace UnrealBuildTool.ModuleRules
{
    public class SpoutPlugin : ModuleRules
{

    private string ModulePath
    {
        get { return ModuleDirectory; }
    }

    private string ThirdPartyPath
    {
        get { return Path.GetFullPath(Path.Combine(ModulePath, "../../ThirdParty/")); }
    }
    
    public SpoutPlugin(ReadOnlyTargetRules Target) : base(Target)
    {
        PrivatePCHHeaderFile = "Private/SpoutPluginPrivatePCH.h";

        PublicIncludePaths.AddRange(
            new string[] {
                Path.Combine(ModulePath, "Public"),
                Path.Combine(ThirdPartyPath, "Spout/include")
                
                // ... add public include paths required here ...
            }
            );
        

        PrivateIncludePaths.AddRange(
            new string[] {
                "SpoutPlugin/Private",
                // ... add other private include paths required here ...
            }
            );

        //AddThirdPartyPrivateStaticDependencies(Target, "Spout");

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "RHI",
                "RenderCore"
                // ... add other public dependencies that you statically link with here ...
            }
            );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "Slate", "SlateCore"
                // ... add private dependencies that you statically link with here ...  
            }
            );


        DynamicallyLoadedModuleNames.AddRange(
            new string[]
            {
                
                // ... add any modules that your module loads dynamically here ...
            }
            );
            
    

        if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))
        {
            string PlatformString = (Target.Platform == UnrealTargetPlatform.Win64) ? "amd64" : "x86";
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyPath, "Spout/lib", PlatformString, "Spout.lib"));

            RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(ThirdPartyPath, "Spout/lib", PlatformString, "Spout.dll")));
            
            // Delay-load the DLL, so we can load it from the right place first
            PublicDelayLoadDLLs.Add(Path.Combine(ThirdPartyPath, "Spout/lib", PlatformString, "Spout.dll"));
            
            
        }
        
    }
}

}
