#include "DirectXRenderer.h"

#include <iostream>
#include <d3dcompiler.h>
#include <dxgi.h>

#include <GL/glew.h>
#include <GL/wglew.h>

using namespace std;


char dxShader[] = R"(
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

struct matrix { float m[4][4]; };
struct float3 { float x, y, z; };
struct Constants
{
	matrix Transform;
	matrix Projection;
	float3 LightVector;
};

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

bool DirectXRenderer::init()
{
	return false;
}

std::string DirectXRenderer::getRenderExtensionName()
{
	return XR_KHR_D3D11_ENABLE_EXTENSION_NAME;
}

bool DirectXRenderer::initPlatformResources(int width, int heigth)
{
	this->sizeX = width;
	this->sizeY = heigth;

	/////////////////////////////////////////
	//			DIRECTX SHADERS
	ID3DBlob* vsBlob;
	D3DCompile(dxShader, sizeof(dxShader), nullptr, nullptr, nullptr, "vs_main", "vs_5_0", 0, 0, &vsBlob, nullptr);
	//D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "vs_main", "vs_5_0", 0, 0, &vsBlob, nullptr);

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
	//D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "ps_main", "ps_5_0", 0, 0, &psBlob, nullptr);

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
	float VertexData[] = // float3 position, float3 normal, float2 texcoord, float4 color
	{
		 sizeX,  sizeY,	 0.0f,		0.0f,  0.0f, -1.0f,		1.0f,  1.0f,	1.f,  1.f,  1.f,  1.000f,
		-sizeX,  sizeY,	 0.0f,		0.0f,  0.0f, -1.0f,		0.0f,  1.0f,	1.f,  1.f,  1.f,  1.000f,
		-sizeX,  -sizeY, 0.0f,		0.0f,  0.0f, -1.0f,		0.0f,  0.0f,	1.f,  1.f,  1.f,  1.000f,
		 sizeX,  -sizeY, 0.0f,		0.0f,  0.0f, -1.0f,		1.0f,  0.0f,	1.f,  1.f,  1.f,  1.000f,
	};

	unsigned int IndexData[] =
	{
		  2,   1,   0,
		  2,   0,   3,
	};

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

	initInterop();

	return true;
}

bool DirectXRenderer::free()
{
	return false;
}

bool DirectXRenderer::beginEyeFrame(int eye, int textureIndex)
{
	D3D11_VIEWPORT viewport = CD3D11_VIEWPORT(0.f, 0.f, (float)sizeX, (float)sizeY);
	dxContext->RSSetViewports(1, &viewport);

	DXSwapchainSurface surface = swapchains[eye].surfaceData[textureIndex];

	// Wipe our swapchain color and depth target clean, and then set them up for rendering!
	float clear[] = { 0, 0, 0, 1 };
	dxContext->ClearRenderTargetView(surface.targetView, clear);
	dxContext->ClearDepthStencilView(surface.depthView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	dxContext->OMSetRenderTargets(1, &surface.targetView, surface.depthView);

	bool ok = wglDXLockObjectsNV(interopHandle, 1, &textureHandle);

	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	glViewport(0, 0, sizeX, sizeY);

	return true;
}

bool DirectXRenderer::endEyeRender(int eye, int textureIndex)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	bool ok = wglDXUnlockObjectsNV(interopHandle, 1, &textureHandle);

	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	dxContext->Map(constantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);

	Constants* constants = reinterpret_cast<Constants*>(mappedSubresource.pData);

	//constants->Transform = rotateX * rotateY * rotateZ * scale * translate;
	matrix identity;
	ZeroMemory(&identity, sizeof(identity));
	identity.m[0][0] = identity.m[1][1] = identity.m[2][2] = identity.m[3][3] = 1.f;
	constants->Transform = identity;
	constants->Projection = createOrthoLH(sizeX, sizeY, -1.f, 1.f);
	constants->LightVector = { 1.0f, -1.0f, 1.0f };

	dxContext->Unmap(constantBuffer, 0);

	///////////////////////////////////////////////////////////////////////////////////////////

	FLOAT backgroundColor[4] = { 0.025f, 0.025f, 0.025f, 1.0f };

	UINT stride = 12 * 4; // vertex size (12 floats: float3 position, float3 normal, float2 texcoord, float4 color)
	UINT offset = 0;

	///////////////////////////////////////////////////////////////////////////////////////////

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

	//d3d_context->OMSetRenderTargets(1, &frameBufferView, depthBufferView);

	//d3d_context->OMSetDepthStencilState(depthStencilState, 0);
	//d3d_context->OMSetBlendState(blendState, nullptr, 0xffffffff);

	///////////////////////////////////////////////////////////////////////////////////////////
	dxContext->DrawIndexed(6, 0, 0);

	return true;
}

