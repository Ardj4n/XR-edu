#include "DirectXRenderer.h"

#include <iostream>
#include <d3dcompiler.h>
#include <dxgi.h>

#include <GL/glew.h>
#include <GL/wglew.h>

using namespace std;


char dxShader[]  = R"(
cbuffer constants : register(b0)
{
	row_major float4x4 transform;
	row_major float4x4 projection;
	float3 lightvector;
}

struct vs_in
{
	float3 position : POS;
	float3 normal : NOR;
	float2 texcoord : TEX;
	float4 color : COL;
};

struct vs_out
{
	float4 position : SV_POSITION;
	float2 texcoord : TEX;
	float4 color : COL;
};

Texture2D    mytexture : register(t0);
SamplerState mysampler : register(s0);

vs_out vs_main(vs_in input)
{
	float light = clamp(dot(normalize(mul(float4(input.normal, 0.0f), transform).xyz), normalize(-lightvector)), 0.0f, 1.0f) * 0.8f + 0.2f;

	vs_out output;

	output.position = mul(float4(input.position, 1.0f), mul(transform, projection));
	output.texcoord = input.texcoord;
	output.color = float4(input.color.rgb * light, input.color.a);

	return output;
}

float4 ps_main(vs_out input) : SV_TARGET
{
	return mytexture.Sample(mysampler, input.texcoord) * input.color;
}
)";

//DirectX compatible structures for vertex buffers and matrices
struct matrix { float m[4][4]; };
struct float3 { float x, y, z; };
struct Constants
{
	matrix Transform;
	matrix Projection;
	float3 LightVector;
};

//Creates Orthogonal left handed camera
matrix inline createOrthoLH
(
	float ViewWidth,
	float ViewHeight,
	float NearZ,
	float FarZ
) noexcept
{
	float fRange = 1.0f / (FarZ - NearZ);

	matrix M;
	M.m[0][0] = 2.0f / ViewWidth;
	M.m[0][1] = 0.0f;
	M.m[0][2] = 0.0f;
	M.m[0][3] = 0.0f;

	M.m[1][0] = 0.0f;
	M.m[1][1] = 2.0f / ViewHeight;
	M.m[1][2] = 0.0f;
	M.m[1][3] = 0.0f;

	M.m[2][0] = 0.0f;
	M.m[2][1] = 0.0f;
	M.m[2][2] = fRange;
	M.m[2][3] = 0.0f;

	M.m[3][0] = 0.0f;
	M.m[3][1] = 0.0f;
	M.m[3][2] = -fRange * NearZ;
	M.m[3][3] = 1.0f;
	return M;
}

DirectXRenderer::DirectXRenderer()
{
}


DirectXRenderer::~DirectXRenderer()
{
}

//extension that must be enabled before calling xrCreateInstance
std::string DirectXRenderer::getRenderExtensionName()
{
	return XR_KHR_D3D11_ENABLE_EXTENSION_NAME;
}

