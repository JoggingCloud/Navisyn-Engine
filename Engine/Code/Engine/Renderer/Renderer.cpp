#define WIN32_LEAN_AND_MEAN
#define STB_IMAGE_IMPLEMENTATION // Exactly one .CPP (this Image.cpp) should #define this before #including stb_image.h
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <dxgi.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#if defined(_DEBUG)
#define ENGINE_DEBUG_RENDER
#endif

#if defined(ENGINE_DEBUG_RENDER)
#include <dxgidebug.h>
#pragma comment(lib, "dxguid.lib")
#endif

#include "ThirdParty/ImGui/imgui.h"
#include "ThirdParty/ImGui/imgui_impl_win32.h"
#include "ThirdParty/ImGui/imgui_impl_dx11.h"
#include "ThirdParty/stb/stb_image.h"
#include "Engine/Renderer/Window.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/ErrorWarningAssert.hpp"
#include "Engine/Renderer/Shader.hpp"
#include "Engine/Renderer/IndexBuffer.hpp"
#include "Engine/Renderer/VertexBuffer.hpp"
#include "Engine/Core/FileUtils.hpp"
#include "Engine//Renderer/DefaultShader.hpp"
#include "Engine/Core/Image.hpp"
#include "Engine/Math/Vec4.hpp"
#include "Engine/Math/AABB2.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include <filesystem>

#if defined(ENGINE_DEBUG_RENDER)
void* m_dxgiDebug = nullptr;
void* m_dxgiDebugModule = nullptr;
#endif

struct BlurSample
{
	Vec2        m_offset;
	float        m_weight;
	int            m_padding;
};

static int const k_blurMaxSamples = 64;

struct BlurConstants
{
	Vec2 m_texelSize;
	float m_lerpT;
	int m_numSamples;
	BlurSample m_samples[k_blurMaxSamples];
};

static int const k_blurConstantSlot = 5;

struct LightConstants
{
	Vec3 SunDirection;
	float SunIntensity;
	float AmbientIntensity;
	Vec3 WorldEyePosition;
	float minFallOff;
	float maxFallOff;
	float minFallOffMultiplier;
	float maxFallOffMultiplier;
	
	int renderAmbientFlag;
	int renderDiffuseFlag;
	int renderSpecularFlag;
	int renderEmissiveFlag;
	int useDiffuseMapFlag;
	int useNormalMapFlag;
	int useSpecularMapFlag;
	int useGlossinessMapFlag;
	int useEmissiveMapFlag;
	float padding[3];
};
static const int k_lightConstantsSlot = 1;

struct ModelConstants
{
	Mat44 ModelMatrix;
	Vec4 ModelColor;
};
static const int k_modelConstantsSlot = 3;

struct CameraConstants
{
	Mat44 ProjectionMatrix;
	Mat44 ViewMatrix;
};
static const int k_cameraConstantsSlot = 2;

Renderer::Renderer(RenderConfig const& config)
	:m_config(config)
{
	
}

void Renderer::Startup()
{
	InitializeDXGIDebug();
	InitializeD3D();

	m_immediateVBO = CreateVertexBuffer(sizeof(Vertex_PCU));
	m_cameraCBO = CreateConstantBuffer(sizeof(CameraConstants));
	m_modelCBO = CreateConstantBuffer(sizeof(ModelConstants));
	m_lightCBO = CreateConstantBuffer(sizeof(LightConstants));
	m_blurCBO = CreateConstantBuffer(sizeof(BlurConstants));

	CreateEmissiveBlur();

	CreateRasterizerState();

	CreateBlendModes();
	m_defaultTexture = CreateTextureFromImage(*Image::CreateDefaultTexture());

	BindTexture(0, m_defaultTexture);

	CreateSamplerModes();
	CreateAndBindShader();

	CreateDepthStencil();

	CreateImGui();
}

void Renderer::Shutdown()
{
	ShutdownImGui();

	ReleaseD3D();
	ReleaseDepthStates();
	ReleaseRasterizerState();
	ReleaseBlendStates();
	ReleaseSamplerStates();

	SafeDelete(m_loadedTextures);
	SafeDelete(m_blurDownTextures);
	SafeDelete(m_blurUpTextures);
	SafeDelete(m_emissiveBlurredTexture);
	SafeDelete(m_emissiveRenderTexture);

	// Report error leaks and release debug module
#if defined(ENGINE_DEBUG_RENDER)
	((IDXGIDebug*)m_dxgiDebug)->ReportLiveObjects(DXGI_DEBUG_ALL, (DXGI_DEBUG_RLO_FLAGS)(DXGI_DEBUG_RLO_DETAIL | DXGI_DEBUG_RLO_IGNORE_INTERNAL));

	((IDXGIDebug*)m_dxgiDebug)->Release();
	m_dxgiDebug = nullptr;

	::FreeLibrary((HMODULE)m_dxgiDebugModule);
	m_dxgiDebugModule = nullptr;
#endif
}

void Renderer::CreateImGui()
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->AddFontDefault();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(g_theWindow->GetHwnd());
	ImGui_ImplDX11_Init(m_device, m_deviceContext);
}

void Renderer::ImGuiBeginFrame()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	//ImGui::ShowDemoWindow(); // Show demo window! 
}

