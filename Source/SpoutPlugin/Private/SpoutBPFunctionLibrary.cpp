#include "SpoutPluginPrivatePCH.h"
#include "../Public/SpoutBPFunctionLibrary.h"

#include <string>
#include <iostream>
#include <iomanip>
#include <sstream>



static ID3D11Device* g_D3D11Device;
ID3D11DeviceContext* g_pImmediateContext = NULL;

spoutSenderNames * sender;
spoutGLDXinterop * interop;
spoutDirectX * sdx;

TArray<FSenderStruct> FSenders;

UMaterialInterface* BaseMaterial;

FName TextureParameterName = "SpoutTexture";




void DestroyTexture(UTexture2D*& Texture)
{
	// Here we destory the texture and its resource
	if (Texture){
		Texture->RemoveFromRoot();

		if (Texture->Resource)
		{
			BeginReleaseResource(Texture->Resource);
			FlushRenderingCommands();
		}

		Texture->MarkPendingKill();
		Texture = nullptr;
	}else{
		UE_LOG(SpoutLog, Warning, TEXT("Texture is ready"));
	}
}

void ResetMatInstance(UTexture2D*& Texture, UMaterialInstanceDynamic*& MaterialInstance)
{


	if (!Texture || !BaseMaterial || TextureParameterName.IsNone())
	{
		UE_LOG(SpoutLog, Warning, TEXT("UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU"));
		return;
	}

	// Create material instance
	if (!MaterialInstance)
	{
		MaterialInstance = UMaterialInstanceDynamic::Create(BaseMaterial, NULL);
		if (!MaterialInstance)
		{
			UE_LOG(SpoutLog, Warning, TEXT("Material instance can't be created"));
			return;
		}
	}

	// Check again, we must have material instance
	if (!MaterialInstance)
	{
		UE_LOG(SpoutLog, Error, TEXT("Material instance wasn't created"));
		return;
	}

	// Check we have desired parameter
	UTexture* Tex = nullptr;
	if (!MaterialInstance->GetTextureParameterValue(TextureParameterName, Tex))
	{
		UE_LOG(SpoutLog, Warning, TEXT("UI Material instance Texture parameter not found"));
		return;
	}

	MaterialInstance->SetTextureParameterValue(TextureParameterName, Texture);
}

void ResetTexture(UTexture2D*& Texture, UMaterialInstanceDynamic*& MaterialInstance, FSenderStruct*& SenderStruct)
{

	// Here we init the texture to its initial state
	DestroyTexture(Texture);
	//UE_LOG(SpoutLog, Warning, TEXT("Texture is ready???????2222"));
	// init the new Texture2D
	Texture = UTexture2D::CreateTransient(SenderStruct->w, SenderStruct->h, PF_B8G8R8A8);
	Texture->AddToRoot();
	Texture->UpdateResource();
	//UE_LOG(SpoutLog, Warning, TEXT("Texture is ready???????333333"));
	SenderStruct->Texture2DResource = (FTexture2DResource*)Texture->Resource;

	////////////////////////////////////////////////////////////////////////////////
	ResetMatInstance(Texture, MaterialInstance);


}

UTextureRenderTarget2D* USpoutBPFunctionLibrary::CreateTextureRenderTarget2D(int32 w, int32 h, EPixelFormat pixelFormat, bool forceLinearGamma) {
	UTextureRenderTarget2D* textureTarget = NewObject<UTextureRenderTarget2D>();
	textureTarget->bNeedsTwoCopies = true;
	textureTarget->InitCustomFormat(w, h, pixelFormat, forceLinearGamma);
	textureTarget->AddressX = TextureAddress::TA_Wrap;
	textureTarget->AddressY = TextureAddress::TA_Wrap;
	#if WITH_EDITORONLY_DATA
	textureTarget->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
	#endif

	textureTarget->AddToRoot();
	//textureTarget->UpdateResourceW();
	textureTarget->UpdateResource();

	return textureTarget;
}

