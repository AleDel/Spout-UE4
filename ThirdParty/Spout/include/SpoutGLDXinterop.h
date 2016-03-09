/*

			spoutGLDXinterop.h

		LJ - leadedge@adam.com.au

		Functions to manage texture sharing using the NVIDIA GL/DX opengl extensions

		https://www.opengl.org/registry/specs/NV/DX_interop.txt


		Copyright (c) 2014-2015, Lynn Jarvis. All rights reserved.

		Redistribution and use in source and binary forms, with or without modification, 
		are permitted provided that the following conditions are met:

		1. Redistributions of source code must retain the above copyright notice, 
		   this list of conditions and the following disclaimer.

		2. Redistributions in binary form must reproduce the above copyright notice, 
		   this list of conditions and the following disclaimer in the documentation 
		   and/or other materials provided with the distribution.

		THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"	AND ANY 
		EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
		OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE	ARE DISCLAIMED. 
		IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
		INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, 
		PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
		INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
		LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
		OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
#pragma once
#ifndef __spoutGLDXinterop__ // standard way as well
#define __spoutGLDXinterop__

#include "SpoutCommon.h"
#include "spoutGLextensions.h"
#include "spoutDirectX.h"
#include "spoutSenderNames.h"
#include "SpoutMemoryShare.h"

#include <windowsx.h>
#include <d3d9.h>	// DX9
#include <d3d11.h>	// DX11
#include <gl/gl.h>
#include <gl/glu.h> // For glerror
#include <shlwapi.h> // for path functions

// LJ DEBUG
// #include <d3dx9.h> // Needed for D3DXLoadSurfaceFromSurface() 
// #pragma comment(lib, "d3dx9.lib") 


class SPOUT_DLLEXP spoutGLDXinterop {

	public:

		spoutGLDXinterop();
		~spoutGLDXinterop();

		// Initialization functions
		bool LoadGLextensions(); // Load required opengl extensions
		bool CreateInterop(HWND hWnd, const char* sendername, unsigned int width, unsigned int height, DWORD dwFormat, bool bReceive = true);
		bool CheckInterop(HWND hWnd); // Check for successful open of the interop
		void CleanupInterop(bool bExit = false); // Cleanup with flag to avoid unknown crash bug

		void setSharedMemoryName(char* sharedMemoryName, bool bReceive = true); 
		bool getSharedInfo(char* sharedMemoryName, SharedTextureInfo* info);
		bool setSharedInfo(char* sharedMemoryName, SharedTextureInfo* info);
		
		bool WriteTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert=true,  GLuint HostFBO=0);
		bool ReadTexture (GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert=false, GLuint HostFBO=0);

		bool WriteTexturePixels(unsigned char *pixels, unsigned int width, unsigned int height, GLenum glFormat = GL_RGBA, bool bAlignment = true);
		bool ReadTexturePixels (unsigned char *pixels, unsigned int width, unsigned int height, GLenum glFormat = GL_RGBA, GLuint HostFBO=0);

		bool WriteDword(GLuint TextureID, GLuint TextureTarget, DWORD dwValue);
		bool ReadDword (GLuint TextureID, GLuint TextureTarget, DWORD &dwValue, unsigned int width, unsigned int height, bool bInvert = false);
		unsigned __int32 iLastFrame;

		bool InitOpenGL();
		bool CloseOpenGL();



		#ifdef USE_PBO_EXTENSIONS
		bool LoadTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, unsigned char *data);
		#endif

		bool BindSharedTexture();
		bool UnBindSharedTexture();

		bool DrawSharedTexture(float max_x = 1.0, float max_y = 1.0, float aspect = 1.0, bool bInvert = true);
		bool DrawToSharedTexture(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, float max_x = 1.0, float max_y = 1.0, float aspect = 1.0, bool bInvert = false, GLuint HostFBO = 0);

		// DX9
		bool bUseDX9; // Use DX11 (default) or DX9
		bool UseDX9(bool bDX9);
		bool isDX9();

		// Set and get flags only
		void SetDX9(bool bDX9);
		bool GetDX9();

		bool bUseMemory; // Use memoryshare
		bool GetMemoryShareMode();
		bool SetMemoryShareMode(bool bMem = true);

		D3DFORMAT DX9format; // the DX9 texture format to be used
		void SetDX9format(D3DFORMAT textureformat);

		int GetNumAdapters(); // Get the number of graphics adapters in the system
		bool GetAdapterName(int index, char *adaptername, int maxchars); // Get an adapter name
		bool SetAdapter(int index); // Set required graphics adapter for output
		int GetAdapter(); // Get the SpoutDirectX global adapter index

		bool GetHostPath(const char *sendername, char *hostpath, int maxchars); // The path of the host that produced the sender

		bool CreateDX9interop(unsigned int width, unsigned int height, DWORD dwFormat, bool bReceive = true);
		bool OpenDirectX9(HWND hWnd); // Initialize and prepare DirectX9
		void CleanupDX9();

		// DX11
		DXGI_FORMAT	DX11format; // the DX11 texture format to be used
		void SetDX11format(DXGI_FORMAT textureformat); // set format by user

		bool CreateDX11interop(unsigned int width, unsigned int height, DWORD dwFormat, bool bReceive);
		bool OpenDirectX11(); // Initialize and prepare DirectX11
		bool DX11available(); // Test for DX11 by attempting to open a device
		void CleanupDX11();

		// Common
		bool OpenDirectX(HWND hWnd, bool bDX9);
		void CleanupDirectX();
		HANDLE LinkGLDXtextures(void* pDXdevice, void* pSharedTexture, HANDLE dxShareHandle, GLuint glTextureID);

		// Utilities
		bool GLDXcompatible();
		int GetVerticalSync();
		bool SetVerticalSync(bool bSync = true);
		bool GetAdapterInfo(char *renderadapter, 
						    char *renderdescription, char *renderversion,
							char *displaydescription, char *displayversion,
							int maxsize, bool &bUseDX9);
		bool CheckSpout2004(); // LJ DEBUG - 2.005 transition utility TODO : remove
		
		// Registry read/write
		// 20.11.15 - moved to SpoutDirectX class
		// bool ReadDwordFromRegistry(DWORD *pValue, const char *subkey, const char *valuename);
		// bool WriteDwordToRegistry(DWORD dwValue, const char *subkey, const char *valuename);

		// Spout objects
		spoutMemoryShare memoryshare; // Memory sharing
		spoutSenderNames senders; // Sender management
		spoutDirectX spoutdx; // DirectX class
		
		// Locks for gl/dx interop functions
		HRESULT LockInteropObject(HANDLE hDevice, HANDLE *hObject);
		HRESULT UnlockInteropObject(HANDLE hDevice, HANDLE *hObject);
		
		// Memoryshare functions
		bool WriteMemory (GLuint TexID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert = false);
		bool ReadMemory  (GLuint TexID, GLuint TextureTarget, unsigned int width, unsigned int height, bool bInvert = false);
		bool WriteMemoryPixels (unsigned char *pixels, unsigned int width, unsigned int height);
		bool ReadMemoryPixels  (unsigned char *pixels, unsigned int width, unsigned int height);
		bool DrawSharedMemory  (float max_x = 1.0, float max_y = 1.0, float aspect = 1.0, bool bInvert = false);
		bool DrawToSharedMemory(GLuint TextureID, GLuint TextureTarget, unsigned int width, unsigned int height, float max_x = 1.0, float max_y = 1.0, float aspect = 1.0, bool bInvert = false, GLuint HostFBO = 0);

		GLuint GetGLtextureID();
		// GLuint GetTextureWidth();
		// GLuint GetTextureHeight();

		// Global texture and fbo used for texture sharing
		GLuint m_glTexture; // the OpenGL texture linked to the shared DX texture
		GLuint m_fbo; // General fbo used for texture transfers

		// public for external access
		IDirect3DDevice9Ex* m_pDevice;     // DX9 device
		LPDIRECT3DTEXTURE9  m_dxTexture;   // the shared DX9 texture
		HANDLE m_dxShareHandle;            // the shared DX texture handle

		ID3D11Device* g_pd3dDevice;        // DX11 device
		ID3D11Texture2D* g_pSharedTexture; // The shared DX11 texture

		void GLerror();
		void PrintFBOstatus(GLenum status); // debug


protected:

		bool m_bInitialized;    // this instance initialized flag
		bool bExtensionsLoaded; // extensions have been loaded
		bool bFBOavailable;     // fbo extensions available
		bool bBLITavailable;    // fbo blit extensions available
		bool bPBOavailable;     // pbo extensions available
		bool bSWAPavailable;    // swap extensions available

		HWND              m_hWnd;          // parent window
		HANDLE            m_hSharedMemory; // handle to the texture info shared memory
		SharedTextureInfo m_TextureInfo;   // local texture info structure
		GLuint            m_TexID;         // Local texture used for memoryshare functions
		unsigned int      m_TexWidth;      // width and height of local texture
		unsigned int      m_TexHeight;     // for testing changes of memoryshare sender size

		// DX11
		ID3D11DeviceContext* g_pImmediateContext;
		D3D_DRIVER_TYPE      g_driverType;
		D3D_FEATURE_LEVEL    g_featureLevel;

		// DX9
		IDirect3D9Ex* m_pD3D; // DX9 object
	
		HANDLE m_hInteropDevice; // handle to the DX/GL interop device
		HANDLE m_hInteropObject; // handle to the DX/GL interop object (the shared texture)
		HANDLE m_hAccessMutex;   // Texture access mutex lock handle

		// LJ DEBUG
		HANDLE m_hReceiverAccessMutex;   // Texture access mutex lock handle

		bool getSharedTextureInfo(const char* sharedMemoryName);
		bool setSharedTextureInfo(const char* sharedMemoryName);

		bool OpenDeviceKey(const char* key, int maxsize, char *description, char *version);
		void trim(char * s);
		bool InitTexture(GLuint &texID, GLenum GLformat, unsigned int width, unsigned int height);


};

#endif