void Renderer::ImGuiEndFrame()
{
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void Renderer::CreateRasterizerState()
{
	HRESULT hr;

	for (int rasterizerModeIndex = 0; rasterizerModeIndex < static_cast<int>(RasterizerMode::COUNT); rasterizerModeIndex++)
	{
		D3D11_RASTERIZER_DESC rasterizerDesc = {};

		switch (static_cast<RasterizerMode>(rasterizerModeIndex))
		{
		case RasterizerMode::SOLID_CULL_NONE:
			rasterizerDesc.FillMode = D3D11_FILL_SOLID;
			rasterizerDesc.CullMode = D3D11_CULL_NONE;
			break;
		case RasterizerMode::SOLID_CULL_BACK:
			rasterizerDesc.FillMode = D3D11_FILL_SOLID;
			rasterizerDesc.CullMode = D3D11_CULL_BACK;
			break;
		case RasterizerMode::WIREFRAME_CULL_NONE:
			rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
			rasterizerDesc.CullMode = D3D11_CULL_NONE;
			break;
		case RasterizerMode::WIREFRAME_CULL_BACK:
			rasterizerDesc.FillMode = D3D11_FILL_WIREFRAME;
			rasterizerDesc.CullMode = D3D11_CULL_BACK;
		}

		rasterizerDesc.FrontCounterClockwise = true;
		rasterizerDesc.DepthClipEnable = true;
		rasterizerDesc.AntialiasedLineEnable = true;

		hr = m_device->CreateRasterizerState(&rasterizerDesc, &m_rasterizerStates[rasterizerModeIndex]);
		if (!SUCCEEDED(hr))
		{
			ERROR_AND_DIE("Could not create rasterizer state.");
		}
	}
}

void Renderer::SetRasterizerState(RasterizerMode rasterizerMode)
{
	m_desiredRasterizerMode = rasterizerMode;
}

void Renderer::SetRasterizerStateIfChanged()
{
	if (m_rasterizerStates[(int)m_desiredRasterizerMode] != m_rasterizerState)
	{
		m_rasterizerState = m_rasterizerStates[static_cast<int>(m_desiredRasterizerMode)];

		m_deviceContext->RSSetState(m_rasterizerState);
	}
}

void Renderer::ReleaseRasterizerState()
{
	// Release all rasterizer states 
	for (int rasterizerStates = 0; rasterizerStates < static_cast<int>(RasterizerMode::COUNT); rasterizerStates++)
	{
		DX_SAFE_RELEASE(m_rasterizerStates[rasterizerStates]);
	}
	m_rasterizerState = nullptr;
}

void Renderer::CreateAndBindShader()
{
	if (std::filesystem::exists("Data/Shaders/BlurDown"))
	{
		m_blurDownShader = CreateShader("Data/Shaders/BlurDown");
	}

	if (std::filesystem::exists("Data/Shaders/BlurUp"))
	{
		m_blurUpShader = CreateShader("Data/Shaders/BlurUp");
	}

	if (std::filesystem::exists("Data/Shaders/Composite"))
	{
		m_compositeShader = CreateShader("Data/Shaders/Composite");
	}

	m_defaultShader = CreateShader("Default", g_shaderSource);
	BindShader(m_currentShader);
}

void Renderer::InitializeDXGIDebug()
{
	// Create debug module 
#if defined(ENGINE_DEBUG_RENDER)
	m_dxgiDebugModule = (void*)::LoadLibraryA("dxgidebug.dll");
	if (m_dxgiDebugModule == nullptr)
	{
		ERROR_AND_DIE("Could not load dxgidebug.dll.");
	}

	typedef HRESULT(WINAPI* GetDebugModuleCB)(REFIID, void**);
	((GetDebugModuleCB)::GetProcAddress((HMODULE)m_dxgiDebugModule, "DXGIGetDebugInterface"))
		(__uuidof(IDXGIDebug), &m_dxgiDebug);

	if (m_dxgiDebug == nullptr)
	{
		ERROR_AND_DIE("Could not load debug module.");
	}
#endif
}

void Renderer::InitializeD3D()
{
	// Render Startup
	unsigned int deviceFlags = 0;
#if defined(ENGINE_DEBUG_RENDER)
	deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// Create device and swap chain
	DXGI_SWAP_CHAIN_DESC swapChainDesc = { 0 };
	swapChainDesc.BufferDesc.Width = g_theWindow->GetClientDimensions().x;
	swapChainDesc.BufferDesc.Height = g_theWindow->GetClientDimensions().y;
	swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.OutputWindow = (HWND)g_theWindow->GetHwnd();
	swapChainDesc.Windowed = true;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	HRESULT hr;
	hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, NULL, deviceFlags, nullptr, 0, D3D11_SDK_VERSION, &swapChainDesc, &m_swapChain, &m_device, nullptr, &m_deviceContext);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create D3D 11 device and swap chain.");
	}

	// Get back buffer texture
	ID3D11Texture2D* backBuffer;
	hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not get swap chain buffer.");
	}

	// Create render target view 
	hr = m_device->CreateRenderTargetView(backBuffer, NULL, &m_renderTargetView);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could create render target view for swap chain buffer.");
	}

	backBuffer->Release();
}

void Renderer::RenderFrame()
{
	// Present spawn chain 
	HRESULT hr;
	hr = m_swapChain->Present(0, 0);
	if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
	{
		ERROR_AND_DIE("Device has been lost, application will now terminate.");
	}
}