bool DirectXRenderer::initPlatformResources(int width, int heigth)
{
     //set swapchain image size
	this->sizeX = width;
	this->sizeY = heigth;

	/////////////////////////////////////////
	//			DIRECTX SHADERS
	ID3DBlob* vsBlob;
	D3DCompile(dxShader, sizeof(dxShader), nullptr, nullptr, nullptr, "vs_main", "vs_5_0", 0, 0, &vsBlob, nullptr);

	dxDevice->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &vertexShader);

	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{ "POS", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,                            0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NOR", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEX", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	dxDevice->CreateInputLayout(inputElementDesc, ARRAYSIZE(inputElementDesc), vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &inputLayout);

	ID3DBlob* psBlob;
	D3DCompile(dxShader, sizeof(dxShader), nullptr, nullptr, nullptr, "ps_main", "ps_5_0", 0, 0, &psBlob, nullptr);

	dxDevice->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &pixelShader);

	/////////////////////////////////////////
	//			DIRECTX RASTERIZER & TEXTURE SAMPLER
	D3D11_RASTERIZER_DESC rasterizerDesc = {};
	rasterizerDesc.FillMode = D3D11_FILL_SOLID;
	rasterizerDesc.CullMode = D3D11_CULL_BACK;

	dxDevice->CreateRasterizerState(&rasterizerDesc, &rasterizerState);

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	samplerDesc.BorderColor[0] = 1.0f;
	samplerDesc.BorderColor[1] = 1.0f;
	samplerDesc.BorderColor[2] = 1.0f;
	samplerDesc.BorderColor[3] = 1.0f;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;

	dxDevice->CreateSamplerState(&samplerDesc, &samplerState);

	/////////////////////////////////////////
	//			DIRECTX BUFFERS
	float sizeX = width / 2.f;
	float sizeY = heigth / 2.f;
	//DirectX pasthrough box vertices
	float VertexData[] = // float3 position, float3 normal, float2 texcoord, float4 color
	{
		 sizeX,  sizeY,	 0.0f,		0.0f,  0.0f, -1.0f,		1.0f,  1.0f,	1.f,  1.f,  1.f,  1.000f,
		-sizeX,  sizeY,	 0.0f,		0.0f,  0.0f, -1.0f,		0.0f,  1.0f,	1.f,  1.f,  1.f,  1.000f,
		-sizeX,  -sizeY, 0.0f,		0.0f,  0.0f, -1.0f,		0.0f,  0.0f,	1.f,  1.f,  1.f,  1.000f,
		 sizeX,  -sizeY, 0.0f,		0.0f,  0.0f, -1.0f,		1.0f,  0.0f,	1.f,  1.f,  1.f,  1.000f,
	};

	//DirectX pasthrough box faces
	unsigned int IndexData[] =
	{
		  2,   1,   0,
		  2,   0,   3,
	};

	//Load buffer's data in gpu memory (vertex array object and face data object)
	D3D11_BUFFER_DESC constantBufferDesc = {};
	constantBufferDesc.ByteWidth = sizeof(Constants) + 0xf & 0xfffffff0;
	constantBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	constantBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constantBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	dxDevice->CreateBuffer(&constantBufferDesc, nullptr, &constantBuffer);

	D3D11_BUFFER_DESC vertexBufferDesc = {};
	vertexBufferDesc.ByteWidth = sizeof(VertexData);
	vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertexData = { VertexData };
	dxDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer);

	D3D11_BUFFER_DESC indexBufferDesc = {};
	indexBufferDesc.ByteWidth = sizeof(IndexData);
	indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA indexData = { IndexData };
	dxDevice->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer);

	/////////////////////////////////////////
	//			INTEROP INIT
	initInterop();

	return true;
}

bool DirectXRenderer::free()
{
     if(swapchains.size() > 0) {
          for (int i = 0; i < swapchains.size(); i++) {
               // destroy each swapchain created by OpenXR
               xrDestroySwapchain(swapchains[i].handle);
               for (uint32_t j = 0; j < swapchains[i].surfaceData.size(); j++) {
                    swapchains[i].surfaceData[j].depthView->Release();
                    swapchains[i].surfaceData[j].targetView->Release();
               }
          }
          swapchains.clear();
     }

	// free interop resources
	if (textureHandle)		{ wglDXUnregisterObjectNV(interopHandle, textureHandle); }
	if (interopHandle)		{ wglDXCloseDeviceNV(interopHandle); }

	if (colorBuf)			{ glDeleteRenderbuffers(1, &colorBuf); }
	if (rboDepthId)			{ glDeleteRenderbuffers(1, &rboDepthId); }
	if (fboId)				{ glDeleteFramebuffers(1, &fboId); }

	// free DirectX resources
	if (dxSwapchain)		{ dxSwapchain->Release(); dxSwapchain = nullptr; }
	if (dxTexture)			{ dxTexture->Release(); dxTexture = nullptr; }
	if (dxTextureResource)	{ dxTextureResource->Release(); dxTextureResource = nullptr; }

	if (samplerState)		{ samplerState->Release(); samplerState = nullptr; }
	if (rasterizerState)	{ rasterizerState->Release(); rasterizerState = nullptr; }
	if (pixelShader)		{ pixelShader->Release(); pixelShader = nullptr; }
	if (vertexShader)		{ vertexShader->Release(); vertexShader = nullptr; }

	if (indexBuffer)		{ indexBuffer->Release(); indexBuffer = nullptr; }
	if (vertexBuffer)		{ vertexBuffer->Release(); vertexBuffer = nullptr; }
	if (constantBuffer)		{ constantBuffer->Release(); constantBuffer = nullptr; }
	if (inputLayout)		{ inputLayout->Release(); inputLayout = nullptr; }

	if (dxContext)			{ dxContext->Release(); dxContext = nullptr; }
	if (dxDevice)			{ dxDevice->Release();  dxDevice = nullptr; }

	return true;
}

