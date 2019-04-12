// Some copyright should be here...

#include "SpoutPluginPrivatePCH.h"

#include "Core.h"
#include "ModuleManager.h"
//#include "IPluginManager.h"

DEFINE_LOG_CATEGORY(SpoutLog);

#define LOCTEXT_NAMESPACE "FSpoutModule"

void FSpoutModule::StartupModule()
{
	/// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	// Get the base directory of this plugin
	//FString BaseDir = IPluginManager::Get().FindPlugin("SpoutUE4")->GetBaseDir();

	/*
	// Add on the relative location of the third party dll and load it
	FString LibraryPath;
#if PLATFORM_WINDOWS
	LibraryPath = FPaths::Combine(*BaseDir, TEXT("Binaries/ThirdParty/SpoutUE4Library/Win64/Spout.dll"));
	//LibraryPath = FPaths::Combine(*BaseDir, TEXT("Plugins/Spout/ThirdParty/Spout/lib/amd64/Spout.dll"));

#elif PLATFORM_MAC
    //LibraryPath = FPaths::Combine(*BaseDir, TEXT("Source/ThirdParty/SpoutUE4Library/Mac/Release/libExampleLibrary.dylib"));
#endif // PLATFORM_WINDOWS

	ExampleLibraryHandle = !LibraryPath.IsEmpty() ? FPlatformProcess::GetDllHandle(*LibraryPath) : nullptr;

	if (ExampleLibraryHandle)
	{
		UE_LOG(SpoutLog, Warning, TEXT("Modulo Spout Cargado"));
		// Call the test function in the third party library that opens a message box
		//ExampleLibraryFunction();
	}
	else
	{
		FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("ThirdPartyLibraryError", "Failed to load example third party library"));
	}*/
	UE_LOG(SpoutLog, Warning, TEXT("Modulo Spout Cargado"));
}

void FSpoutModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	
	// Free the dll handle
	/*FPlatformProcess::FreeDllHandle(ExampleLibraryHandle);
	ExampleLibraryHandle = nullptr;*/
	
	UE_LOG(SpoutLog, Warning, TEXT("Modulo Spout Descargado"));
}


	
IMPLEMENT_MODULE(FSpoutModule, SpoutPlugin)

#undef LOCTEXT_NAMESPACE