void Renderer::ShutdownImGui()
{
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void Renderer::ReleaseD3D()
{
	SafeDelete(m_immediateVBO);
	SafeDelete(m_fullScreenQuadVBO);
	SafeDelete(m_cameraCBO);
	SafeDelete(m_modelCBO);
	SafeDelete(m_lightCBO);
	SafeDelete(m_blurCBO);
	SafeDelete(m_blurDownShader);
	SafeDelete(m_blurUpShader);
	SafeDelete(m_compositeShader);
	SafeDelete(m_defaultShader);
	m_loadShaders.clear();

	DX_SAFE_RELEASE(m_device);
	DX_SAFE_RELEASE(m_deviceContext);
	DX_SAFE_RELEASE(m_swapChain);
	DX_SAFE_RELEASE(m_renderTargetView);
	DX_SAFE_RELEASE(m_depthStencilView);
	DX_SAFE_RELEASE(m_depthStencilTexture);
}

void Renderer::BeginFrame()
{
	ImGuiBeginFrame();

	// Set render target & depth stencil view 
	ID3D11ShaderResourceView* nullSRVs[] = { nullptr };
	m_deviceContext->PSSetShaderResources(0, 1, nullSRVs);

	ID3D11RenderTargetView* rtvs[] = {
		m_renderTargetView,
		m_emissiveRenderTexture->m_renderTargetView
	};

	m_deviceContext->OMSetRenderTargets(2, rtvs, m_depthStencilView);
}

void Renderer::EndFrame()
{
	ImGuiEndFrame();

	RenderFrame();
}

void Renderer::ClearScreen(Rgba8 const& clearColor)
{
	float colorAsFloats[4];
	clearColor.GetAsFloats(colorAsFloats);

	m_deviceContext->ClearRenderTargetView(m_renderTargetView, colorAsFloats);
	m_deviceContext->ClearDepthStencilView(m_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.f, 0);
	
	float blackAsFloats[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_deviceContext->ClearRenderTargetView(m_emissiveRenderTexture->m_renderTargetView, blackAsFloats);
	m_deviceContext->ClearRenderTargetView(m_emissiveBlurredTexture->m_renderTargetView, blackAsFloats);
}

void Renderer::BeginCamera(Camera camera)
{
	SetModelConstants();

	SetViewport(camera);

	// Create a local CameraConstants structure
	CameraConstants cameraConstants;

	cameraConstants.ProjectionMatrix = camera.GetProjectionMatrix();
	cameraConstants.ViewMatrix = camera.GetViewMatrix();

	// Call Renderer::CopyCPUToGPU to copy the data from the local structure to the constant buffer
	CopyCPUToGPU(&cameraConstants, sizeof(CameraConstants), m_cameraCBO);

	// Call Renderer::BindConstantBuffer passing the buffer and slot
	BindConstantBuffer(k_cameraConstantsSlot, m_cameraCBO);
}

void Renderer::EndCamera([[maybe_unused]] Camera camera)
{
}

void Renderer::SetViewport(const Camera& camera)
{
	const AABB2& cameraViewport = camera.GetViewport();

	// Set viewport
	D3D11_VIEWPORT viewport = { 0 };
	viewport.TopLeftX = cameraViewport.m_mins.x * (float)g_theWindow->GetClientDimensions().x;
	viewport.TopLeftY = cameraViewport.m_mins.y * (float)g_theWindow->GetClientDimensions().y;
	viewport.Width = (cameraViewport.m_maxs.x - cameraViewport.m_mins.x) * (float)g_theWindow->GetClientDimensions().x;
	viewport.Height = (cameraViewport.m_maxs.y - cameraViewport.m_mins.y) * (float)g_theWindow->GetClientDimensions().y;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	m_deviceContext->RSSetViewports(1, &viewport);
}

void Renderer::CreateBlendModes()
{
	HRESULT hr;

	for (int blendModeIndex = 0; blendModeIndex < static_cast<int>(BlendMode::COUNT); blendModeIndex++)
	{
		D3D11_BLEND_DESC blendDesc = {};
		blendDesc.RenderTarget[0].BlendEnable = TRUE;

		switch (static_cast<BlendMode>(blendModeIndex))
		{
		case BlendMode::OPAQUE:
			blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
			blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
			break;
		case BlendMode::ALPHA:
			blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
			blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
			break;
		case BlendMode::ADDITIVE:
			blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
			blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
			break;
		case BlendMode::COUNT:
			break;
		}

		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = blendDesc.RenderTarget[0].SrcBlend;
		blendDesc.RenderTarget[0].DestBlendAlpha = blendDesc.RenderTarget[0].DestBlend;
		blendDesc.RenderTarget[0].BlendOpAlpha = blendDesc.RenderTarget[0].BlendOp;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		hr = m_device->CreateBlendState(&blendDesc, &m_blendStates[blendModeIndex]);
		if (!SUCCEEDED(hr))
		{
			ERROR_AND_DIE("CreateBlendState for the desired blend mode failed.");
		}
	}
}

void Renderer::SetBlendMode(BlendMode blendMode)
{
	m_desiredBlendMode = blendMode;
}

void Renderer::SetBlendStatesIfChanged()
{
	if (m_blendStates[(int)m_desiredBlendMode] != m_blendState)
	{
		m_blendState = m_blendStates[static_cast<int>(m_desiredBlendMode)];
	
		float blendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
		UINT sampleMask = 0xffffffff;
		m_deviceContext->OMSetBlendState(m_blendState, blendFactor, sampleMask);
	}
}

void Renderer::ReleaseBlendStates()
{
	// Release all blend states 
	for (int blendStates = 0; blendStates < static_cast<int>(BlendMode::COUNT); blendStates++)
	{
		DX_SAFE_RELEASE(m_blendStates[blendStates]);
	}
	m_blendState = nullptr;
}

void Renderer::CreateSamplerModes()
{
	HRESULT hr;

	for (int samplerModeIndex = 0; samplerModeIndex < static_cast<int>(SamplerMode::COUNT); samplerModeIndex++)
	{
		D3D11_SAMPLER_DESC samplerDesc = {};

		switch (static_cast<SamplerMode>(samplerModeIndex))
		{
		case SamplerMode::POINT_CLAMP:
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			break;
		case SamplerMode::BILINEAR_WRAP:
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			break;
		case SamplerMode::BILINEAR_CLAMP:
			samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
			samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
			break;
		case SamplerMode::COUNT:
			break;
		}

		samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

		hr = m_device->CreateSamplerState(&samplerDesc, &m_samplerStates[samplerModeIndex]);
		if (!SUCCEEDED(hr))
		{
			ERROR_AND_DIE("CreateSamplerState for SamplerMode failed.");
		}
	}
}

void Renderer::SetSamplerMode(SamplerMode samplerMode)
{
	m_desiredSamplerMode = samplerMode;
}

void Renderer::SetSamplerStatesIfChanged()
{
	if (m_samplerStates[(int)m_desiredSamplerMode] != m_samplerState)
	{
		m_samplerState = m_samplerStates[static_cast<int>(m_desiredSamplerMode)];
	}
	m_deviceContext->PSSetSamplers(0, 1, &m_samplerStates[static_cast<int>(m_desiredSamplerMode)]);
}

void Renderer::ReleaseSamplerStates()
{
	// Release all sampler states 
	for (int samplerStates = 0; samplerStates < static_cast<int>(SamplerMode::COUNT); samplerStates++)
	{
		DX_SAFE_RELEASE(m_samplerStates[samplerStates]);
	}
	m_samplerState = nullptr;
}

void Renderer::CreateDepthStencil()
{
	HRESULT hr;

	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = m_config.m_window->GetClientDimensions().x;
	textureDesc.Height = m_config.m_window->GetClientDimensions().y;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	textureDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	textureDesc.SampleDesc.Count = 1;

	hr = m_device->CreateTexture2D(&textureDesc, nullptr, &m_depthStencilTexture);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Couldn't create texture for depth stencil.");
	}

	for (int depthModeIndex = 0; depthModeIndex < static_cast<int>(DepthMode::COUNT); depthModeIndex++)
	{
		D3D11_DEPTH_STENCIL_DESC depthStencilDesc = {};
		depthStencilDesc.DepthEnable = TRUE;

		switch (static_cast<DepthMode>(depthModeIndex))
		{
		case DepthMode::DISABLED:
			depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
			depthStencilDesc.DepthFunc = D3D11_COMPARISON_ALWAYS;
			hr = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStates[(int)DepthMode::DISABLED]);
			break;
		case DepthMode::ENABLED:
			depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
			depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
			hr = m_device->CreateDepthStencilState(&depthStencilDesc, &m_depthStates[(int)DepthMode::ENABLED]);
			break;
		case DepthMode::COUNT:
			break;
		}
	}

	hr = m_device->CreateDepthStencilView(m_depthStencilTexture, nullptr, &m_depthStencilView);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Couldn't create depth stencil view.");
	}
}

void Renderer::SetDepthMode(DepthMode depthMode)
{
	m_desiredDepthMode = depthMode;
}

void Renderer::SetDepthStatesIfChanged()
{
	if (m_depthStates[(int)m_desiredDepthMode] != m_depthState)
	{
		m_depthState = m_depthStates[static_cast<int>(m_desiredDepthMode)];

		m_deviceContext->OMSetDepthStencilState(m_depthState, 0);
	}
}

void Renderer::ReleaseDepthStates()
{
	// Release all rasterizer states 
	for (int depthStates = 0; depthStates < static_cast<int>(DepthMode::COUNT); depthStates++)
	{
		DX_SAFE_RELEASE(m_depthStates[depthStates]);
	}
	m_depthState = nullptr;
}