bool DirectXRenderer::beginEyeFrame(int eye, int textureIndex)
{
	// reset DirectX viewport
	D3D11_VIEWPORT viewport = CD3D11_VIEWPORT(0.f, 0.f, (float)sizeX, (float)sizeY);
	dxContext->RSSetViewports(1, &viewport);

	// get collection of DirectX render target view and depth view
	DXSwapchainSurface surface = swapchains[eye].surfaceData[textureIndex];

	//clear color and depth buffer
	float clear[] = { 0, 0, 0, 1 };
	dxContext->ClearRenderTargetView(surface.targetView, clear);
	dxContext->ClearDepthStencilView(surface.depthView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	// set the new render target view associated to the current OpenXR render buffer
	dxContext->OMSetRenderTargets(1, &surface.targetView, surface.depthView);

	// wglDXLockObjectsNV enables rendering with OpenGL in a shared texture  
	bool ok = wglDXLockObjectsNV(interopHandle, 1, &textureHandle);

	// set fbo FBO and viewport so the application is ready to render
	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glViewport(0, 0, sizeX, sizeY);

	return true;
}

bool DirectXRenderer::endEyeRender(int eye, int textureIndex)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	//wglDXUnlockObjectsNV disables rendering with OpenGL
	bool ok = wglDXUnlockObjectsNV(interopHandle, 1, &textureHandle);

	/////////////////////////////////////////
	// DirectX passthroug
	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	dxContext->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);

	Constants* constants = reinterpret_cast<Constants*>(mappedSubresource.pData);

	//set modelview matrix as identity
	matrix identity;
	ZeroMemory(&identity, sizeof(identity));
	identity.m[0][0] = identity.m[1][1] = identity.m[2][2] = identity.m[3][3] = 1.f;
	constants->Transform = identity;
	//set orthogonal projection matrix matrix 
	constants->Projection = createOrthoLH(sizeX, sizeY, -1.f, 1.f);
	constants->LightVector = { 1.0f, -1.0f, 1.0f };

	dxContext->Unmap(constantBuffer, 0);

	// activate vertex and face buffers 
	FLOAT backgroundColor[4] = { 0.025f, 0.025f, 0.025f, 1.0f };

	UINT stride = 12 * 4; // vertex size (12 floats: float3 position, float3 normal, float2 texcoord, float4 color)
	UINT offset = 0;

	dxContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	dxContext->IASetInputLayout(inputLayout);
	dxContext->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	dxContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	dxContext->VSSetShader(vertexShader, nullptr, 0);
	dxContext->VSSetConstantBuffers(0, 1, &constantBuffer);

	dxContext->RSSetState(rasterizerState);

	dxContext->PSSetShader(pixelShader, nullptr, 0);
	dxContext->PSSetShaderResources(0, 1, &dxTextureResource);
	dxContext->PSSetSamplers(0, 1, &samplerState);

	// draw triangles 
	dxContext->DrawIndexed(6, 0, 0);

	return true;
}

void * DirectXRenderer::getGraphicsBinding(XrInstance &xrInstance, XrSystemId &xrSystem)
{
	XrGraphicsBindingD3D11KHR* dxGraphicsBinding = new XrGraphicsBindingD3D11KHR{};

	// Load method enabled by XR_USE_GRAPHICS_API_D3D11
	PFN_xrGetD3D11GraphicsRequirementsKHR getD3D11GraphicsRequirements;
	xrGetInstanceProcAddr(xrInstance, "xrGetD3D11GraphicsRequirementsKHR",
		reinterpret_cast<PFN_xrVoidFunction*>(&getD3D11GraphicsRequirements));

	XrGraphicsRequirementsD3D11KHR dxRequirements = {};
	dxRequirements.type = XR_TYPE_GRAPHICS_REQUIREMENTS_D3D11_KHR;
	dxRequirements.next = nullptr;

	// The OpenXR runtime requires getD3D11GraphicsRequirements to be called before creating an XrSession in Windows
	// this method populates an XrGraphicsRequirementsD3D11KHR structure which contains the adapter LUID neccessary to create 
	// the DirectX device
	getD3D11GraphicsRequirements(xrInstance, xrSystem, &dxRequirements);

	unsigned int i = 0;
	IDXGIAdapter * pAdapter;
	IDXGIFactory1* dxgiFactory;
	CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));

	// Loop through available adapters until the one with the returned LUID is found
	DXGI_ADAPTER_DESC adapterDesc;
	while (dxgiFactory->EnumAdapters(i, &pAdapter) != DXGI_ERROR_NOT_FOUND)
	{
		pAdapter->GetDesc(&adapterDesc);

		if (memcmp(&adapterDesc.AdapterLuid, &dxRequirements.adapterLuid, sizeof(dxRequirements.adapterLuid)) == 0)
			break;

		++i;
	}

	std::wcout << "Using graphics adapter " << adapterDesc.Description << std::endl;

	unsigned int creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
	creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_DRIVER_TYPE driverType = ((pAdapter == nullptr) ? D3D_DRIVER_TYPE_HARDWARE : D3D_DRIVER_TYPE_UNKNOWN);

	// feature leve must be DirectX 11
	dxRequirements.minFeatureLevel = D3D_FEATURE_LEVEL_11_0;

	// Create the Direct3D 11 API device object and a corresponding context.
	HRESULT res = D3D11CreateDevice(pAdapter, driverType, 0, creationFlags, &dxRequirements.minFeatureLevel, 1,
		D3D11_SDK_VERSION, &dxDevice, nullptr, &dxContext);

	dxGraphicsBinding->type = XR_TYPE_GRAPHICS_BINDING_D3D11_KHR;
	dxGraphicsBinding->device = dxDevice;
	return reinterpret_cast<void*>(dxGraphicsBinding);
}

