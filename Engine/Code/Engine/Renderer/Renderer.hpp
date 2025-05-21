#pragma once
#include "Engine/Core/Rgba8.hpp"
#include "Engine/Core/Vertex_PCU.hpp"
#include "Engine/Core/Vertex_PCUTBN.hpp"
#include "Engine/Renderer/Camera.hpp"
#include "Engine/Renderer/Texture.hpp"
#include "Engine/Renderer/BitmapFont.hpp"
#include "Game/EngineBuildPreferences.hpp"
#include "Engine/Renderer/ConstantBuffer.hpp"

#define DX_SAFE_RELEASE(dxObject)\
{								 \
if ((dxObject) != nullptr)		 \
{								 \
	(dxObject)->Release();		 \
	(dxObject) = nullptr;		 \
}								 \
}

#if defined(OPAQUE)
#undef OPAQUE
#endif

class Window; 
class Shader;
class IndexBuffer;
class VertexBuffer;
class Image;
struct BlurConstants;

extern Window* g_theWindow;

struct LightingDebug
{
	int RenderAmbient = true;
	int RenderDiffuse = true;
	int RenderSpecular = true;
	int RenderEmissive = true;
	int UseDiffuseMap = true;
	int UseNormalMap = true;
	int UseSpecularMap = true;
	int UseGlossinessMap = true;
	int UseEmissiveMap = true;
};

enum class VertexType
{
	Vertex_PCU,
	Vertex_PCUTBN,
	COUNT
};

enum class DepthMode
{
	DISABLED,
	ENABLED,
	COUNT
};

enum class RasterizerMode
{
	SOLID_CULL_NONE,
	SOLID_CULL_BACK,
	WIREFRAME_CULL_NONE,
	WIREFRAME_CULL_BACK,
	COUNT
};

enum class SamplerMode
{
	POINT_CLAMP,
	BILINEAR_WRAP,
	BILINEAR_CLAMP,
	COUNT
};

enum class BlendMode
{
	OPAQUE,
	ALPHA,
	ADDITIVE,
	COUNT
};

struct RenderConfig
{
	Window* m_window = nullptr;
};

class Renderer
{
public:
	Renderer(RenderConfig const& config);

	void* m_apiRenderingContext = nullptr;

	void Startup();
	void BeginFrame();
	void EndFrame();
	void Shutdown();

	void CreateImGui();
	void ImGuiBeginFrame();
	void ImGuiEndFrame();
	void ShutdownImGui();

	void CreateAndBindShader();
	void InitializeDXGIDebug();
	void InitializeD3D();
	void RenderFrame();
	void ReleaseD3D();

	void ClearScreen(Rgba8 const& clearColor);
	void BeginCamera(Camera camera); // camera parameter
	void EndCamera(Camera camera);
	void SetViewport(const Camera& camera);

	void CreateRasterizerState();
	void SetRasterizerState(RasterizerMode rasterizerMode);
	void SetRasterizerStateIfChanged();
	void ReleaseRasterizerState();

	void CreateBlendModes();
	void SetBlendMode(BlendMode blendMode);
	void SetBlendStatesIfChanged();
	void ReleaseBlendStates();

	void CreateSamplerModes();
	void SetSamplerMode(SamplerMode samplerMode);
	void SetSamplerStatesIfChanged();
	void ReleaseSamplerStates();

	void CreateDepthStencil();
	void SetDepthMode(DepthMode depthMode);
	void SetDepthStatesIfChanged();
	void ReleaseDepthStates();

	void SetLightConstants(const Vec3& sunDirection, const float& sunIntensity, const float& ambientIntensity, const Vec3& worldEyePosition, const float& minFallOff, const float& maxFallOff, const float& minFallOffMultiplier, const float& maxFallOffMultiplier, LightingDebug debugLight);
	void SetModelConstants(const Mat44& modelMatrix = Mat44(), const Rgba8& modelColor = Rgba8::WHITE);

	void CreateEmissiveBlur();
	void SetBlurConstants(BlurConstants blurConstants);
	void RenderEmissive();

	BitmapFont* CreateOrGetBitmapFont(const char* bitmapFontFilePathWithNoExtension);
	
	Texture* CreateRenderTexture(IntVec2 dimensions, char const* name);
	Texture* CreateOrGetTextureFromFile(char const* imageFilePath);
	Texture* CreateTextureFromData(char const* name, IntVec2 dimensions, int bytesPerTexel, uint8_t* texelData);
	Texture* GetTextureFromFile(char const* imageFilePath);
	Texture* CreateTextureFromImage(const Image& image);
	void BindTexture(unsigned int textureSlot, const Texture* texture);
	void DrawVertexArray(int numVertexes, Vertex_PCU const* vertexes);
	void DrawVertexArray(int numVertexes, Vertex_PCUTBN const* vertexes);
	//void DrawVertexArrayIndex(int numVertexes, Vertex_PCU const* vertexes, IndexBuffer const* ibo);