void Renderer::SetLightConstants(const Vec3& sunDirection, const float& sunIntensity, const float& ambientIntensity, const Vec3& worldEyePosition, const float& minFallOff, const float& maxFallOff, const float& minFallOffMultiplier, const float& maxFallOffMultiplier, LightingDebug debugLight)
{
	LightConstants lightConstants;

	lightConstants.SunDirection = sunDirection.GetNormalized();
	lightConstants.SunIntensity = sunIntensity;
	
	lightConstants.renderAmbientFlag = debugLight.RenderAmbient;
	lightConstants.renderDiffuseFlag = debugLight.RenderDiffuse;
	lightConstants.renderSpecularFlag = debugLight.RenderSpecular;
	lightConstants.renderEmissiveFlag = debugLight.RenderEmissive;

	lightConstants.useDiffuseMapFlag = debugLight.UseDiffuseMap;
	lightConstants.useNormalMapFlag = debugLight.UseNormalMap;
	lightConstants.useSpecularMapFlag = debugLight.UseSpecularMap;
	lightConstants.useGlossinessMapFlag = debugLight.UseGlossinessMap;
	lightConstants.useEmissiveMapFlag = debugLight.UseEmissiveMap;

	lightConstants.AmbientIntensity = ambientIntensity;
	lightConstants.WorldEyePosition = worldEyePosition;
	lightConstants.minFallOff = minFallOff;
	lightConstants.maxFallOff = maxFallOff;
	lightConstants.minFallOffMultiplier = minFallOffMultiplier;
	lightConstants.maxFallOffMultiplier = maxFallOffMultiplier;

	CopyCPUToGPU(&lightConstants, sizeof(LightConstants), m_lightCBO);

	BindConstantBuffer(k_lightConstantsSlot, m_lightCBO);
}

void Renderer::SetModelConstants(const Mat44& modelMatrix, const Rgba8& modelColor)
{
	// Create a local CameraConstants structure
	ModelConstants modelConstants;

	modelConstants.ModelMatrix = modelMatrix;
	modelColor.GetAsFloats(&modelConstants.ModelColor.x);

	// Call Renderer::CopyCPUToGPU to copy the data from the local structure to the constant buffer
	CopyCPUToGPU(&modelConstants, sizeof(ModelConstants), m_modelCBO);

	// Call Renderer::BindConstantBuffer passing the buffer and slot
	BindConstantBuffer(k_modelConstantsSlot, m_modelCBO);
}

void Renderer::CreateEmissiveBlur()
{
	m_emissiveRenderTexture = CreateRenderTexture(m_config.m_window->GetClientDimensions(), "EmissiveRT");
	m_emissiveBlurredTexture = CreateRenderTexture(m_config.m_window->GetClientDimensions(), "EmissiveRT");

	float minTextureHeight = 16.f;
	float currentTextureHeight = (float)g_theWindow->GetClientDimensions().y * 0.5f;

	while (currentTextureHeight >= minTextureHeight)
	{
		// BLUR DOWN TEXTURE INFO

		Texture* blurDownRenderTexture = CreateRenderTexture(IntVec2((int)(currentTextureHeight * 2.0f), (int)currentTextureHeight), "BlurDownRT");

		m_blurDownTextures.emplace_back(blurDownRenderTexture);

		// BLUR UP TEXTURE INFO

		if (currentTextureHeight > 30.f)
		{
			Texture* blurUpRenderTexture = CreateRenderTexture(IntVec2((int)(currentTextureHeight * 2.0f), (int)currentTextureHeight), "BlurUpRT");

			m_blurUpTextures.emplace_back(blurUpRenderTexture);
		}

		m_blurTextureViewportSize.emplace_back(IntVec2(int(currentTextureHeight * 2.0f), (int)currentTextureHeight));

		currentTextureHeight *= 0.5f;
	}

	std::vector<Vertex_PCU> fullScreenVerts;
	AABB2 fullScreenQuad = AABB2(-1.0f, -1.0f, 1.0f, 1.0f);

	AddVertsForAABB2D(fullScreenVerts, fullScreenQuad, Rgba8::WHITE, Vec2(0.0f, 1.0f), Vec2(1.0f, 0.0f));

	m_fullScreenQuadVBO = CreateVertexBuffer((int)fullScreenVerts.size() * sizeof(Vertex_PCU));
	CopyCPUToGPU(fullScreenVerts.data(), (int)fullScreenVerts.size() * sizeof(Vertex_PCU), m_fullScreenQuadVBO);
}

void Renderer::SetBlurConstants(BlurConstants blurConstants)
{
	CopyCPUToGPU(&blurConstants, sizeof(BlurConstants), m_blurCBO);
	BindConstantBuffer(k_blurConstantSlot, m_blurCBO);
}