void * DirectXRenderer::getGraphicsBinding(XrInstance &xrInstance, XrSystemId &xrSystem)
{
	XrGraphicsBindingD3D11KHR* dxGraphicsBinding = new XrGraphicsBindingD3D11KHR{};

	PFN_xrGetD3D11GraphicsRequirementsKHR getD3D11GraphicsRequirements;
	xrGetInstanceProcAddr(xrInstance, "xrGetD3D11GraphicsRequirementsKHR",
		reinterpret_cast<PFN_xrVoidFunction*>(&getD3D11GraphicsRequirements));

	XrGraphicsRequirementsD3D11KHR dxRequirements = {};
	dxRequirements.type = XR_TYPE_GRAPHICS_REQUIREMENTS_D3D11_KHR;
	dxRequirements.next = nullptr;

	getD3D11GraphicsRequirements(xrInstance, xrSystem, &dxRequirements);

	unsigned int i = 0;
	IDXGIAdapter * pAdapter;
	IDXGIFactory1* dxgiFactory;
	CreateDXGIFactory1(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));

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

	// Create the Direct3D 11 API device object and a corresponding context.
	D3D_DRIVER_TYPE driverType = ((pAdapter == nullptr) ? D3D_DRIVER_TYPE_HARDWARE : D3D_DRIVER_TYPE_UNKNOWN);

	dxRequirements.minFeatureLevel = D3D_FEATURE_LEVEL_11_0;


	/*
	HRESULT res = D3D11CreateDeviceAndSwapChain(pAdapter, driverType, 0, creationFlags, &dxRequirements.minFeatureLevel, 1, D3D11_SDK_VERSION, &swapChainDesc, &dxSwapchain, &dxDevice, nullptr, &dxContext);

		dxSwapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**) &tex);
	*/
	HRESULT res = D3D11CreateDevice(pAdapter, driverType, 0, creationFlags, &dxRequirements.minFeatureLevel, 1,
		D3D11_SDK_VERSION, &dxDevice, nullptr, &dxContext);

	dxGraphicsBinding->type = XR_TYPE_GRAPHICS_BINDING_D3D11_KHR;
	dxGraphicsBinding->device = dxDevice;
	return reinterpret_cast<void*>(dxGraphicsBinding);
}

