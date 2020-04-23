#pragma once

#include <d3d11_1.h>
#include <vector>

#define XR_USE_PLATFORM_WIN32
#define XR_USE_GRAPHICS_API_D3D11
#include <openxr/openxr_platform.h>
#include <openxr/openxr.h>

#include "PlatformRenderer.h"


/**
 * @brief This class enables rendering from OpenGL to an OpenXR application
 * using NV_DX_interop to share framebufferrs across DirectX 11 and OpenGL in a Windows enviroment.
 *
 * @paragrpah This class provides the mechanisms necessary for an application to generate a valid
 * XrGraphicsBindingD3D11KHR structure in order to create a D3D11-based XrSession. 
 * This class handles the creation of all the required D3D11 objects, including a graphics device to be used for rendering.
 * This class also provides mechanisms for the OvXR class to interact with images acquired by calling xrEnumerateSwapchainImages.
 */
class DirectXRenderer : public virtual PlatformRenderer
{
public:
	DirectXRenderer();
	~DirectXRenderer();

	std::string getRenderExtensionName();
	bool initPlatformResources(int width, int height);
	bool free();

	bool beginEyeFrame(int eye, int textureIndex);
	bool endEyeRender(int eye, int textureIndex);

	void* getGraphicsBinding(XrInstance &xrInstance, XrSystemId &xrSystem);
	bool initSwapchains(XrSession &xrSession, std::vector<XrViewConfigurationView> &views);
	
	XrSwapchain getSwapchian(int eye);

private:
	struct DXSwapchainSurface {
		ID3D11DepthStencilView					*depthView;
		ID3D11RenderTargetView					*targetView;
	};

	struct Swapchain {
		XrSwapchain								handle;
		int32_t									width;
		int32_t									height;
		std::vector<XrSwapchainImageD3D11KHR>	surfaceImages;
		std::vector<DXSwapchainSurface>			surfaceData;
	};

	DXSwapchainSurface initSurfaces(XrSwapchainImageD3D11KHR& surfaceImage);
	void initInterop();

	std::vector<Swapchain> swapchains;

	int sizeX, sizeY;

	unsigned int fboId;
	unsigned int colorBuf;
	unsigned int rboDepthId;
	HANDLE interopHandle;
	HANDLE textureHandle;

	ID3D11Device				*dxDevice;
	ID3D11DeviceContext			*dxContext;

	IDXGISwapChain				*dxSwapchain;
	ID3D11Texture2D				*dxTexture;
	ID3D11ShaderResourceView	*dxTextureResource;

	ID3D11Buffer				*constantBuffer;
	ID3D11Buffer				*vertexBuffer;
	ID3D11Buffer				*indexBuffer;
	ID3D11InputLayout			*inputLayout;

	ID3D11VertexShader			*vertexShader;
	ID3D11PixelShader			*pixelShader;
	ID3D11RasterizerState		*rasterizerState;
	ID3D11SamplerState			*samplerState;
};