void initSpout()
{
	UE_LOG(SpoutLog, Warning, TEXT("-----------> Init Spout"));
	
	sender = new spoutSenderNames;
	interop = new spoutGLDXinterop;
	sdx = new spoutDirectX;
}

void GetDevice()
{
	UE_LOG(SpoutLog, Warning, TEXT("-----------> Set Graphics Device D3D11"));

	g_D3D11Device = (ID3D11Device*)GDynamicRHI->RHIGetNativeDevice();
	g_D3D11Device->GetImmediateContext(&g_pImmediateContext);
}

int32 USpoutBPFunctionLibrary::SetMaxSenders(int32 max){
	if (sender == nullptr)
	{
		initSpout();

	};

	sender->SetMaxSenders(max);
	return max;
}

void USpoutBPFunctionLibrary::GetMaxSenders(int32& max) {
	if (sender == nullptr)
	{
		initSpout();

	};

	max = sender->GetMaxSenders();
}


bool GetSpoutRegistred(FName spoutName, FSenderStruct*& SenderStruct) {

	auto MyPredicate = [&](const FSenderStruct InItem) {return InItem.sName == spoutName; };
	SenderStruct = FSenders.FindByPredicate(MyPredicate);

	bool bIsInListSenders = FSenders.ContainsByPredicate(MyPredicate);
	return bIsInListSenders;

}

void UnregisterSpout(FName spoutName) {
	auto MyPredicate = [&](const FSenderStruct InItem) {return InItem.sName == spoutName; };
	FSenders.RemoveAll(MyPredicate);
}

void ClearRegister() {

	FSenders.Empty();
}

FSenderStruct* RegisterReceiver(FName spoutName){
	
	unsigned int w;
	unsigned int h;
	HANDLE sHandle;
	unsigned long format;

	sender->GetSenderInfo(TCHAR_TO_ANSI(*spoutName.ToString()), w, h, sHandle, format);

	FSenderStruct* newFSenderStruc = new FSenderStruct();
	newFSenderStruc->SetH(h);
	newFSenderStruc->SetW(w);
	newFSenderStruc->SetHandle(sHandle);
	newFSenderStruc->SetName(spoutName);
	newFSenderStruc->bIsAlive = true;
	newFSenderStruc->spoutType = ESpoutType::Receiver;
	newFSenderStruc->MaterialInstanceColor = nullptr;
	newFSenderStruc->TextureColor = nullptr;
	newFSenderStruc->sharedResource = nullptr;
	newFSenderStruc->rView = nullptr;
	newFSenderStruc->texTemp = NULL;

	//TextureColor = new
	UE_LOG(SpoutLog, Warning, TEXT("No material instance, creating...//////"));
	// Prepara Textura, Set the texture update region
	newFSenderStruc->UpdateRegions = new FUpdateTextureRegion2D(0, 0, 0, 0, newFSenderStruc->w, newFSenderStruc->h);
	ResetTexture(newFSenderStruc->TextureColor, newFSenderStruc->MaterialInstanceColor, newFSenderStruc);

	UE_LOG(SpoutLog, Warning, TEXT("--starting...--___Open Shared Resource___---"));
	HRESULT openResult = g_D3D11Device->OpenSharedResource(newFSenderStruc->sHandle, __uuidof(ID3D11Resource), (void**)(&newFSenderStruc->sharedResource));

	if (FAILED(openResult)) {
		UE_LOG(SpoutLog, Error, TEXT("--FAIL--___Open Shared Resource___---"));
		return false;

	}
	UE_LOG(SpoutLog, Warning, TEXT("--starting...--___Create Shader Resource View___---"));
	HRESULT createShaderResourceViewResult = g_D3D11Device->CreateShaderResourceView(newFSenderStruc->sharedResource, NULL, &newFSenderStruc->rView);
	if (FAILED(createShaderResourceViewResult)) {
		UE_LOG(SpoutLog, Error, TEXT("--FAIL--___Create Shader Resource View___---"));
		return false;

	}

	//texture shared for the spout resource handle
	ID3D11Texture2D* tex = (ID3D11Texture2D*)newFSenderStruc->sharedResource;

	if (tex == nullptr) {
		UE_LOG(SpoutLog, Error, TEXT("---|||------||||----"));
		return false;
	}
	D3D11_TEXTURE2D_DESC description;
	tex->GetDesc(&description);
	description.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	description.Usage = D3D11_USAGE_STAGING;
	description.BindFlags = 0;
	description.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	description.MiscFlags = 0;
	description.MipLevels = 1;
	description.ArraySize = 1;


	UE_LOG(SpoutLog, Warning, TEXT("--- Creating d3d11 Texture 2D---"));

	HRESULT hr = g_D3D11Device->CreateTexture2D(&description, NULL, &newFSenderStruc->texTemp);

	if (FAILED(hr))
	{
		std::stringstream ss;
		ss << " Error code = 0x" << std::hex << hr << std::endl;
		std::cout << ss.str() << std::endl;
		std::string TestString = ss.str();
		UE_LOG(SpoutLog, Error, TEXT("Failed to create texture of name: ----> %s"), *FString(TestString.c_str()));

		if (hr == E_OUTOFMEMORY) {
			UE_LOG(SpoutLog, Error, TEXT("OUT OF MEMORY"));
		}
		if (newFSenderStruc->texTemp)
		{
			newFSenderStruc->texTemp->Release();
			newFSenderStruc->texTemp = NULL;
		}
		UE_LOG(SpoutLog, Error, TEXT("Error creating temporal textura"));
		return false;

	}
	
	FSenders.Add(*newFSenderStruc);
	
	return newFSenderStruc;

}

