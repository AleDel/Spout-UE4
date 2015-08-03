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
		UE_LOG(SpoutLog, Warning, TEXT("Textura ya está limpia"));
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

void InitDevice()
{
	UE_LOG(SpoutLog, Warning, TEXT("-----------> Set Graphics Device D3D11"));

	g_D3D11Device = (ID3D11Device*)GDynamicRHI->RHIGetNativeDevice();
	g_D3D11Device->GetImmediateContext(&g_pImmediateContext);
}


bool CheckSpoutSenderName(FName SenderName, FSenderStruct*& SenderStruct){
	//Existe en realidad ??
	if (!sender->FindSenderName(SenderName.GetPlainANSIString())){
		UE_LOG(SpoutLog, Warning, TEXT("222222no encuentro ningun sender con el nombre %s"), *SenderName.GetPlainNameString());
		return false;
	}
	unsigned int w;
	unsigned int h;
	HANDLE sHandle;
	unsigned long format;

	sender->GetSenderInfo(SenderName.GetPlainANSIString(), w, h, sHandle, format);

	//Existe en mi lista ??
	auto MyPredicate = [&](const FSenderStruct InItem) {return InItem.sName == SenderName; };
	//bool bIsInListSenders = FSenders.ContainsByPredicate(MyPredicate);
	FSenderStruct* EncontradoSenderStruct = FSenders.FindByPredicate(MyPredicate);

	//if (!bIsInListSenders){
	if (EncontradoSenderStruct == nullptr){

		FSenderStruct* newFSenderStruc = new FSenderStruct();
		newFSenderStruc->SetH(h);
		newFSenderStruc->SetW(w);
		newFSenderStruc->SetHandle(sHandle);
		newFSenderStruc->SetName(SenderName);
		newFSenderStruc->MaterialInstanceColor = nullptr;
		newFSenderStruc->TextureColor = nullptr;

		FSenders.Add(*newFSenderStruc);

		SenderStruct = newFSenderStruc;
	}
	else
	{
		SenderStruct = EncontradoSenderStruct;
	}
	
	// check if is in our list of senders

	return true;
}

bool USpoutBPFunctionLibrary::SpoutInfo(TArray<FSenderStruct>& Senders){
	Senders = FSenders;
	return true;
}

bool USpoutBPFunctionLibrary::SpoutInfoFrom(FName SenderName, FSenderStruct& SenderStruct){
	
	//Existe en mi lista ??
	auto MyPredicate = [&](const FSenderStruct InItem) {return InItem.sName == SenderName; };
	FSenderStruct* EncontradoSenderStruct = FSenders.FindByPredicate(MyPredicate);

	if (EncontradoSenderStruct == nullptr){
		UE_LOG(SpoutLog, Warning, TEXT("No Encontrado sender con nombre : %s"), *SenderName.GetPlainNameString());
		return false;
	}
	else
	{
		SenderStruct = *EncontradoSenderStruct;
	}

	return true;
}


bool USpoutBPFunctionLibrary::CreateSender(FName SenderName, UTextureRenderTarget2D* RenderTexture, int32 texFormatIndex)
{
	
	if (RenderTexture == nullptr)
	{
		UE_LOG(SpoutLog, Warning, TEXT("RenderTexture pointer is null"));
		return false;
	}
	
	if (sender == nullptr)
	{
		initSpout();
		//return
	};
	if (g_D3D11Device == nullptr || g_pImmediateContext == NULL){
		InitDevice();
		const FString tmp = SenderName.GetPlainNameString();
		UE_LOG(SpoutLog, Warning, TEXT("ggggggggggg : %s"), *tmp);
	}

	HANDLE sharedSendingHandle = NULL;
	bool texResult = false;
	bool updateResult = false;
	bool senderResult = false;
	
	ID3D11Texture2D* baseTexture = (ID3D11Texture2D*)RenderTexture->Resource->TextureRHI->GetTexture2D()->GetNativeResource();

	D3D11_TEXTURE2D_DESC desc;
	baseTexture->GetDesc(&desc);
	ID3D11Texture2D * sendingTexture;
	UE_LOG(SpoutLog, Warning, TEXT("textura : ancho_%i, alto_%i"), desc.Width, desc.Height);

	DXGI_FORMAT texFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;//DXGI_FORMAT_B8G8R8A8_UNORM;
	switch (texFormatIndex)
	{
	case 2:
		texFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
		break;
	case 24:
		texFormat = DXGI_FORMAT_R10G10B10A2_UNORM;
		break;
	case 28:
		texFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
		break;
	case 87:
		texFormat = DXGI_FORMAT_B8G8R8A8_UNORM;
		break;
	case 10:
		texFormat = DXGI_FORMAT_R16G16B16A16_FLOAT;
		break;
	}

	UE_LOG(SpoutLog, Warning, TEXT("baseTexture Format is %i"), int(desc.Format));

	texResult = sdx->CreateSharedDX11Texture(g_D3D11Device, desc.Width, desc.Height, texFormat, &sendingTexture, sharedSendingHandle);
	UE_LOG(SpoutLog, Warning, TEXT("Create shared Texture with SDX : %i"), texResult);

	if (!texResult)
	{
		UE_LOG(SpoutLog, Error, TEXT("SharedDX11Texture creation failed"));
		return 0;
	}

	const auto tmp = SenderName.GetPlainNameString();
	UE_LOG(SpoutLog, Warning, TEXT("create Sender: name --> %s"), *tmp);

	//
	senderResult = sender->CreateSender(SenderName.GetPlainANSIString(), desc.Width, desc.Height, sharedSendingHandle, texFormat);
	UE_LOG(SpoutLog, Warning, TEXT("Create sender DX11 with sender name : %s (%i)"), *tmp, senderResult);


	g_pImmediateContext->CopyResource(sendingTexture, baseTexture);
	g_pImmediateContext->Flush();

	updateResult = sender->UpdateSender(SenderName.GetPlainANSIString(), desc.Width, desc.Height, sharedSendingHandle);

	FSenderStruct* newFSenderStruc = new FSenderStruct();
	newFSenderStruc->SetW(desc.Width);
	newFSenderStruc->SetH(desc.Height);
	newFSenderStruc->SetName(SenderName);
	newFSenderStruc->SetHandle(sharedSendingHandle);
	newFSenderStruc->activeTextures = sendingTexture;

	newFSenderStruc->MaterialInstanceColor = nullptr;
	newFSenderStruc->TextureColor = nullptr;

	FSenders.Add(*newFSenderStruc);

	return senderResult;
	
}