void Renderer::RenderEmissive()
{
	ID3D11ShaderResourceView* nullSRVs[] = { nullptr };
	m_deviceContext->PSSetShaderResources(0, 1, nullSRVs);

	SetDepthMode(DepthMode::DISABLED);
	SetBlendMode(BlendMode::OPAQUE);
	SetSamplerMode(SamplerMode::BILINEAR_CLAMP);
	SetRasterizerState(RasterizerMode::SOLID_CULL_BACK);
	BindShader(m_blurDownShader);

	BlurConstants blurDownConstants;
	blurDownConstants.m_numSamples = 13;
	blurDownConstants.m_lerpT = 1.0f;

	blurDownConstants.m_samples[0].m_weight = 0.0323f;
	blurDownConstants.m_samples[0].m_offset = Vec2(-2, -2);

	blurDownConstants.m_samples[1].m_weight = 0.0645f;
	blurDownConstants.m_samples[1].m_offset = Vec2(-2, 0);

	blurDownConstants.m_samples[2].m_weight = 0.0323f;
	blurDownConstants.m_samples[2].m_offset = Vec2(-2, 2);

	blurDownConstants.m_samples[3].m_weight = 0.1290f;
	blurDownConstants.m_samples[3].m_offset = Vec2(-1, -1);

	blurDownConstants.m_samples[4].m_weight = 0.1290f;
	blurDownConstants.m_samples[4].m_offset = Vec2(-1, 1);

	blurDownConstants.m_samples[5].m_weight = 0.0645f;
	blurDownConstants.m_samples[5].m_offset = Vec2(0, -2);

	blurDownConstants.m_samples[6].m_weight = 0.0968f;
	blurDownConstants.m_samples[6].m_offset = Vec2(0, 0);

	blurDownConstants.m_samples[7].m_weight = 0.0645f;
	blurDownConstants.m_samples[7].m_offset = Vec2(0, 2);

	blurDownConstants.m_samples[8].m_weight = 0.1290f;
	blurDownConstants.m_samples[8].m_offset = Vec2(1, -1);

	blurDownConstants.m_samples[9].m_weight = 0.1290f;
	blurDownConstants.m_samples[9].m_offset = Vec2(1, 1);

	blurDownConstants.m_samples[10].m_weight = 0.0323f;
	blurDownConstants.m_samples[10].m_offset = Vec2(2, -2);

	blurDownConstants.m_samples[11].m_weight = 0.0645f;
	blurDownConstants.m_samples[11].m_offset = Vec2(2, 0);

	blurDownConstants.m_samples[12].m_weight = 0.0323f;
	blurDownConstants.m_samples[12].m_offset = Vec2(2, 2);

	for (int i = 0; i < (int)m_blurDownTextures.size(); i++)
	{
		m_deviceContext->PSSetShaderResources(0, 1, nullSRVs);

		m_deviceContext->OMSetRenderTargets(1, &m_blurDownTextures[i]->m_renderTargetView, nullptr);

		if (i > 0)
		{
			m_deviceContext->PSSetShaderResources(0, 1, nullSRVs);

			m_deviceContext->PSSetShaderResources(0, 1, &m_blurDownTextures[i - 1]->m_shaderResourceView);

			blurDownConstants.m_texelSize.x = float(1.0f / (float)m_blurTextureViewportSize[i - 1].x);
			blurDownConstants.m_texelSize.y = float(1.0f / (float)m_blurTextureViewportSize[i - 1].y);
		}
		else
		{
			m_deviceContext->PSSetShaderResources(0, 1, nullSRVs);

			m_deviceContext->PSSetShaderResources(0, 1, &m_emissiveRenderTexture->m_shaderResourceView);

			blurDownConstants.m_texelSize.x = float(1.0f / (float)g_theWindow->GetClientDimensions().x);
			blurDownConstants.m_texelSize.y = float(1.0f / (float)g_theWindow->GetClientDimensions().y);
		}

		SetBlurConstants(blurDownConstants);

		D3D11_VIEWPORT viewport = { 0 };
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.Width = (FLOAT)m_blurTextureViewportSize[i].x;
		viewport.Height = (FLOAT)m_blurTextureViewportSize[i].y;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		m_deviceContext->RSSetViewports(1, &viewport);

		DrawVertexBuffer(m_fullScreenQuadVBO, VertexType::Vertex_PCU, 6);
	}

	BlurConstants blurUpConstants;
	blurUpConstants.m_numSamples = 9;
	blurUpConstants.m_lerpT = 0.85f;

	blurUpConstants.m_samples[0].m_weight = 0.0625f;
	blurUpConstants.m_samples[0].m_offset = Vec2(-1, -1);

	blurUpConstants.m_samples[1].m_weight = 0.1250f;
	blurUpConstants.m_samples[1].m_offset = Vec2(-1, 0);

	blurUpConstants.m_samples[2].m_weight = 0.0625f;
	blurUpConstants.m_samples[2].m_offset = Vec2(-1, 1);

	blurUpConstants.m_samples[3].m_weight = 0.1250f;
	blurUpConstants.m_samples[3].m_offset = Vec2(0, -1);

	blurUpConstants.m_samples[4].m_weight = 0.2500f;
	blurUpConstants.m_samples[4].m_offset = Vec2(0, 0);

	blurUpConstants.m_samples[5].m_weight = 0.1250f;
	blurUpConstants.m_samples[5].m_offset = Vec2(0, 1);

	blurUpConstants.m_samples[6].m_weight = 0.0625f;
	blurUpConstants.m_samples[6].m_offset = Vec2(1, -1);

	blurUpConstants.m_samples[7].m_weight = 0.1250f;
	blurUpConstants.m_samples[7].m_offset = Vec2(1, 0);

	blurUpConstants.m_samples[8].m_weight = 0.0625f;
	blurUpConstants.m_samples[8].m_offset = Vec2(1, 1);

	for (int i = (int)m_blurUpTextures.size() - 1; i >= 0; i--)
	{
		m_deviceContext->PSSetShaderResources(0, 1, nullSRVs);
		m_deviceContext->PSSetShaderResources(1, 1, nullSRVs);

		m_deviceContext->OMSetRenderTargets(1, &m_blurUpTextures[i]->m_renderTargetView, nullptr);

		m_deviceContext->PSSetShaderResources(0, 1, &m_blurDownTextures[i]->m_shaderResourceView);

		if (i == (int)m_blurUpTextures.size() - 1 && i + 1 < (int)m_blurDownTextures.size())
		{
			m_deviceContext->PSSetShaderResources(1, 1, &m_blurDownTextures[i + 1]->m_shaderResourceView);

			if (i + 1 < (int)m_blurTextureViewportSize.size())
			{
				blurUpConstants.m_texelSize.x = float(1.f / (float)m_blurTextureViewportSize[i + 1].x);
				blurUpConstants.m_texelSize.y = float(1.f / (float)m_blurTextureViewportSize[i + 1].y);
			}
		}
		else if (i < (int)m_blurUpTextures.size() - 1)
		{
			m_deviceContext->PSSetShaderResources(1, 1, &m_blurUpTextures[i + 1]->m_shaderResourceView);

			blurUpConstants.m_texelSize.x = float(1.f / (float)m_blurTextureViewportSize[i + 1].x);
			blurUpConstants.m_texelSize.y = float(1.f / (float)m_blurTextureViewportSize[i + 1].y);
		}

		SetBlurConstants(blurUpConstants);

		D3D11_VIEWPORT viewport = { 0 };
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.Width = (FLOAT)m_blurTextureViewportSize[i].x;
		viewport.Height = (FLOAT)m_blurTextureViewportSize[i].y;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;

		m_deviceContext->RSSetViewports(1, &viewport);

		BindShader(m_blurUpShader);
		DrawVertexBuffer(m_fullScreenQuadVBO, VertexType::Vertex_PCU, 6);
	}

	m_deviceContext->PSSetShaderResources(0, 1, nullSRVs);
	m_deviceContext->PSSetShaderResources(1, 1, nullSRVs);

	m_deviceContext->OMSetRenderTargets(1, &m_emissiveBlurredTexture->m_renderTargetView, nullptr);

	m_deviceContext->PSSetShaderResources(0, 1, &m_emissiveRenderTexture->m_shaderResourceView);
	m_deviceContext->PSSetShaderResources(1, 1, &m_blurUpTextures[0]->m_shaderResourceView);

	blurUpConstants.m_texelSize.x = float(1.0f / (float)m_blurTextureViewportSize[0].x);
	blurUpConstants.m_texelSize.y = float(1.0f / (float)m_blurTextureViewportSize[0].y);

	SetBlurConstants(blurUpConstants);

	D3D11_VIEWPORT viewport = { 0 };
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = (float)g_theWindow->GetClientDimensions().x;
	viewport.Height = (float)g_theWindow->GetClientDimensions().y;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	m_deviceContext->RSSetViewports(1, &viewport);

	BindShader(m_blurUpShader);
	DrawVertexBuffer(m_fullScreenQuadVBO, VertexType::Vertex_PCU, 6);

	m_deviceContext->PSSetShaderResources(0, 1, nullSRVs);
	m_deviceContext->PSSetShaderResources(1, 1, nullSRVs);

	ID3D11RenderTargetView* rtvs[] = {
		m_renderTargetView,
		m_emissiveRenderTexture->m_renderTargetView
	};

	m_deviceContext->OMSetRenderTargets(2, rtvs, m_depthStencilView);

	m_deviceContext->PSSetShaderResources(0, 1, &m_emissiveBlurredTexture->m_shaderResourceView);
	SetBlendMode(BlendMode::ADDITIVE);
	BindShader(m_compositeShader);
	DrawVertexBuffer(m_fullScreenQuadVBO, VertexType::Vertex_PCU, 6);
}