bool DirectXRenderer::initSwapchains(XrSession &xrSession, std::vector<XrViewConfigurationView> &views)
{
	for (uint32_t i = 0; i < views.size(); i++) {

		// Swapchain creation. IMPORTANT! format must be DXGI_FORMAT_R8G8B8A8_UNORM
		XrSwapchain handle;
		XrSwapchainCreateInfo swapchain_info	= { XR_TYPE_SWAPCHAIN_CREATE_INFO };
		swapchain_info.arraySize				= 1;
		swapchain_info.mipCount					= 1;
		swapchain_info.faceCount				= 1;
		swapchain_info.format					= DXGI_FORMAT_R8G8B8A8_UNORM;
		swapchain_info.width					= views[i].recommendedImageRectWidth;
		swapchain_info.height					= views[i].recommendedImageRectHeight;
		swapchain_info.sampleCount				= views[i].recommendedSwapchainSampleCount;
		swapchain_info.usageFlags				= XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
		xrCreateSwapchain(xrSession, &swapchain_info, &handle);

		// Find out how many textures were generated for the swapchain
		uint32_t surface_count = 0;
		xrEnumerateSwapchainImages(handle, 0, &surface_count, nullptr);

		Swapchain swapchain				= {};
		swapchain.width					= swapchain_info.width;
		swapchain.height				= swapchain_info.height;
		swapchain.handle				= handle;
		swapchain.surfaceImages.resize(surface_count, { XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR });
		swapchain.surfaceData.resize(surface_count);

		// xrEnumerateSwapchainImages fills an array of graphics API-specific XrSwapchainImage (XrSwapchainImageD3D11KHR) 
		// Each image created needs a render surface to be initialized
		xrEnumerateSwapchainImages(swapchain.handle, surface_count, &surface_count, (XrSwapchainImageBaseHeader*)swapchain.surfaceImages.data());
		for (uint32_t i = 0; i < surface_count; i++) {
			swapchain.surfaceData[i] = initSurfaces(swapchain.surfaceImages[i]);
		}
		swapchains.push_back(swapchain);
	}

	return true;
}

XrSwapchain DirectXRenderer::getSwapchian(int eye)
{
	return swapchains[eye].handle;
}

DirectXRenderer::DXSwapchainSurface DirectXRenderer::initSurfaces(XrSwapchainImageD3D11KHR& swapchainImage)
{
	DirectXRenderer::DXSwapchainSurface result = {};

	// Get information about the swapchain image created by the OpenXR runtime
	D3D11_TEXTURE2D_DESC      color_desc;
	swapchainImage.texture->GetDesc(&color_desc);

	// Create a view resource for the swapchain image target used to set up rendering.
	D3D11_RENDER_TARGET_VIEW_DESC target_desc = {};
	target_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	// The Format of the OpenXR created swapchain is TYPELESS, but in order to
	// create a View for the texture, we need a concrete variant of the texture format like UNORM.
	target_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxDevice->CreateRenderTargetView(swapchainImage.texture, &target_desc, &result.targetView);

	// Create a depth buffer that matches 
	ID3D11Texture2D     *depth_texture;
	D3D11_TEXTURE2D_DESC depth_desc		= {};
	depth_desc.SampleDesc.Count			= 1;
	depth_desc.MipLevels				= 1;
	depth_desc.Width					= color_desc.Width;
	depth_desc.Height					= color_desc.Height;
	depth_desc.ArraySize				= color_desc.ArraySize;
	depth_desc.Format					= DXGI_FORMAT_R32_TYPELESS;
	depth_desc.BindFlags				= D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
	dxDevice->CreateTexture2D(&depth_desc, nullptr, &depth_texture);

	// Create a view resource for the depth buffer
	D3D11_DEPTH_STENCIL_VIEW_DESC stencil_desc = {};
	stencil_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	stencil_desc.Format = DXGI_FORMAT_D32_FLOAT;
	dxDevice->CreateDepthStencilView(depth_texture, &stencil_desc, &result.depthView);

	// Direct access to the ID3D11Texture2D object is not needed anymore, only the view is needed
	depth_texture->Release();

	return result;
}