bool USpoutBPFunctionLibrary::SpoutSender(FName SenderName, UTextureRenderTarget2D* RenderTexture)
{

	//Existe en mi lista ??
	auto MyPredicate = [&](const FSenderStruct InItem) {return InItem.sName == SenderName; };
	FSenderStruct* EncontradoSenderStruct = FSenders.FindByPredicate(MyPredicate);

	//if (!bIsInListSenders){
	if (EncontradoSenderStruct == nullptr){

		UE_LOG(SpoutLog, Warning, TEXT("no Sender, creando uno"));
		CreateSender(SenderName, RenderTexture, 87);
		return false;
	}
	
	bool result = false;
	
	//DX11
	if (EncontradoSenderStruct->activeTextures == nullptr)
	{
		UE_LOG(SpoutLog, Warning, TEXT("activeTextures is null"));
		return false;
	}

	HANDLE targetHandle = EncontradoSenderStruct->sHandle;

	ID3D11Texture2D * targetTex = EncontradoSenderStruct->activeTextures;

	if (targetTex == nullptr){
		UE_LOG(SpoutLog, Warning, TEXT("targetTex is null"));
		return false;
	}

	ID3D11Texture2D* baseTexture = (ID3D11Texture2D*)RenderTexture->Resource->TextureRHI->GetTexture2D()->GetNativeResource();
	if (baseTexture == nullptr){
		UE_LOG(SpoutLog, Warning, TEXT("baseTexture is null"));
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

	result = sender->UpdateSender(SenderName.GetPlainANSIString(), td.Width, td.Height, targetHandle);
	//UE_LOG(SpoutLog, Warning, TEXT("ok"));
	return result;
}

bool USpoutBPFunctionLibrary::SpoutReceiver(const FName SenderName, UMaterialInterface* Base_Material, UMaterialInstanceDynamic*& mat)
{
	const FString SenderNameString = SenderName.GetPlainNameString();
	int32 Width;
	int32 Height;

	if (BaseMaterial == NULL)
	{
		BaseMaterial = Base_Material;
	}
	
	if (sender == nullptr)
	{ 
		initSpout();
		//return
	};
	if (g_D3D11Device == nullptr || g_pImmediateContext == NULL){
		InitDevice();
		UE_LOG(SpoutLog, Warning, TEXT("ggggggggggg : %s"), *SenderNameString);
	}

	FSenderStruct* SenderStruct;

	if (!CheckSpoutSenderName(SenderName, SenderStruct)){
		UE_LOG(SpoutLog, Warning, TEXT("11111 no encuentro ningun sender con el nombre %s"), *SenderNameString);
		return false;
	}
	
	if (!SenderStruct->MaterialInstanceColor || SenderStruct->TextureColor == nullptr || SenderStruct->MaterialInstanceColor == nullptr)
	{
		
		//TextureColor = new
		UE_LOG(SpoutLog, Warning, TEXT("No hay material intance, creando e iniciando//////"));
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
		UE_LOG(SpoutLog, Error, TEXT("Fallo crear textura temporal"));
	}
	
	ENQUEUE_UNIQUE_RENDER_COMMAND_FOURPARAMETER(
		void,
		ID3D11Texture2D*, t_texTemp, texTemp,
		ID3D11Texture2D*, t_tex, tex,
		int32, Stride, SenderStruct->w * 4,
		FSenderStruct*, Params, SenderStruct,
		{

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
				//return NULL;
				UE_LOG(SpoutLog, Error, TEXT("Fallo crear mapped"));
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

void USpoutBPFunctionLibrary::CloseSender(FName SenderName)
{

	//Existe en mi lista ??
	auto MyPredicate = [&](const FSenderStruct InItem) {return InItem.sName == SenderName; };
	//bool bIsInListSenders = FSenders.ContainsByPredicate(MyPredicate);
	FSenderStruct* EncontradoSenderStruct = FSenders.FindByPredicate(MyPredicate);

	//if (!bIsInListSenders){
	if (EncontradoSenderStruct != nullptr){
		sender->ReleaseSenderName(SenderName.GetPlainANSIString());
		
		//FSenders.Remove(*EncontradoSenderStruct);

		
	}
	if (sender){
		delete sender;
		sender = nullptr;
	}
	
	FSenders.Empty();
	
	if (g_D3D11Device){
		UE_LOG(SpoutLog, Warning, TEXT("Release Context and Graphics Device D3D11"));
		g_pImmediateContext->Release();
		g_D3D11Device->Release();
	}
	UE_LOG(SpoutLog, Warning, TEXT("There are now %i senders remaining "), FSenders.Num());

}