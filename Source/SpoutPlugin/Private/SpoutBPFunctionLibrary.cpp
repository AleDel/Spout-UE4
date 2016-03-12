#include "SpoutPluginPrivatePCH.h"

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
	if (Texture)
	{
		Texture->RemoveFromRoot();

		if (Texture->Resource)
		{
			BeginReleaseResource(Texture->Resource);
			FlushRenderingCommands();
		}

		Texture->MarkPendingKill();
		Texture = nullptr;
	}
	else{
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

	// init the new Texture2D
	Texture = UTexture2D::CreateTransient(SenderStruct->w, SenderStruct->h, PF_B8G8R8A8);
	Texture->AddToRoot();
	Texture->UpdateResource();

	SenderStruct->Texture2DResource = (FTexture2DResource*)Texture->Resource;

	////////////////////////////////////////////////////////////////////////////////
	ResetMatInstance(Texture, MaterialInstance);


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

void RegisterReceiver(FName spoutName, FSenderStruct*& SenderStruct){
	
	unsigned int w;
	unsigned int h;
	HANDLE sHandle;
	unsigned long format;

	sender->GetSenderInfo(spoutName.GetPlainANSIString(), w, h, sHandle, format);

	FSenderStruct* newFSenderStruc = new FSenderStruct();
	newFSenderStruc->SetH(h);
	newFSenderStruc->SetW(w);
	newFSenderStruc->SetHandle(sHandle);
	newFSenderStruc->SetName(spoutName);
	newFSenderStruc->bIsAlive = true;
	newFSenderStruc->spoutType = ESpoutType::Receiver;
	newFSenderStruc->MaterialInstanceColor = nullptr;
	newFSenderStruc->TextureColor = nullptr;

	FSenders.Add(*newFSenderStruc);

	SenderStruct = newFSenderStruc;
	

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
		UE_LOG(SpoutLog, Warning, TEXT("No Encontrado sender con nombre : %s"), *spoutName.GetPlainNameString());
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

	const auto tmp = spoutName.GetPlainNameString();
	UE_LOG(SpoutLog, Warning, TEXT("Created Sender: name --> %s"), *tmp);

	//
	senderResult = sender->CreateSender(spoutName.GetPlainANSIString(), desc.Width, desc.Height, sharedSendingHandle, texFormat);
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

	if (sender->FindSenderName(spoutName.GetPlainANSIString())) {
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

	//UTextureRenderTarget2D* OutputTexture = NewObject<UTextureRenderTarget2D>();
	
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
		UE_LOG(SpoutLog, Warning, TEXT("New Sender creating, registering..."));
		CreateRegisterSender(spoutName, baseTexture);
		return false;
	}
	if (state == ESpoutState::EnoR) {
		UE_LOG(SpoutLog, Warning, TEXT("Already exist a Sender with the name %s"), *spoutName.GetPlainNameString());
		return false;
	}
	if (state == ESpoutState::ER) {
		GetSpoutRegistred(spoutName, SenderStruct);
		if (SenderStruct->spoutType == ESpoutType::Sender) {

		}
		if (SenderStruct->spoutType == ESpoutType::Receiver) {
			UE_LOG(SpoutLog, Warning, TEXT("Already exist a Sender with the name %s and you have a receiver in unreal receiving"), *spoutName.GetPlainNameString());
			
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
	
	//Sincroniza el thread del render y la copia de la textura
	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
		void,
		ID3D11Texture2D*, tt, targetTex,
		ID3D11Texture2D*, bt, baseTexture,
		{
			
			g_pImmediateContext->CopyResource(tt, bt);
			g_pImmediateContext->Flush(); 	
			
		});
	
	D3D11_TEXTURE2D_DESC td;
	baseTexture->GetDesc(&td);

	result = sender->UpdateSender(spoutName.GetPlainANSIString(), td.Width, td.Height, targetHandle);

	return result;
}

bool USpoutBPFunctionLibrary::SpoutReceiver(const FName spoutName, UMaterialInstanceDynamic*& mat)
{
	const FString SenderNameString = spoutName.GetPlainNameString();
	int32 Width;
	int32 Height;

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

	FSenderStruct* SenderStruct=0;
	ESpoutState state = CheckSenderState(spoutName);

	if (state == ESpoutState::noEnoR) {
		UE_LOG(SpoutLog, Warning, TEXT("Not found any sender and no registred with the name %s"), *spoutName.GetPlainNameString());
		UE_LOG(SpoutLog, Warning, TEXT("try to rename it, or resend %s"), *SenderNameString);
		return false;
	}
		
	if(state == ESpoutState::noER) {
		UE_LOG(SpoutLog, Warning, TEXT("why are you registred??, unregistre"));
		UnregisterSpout(spoutName);

		return false;
	}
	if (state == ESpoutState::EnoR) {
		UE_LOG(SpoutLog, Warning, TEXT("Sender %s found, registring, receiving..."), *spoutName.GetPlainNameString());
		RegisterReceiver(spoutName, SenderStruct);
		return false;
	}
	if (state == ESpoutState::ER) {

		GetSpoutRegistred(spoutName, SenderStruct);
		
		if (SenderStruct->spoutType == ESpoutType::Sender) {
			//UE_LOG(SpoutLog, Warning, TEXT("Receiving from sender inside ue4 with the name %s"), *spoutName.GetPlainNameString());
		}
		if (SenderStruct->spoutType == ESpoutType::Receiver) {
			//UE_LOG(SpoutLog, Warning, TEXT("Continue Receiver with the name %s"), *SenderName.GetPlainNameString());

		}
	}
		
	if (!SenderStruct->MaterialInstanceColor || SenderStruct->TextureColor == nullptr || SenderStruct->MaterialInstanceColor == nullptr)
	{
		
		//TextureColor = new
		UE_LOG(SpoutLog, Warning, TEXT("No material intance, creating...//////"));
		// Prepara Textura, Set the texture update region
		Width = SenderStruct->w;
		Height = SenderStruct->h;
		SenderStruct->UpdateRegions = new FUpdateTextureRegion2D(0, 0, 0, 0, Width, Height);
		ResetTexture(SenderStruct->TextureColor, SenderStruct->MaterialInstanceColor, SenderStruct);

	}

	if (SenderStruct->MaterialInstanceColor == nullptr){
		UE_LOG(SpoutLog, Warning, TEXT("___________________________no MaterialInstanceColor nullptr"));
		return false;
	}

	if (SenderStruct->TextureColor == nullptr){
		UE_LOG(SpoutLog, Warning, TEXT("___________________________no textureColor nullptr" ));
		return false;
	}
	
	ID3D11Resource * tempResource11 = nullptr;
	ID3D11ShaderResourceView * rView = nullptr;
	
	HRESULT openResult = g_D3D11Device->OpenSharedResource(SenderStruct->sHandle, __uuidof(ID3D11Resource), (void**)(&tempResource11));
	g_D3D11Device->CreateShaderResourceView(tempResource11, NULL, &rView);

	ID3D11Texture2D* tex = (ID3D11Texture2D*)tempResource11;
	if (tex == nullptr) {
		return false;
	}
	D3D11_TEXTURE2D_DESC description;
	tex->GetDesc(&description);
	description.BindFlags = 0;
	description.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	description.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	description.Usage = D3D11_USAGE_STAGING;

	ID3D11Texture2D* texTemp = NULL;

	HRESULT hr = g_D3D11Device->CreateTexture2D(&description, NULL, &texTemp);
	if (FAILED(hr))
	{
		if (texTemp)
		{
			texTemp->Release();
			texTemp = NULL;
		}
		//return NULL;
		UE_LOG(SpoutLog, Error, TEXT("error creating temporal textura"));
	}
	
	ENQUEUE_UNIQUE_RENDER_COMMAND_FOURPARAMETER(
		void,
		ID3D11Texture2D*, t_texTemp, texTemp,
		ID3D11Texture2D*, t_tex, tex,
		int32, Stride, SenderStruct->w * 4,
		FSenderStruct*, Params, SenderStruct,
		{

			if (Params == nullptr) {
				return;
			}

			g_pImmediateContext->CopyResource(t_texTemp, t_tex);
			g_pImmediateContext->Flush();
			D3D11_MAPPED_SUBRESOURCE  mapped;
			//Gets a pointer to the data contained in a subresource, and denies the GPU access to that subresource.
			// with CPU read permissions (D3D11_MAP_READ)
			HRESULT hr = g_pImmediateContext->Map(t_texTemp, 0, D3D11_MAP_READ, 0, &mapped);
			if (FAILED(hr))
			{
				t_texTemp->Release();
				t_texTemp = NULL;
				
				UE_LOG(SpoutLog, Error, TEXT("Fallo crear mapped"));
				return;
			}
			BYTE *pixel = (BYTE *)mapped.pData;
			g_pImmediateContext->Unmap(t_texTemp, 0);

			//Update Texture
			RHIUpdateTexture2D(Params->Texture2DResource->GetTexture2DRHI(), 0, *Params->UpdateRegions, Stride, (uint8*)pixel);
			
			//Clean Textura Temporal
			t_texTemp->Release();
			t_texTemp = NULL;
		});
		
	/**/
	if (SenderStruct->MaterialInstanceColor != nullptr)
	{
		//mat = MaterialInstanceColor;
		mat = SenderStruct->MaterialInstanceColor;
		return true;
	}
	else{
		//mat = static_cast<UMaterialInstanceDynamic*>(BaseMaterial);
		mat = NULL;
		return false;
	}
}

void USpoutBPFunctionLibrary::CloseSender(FName spoutName)
{

	UE_LOG(SpoutLog, Warning, TEXT("Closing... sender %s "), *spoutName.GetPlainNameString());

	if (sender == nullptr)
	{
		initSpout();

	};
	if (g_D3D11Device == nullptr || g_pImmediateContext == NULL) {

		GetDevice();
	}

	ESpoutState state = CheckSenderState(spoutName);

	if (state == ESpoutState::noEnoR) {
		UE_LOG(SpoutLog, Warning, TEXT("already %s closed, there is nothing to close!! check the name"), *spoutName.GetPlainNameString());
		//return;
	}
	if (state == ESpoutState::noER) {
		UE_LOG(SpoutLog, Warning, TEXT("+++++++++++++++"));
		UnregisterSpout(spoutName);
		//return;
	}
	if (state == ESpoutState::EnoR) {
			UE_LOG(SpoutLog, Warning, TEXT("Already exist a Sender with the name %s, You can not close a sender that is not yours.??"), *spoutName.GetPlainNameString());	
		//return;
	}
	if (state == ESpoutState::ER) {

		FSenderStruct* tempSenderStruct = 0;
		GetSpoutRegistred(spoutName, tempSenderStruct);
		
		if (tempSenderStruct->spoutType == ESpoutType::Sender) {
			UE_LOG(SpoutLog, Warning, TEXT("releasing sender %s"), *spoutName.GetPlainNameString());
			// here really release the sender
			sender->ReleaseSenderName(spoutName.GetPlainANSIString());
			UE_LOG(SpoutLog, Warning, TEXT("sender %s released"), *spoutName.GetPlainNameString());
			
		}
		else {
			UE_LOG(SpoutLog, Warning, TEXT("receiver always listening"));
			//return;
		}

		UnregisterSpout(spoutName);
	}

	UE_LOG(SpoutLog, Warning, TEXT("There are now %i senders remaining "), FSenders.Num());
	

}