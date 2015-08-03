// Some copyright should be here...

#include "SpoutPluginPrivatePCH.h"

DEFINE_LOG_CATEGORY(SpoutLog);

#define LOCTEXT_NAMESPACE "FSpoutModule"

void FSpoutModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	UE_LOG(SpoutLog, Warning, TEXT("Modulo Spout Cargado"));
}

void FSpoutModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	UE_LOG(SpoutLog, Warning, TEXT("Modulo Spout Descargado"));
}


	
IMPLEMENT_MODULE(FSpoutModule, SpoutPlugin)

#undef LOCTEXT_NAMESPACE