#pragma once

#if PLATFORM_WINDOWS
#include "AllowWindowsPlatformTypes.h"
#endif
#include "Spout.h"
#include <d3d11.h>
#if PLATFORM_WINDOWS
#include "HideWindowsPlatformTypes.h"
#endif

#include "SpoutBPFunctionLibrary.generated.h"


UCLASS(ClassGroup = Kinect, Blueprintable)
class USpoutBPFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "Spout")
	static bool CreateSender(FName SenderName, UTextureRenderTarget2D* RenderTexture, int32 texFormatIndex);

	UFUNCTION(BlueprintCallable, Category = "Spout")
		static bool UpdateSender(FName SenderName, UTextureRenderTarget2D* RenderTexture);

	UFUNCTION(BlueprintCallable, Category = "Spout")
		static void CloseSender(FName SenderName);

	/*UFUNCTION(BlueprintCallable, Category = "Spout")
		static UMaterialInstanceDynamic* GetMaterialRGB();*/

	UFUNCTION(BlueprintCallable, Category = "Spout")
		static bool Receiver(UMaterialInterface* Base_Material, int32& numSenders, UMaterialInstanceDynamic*& mat);
	//static void Receiver(UMaterialInterface* Base_Material, int32& numSenders, UMaterialInstanceDynamic*& mat);
	
private:
	

};