/*l'interop viene realizzato attraverso i colorbuffer. Le swapchain OpenXR sono inizializzate dal runtime con un formato TYPELESS 
* che non è compatibile con il formato supportato dall'interop. Dunque è necessario creare una nostra swapchain DirectX con il formato
* DXGI_FORMAT_B8G8R8A8_UNORM mentre le risorge opengl con il formato GL_RGBA8. 
* In questo modo vengono inizializzati dei colorbuffer da DirectX che possiamo usare per l'interop. Un ulteriore accorgimento è quello
* di settare il BufferUsage = DXGI_USAGE_SHADER_INPUT altrimenti non è possibile creare una ShaderResourceView e utilizzare la texture DirectX 
* per il passtrough.
*/
void DirectXRenderer::initInterop()
{
	IDXGIDevice1* dxgiDevice;
	dxDevice->QueryInterface(__uuidof(IDXGIDevice1), reinterpret_cast<void**>(&dxgiDevice));

	IDXGIAdapter* dxgiAdapter;
	dxgiDevice->GetAdapter(&dxgiAdapter);

	IDXGIFactory2* dxgiFactory;
	dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory));

	/////////////////////////////////////////
	//			DirectX swapchain creation
	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	swapChainDesc.BufferDesc.Width = sizeX; // use window width
	swapChainDesc.BufferDesc.Height = sizeY; // use window height
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	//swapChainDesc.Stereo = FALSE;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.BufferUsage = DXGI_USAGE_SHADER_INPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	//swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	swapChainDesc.Flags = 0;

	dxgiFactory->CreateSwapChain(dxDevice, &swapChainDesc, &dxSwapchain);

	/////////////////////////////////////////
	//			set DirectX texture as a resource to be used during texture mapping
	dxSwapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&dxTexture));
	dxDevice->CreateShaderResourceView(dxTexture, nullptr, &dxTextureResource);


	/////////////////////////////////////////
	//			OpenGL Framebuffer set-up
	glGenFramebuffers(1, &fboId);
	glBindFramebuffer(GL_FRAMEBUFFER, fboId);

	/////////////////////////////////////////
	//			OpenGL Colorbuffer set-up
	glGenRenderbuffers(1, &colorBuf);
	glBindRenderbuffer(GL_RENDERBUFFER, colorBuf);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, sizeX, sizeY);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorBuf);

	/////////////////////////////////////////
	//			OpenGL depthbuffer set-up
	glGenRenderbuffers(1, &rboDepthId);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepthId);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, sizeX, sizeY);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepthId);


	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "[ERROR] FBO not complete (error: " << status << ")" << std::endl;
		return;
	}

	/////////////////////////////////////////
	//			Interop handles creation
	// NV_DX_interop specification:					https://www.khronos.org/registry/OpenGL/extensions/NV/WGL_NV_DX_interop.txt
	// NV_DX_interop2 updates compatible resources	https://www.khronos.org/registry/OpenGL/extensions/NV/WGL_NV_DX_interop2.txt
	//wglDXOpenDeviceNV creates the interop main handle which is used to manage shared resources
	interopHandle = wglDXOpenDeviceNV(dxDevice);
	//wglDXRegisterObjectNV creates an handle used during rendering with lock/unlock calls
	textureHandle = wglDXRegisterObjectNV(interopHandle, dxTexture, colorBuf, GL_RENDERBUFFER, WGL_ACCESS_READ_WRITE_NV);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


static void screenshot_ppm(const char *filename, unsigned int width,
	unsigned int height, GLubyte **pixels) {
	size_t i, j, cur;
	const size_t format_nchannels = 3;
	FILE *f = fopen(filename, "w");
	fprintf(f, "P3\n%d %d\n%d\n", width, height, 255);
	*pixels = (GLubyte*)realloc(*pixels, format_nchannels * sizeof(GLubyte) * width * height);
	glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, *pixels);
	for (i = 0; i < height; i++) {
		for (j = 0; j < width; j++) {
			cur = format_nchannels * ((height - i - 1) * width + j);
			fprintf(f, "%3d %3d %3d ", (*pixels)[cur], (*pixels)[cur + 1], (*pixels)[cur + 2]);
		}
		fprintf(f, "\n");
	}
	fclose(f);
}