bool USpoutBPFunctionLibrary::SpoutInfo(TArray<FSenderStruct>& Senders){
	Senders = FSenders;
	return true;
}

bool USpoutBPFunctionLibrary::SpoutInfoFrom(FName spoutName, FSenderStruct& SenderStruct){
	
	//Existe en mi lista ??
	auto MyPredicate = [&](const FSenderStruct InItem) {return InItem.sName == spoutName; };
	FSenderStruct* EncontradoSenderStruct = FSenders.FindByPredicate(MyPredicate);

	if (EncontradoSenderStruct == nullptr){
		UE_LOG(SpoutLog, Warning, TEXT("No Sender was found with the name : %s"), *spoutName.GetPlainNameString());
		return false;
	}
	else
	{
		SenderStruct = *EncontradoSenderStruct;
	}

	return true;
}


bool USpoutBPFunctionLibrary::CreateRegisterSender(FName spoutName, ID3D11Texture2D* baseTexture)
{

	if (g_D3D11Device == nullptr || g_pImmediateContext == NULL){
		UE_LOG(SpoutLog, Warning, TEXT("Getting Device..."));
		GetDevice();
	}

	HANDLE sharedSendingHandle = NULL;
	bool texResult = false;
	bool updateResult = false;
	bool senderResult = false;
	
	D3D11_TEXTURE2D_DESC desc;
	baseTexture->GetDesc(&desc);
	ID3D11Texture2D * sendingTexture;

	UE_LOG(SpoutLog, Warning, TEXT("ID3D11Texture2D Info : width_%i, height_%i"), desc.Width, desc.Height);
	UE_LOG(SpoutLog, Warning, TEXT("ID3D11Texture2D Info : Format is %i"), int(desc.Format));

	//use the pixel format from basetexture (the native texture textureRenderTarget2D)
	DXGI_FORMAT texFormat = desc.Format;
	if (desc.Format == DXGI_FORMAT_B8G8R8A8_TYPELESS) {
		texFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
	}
	
	texResult = sdx->CreateSharedDX11Texture(g_D3D11Device, desc.Width, desc.Height, texFormat, &sendingTexture, sharedSendingHandle);
	UE_LOG(SpoutLog, Warning, TEXT("Create shared Texture with SDX : %i"), texResult);

	if (!texResult)
	{
		UE_LOG(SpoutLog, Error, TEXT("SharedDX11Texture creation failed"));
		return 0;
	}

	const auto tmp = spoutName.GetPlainNameString();
	UE_LOG(SpoutLog, Warning, TEXT("Created Sender: name --> %s"), *tmp);

	//
	senderResult = sender->CreateSender(TCHAR_TO_ANSI(*spoutName.ToString()), desc.Width, desc.Height, sharedSendingHandle, texFormat);
	UE_LOG(SpoutLog, Warning, TEXT("Created sender DX11 with sender name : %s"), *tmp);

	// remove old sender register
	auto MyPredicate = [&](const FSenderStruct InItem) {return InItem.sName == spoutName; };
	FSenders.RemoveAll(MyPredicate);

	// add new register
	UE_LOG(SpoutLog, Warning, TEXT("Adding Sender to Sender list"));
	FSenderStruct* newFSenderStruc = new FSenderStruct();
	newFSenderStruc->SetW(desc.Width);
	newFSenderStruc->SetH(desc.Height);
	newFSenderStruc->SetName(spoutName);
	newFSenderStruc->bIsAlive = true;
	newFSenderStruc->spoutType = ESpoutType::Sender;
	newFSenderStruc->SetHandle(sharedSendingHandle);
	newFSenderStruc->activeTextures = sendingTexture;

	newFSenderStruc->MaterialInstanceColor = nullptr;
	newFSenderStruc->TextureColor = nullptr;

	FSenders.Add(*newFSenderStruc);

	return senderResult;
	
}