bool DirectXRenderer::initSwapchains(XrSession &xrSession, std::vector<XrViewConfigurationView> &views)
{
	for (uint32_t i = 0; i < views.size(); i++) {
		// Create a swapchain for this viewpoint! A swapchain is a set of texture buffers used for displaying to screen,
		// typically this is a backbuffer and a front buffer, one for rendering data to, and one for displaying on-screen.
		// A note about swapchain image format here! OpenXR doesn't create a concrete image format for the texture, like 
		// DXGI_FORMAT_R8G8B8A8_UNORM. Instead, it switches to the TYPELESS variant of the provided texture format, like 
		// DXGI_FORMAT_R8G8B8A8_TYPELESS. When creating an ID3D11RenderTargetView for the swapchain texture, we must specify
		// a concrete type like DXGI_FORMAT_R8G8B8A8_UNORM, as attempting to create a TYPELESS view will throw errors, so 
		// we do need to store the format separately and remember it later.

		XrSwapchain handle;
		XrSwapchainCreateInfo swapchain_info = { XR_TYPE_SWAPCHAIN_CREATE_INFO };
		swapchain_info.arraySize = 1;
		swapchain_info.mipCount = 1;
		swapchain_info.faceCount = 1;
		swapchain_info.format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapchain_info.width = views[i].recommendedImageRectWidth;
		swapchain_info.height = views[i].recommendedImageRectHeight;
		swapchain_info.sampleCount = views[i].recommendedSwapchainSampleCount;
		swapchain_info.usageFlags = XR_SWAPCHAIN_USAGE_SAMPLED_BIT | XR_SWAPCHAIN_USAGE_COLOR_ATTACHMENT_BIT;
		xrCreateSwapchain(xrSession, &swapchain_info, &handle);

		// Find out how many textures were generated for the swapchain
		uint32_t surface_count = 0;
		xrEnumerateSwapchainImages(handle, 0, &surface_count, nullptr);

		// We'll want to track our own information about the swapchain, so we can draw stuff onto it! We'll also create
		// a depth buffer for each generated texture here as well with make_surfacedata.
		Swapchain swapchain = {};
		swapchain.width = swapchain_info.width;
		swapchain.height = swapchain_info.height;
		swapchain.handle = handle;
		swapchain.surfaceImages.resize(surface_count, { XR_TYPE_SWAPCHAIN_IMAGE_D3D11_KHR });
		swapchain.surfaceData.resize(surface_count);

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

	// Get information about the swapchain image that OpenXR made for us!
	D3D11_TEXTURE2D_DESC      color_desc;
	swapchainImage.texture->GetDesc(&color_desc);

	// Create a view resource for the swapchain image target that we can use to set up rendering.
	D3D11_RENDER_TARGET_VIEW_DESC target_desc = {};
	target_desc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	// NOTE: Why not use color_desc.Format? Check the notes over near the xrCreateSwapchain call!
	// Basically, the color_desc.Format of the OpenXR created swapchain is TYPELESS, but in order to
	// create a View for the texture, we need a concrete variant of the texture format like UNORM.
	target_desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxDevice->CreateRenderTargetView(swapchainImage.texture, &target_desc, &result.targetView);

	// Create a depth buffer that matches 
	ID3D11Texture2D     *depth_texture;
	D3D11_TEXTURE2D_DESC depth_desc = {};
	depth_desc.SampleDesc.Count = 1;
	depth_desc.MipLevels = 1;
	depth_desc.Width = color_desc.Width;
	depth_desc.Height = color_desc.Height;
	depth_desc.ArraySize = color_desc.ArraySize;
	depth_desc.Format = DXGI_FORMAT_R32_TYPELESS;
	depth_desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_DEPTH_STENCIL;
	dxDevice->CreateTexture2D(&depth_desc, nullptr, &depth_texture);

	// And create a view resource for the depth buffer, so we can set that up for rendering to as well!
	D3D11_DEPTH_STENCIL_VIEW_DESC stencil_desc = {};
	stencil_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	stencil_desc.Format = DXGI_FORMAT_D32_FLOAT;
	dxDevice->CreateDepthStencilView(depth_texture, &stencil_desc, &result.depthView);

	// We don't need direct access to the ID3D11Texture2D object anymore, we only need the view
	depth_texture->Release();

	return result;
}

void DirectXRenderer::initInterop()
{
	IDXGIDevice1* dxgiDevice;
	dxDevice->QueryInterface(__uuidof(IDXGIDevice1), reinterpret_cast<void**>(&dxgiDevice));

	IDXGIAdapter* dxgiAdapter;
	dxgiDevice->GetAdapter(&dxgiAdapter);

	IDXGIFactory2* dxgiFactory;
	dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory));

	///////////////////////////////////////////////////////////////////////////////////////////////

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	swapChainDesc.BufferDesc.Width = 2200; // use window width
	swapChainDesc.BufferDesc.Height = 2200; // use window height
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

	dxSwapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&dxTexture));
	dxDevice->CreateShaderResourceView(dxTexture, nullptr, &dxTextureResource);

	glGenRenderbuffers(1, &colorBuf);
	glBindRenderbuffer(GL_RENDERBUFFER, colorBuf);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, sizeX, sizeY);

	glGenRenderbuffers(1, &rboDepthId);
	glBindRenderbuffer(GL_RENDERBUFFER, rboDepthId);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, sizeX, sizeY);

	glGenFramebuffers(1, &fboId);
	glBindFramebuffer(GL_FRAMEBUFFER, fboId);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, colorBuf);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepthId);


	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "[ERROR] FBO not complete (error: " << status << ")" << std::endl;
		return;
	}

	interopHandle = wglDXOpenDeviceNV(dxDevice);
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