BitmapFont* Renderer::CreateOrGetBitmapFont(const char* bitmapFontFilePathWithNoExtension)
{
	BitmapFont* currentFont = GetBitmapFont(bitmapFontFilePathWithNoExtension);
	if (currentFont)
	{
		return currentFont;
	}
	else
	{
		// Never seen this font before!  Let's load it.
		BitmapFont* newfont = CreateBitmapFont(bitmapFontFilePathWithNoExtension);
		return newfont;
	}
}

Texture* Renderer::CreateRenderTexture(IntVec2 dimensions, char const* name)
{
	Texture* renderTexture = new Texture();
	renderTexture->m_dimensions = dimensions;
	renderTexture->m_name = name;

	D3D11_TEXTURE2D_DESC renderTextureDesc = {};
	renderTextureDesc.Width = dimensions.x;
	renderTextureDesc.Height = dimensions.y;
	renderTextureDesc.MipLevels = 1;
	renderTextureDesc.ArraySize = 1;
	renderTextureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	renderTextureDesc.SampleDesc.Count = 1;
	renderTextureDesc.Usage = D3D11_USAGE_DEFAULT;
	renderTextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

	HRESULT hr = m_device->CreateTexture2D(&renderTextureDesc, NULL, &renderTexture->m_texture);

	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("Failed for image file"));
	}

	hr = m_device->CreateShaderResourceView(renderTexture->m_texture, NULL, &renderTexture->m_shaderResourceView);

	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("Failed for image file"));
	}

	hr = m_device->CreateRenderTargetView(renderTexture->m_texture, NULL, &renderTexture->m_renderTargetView);

	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create render target view");
	}

	return renderTexture;
}

BitmapFont* Renderer::GetBitmapFont(const char* bitmapFontFilePathWithNoExtension)
{
	for (size_t i = 0; i < m_loadedFonts.size(); i++)
	{
		if (m_loadedFonts[i]->m_fontFilePathNameWithNoExtension == bitmapFontFilePathWithNoExtension)
		{
			return m_loadedFonts[i];
		}
	}
	return nullptr;
}

BitmapFont* Renderer::CreateBitmapFont(const char* bitmapFontFilePathWithNoExtension)
{
	Texture* fontText = CreateOrGetTextureFromFile(bitmapFontFilePathWithNoExtension);
	BitmapFont* newFont = new BitmapFont(bitmapFontFilePathWithNoExtension, *fontText);

	m_loadedFonts.emplace_back(newFont);
	return newFont;
}

Texture* Renderer::CreateOrGetTextureFromFile(char const* imageFilePath)
{
	// See if we already have this texture previously loaded
	Texture* existingTexture = GetTextureFromFile(imageFilePath);
	if (existingTexture)
	{
		return existingTexture;
	}

	// Never seen this texture before!  Let's load it.
	Texture* newTexture = CreateTextureFromFile(imageFilePath);
	return newTexture;
}

Texture* Renderer::CreateTextureFromFile(char const* imageFilePath)
{
	IntVec2 dimensions = IntVec2(0,0);		// This will be filled in for us to indicate image width & height
	int bytesPerTexel = 0; // This will be filled in for us to indicate how many color components the image had (e.g. 3=RGB=24bit, 4=RGBA=32bit)
	int numComponentsRequested = 0; // don't care; we support 3 (24-bit RGB) or 4 (32-bit RGBA)

									// Load (and decompress) the image RGB(A) bytes from a file on disk into a memory buffer (array of bytes)
	stbi_set_flip_vertically_on_load(1); // We prefer uvTexCoords has origin (0,0) at BOTTOM LEFT
	unsigned char* texelData = stbi_load(imageFilePath, &dimensions.x, &dimensions.y, &bytesPerTexel, numComponentsRequested);

	// Check if the load was successful
	GUARANTEE_OR_DIE(texelData, Stringf("Failed to load image \"%s\"", imageFilePath));

	//Texture* newTexture = CreateTextureFromData(imageFilePath, dimensions, bytesPerTexel, texelData);
	Texture* newTexture = CreateTextureFromImage(imageFilePath);

	// Free the raw image texel data now that we've sent a copy of it down to the GPU to be stored in video memory
	stbi_image_free(texelData);

	return newTexture;
}

Texture* Renderer::CreateTextureFromData(char const* name, IntVec2 dimensions, int bytesPerTexel, uint8_t* texelData)
{
	// Check if the load was successful
	GUARANTEE_OR_DIE(texelData, Stringf("CreateTextureFromData failed for \"%s\" - texelData was null!", name));
	GUARANTEE_OR_DIE(bytesPerTexel >= 3 && bytesPerTexel <= 4, Stringf("CreateTextureFromData failed for \"%s\" - unsupported BPP=%i (must be 3 or 4)", name, bytesPerTexel));
	GUARANTEE_OR_DIE(dimensions.x > 0 && dimensions.y > 0, Stringf("CreateTextureFromData failed for \"%s\" - illegal texture dimensions (%i x %i)", name, dimensions.x, dimensions.y));

	Texture* newTexture = new Texture();
	newTexture->m_name = name; // NOTE: m_name must be a std::string, otherwise it may point to temporary data!
	newTexture->m_dimensions = dimensions;

	m_loadedTextures.emplace_back(newTexture);
	return newTexture;
}

Texture* Renderer::GetTextureFromFile(char const* imageFilePath)
{
    for (int i = 0; i < m_loadedTextures.size(); i++)
    {
        if (m_loadedTextures[i]->m_name == imageFilePath)
        {
            return m_loadedTextures[i];
        }
    }
    return nullptr;
}

Texture* Renderer::CreateTextureFromImage(const Image& image)
{
	HRESULT hr;
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = image.GetDimensions().x;
	textureDesc.Height = image.GetDimensions().y;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_IMMUTABLE;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;

	D3D11_SUBRESOURCE_DATA textureData;
	textureData.pSysMem = image.GetRawData();
	textureData.SysMemPitch = 4 * image.GetDimensions().x;
	
	Texture* newTexture = new Texture();
	hr = m_device->CreateTexture2D(&textureDesc, &textureData, &newTexture->m_texture);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("CreateTextureFromImage failed for image file \"%s\".", image.GetImageFilePath().c_str()));
	}

	hr = m_device->CreateShaderResourceView(newTexture->m_texture, NULL, &newTexture->m_shaderResourceView);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("CreateShaderResourceView failed for image file \"%s\".", image.GetImageFilePath().c_str()));
	}

	newTexture->m_name = image.GetImageFilePath();
	newTexture->m_dimensions = image.GetDimensions();
	m_loadedTextures.emplace_back(newTexture);

	return newTexture;
}