ESpoutState CheckSenderState(FName spoutName){

	auto MyPredicate = [&](const FSenderStruct InItem) {return InItem.sName == spoutName; };
	bool bIsInListSenders = FSenders.ContainsByPredicate(MyPredicate);

	ESpoutState state = ESpoutState::noEnoR;

	if (sender->FindSenderName(TCHAR_TO_ANSI(*spoutName.ToString()))) {
		//UE_LOG(SpoutLog, Warning, TEXT("Sender State: --> Exist"));
		if (bIsInListSenders) {
			//UE_LOG(SpoutLog, Warning, TEXT("Sender State: --> Exist y Registred"));
			state = ESpoutState::ER;
		}
		else {
			//UE_LOG(SpoutLog, Warning, TEXT("Sender State: --> Exist y No Registred"));
			state = ESpoutState::EnoR;
		}
	}
	else {
		//UE_LOG(SpoutLog, Warning, TEXT("Sender State: --> No Exist"));
		if (bIsInListSenders) {
			//UE_LOG(SpoutLog, Warning, TEXT("Sender State: --> No Exist y Registred"));
			state = ESpoutState::noER;
		}
		else {
			//UE_LOG(SpoutLog, Warning, TEXT("Sender State: --> No Exist y No Registred"));
			state = ESpoutState::noEnoR;
		}
	}

	return state;

}

