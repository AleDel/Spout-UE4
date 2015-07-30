#include "SpoutPluginPrivatePCH.h"

static ID3D11Device* g_D3D11Device;
ID3D11DeviceContext*    g_pImmediateContext = NULL;

static FRenderCommandFence* ReleaseRendererFence;

spoutSenderNames * sender;
spoutGLDXinterop * interop;
spoutDirectX * sdx;
/**/
//DX11
vector<ID3D11Texture2D *> activeTextures;
vector<HANDLE> activeHandles;
vector<string> activeNames;
int numActiveSenders;/**/

int getIndexForSenderName(const char * senderName)
{

	UE_LOG(SpoutLog, Warning, TEXT("index de %s es %i"), senderName, numActiveSenders);
	for (int i = 0; i < numActiveSenders; i++)
	{
		//UE_LOG(SpoutLog, Warning, TEXT("active name %s"), activeNames[i].c_str());
		if (strcmp(senderName, activeNames[i].c_str()) == 0) return i;
	}
	
	return -1;
}

void initSpout(){
	UE_LOG(SpoutLog, Warning, TEXT("Init Spout"));
	sender = new spoutSenderNames;
	interop = new spoutGLDXinterop;
	sdx = new spoutDirectX;

	numActiveSenders = 0;
}

void InitDevice(){
	UE_LOG(SpoutLog, Warning, TEXT("Set Graphics Device D3D11"));

	g_D3D11Device = (ID3D11Device*)GDynamicRHI->RHIGetNativeDevice();
	g_D3D11Device->GetImmediateContext(&g_pImmediateContext);
}

bool USpoutBPFunctionLibrary::CreateSender(FName SenderName, UTextureRenderTarget2D* RenderTexture, int32 texFormatIndex)
{
	
	if (RenderTexture == nullptr)
	{
		UE_LOG(SpoutLog, Warning, TEXT("RenderTexture pointer is null"));
		return false;
	}
	
	int checkSenderIndex = getIndexForSenderName(SenderName.GetPlainANSIString());
	
	if (checkSenderIndex != -1)
	{
		UE_LOG(SpoutLog, Warning, TEXT("sender already exists"));
		return false;
	}

	sender = new spoutSenderNames;
	interop = new spoutGLDXinterop;
	sdx = new spoutDirectX;

	ReleaseRendererFence = new FRenderCommandFence();

	HANDLE sharedSendingHandle = NULL;
	bool texResult = false;
	bool updateResult = false;
	bool senderResult = false;
	string sName = string(SenderName.GetPlainANSIString());
	//DX11

	UE_LOG(SpoutLog, Warning, TEXT("Set Graphics Device D3D11"));

	g_D3D11Device = (ID3D11Device*)GDynamicRHI->RHIGetNativeDevice();
	g_D3D11Device->GetImmediateContext(&g_pImmediateContext);
	
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

	senderResult = sender->CreateSender(SenderName.GetPlainANSIString(), desc.Width, desc.Height, sharedSendingHandle, texFormat);
	UE_LOG(SpoutLog, Warning, TEXT("Create sender DX11 with sender name : %s (%i)"), *tmp, senderResult);


	g_pImmediateContext->CopyResource(sendingTexture, baseTexture);
	g_pImmediateContext->Flush();

	updateResult = sender->UpdateSender(SenderName.GetPlainANSIString(), desc.Width, desc.Height, sharedSendingHandle);

	activeTextures.push_back(sendingTexture);
	activeNames.push_back(sName);
	activeHandles.push_back(sharedSendingHandle);
	numActiveSenders++;
	/**/


	int senderIndex = getIndexForSenderName(SenderName.GetPlainANSIString());
	UE_LOG(SpoutLog, Warning, TEXT("Index of the sender %i"), senderIndex);

	return senderResult;
	
}

bool USpoutBPFunctionLibrary::UpdateSender(FName SenderName, UTextureRenderTarget2D* RenderTexture)
{
	int senderIndex = getIndexForSenderName(SenderName.GetPlainANSIString());

	if (senderIndex == -1)
	{
		UE_LOG(SpoutLog, Warning, TEXT("no Sender, creando uno"));
		CreateSender(SenderName, RenderTexture, 0);
		return false;
	}

	bool result = false;
	
	//DX11
	if (activeTextures[senderIndex] == nullptr)
	{
		UE_LOG(SpoutLog, Warning, TEXT("activeTextures[%i] is null"), senderIndex);
		return false;
	}

	HANDLE targetHandle = activeHandles[senderIndex];

	ID3D11Texture2D * targetTex = activeTextures[senderIndex];

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
	/*
	ReleaseRendererFence->BeginFence();
	ReleaseRendererFence->Wait();
	if (ReleaseRendererFence->IsFenceComplete()){
		UE_LOG(SpoutLog, Warning, TEXT("tessss"));
	}
	*/
	

	D3D11_TEXTURE2D_DESC td;
	baseTexture->GetDesc(&td);

	result = sender->UpdateSender(SenderName.GetPlainANSIString(), td.Width, td.Height, targetHandle);
	//UE_LOG(SpoutLog, Warning, TEXT("ok"));
	return result;
}

