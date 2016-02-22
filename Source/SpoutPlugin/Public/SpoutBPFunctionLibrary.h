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

USTRUCT()
struct FSenderStruct
{
	GENERATED_USTRUCT_BODY()

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spout Struct")
		FName sName;

		//UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spout Struct")
		HANDLE sHandle;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spout Struct")
		int32 w;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spout Struct")
		int32 h;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spout Struct")
		int32 SenderID;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spout Struct")
		UTexture2D* TextureColor;

		UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spout Struct")
		UMaterialInstanceDynamic* MaterialInstanceColor;

		// Pointer to our Texture's resource
		FTexture2DResource* Texture2DResource;

		// Regions we need to update
		FUpdateTextureRegion2D* UpdateRegions;

		//Sender
		ID3D11Texture2D *activeTextures;

	void SetName(FName NewsName)
	{
		sName = NewsName;
	}
	void SetSenderID(int32 NewSenderID)
	{
		SenderID = NewSenderID;
	}
	void SetHandle(HANDLE NewsHandle)
	{
		sHandle = NewsHandle;
	}
	void SetW(int32 NewW)
	{
		w = NewW;
	}
	void SetH(int32 NewH)
	{
		h = NewH;
	}

	FSenderStruct(){

	}

};

UENUM(BlueprintType)
enum class ESpoutSendTextureFrom
{
	GameViewport,
	TextureRenderTarget2D
};

UCLASS(ClassGroup = Spout, Blueprintable)
class USpoutBPFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	static bool CreateSender(FName SenderName, ID3D11Texture2D* baseTexture, int32 texFormatIndex);

	UFUNCTION(BlueprintCallable, Category = "Spout", meta = (AdvancedDisplay = "2"))
		static bool SpoutSender(FName SenderName, ESpoutSendTextureFrom sendTextureFrom, UTextureRenderTarget2D* textureRenderTarget2D);

	UFUNCTION(BlueprintCallable, Category = "Spout")
		static void CloseSender(FName SenderName);

	UFUNCTION(BlueprintCallable, Category = "Spout")
		static bool SpoutReceiver(const FName SenderName, UMaterialInstanceDynamic*& mat);
	
	UFUNCTION(BlueprintCallable, Category = "Spout")
		static bool SpoutInfo(TArray<FSenderStruct>& Senders);
	
	UFUNCTION(BlueprintCallable, Category = "Spout")
		static bool SpoutInfoFrom(FName SenderName, FSenderStruct& SenderStruct);
};