bool USpoutBPFunctionLibrary::SpoutSender(FName spoutName, ESpoutSendTextureFrom sendTextureFrom, UTextureRenderTarget2D* textureRenderTarget2D, float targetGamma)
{
	if (sender == nullptr)
	{
		initSpout();

	};
	if (g_D3D11Device == nullptr || g_pImmediateContext == NULL) {

		GetDevice();
	}

	ID3D11Texture2D* baseTexture = 0;
	FSenderStruct* SenderStruct = 0;
	
	switch (sendTextureFrom)
	{
	case ESpoutSendTextureFrom::GameViewport:
		baseTexture = (ID3D11Texture2D*)GEngine->GameViewport->Viewport->GetRenderTargetTexture()->GetNativeResource();
		break;
	case ESpoutSendTextureFrom::TextureRenderTarget2D:
		if (textureRenderTarget2D == nullptr) {
			UE_LOG(SpoutLog, Warning, TEXT("No TextureRenderTarget2D Selected!!"));
			return false;
		}
		textureRenderTarget2D->TargetGamma = targetGamma;
		baseTexture = (ID3D11Texture2D*)textureRenderTarget2D->Resource->TextureRHI->GetTexture2D()->GetNativeResource();
		break;
	default:
		break;
	}

	if (baseTexture == nullptr) {
		UE_LOG(SpoutLog, Warning, TEXT("baseTexture is null"));
		return false;
	}

	ESpoutState state = CheckSenderState(spoutName);

	if (state == ESpoutState::noEnoR || state == ESpoutState::noER) {
		UE_LOG(SpoutLog, Warning, TEXT("Creating and registering new Sender..."));
		CreateRegisterSender(spoutName, baseTexture);
		return false;
	}
	if (state == ESpoutState::EnoR) {
		UE_LOG(SpoutLog, Warning, TEXT("A Sender with the name %s already exists"), *spoutName.GetPlainNameString());
		return false;
	}
	if (state == ESpoutState::ER) {
		GetSpoutRegistred(spoutName, SenderStruct);
		if (SenderStruct->spoutType == ESpoutType::Sender) {
			// Check whether texture size has changed
			D3D11_TEXTURE2D_DESC td;
			baseTexture->GetDesc(&td);
			if (td.Width != SenderStruct->w || td.Height != SenderStruct->h) {
				UE_LOG(SpoutLog, Warning, TEXT("Texture Size has changed, Updating registered spout: "), *spoutName.GetPlainNameString());
				UpdateRegisteredSpout(spoutName, baseTexture);
				return false;
			}
		}
		if (SenderStruct->spoutType == ESpoutType::Receiver) {
			UE_LOG(SpoutLog, Warning, TEXT("A Sender with the name %s already exists, and you have a receiver in Unreal that is receiving"), *spoutName.GetPlainNameString());
			
			return false;
		}	
	}

	bool result = false;
	
	if (SenderStruct->activeTextures == nullptr)
	{
		UE_LOG(SpoutLog, Warning, TEXT("activeTextures is null"));
		return false;
	}

	HANDLE targetHandle = SenderStruct->sHandle;

	ID3D11Texture2D * targetTex = SenderStruct->activeTextures;

	if (targetTex == nullptr){
		UE_LOG(SpoutLog, Warning, TEXT("targetTex is null"));
		return false;
	}
	
	ENQUEUE_RENDER_COMMAND(void)(
		[targetTex, baseTexture](FRHICommandListImmediate& RHICmdList) {
			g_pImmediateContext->CopyResource(targetTex, baseTexture);
			g_pImmediateContext->Flush();
		});
	
	D3D11_TEXTURE2D_DESC td;
	baseTexture->GetDesc(&td);

	result = sender->UpdateSender(TCHAR_TO_ANSI(*spoutName.ToString()), td.Width, td.Height, targetHandle);

	return result;
}