	Shader* CreateShader(char const* shaderName, VertexType type = VertexType::Vertex_PCU);
	Shader* CreateShader(char const* shaderName, char const* shaderSource, VertexType type = VertexType::Vertex_PCU);
	Shader* CreateOrGetShader(char const* shaderName, VertexType type = VertexType::Vertex_PCU);
	Shader* GetShaderFromFile(char const* imageFilePath);
	bool CompileShaderToByteCode(std::vector<unsigned char>& outByteCode, char const* name, char const* source, char const* entryPoint, char const* target);
	void BindShader(Shader* shader);

	IndexBuffer* CreateIndexBuffer(const size_t size);
	void CopyCPUToGPU(const void* data, size_t size, IndexBuffer*& ibo);
	void BindIndexBuffer(IndexBuffer* ibo);
	void DrawVertexBufferIndex(VertexBuffer* vbo, IndexBuffer* ibo, VertexType type, int indexCount);

	VertexBuffer* CreateVertexBuffer(const size_t size);
	void CopyCPUToGPU(const void* data, size_t size, VertexBuffer*& vbo);

	void BindVertexBuffer(VertexBuffer* vbo, VertexType type);
	void DrawVertexBuffer(VertexBuffer* vbo, VertexType type, int vertexCount, int vertexOffset = 0);

	ConstantBuffer* CreateConstantBuffer(const size_t size);
	void CopyCPUToGPU(const void* data, size_t size, ConstantBuffer* cbo);
	void BindConstantBuffer(int slot, ConstantBuffer* cbo);

public:
	std::vector<Texture*> m_loadedTextures;
	std::vector< BitmapFont* > m_loadedFonts;

	const Texture* m_defaultTexture = nullptr;
	const Texture* m_currentTexture = nullptr;

	const Texture* m_emissiveRenderTexture = nullptr;
	const Texture* m_emissiveBlurredTexture = nullptr;

	std::vector<Texture*> m_blurDownTextures;
	std::vector<Texture*> m_blurUpTextures;

	std::vector<IntVec2> m_blurTextureViewportSize;

private:
	BitmapFont* GetBitmapFont(const char* bitmapFontFilePathWithNoExtension);
	BitmapFont* CreateBitmapFont(const char* bitmapFontFilePathWithNoExtension);
	Texture* CreateTextureFromFile(char const* imageFilePath);

protected:
	struct ID3D11RenderTargetView* m_renderTargetView = nullptr;
	struct ID3D11Device* m_device = nullptr;
	struct ID3D11DeviceContext* m_deviceContext = nullptr;
	struct IDXGISwapChain* m_swapChain = nullptr;

	struct ID3D11RasterizerState* m_rasterizerState = nullptr;
	RasterizerMode m_desiredRasterizerMode = RasterizerMode::SOLID_CULL_BACK;
	ID3D11RasterizerState* m_rasterizerStates[(int)(RasterizerMode::COUNT)] = {};

	struct ID3D11BlendState* m_blendState = nullptr;
	BlendMode m_desiredBlendMode = BlendMode::COUNT;
	ID3D11BlendState* m_blendStates[(int)(BlendMode::COUNT)] = {};

	struct ID3D11SamplerState* m_samplerState = nullptr;
	SamplerMode m_desiredSamplerMode = SamplerMode::POINT_CLAMP;
	ID3D11SamplerState* m_samplerStates[(int)(SamplerMode::COUNT)] = {};

	struct ID3D11DepthStencilView* m_depthStencilView = nullptr;
	struct ID3D11DepthStencilState* m_depthState = nullptr;
	DepthMode m_desiredDepthMode = DepthMode::DISABLED;
	ID3D11DepthStencilState* m_depthStates[(int)(DepthMode::COUNT)] = {};
	struct ID3D11Texture2D* m_depthStencilTexture = nullptr;

protected:
	RenderConfig m_config;

	std::vector<Shader*> m_loadShaders;
	Shader* m_currentShader = nullptr;
	Shader* m_defaultShader = nullptr;
	Shader* m_blurDownShader = nullptr;
	Shader* m_blurUpShader = nullptr;
	Shader* m_compositeShader = nullptr;
	VertexBuffer* m_fullScreenQuadVBO = nullptr;
	VertexBuffer* m_immediateVBO = nullptr;
	ConstantBuffer* m_cameraCBO = nullptr;
	ConstantBuffer* m_modelCBO = nullptr;
	ConstantBuffer* m_lightCBO = nullptr;
	ConstantBuffer* m_blurCBO = nullptr;
};