void Renderer::BindTexture(unsigned int textureSlot, const Texture* texture)
{
	SetSamplerStatesIfChanged();

	if (texture == nullptr)
	{
		texture = m_defaultTexture;
	}

	if (texture != m_currentTexture)
	{
		m_currentTexture = texture;
		m_deviceContext->PSSetShaderResources(textureSlot, 1, &m_currentTexture->m_shaderResourceView);
	}

	if (texture == m_currentTexture)
	{
		m_deviceContext->PSSetShaderResources(textureSlot, 1, &m_currentTexture->m_shaderResourceView);
	}
}
 
Shader* Renderer::CreateShader(char const* shaderName, char const* shaderSource, VertexType type)
{
	std::vector<uint8_t> vertexShaderByteCode;
	std::vector<uint8_t> pixelShaderByteCode;

	HRESULT hr;

	// Create a new shader config
	ShaderConfig shaderConfig;
	shaderConfig.m_name = shaderName;
	shaderConfig.m_vertextEntryPoint = "VertexMain";
	shaderConfig.m_pixelEntryPoint = "PixelMain";

	// Create a new shader with the config 
	Shader* shader = new Shader(shaderConfig);

	// Compile vertex shader
	if (!CompileShaderToByteCode(vertexShaderByteCode, shader->GetName().c_str(), shaderSource, shader->m_config.m_vertextEntryPoint.c_str(), "vs_5_0"))
	{
		ERROR_AND_DIE(Stringf("Could not compile vertex shader."));
	}

	// Create vertex shader
	hr = m_device->CreateVertexShader(vertexShaderByteCode.data(), vertexShaderByteCode.size(), NULL, &shader->m_vertexShader);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("Could not create vertex shader."));
	}

	// Compile pixel shader
	if (!CompileShaderToByteCode(pixelShaderByteCode, shader->GetName().c_str(), shaderSource, shader->m_config.m_pixelEntryPoint.c_str(), "ps_5_0"))
	{
		ERROR_AND_DIE(Stringf("Could not compile pixel shader."));
	}

	// Create pixel shader
	hr = m_device->CreatePixelShader(pixelShaderByteCode.data(), pixelShaderByteCode.size(), NULL, &shader->m_pixelShader);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE(Stringf("Could not create pixel shader."));
	}

	if (type == VertexType::Vertex_PCU)
	{
		// Create input layout 
		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
		UINT numElements = ARRAYSIZE(inputElementDesc);

		hr = m_device->CreateInputLayout(inputElementDesc, numElements, vertexShaderByteCode.data(), vertexShaderByteCode.size(), &shader->m_vertexPCUInputLayout);
	
		if (!SUCCEEDED(hr))
		{
			ERROR_AND_DIE("Could not create input layout for vertex_pcu.");
		}
	}
	else if (type == VertexType::Vertex_PCUTBN)
	{
		// Create input layout 
		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};
		UINT numElements = ARRAYSIZE(inputElementDesc);

		hr = m_device->CreateInputLayout(inputElementDesc, numElements, vertexShaderByteCode.data(), vertexShaderByteCode.size(), &shader->m_vertexPCUTBNInputLayout);
	
		if (!SUCCEEDED(hr))
		{
			ERROR_AND_DIE("Could not create input layout for vertex_pcutbn.");
		}
	}

	// Add the shader to the cache 
	m_loadShaders.emplace_back(shader);

	// Return newly created shader 
	return shader;
}

Shader* Renderer::CreateShader(char const* shaderName, VertexType type)
{
	// Append the .hlsl extension to the shader name
	std::string fullShaderName = Stringf("%s.hlsl", shaderName);
	
	// Read the file as text 
	std::string shaderSource;
	FileUtils file;
	int bytesRead = file.FileReadToString(shaderSource, fullShaderName);

	if (bytesRead <= 0)
	{
		char c[MAX_PATH];
		GetCurrentDirectoryA(MAX_PATH, c);
		DebuggerPrintf("%s\n" , c);
		ERROR_AND_DIE("Could not read the shader file");
	}

	// Call Create Shader function
	//m_currentShader = CreateShader(fullShaderName.c_str(), shaderSource.c_str(), type);

	return CreateShader(fullShaderName.c_str(), shaderSource.c_str(), type);
}

Shader* Renderer::CreateOrGetShader(char const* shaderName, VertexType type /*= VertexType::Vertex_PCU*/)
{
	// See if we already have this texture previously loaded
	Shader* existingShader = GetShaderFromFile(shaderName);
	if (existingShader)
	{
		return existingShader;
	}

	// Never seen this texture before!  Let's load it.
	Shader* newShader = CreateShader(shaderName, type);
	return newShader;
}

Shader* Renderer::GetShaderFromFile(char const* imageFilePath)
{
	for (int i = 0; i < m_loadShaders.size(); i++)
	{
		if (m_loadShaders[i]->GetName() == imageFilePath)
		{
			return m_loadShaders[i];
		}
	}
	return nullptr;
}

bool Renderer::CompileShaderToByteCode(std::vector<unsigned char>& outByteCode, char const* name, char const* source, char const* entryPoint, char const* target)
{
	DWORD shaderFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
#if defined(ENGINE_DEBUG_RENDER)
	shaderFlags = D3DCOMPILE_DEBUG;
	shaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
	shaderFlags |= D3DCOMPILE_WARNINGS_ARE_ERRORS;
#endif
	ID3DBlob* shaderBlob = NULL;
	ID3DBlob* errorBlob = NULL;
	
	HRESULT hr;

	hr = D3DCompile(source, strlen(source), name, nullptr, nullptr, entryPoint, target, shaderFlags, 0, &shaderBlob, &errorBlob);

	// Check if the compile succeeded
	if (SUCCEEDED(hr))
	{
		// Copy the blob containing the shader byte code to the output byte code parameter
		outByteCode.resize(shaderBlob->GetBufferSize());
		memcpy(outByteCode.data(), shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize());
	}
	else
	{
		// If compilation failed, print error message
		if (errorBlob != NULL)
		{
			DebuggerPrintf((char*)errorBlob->GetBufferPointer());
		}
		// Release the error blob
		if (errorBlob != NULL)
		{
			errorBlob->Release();
			errorBlob = nullptr;
		}

		if (shaderBlob != NULL)
		{
			shaderBlob->Release();
			shaderBlob = nullptr;
		}
		ERROR_AND_DIE(Stringf("Could not compile shader: %s", name));
	}
	return true;
}

