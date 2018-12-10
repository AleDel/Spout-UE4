// Some copyright should be here...

using UnrealBuildTool;
using System.IO;


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

        PublicIncludePaths.AddRange(
            new string[] {
				"SpoutPlugin/Public",
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
			
		/*Type = ModuleType.External;

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			// Add the import library
			PublicLibraryPaths.Add(Path.Combine(ModuleDirectory, "x64", "Release"));
			PublicAdditionalLibraries.Add("ExampleLibrary.lib");

			// Delay-load the DLL, so we can load it from the right place first
			PublicDelayLoadDLLs.Add("ExampleLibrary.dll");
		}
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            PublicDelayLoadDLLs.Add(Path.Combine(ModuleDirectory, "Mac", "Release", "libExampleLibrary.dylib"));
        }*/

        if ((Target.Platform == UnrealTargetPlatform.Win64) || (Target.Platform == UnrealTargetPlatform.Win32))
        {
            string PlatformString = (Target.Platform == UnrealTargetPlatform.Win64) ? "amd64" : "x86";
            PublicAdditionalLibraries.Add(Path.Combine(ThirdPartyPath, "Spout/lib", PlatformString, "Spout.lib"));

            RuntimeDependencies.Add(new RuntimeDependency(Path.Combine(ThirdPartyPath, "Spout/lib", PlatformString, "Spout.dll")));
            
            // Delay-load the DLL, so we can load it from the right place first
            //PublicDelayLoadDLLs.Add(Path.Combine(ThirdPartyPath, "Spout/lib", PlatformString, "Spout.dll"));
            
            /*PublicIncludePaths.AddRange(
                new string[] {
					Path.Combine(ThirdPartyPath, "Spout/include")
				});*/
        }
        
	}
}