bool USpoutBPFunctionLibrary::SpoutReceiver(const FName spoutName, UMaterialInstanceDynamic*& mat, UTexture2D*& texture)
{
	const FString SenderNameString = spoutName.GetPlainNameString();

	if (BaseMaterial == NULL)
	{
		UMaterialInterface* mBase_Material = 0;
		mBase_Material = LoadObject<UMaterial>(NULL, TEXT("/SpoutPlugin/Materials/SpoutMaterial.SpoutMaterial"), NULL, LOAD_None, NULL);
		BaseMaterial = mBase_Material;
	}
	
	if (sender == nullptr)
	{ 
		initSpout();
	};

	if (g_D3D11Device == nullptr || g_pImmediateContext == NULL){
		
		GetDevice();
	}

	ESpoutState state = CheckSenderState(spoutName);

	if (state == ESpoutState::noEnoR) {
		UE_LOG(SpoutLog, Warning, TEXT("No sender found registered with the name %s"), *spoutName.GetPlainNameString());
		UE_LOG(SpoutLog, Warning, TEXT("Try to rename it, or resend %s"), *SenderNameString);
		return false;
	}
		
	if(state == ESpoutState::noER) {
		UE_LOG(SpoutLog, Warning, TEXT("Why are you registered??, unregister, best to close it"));
		//UnregisterSpout(spoutName);
		CloseSender(spoutName);
		return false;
	}

	if (state == ESpoutState::EnoR) {
		UE_LOG(SpoutLog, Warning, TEXT("Sender %s found, registering, receiving..."), *spoutName.GetPlainNameString());
		RegisterReceiver(spoutName);
		return false;
	}

	if (state == ESpoutState::ER) {
		FSenderStruct* SenderStruct = 0;
		GetSpoutRegistred(spoutName, SenderStruct);
		
		if (SenderStruct->spoutType == ESpoutType::Sender) {
			//UE_LOG(SpoutLog, Warning, TEXT("Receiving from sender inside ue4 with the name %s"), *spoutName.GetPlainNameString());
		}

		if (SenderStruct->spoutType == ESpoutType::Receiver) {
			//UE_LOG(SpoutLog, Warning, TEXT("Continue Receiver with the name %s"), *SenderName.GetPlainNameString());

			//communication between the two threads (rendering thread and game thread)
			// copy pixels from shared resource texture to texture temporal and update 
			int32 Stride = SenderStruct->w * 4;

			ENQUEUE_RENDER_COMMAND(void)(
				[SenderStruct, Stride](FRHICommandListImmediate& RHICmdList)
				{
					ID3D11Texture2D* t_texTemp = SenderStruct->texTemp;
					ID3D11Texture2D* t_tex = (ID3D11Texture2D*)SenderStruct->sharedResource;

					if (SenderStruct == nullptr) {
						return;
					}
		
					g_pImmediateContext->CopyResource(t_texTemp, t_tex);
					//g_pImmediateContext->Flush(); //<------ No Flush
					
					D3D11_MAPPED_SUBRESOURCE  mapped;
					//Gets a pointer to the data contained in a subresource, and denies the GPU access to that subresource.
					// with CPU read permissions (D3D11_MAP_READ)
					HRESULT hr = g_pImmediateContext->Map(t_texTemp, 0, D3D11_MAP_READ, 0, &mapped);
					if (FAILED(hr))
					{
						t_texTemp->Release();
						return;
					}
					BYTE *pixel = (BYTE *)mapped.pData;
					g_pImmediateContext->Unmap(t_texTemp, 0);


					//Update Texture
					RHIUpdateTexture2D(
						SenderStruct->Texture2DResource->GetTexture2DRHI(),
						0,
						*SenderStruct->UpdateRegions,
						mapped.RowPitch,
						(uint8*)pixel
					);


				});

			texture = SenderStruct->TextureColor;
			mat = SenderStruct->MaterialInstanceColor;
		}
	}
	
	return true;
}