void USpoutBPFunctionLibrary::CloseSender(FName SenderName)
{
	int senderIndex = getIndexForSenderName(SenderName.GetPlainANSIString());
	
	const auto tmp = SenderName.GetPlainNameString();
	UE_LOG(SpoutLog, Warning, TEXT("Close Sender : %s"), *tmp);

	if (senderIndex != -1)
	{
		//DX11
		sender->ReleaseSenderName(SenderName.GetPlainANSIString());
		activeNames.erase(activeNames.begin() + senderIndex);
		activeHandles.erase(activeHandles.begin() + senderIndex);
		activeTextures.erase(activeTextures.begin() + senderIndex);
		numActiveSenders--;
		
		if (sender){
			delete sender;
			sender = nullptr;
		}

	}
	if (g_D3D11Device){
		UE_LOG(SpoutLog, Warning, TEXT("Release Context and Graphics Device D3D11"));
		g_pImmediateContext->Release();
		g_D3D11Device->Release();
	}
	UE_LOG(SpoutLog, Warning, TEXT("There are now %i senders remaining "), numActiveSenders, activeNames.size());

}



char(*senderNames)[256];
char(*newNames)[256];
unsigned int w;
unsigned int h;
HANDLE sHandle;


struct FTextureParams
{

	// Pointer to our Texture's resource
	FTexture2DResource* Texture2DResource;

	// Regions we need to update (for now, the whole image)
	FUpdateTextureRegion2D* UpdateRegions;

};

FTextureParams RenderParamsColor;

UMaterialInterface* BaseMaterial;
UTexture2D* TextureColor;
UMaterialInstanceDynamic* MaterialInstanceColor;
FName TextureParameterName = "SpoutTexture";

int32 Width;
int32 Height;



UTexture2D* GetTexture(UTexture2D*& Texture) 
{
	if (!Texture)
	{
		return UTexture2D::CreateTransient(Width, Height);
	}

	return Texture;
}

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

	MaterialInstance->SetTextureParameterValue(TextureParameterName, GetTexture(Texture));
}

void ResetTexture(UTexture2D*& Texture, UMaterialInstanceDynamic*& MaterialInstance, FTextureParams& m_RenderParams)
{

	// Here we init the texture to its initial state
	DestroyTexture(Texture);

	// init the new Texture2D
	Texture = UTexture2D::CreateTransient(Width, Height, PF_B8G8R8A8);
	Texture->AddToRoot();
	Texture->UpdateResource();

	m_RenderParams.Texture2DResource = (FTexture2DResource*)Texture->Resource;

	////////////////////////////////////////////////////////////////////////////////
	ResetMatInstance(Texture, MaterialInstance);


}


/*
UMaterialInstanceDynamic* USpoutBPFunctionLibrary::GetMaterialRGB()
{
	return MaterialInstanceColor;
}
*/
bool USpoutBPFunctionLibrary::Receiver(UMaterialInterface* Base_Material, int32& numSenders, UMaterialInstanceDynamic*& mat)
{

	BaseMaterial = Base_Material;

	numSenders = -1;
	if (sender == nullptr)
	{ 
		initSpout();
		//return
	};
	if (g_D3D11Device == nullptr || g_pImmediateContext == NULL){
		InitDevice();
	}
	
	senderNames = new char[32][256];
	newNames = new char[32][256];

	numSenders = sender->GetSenderCount();
	
	if (numSenders > 0)
	{
		for (int i = 0; i < numSenders; i++)
		{
			// Get sender info given a sender index and knowing the sender count
			// index                        - in
			// sendername                   - out
			// sendernameMaxSize            - in
			// width, height, dxShareHandle - out
			sender->GetSenderNameInfo(i, newNames[i], 256, w, h, sHandle);
			//printf("\t> %s\n", newNames[i]);
		}
	}
	else{
		UE_LOG(SpoutLog, Warning, TEXT("No hay ningun serder para recivir"));
		return false;
	}

	if (!MaterialInstanceColor)
	{
		UE_LOG(SpoutLog, Warning, TEXT("No hay material intance, creando e iniciando"));
		
		// Prepara Textura, Set the texture update region
		Width = w;
		Height = h;
		RenderParamsColor.UpdateRegions = new FUpdateTextureRegion2D(0, 0, 0, 0, Width, Height);
		ResetTexture(TextureColor, MaterialInstanceColor, RenderParamsColor);
		
	}

	ID3D11Resource * tempResource11;
	ID3D11ShaderResourceView * rView;
	
	HRESULT openResult = g_D3D11Device->OpenSharedResource(sHandle, __uuidof(ID3D11Resource), (void**)(&tempResource11));
	g_D3D11Device->CreateShaderResourceView(tempResource11, NULL, &rView);

	ID3D11Texture2D* tex = (ID3D11Texture2D*)tempResource11;

	D3D11_TEXTURE2D_DESC description;
	tex->GetDesc(&description);
	description.BindFlags = 0;
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

	ENQUEUE_UNIQUE_RENDER_COMMAND_FIVEPARAMETER(
		void,
		ID3D11Texture2D*, t_texTemp, texTemp,
		ID3D11Texture2D*, t_tex, tex,
		UTexture2D*, TargetTexture, TextureColor,
		int32, Stride, Width * 4,
		FTextureParams, Params, RenderParamsColor,
		{
			
			g_pImmediateContext->CopyResource(t_texTemp, t_tex);
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
			RHIUpdateTexture2D(Params.Texture2DResource->GetTexture2DRHI(), 0, *Params.UpdateRegions, Stride, (uint8*)pixel);

		});

	mat = MaterialInstanceColor;
	return true;

	//////////////////////////////////////////////////////////////////////////

	//TextureUpdate(dest, TextureColor, RenderParamsColor);
	
	
	//D3D11_SHADER_RESOURCE_VIEW_DESC *pDesc;
	//rView->GetDesc(pDesc);
	//pDesc->Texture2D()
	//pDesc->BufferEx.Flags(D3D11_BUFFEREX_SRV_FLAG_RAW);


	/*

	//clean
	for (int i = 0; i < numActiveSenders; i++)
	{
		closeSender((char *)activeNames[i].c_str());
	}

	delete[] senderNames;
	delete[] newNames;*/
}