void Renderer::BindShader(Shader* shader)
{
// 	if (m_currentShader == shader)
// 	{
// 		return;
// 	}
	m_currentShader = (shader != nullptr) ? shader : m_defaultShader;
	m_deviceContext->VSSetShader(m_currentShader->m_vertexShader, nullptr, 0);
	m_deviceContext->PSSetShader(m_currentShader->m_pixelShader, nullptr, 0);
	if (m_currentShader->m_vertexPCUInputLayout == nullptr)
	{
		m_deviceContext->IASetInputLayout(m_currentShader->m_vertexPCUTBNInputLayout);
	}
	else
	{
		m_deviceContext->IASetInputLayout(m_currentShader->m_vertexPCUInputLayout);
	}
}

IndexBuffer* Renderer::CreateIndexBuffer(const size_t size)
{
	// Create index buffer
	HRESULT hr;
	D3D11_BUFFER_DESC bufferDesc = { 0 };
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = static_cast<UINT>(size);
	bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	IndexBuffer* indexBuffer = new IndexBuffer(size);

	hr = m_device->CreateBuffer(&bufferDesc, nullptr, &indexBuffer->m_buffer);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create index buffer.");
	}

	return indexBuffer;
}

void Renderer::CopyCPUToGPU(const void* data, size_t size, IndexBuffer*& ibo)
{
	// Check if existing immediate index buff is large enough
	if (ibo->m_size < size)
	{
		// Recreate the vertex buff 
		ibo->ReleaseD3D();
		ibo = CreateIndexBuffer(size);
	}

	// Copy vertices 
	D3D11_MAPPED_SUBRESOURCE resource;
	m_deviceContext->Map(ibo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	memcpy(resource.pData, data, size);
	m_deviceContext->Unmap(ibo->m_buffer, 0);
}

void Renderer::BindIndexBuffer(IndexBuffer* ibo)
{
	UINT startOffset = 0;
	m_deviceContext->IASetIndexBuffer(ibo->m_buffer, DXGI_FORMAT_R32_UINT, startOffset);
	//m_deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Renderer::DrawVertexBufferIndex(VertexBuffer* vbo, IndexBuffer* ibo, VertexType type, int indexCount)
{
 	BindVertexBuffer(vbo, type);
 	BindIndexBuffer(ibo);

	SetRasterizerStateIfChanged();
	SetBlendStatesIfChanged();
	SetDepthStatesIfChanged();

	// Draw
	m_deviceContext->DrawIndexed(indexCount, 0, 0);
}

VertexBuffer* Renderer::CreateVertexBuffer(const size_t size)
{
	// Create vertex buffer
	HRESULT hr;
	D3D11_BUFFER_DESC bufferDesc = { 0 };
	bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufferDesc.ByteWidth = static_cast<UINT>(size);
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	VertexBuffer* vertexBuffer = new VertexBuffer(size);

	hr = m_device->CreateBuffer(&bufferDesc, nullptr, &vertexBuffer->m_buffer);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create vertex buffer.");
	}

	return vertexBuffer;
}

ConstantBuffer* Renderer::CreateConstantBuffer(const size_t size)
{
	// Create constant buffer
	HRESULT hr;
	D3D11_BUFFER_DESC cameraBufferDesc = { 0 };
	cameraBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	cameraBufferDesc.ByteWidth = static_cast<UINT>(size);
	cameraBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cameraBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	ConstantBuffer* constantBuffer = new ConstantBuffer(size);

	hr = m_device->CreateBuffer(&cameraBufferDesc, nullptr, &constantBuffer->m_buffer);
	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Could not create constant buffer");
	}

	return constantBuffer;
}

void Renderer::CopyCPUToGPU(const void* data, size_t size, VertexBuffer*& vbo)
{
	// Check if existing immediate vertex buff is large enough
	if (vbo->m_size < size)
	{
		// Recreate the vertex buff 
		vbo->ReleaseD3D();
		vbo = CreateVertexBuffer(size);
	}

	// Copy vertices 
	D3D11_MAPPED_SUBRESOURCE resource;
	m_deviceContext->Map(vbo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);
	memcpy(resource.pData, data, size);
	m_deviceContext->Unmap(vbo->m_buffer, 0);
}

void Renderer::CopyCPUToGPU(const void* data, size_t size, ConstantBuffer* cbo)
{
	// Copy vertices 
	HRESULT hr;
	D3D11_MAPPED_SUBRESOURCE resource;
	hr = m_deviceContext->Map(cbo->m_buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &resource);

	if (!SUCCEEDED(hr))
	{
		ERROR_AND_DIE("Something aint working");
	}
	memcpy(resource.pData, data, size);
	m_deviceContext->Unmap(cbo->m_buffer, 0);
}

void Renderer::BindVertexBuffer(VertexBuffer* vbo, VertexType type)
{
	UINT stride;
	switch (type)
	{
	case VertexType::Vertex_PCU:
		stride = sizeof(Vertex_PCU);
		break;
	case VertexType::Vertex_PCUTBN:
		stride = sizeof(Vertex_PCUTBN);
		break;
	}

	UINT startOffset = 0;
	m_deviceContext->IASetVertexBuffers(0, 1, &vbo->m_buffer, &stride, &startOffset);
	m_deviceContext->IASetPrimitiveTopology(vbo->m_isLinePrimitive ? D3D11_PRIMITIVE_TOPOLOGY_LINELIST : D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void Renderer::BindConstantBuffer(int slot, ConstantBuffer* cbo)
{
	m_deviceContext->VSSetConstantBuffers(slot, 1, &cbo->m_buffer);
	m_deviceContext->PSSetConstantBuffers(slot, 1, &cbo->m_buffer);
}

void Renderer::DrawVertexBuffer(VertexBuffer* vbo, VertexType type, int vertexCount, int vertexOffset /*= 0*/)
{
	BindVertexBuffer(vbo, type);
	
	SetRasterizerStateIfChanged();
	SetBlendStatesIfChanged();
	SetDepthStatesIfChanged();

	// Draw
	m_deviceContext->Draw(vertexCount, vertexOffset);
}

void Renderer::DrawVertexArray(int numVertexes, Vertex_PCU const* vertexArray)
 {
	// Copy vertices
	CopyCPUToGPU(vertexArray, numVertexes * sizeof(Vertex_PCU), m_immediateVBO);
	
	// Draw vertex buffer
	DrawVertexBuffer(m_immediateVBO, VertexType::Vertex_PCU, numVertexes);
}

void Renderer::DrawVertexArray(int numVertexes, Vertex_PCUTBN const* vertexes)
{
	// Copy vertices
	CopyCPUToGPU(vertexes, numVertexes * sizeof(Vertex_PCUTBN), m_immediateVBO);

	// Draw vertex buffer
	DrawVertexBuffer(m_immediateVBO, VertexType::Vertex_PCUTBN, numVertexes);
}