void USpoutBPFunctionLibrary::CloseSender(FName spoutName)
{

	UE_LOG(SpoutLog, Warning, TEXT("Closing... sender %s"), *spoutName.GetPlainNameString());

	if (sender == nullptr)
	{
		initSpout();

	};
	if (g_D3D11Device == nullptr || g_pImmediateContext == NULL) {

		GetDevice();
	}

	ESpoutState state = CheckSenderState(spoutName);

	if (state == ESpoutState::noEnoR) {
		UE_LOG(SpoutLog, Warning, TEXT("%s is already closed; there is nothing to close!! Please check the name."), *spoutName.GetPlainNameString());
		//return;
	}
	if (state == ESpoutState::noER) {
		UE_LOG(SpoutLog, Warning, TEXT("+++++++++++++++"));
		UnregisterSpout(spoutName);
		//return;
	}
	if (state == ESpoutState::EnoR) {
			UE_LOG(SpoutLog, Warning, TEXT("A Sender with the name %s already exists. You can not close a sender that is not yours??"), *spoutName.GetPlainNameString());	
		//return;
	}
	if (state == ESpoutState::ER) {

		FSenderStruct* tempSenderStruct = 0;
		GetSpoutRegistred(spoutName, tempSenderStruct);
		
		if (tempSenderStruct->spoutType == ESpoutType::Sender) {
			UE_LOG(SpoutLog, Warning, TEXT("Releasing sender %s"), *spoutName.GetPlainNameString());
			// here really release the sender
			sender->ReleaseSenderName(TCHAR_TO_ANSI(*spoutName.ToString()));
			UE_LOG(SpoutLog, Warning, TEXT("Sender %s released"), *spoutName.GetPlainNameString());
			
		}
		else {
			UE_LOG(SpoutLog, Warning, TEXT("Receiver always listening"));

			FlushRenderingCommands();
			if (tempSenderStruct->sharedResource != nullptr) {
				UE_LOG(SpoutLog, Warning, TEXT("Release sharedResource"));
				tempSenderStruct->sharedResource->Release();
			}
			if (tempSenderStruct->texTemp != nullptr) {
				UE_LOG(SpoutLog, Warning, TEXT("Release Temporal texTemp"));
				tempSenderStruct->texTemp->Release();
			}
			if (tempSenderStruct->rView != nullptr) {
				UE_LOG(SpoutLog, Warning, TEXT("Release rView"));
				tempSenderStruct->rView->Release();
			}
			UE_LOG(SpoutLog, Warning, TEXT("Released All Temporal Textures"));
			//return;
		}

		UnregisterSpout(spoutName);
	}

	UE_LOG(SpoutLog, Warning, TEXT("There are now %i senders remaining"), FSenders.Num());
	

}


bool USpoutBPFunctionLibrary::UpdateRegisteredSpout(FName spoutName, ID3D11Texture2D * baseTexture)
{
	HANDLE sharedSendingHandle = NULL;
	bool texResult = false;
	bool updateResult = false;
	bool senderResult = false;

	D3D11_TEXTURE2D_DESC desc;
	baseTexture->GetDesc(&desc);
	ID3D11Texture2D * sendingTexture;

	UE_LOG(SpoutLog, Warning, TEXT("ID3D11Texture2D Info : ancho_%i, alto_%i"), desc.Width, desc.Height);
	UE_LOG(SpoutLog, Warning, TEXT("ID3D11Texture2D Info : Format is %i"), int(desc.Format));

	//use the pixel format from basetexture (the native texture textureRenderTarget2D)
	DXGI_FORMAT texFormat = desc.Format;
	if (desc.Format == DXGI_FORMAT_B8G8R8A8_TYPELESS) {
		texFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
	}

	texResult = sdx->CreateSharedDX11Texture(g_D3D11Device, desc.Width, desc.Height, texFormat, &sendingTexture, sharedSendingHandle);
	UE_LOG(SpoutLog, Warning, TEXT("Create shared Texture with SDX : %i"), texResult);

	if (!texResult)
	{
		UE_LOG(SpoutLog, Error, TEXT("SharedDX11Texture creation failed"));
		return 0;
	}

	// update register
	UE_LOG(SpoutLog, Warning, TEXT("Updating Sender in Sender list"));

	for (int32 Index = 0; Index != FSenders.Num(); ++Index)
	{
		if (FSenders[Index].sName == spoutName) {
			FSenders[Index].SetW(desc.Width);
			FSenders[Index].SetH(desc.Height);
			FSenders[Index].SetName(spoutName);
			FSenders[Index].bIsAlive = true;
			FSenders[Index].spoutType = ESpoutType::Sender;
			FSenders[Index].SetHandle(sharedSendingHandle);
			FSenders[Index].activeTextures = sendingTexture;

			FSenders[Index].MaterialInstanceColor = nullptr;
			FSenders[Index].TextureColor = nullptr;

			senderResult = true;
		}
	}

	return senderResult;
}
