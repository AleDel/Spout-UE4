# Spout-UE4
Spout Plugin for Unreal Engine

Currently the plugin has 4 static functions for use with BluePrint:

- CreateSender. 
	- Create a DX11 device and context (GDynamicRHI->RHIGetNativeDevice();).
	- takes the native texture from a UTextureRenderTarget2D (the texture is of type 'DXGI_FORMAT_R16G16B16A16_FLOAT').
	- and create de Spout sender.

- UpdateSender.
	- This updates the Sender, copying the 'TextureRender' on the texture that is going to send, maintaining synchronized the render thread and the copy process. (ENQUEUE_UNIQUE_RENDER_COMMAND).

- CloseSender.
	- Delete the sernder object and release de Dx11 device and context.

- Receiver.
	- check if there 'sender' to receive.
	- Use OpenSharedResource and CreateShaderResourceView to get the resource image, copy the received image to other DX11 texture with CPU read permissions (D3D11_MAP_READ), get de Byte data and use RHIUpdateTexture2D to update the texture of the Unreal material instance, maintaining synchronized the render thread (ENQUEUE_UNIQUE_RENDER_COMMAND).
	- Return a material with the texture.

