#include "Engine/Renderer/DebugRenderer.hpp"
#include "Engine/Core/EngineCommon.hpp"
#include "Engine/Core/Clock.hpp"
#include "Engine/Math/MathUtils.hpp"
#include "Engine/Core/StringUtils.hpp"
#include "Engine/Core/VertexUtils.hpp"
#include "Engine/Renderer/Renderer.hpp"
#include "Engine/Core/Timer.hpp"
#include "Engine/Core/EventSystem.hpp"
#include <vector>
#include <mutex>

struct RenderScreenInformation
{
	std::vector<Vertex_PCU> m_verts;
	
	DebugRenderMode m_renderMode = DebugRenderMode::USE_DEPTH;
	RasterizerMode m_rasterizerMode = RasterizerMode::WIREFRAME_CULL_BACK;
	
	Mat44 m_matrix;
	Vec2 m_textPosition = Vec2::ZERO;
	Rgba8 m_startColor = Rgba8::WHITE;
	Rgba8 m_endColor = Rgba8::WHITE;

	Timer* m_timer = nullptr;
	float m_lifeSpan = 0.f;

	float m_numLinesVisible = 30.f; 
	const Texture* m_texture = nullptr;
};

struct RenderEntity
{
	std::vector<Vertex_PCU> m_verts;
	
	DebugRenderMode m_renderMode = DebugRenderMode::USE_DEPTH;
	RasterizerMode m_rasterizerMode = RasterizerMode::WIREFRAME_CULL_BACK;
	
	Mat44 m_matrix;
	
	Rgba8 m_startColor = Rgba8::WHITE;
	Rgba8 m_endColor = Rgba8::WHITE;

	Timer* m_timer = nullptr;
	float m_lifeSpan = 0.f;

	bool m_isBillboardText = false;
	const Texture* m_texture = nullptr;
};

class DebugRenderer
{
public:
	DebugRenderer();
	~DebugRenderer();

public:
	Clock* m_clock = nullptr;
	
	const Camera* m_worldCamera = nullptr;
	const Camera* m_screenCamera = nullptr;
	
	DebugRenderConfig m_config;

	std::vector<RenderEntity*> m_data;
	std::vector<RenderScreenInformation*> m_textInfo;
	std::vector<RenderScreenInformation*> m_messageInfo;

	bool m_isVisible = true;

	mutable std::mutex m_renderMutex;
};

DebugRenderer*	g_theDebugRenderer = nullptr;

DebugRenderer::DebugRenderer()
{

}

DebugRenderer::~DebugRenderer()
{

}

void DebugRenderSystemStartup(const DebugRenderConfig& config)
{
	g_theDebugRenderer = new DebugRenderer();
	g_theDebugRenderer->m_config = config;
	g_theDebugRenderer->m_clock = &Clock::GetSystemClock();
	SubscribeEventCallbackFunction("DebugRenderClear", Command_DebugRenderClear);
	SubscribeEventCallbackFunction("ToggleDebugRender", Command_DebugRenderToggle);
}

void DebugRenderSystemShutdown()
{
	delete g_theDebugRenderer;
	g_theDebugRenderer = nullptr;
}

void DebugRenderSetVisible()
{
	g_theDebugRenderer->m_isVisible = true;
}

void DebugRenderSetHidden()
{
	g_theDebugRenderer->m_isVisible = false;
}

void DebugRenderClear()
{
	std::lock_guard<std::mutex> lock(g_theDebugRenderer->m_renderMutex);
	g_theDebugRenderer->m_data.clear();
}

void DebugRenderBeginFrame()
{
	std::lock_guard<std::mutex> lock(g_theDebugRenderer->m_renderMutex);
	for (size_t i = 0; i < g_theDebugRenderer->m_data.size(); i++)
	{
		if (g_theDebugRenderer->m_data[i]->m_isBillboardText)
		{
			g_theDebugRenderer->m_data[i]->m_matrix = GetBillboardMatrix(BillBoardType::FULL_CAMERA_OPPOSING, g_theDebugRenderer->m_worldCamera->GetModelMatrix(), g_theDebugRenderer->m_data[i]->m_matrix.GetTranslation3D());
		}
	}
}

void DebugRenderWorld(const Camera& camera)
{
	g_theDebugRenderer->m_worldCamera = &camera;

	g_theDebugRenderer->m_config.m_renderer->BeginCamera(camera);

	g_theDebugRenderer->m_config.m_renderer->SetDepthMode(DepthMode::ENABLED);

	std::lock_guard<std::mutex> lock(g_theDebugRenderer->m_renderMutex);

	if (g_theDebugRenderer->m_isVisible)
	{
		for (size_t i = 0; i < g_theDebugRenderer->m_data.size(); i++)
		{
			g_theDebugRenderer->m_config.m_renderer->BindTexture(0, g_theDebugRenderer->m_data[i]->m_texture);

			Rgba8 color;

			if (g_theDebugRenderer->m_data[i]->m_lifeSpan == -1.f || g_theDebugRenderer->m_data[i]->m_lifeSpan == 0)
			{
				color = g_theDebugRenderer->m_data[i]->m_endColor;
			}
			else
			{
				color = Interpolate(g_theDebugRenderer->m_data[i]->m_startColor, g_theDebugRenderer->m_data[i]->m_endColor, g_theDebugRenderer->m_data[i]->m_timer->GetElapsedFraction());
			}

			if (g_theDebugRenderer->m_data[i]->m_renderMode == DebugRenderMode::ALWAYS)
			{
				g_theDebugRenderer->m_config.m_renderer->SetBlendMode(BlendMode::ALPHA);
				g_theDebugRenderer->m_config.m_renderer->SetDepthMode(DepthMode::DISABLED);
				g_theDebugRenderer->m_config.m_renderer->SetRasterizerState(g_theDebugRenderer->m_data[i]->m_rasterizerMode);
				g_theDebugRenderer->m_config.m_renderer->SetModelConstants(g_theDebugRenderer->m_data[i]->m_matrix, color);
				g_theDebugRenderer->m_config.m_renderer->BindShader(nullptr);
				g_theDebugRenderer->m_config.m_renderer->DrawVertexArray(static_cast<int>(g_theDebugRenderer->m_data[i]->m_verts.size()), g_theDebugRenderer->m_data[i]->m_verts.data());
			}
			else if (g_theDebugRenderer->m_data[i]->m_renderMode == DebugRenderMode::USE_DEPTH)
			{
				g_theDebugRenderer->m_config.m_renderer->SetBlendMode(BlendMode::OPAQUE);
				g_theDebugRenderer->m_config.m_renderer->SetDepthMode(DepthMode::ENABLED);
				g_theDebugRenderer->m_config.m_renderer->SetRasterizerState(g_theDebugRenderer->m_data[i]->m_rasterizerMode);
				g_theDebugRenderer->m_config.m_renderer->SetModelConstants(g_theDebugRenderer->m_data[i]->m_matrix, color);
				g_theDebugRenderer->m_config.m_renderer->BindShader(nullptr);
				g_theDebugRenderer->m_config.m_renderer->DrawVertexArray(static_cast<int>(g_theDebugRenderer->m_data[i]->m_verts.size()), g_theDebugRenderer->m_data[i]->m_verts.data());
			}
		}

		for (size_t i = 0; i < g_theDebugRenderer->m_data.size(); i++)
		{
			g_theDebugRenderer->m_config.m_renderer->SetRasterizerState(g_theDebugRenderer->m_data[i]->m_rasterizerMode);

			Rgba8 color;

			if (g_theDebugRenderer->m_data[i]->m_renderMode == DebugRenderMode::X_RAY)
			{
				// First pass
				g_theDebugRenderer->m_config.m_renderer->SetBlendMode(BlendMode::ALPHA);
				g_theDebugRenderer->m_config.m_renderer->SetDepthMode(DepthMode::DISABLED);

				uint8_t originalR = color.r; // store the original color in a local variable
				uint8_t originalG = color.g; // store the original color in a local variable
				uint8_t originalB = color.b; // store the original color in a local variable
				uint8_t originalA = color.a; // store the original color in a local variable

				color.r = static_cast<uint8_t>(color.r + (255 - color.r) * 0.5f); // Make the first pass color light 
				color.g = static_cast<uint8_t>(color.g + (255 - color.g) * 0.5f); // Make the first pass color light 
				color.b = static_cast<uint8_t>(color.b + (255 - color.b) * 0.5f); // Make the first pass color light 
				color.a = 120; // Make the first pass color transparent 

				g_theDebugRenderer->m_config.m_renderer->SetRasterizerState(g_theDebugRenderer->m_data[i]->m_rasterizerMode);
				g_theDebugRenderer->m_config.m_renderer->SetModelConstants(g_theDebugRenderer->m_data[i]->m_matrix, color);
				g_theDebugRenderer->m_config.m_renderer->BindShader(nullptr);
				g_theDebugRenderer->m_config.m_renderer->DrawVertexArray(static_cast<int>(g_theDebugRenderer->m_data[i]->m_verts.size()), g_theDebugRenderer->m_data[i]->m_verts.data());

				// Second pass 
				g_theDebugRenderer->m_config.m_renderer->SetBlendMode(BlendMode::OPAQUE);
				g_theDebugRenderer->m_config.m_renderer->SetDepthMode(DepthMode::ENABLED);

				color.r = originalR; // use original R to draw the original color
				color.g = originalG; // use original G to draw the original color
				color.b = originalB; // use original B to draw the original color
				color.a = originalA; // use original A to draw the original color

				g_theDebugRenderer->m_config.m_renderer->SetRasterizerState(g_theDebugRenderer->m_data[i]->m_rasterizerMode);
				g_theDebugRenderer->m_config.m_renderer->SetModelConstants(g_theDebugRenderer->m_data[i]->m_matrix, color);
				g_theDebugRenderer->m_config.m_renderer->BindShader(nullptr);
				g_theDebugRenderer->m_config.m_renderer->DrawVertexArray(static_cast<int>(g_theDebugRenderer->m_data[i]->m_verts.size()), g_theDebugRenderer->m_data[i]->m_verts.data());
			}
		}
	}

	g_theDebugRenderer->m_config.m_renderer->EndCamera(camera);
}

void DebugRenderScreen(const Camera& camera)
{
	g_theDebugRenderer->m_screenCamera = &camera;

	g_theDebugRenderer->m_config.m_renderer->BeginCamera(camera);

	std::lock_guard<std::mutex> lock(g_theDebugRenderer->m_renderMutex);

	if (g_theDebugRenderer->m_isVisible)
	{
		for (size_t i = 0; i < g_theDebugRenderer->m_textInfo.size(); i++)
		{
			g_theDebugRenderer->m_config.m_renderer->BindTexture(0, g_theDebugRenderer->m_textInfo[i]->m_texture);

			Rgba8 color;

			if (g_theDebugRenderer->m_textInfo[i]->m_lifeSpan == -1.f || g_theDebugRenderer->m_textInfo[i]->m_lifeSpan == 0)
			{
				color = g_theDebugRenderer->m_textInfo[i]->m_startColor;
			}
			else
			{
				color = Interpolate(g_theDebugRenderer->m_textInfo[i]->m_startColor, g_theDebugRenderer->m_textInfo[i]->m_endColor, g_theDebugRenderer->m_textInfo[i]->m_timer->GetElapsedFraction());
			}

			g_theDebugRenderer->m_textInfo[i]->m_matrix.SetTranslation2D(g_theDebugRenderer->m_textInfo[i]->m_textPosition);

			g_theDebugRenderer->m_config.m_renderer->SetBlendMode(BlendMode::ALPHA);
			g_theDebugRenderer->m_config.m_renderer->SetDepthMode(DepthMode::DISABLED);
			g_theDebugRenderer->m_config.m_renderer->SetRasterizerState(g_theDebugRenderer->m_textInfo[i]->m_rasterizerMode);
			g_theDebugRenderer->m_config.m_renderer->SetModelConstants(g_theDebugRenderer->m_textInfo[i]->m_matrix, color);
			g_theDebugRenderer->m_config.m_renderer->BindShader(nullptr);
			g_theDebugRenderer->m_config.m_renderer->DrawVertexArray(static_cast<int>(g_theDebugRenderer->m_textInfo[i]->m_verts.size()), g_theDebugRenderer->m_textInfo[i]->m_verts.data());
		}

		float xPos = g_theDebugRenderer->m_screenCamera->GetOrthographicTopRight().x - 1600.f;
		float startY = g_theDebugRenderer->m_screenCamera->GetOrthographicTopRight().y - 25.f;
		float lineHeight = 20.f;

		for (size_t i = 0; i < g_theDebugRenderer->m_messageInfo.size(); i++)
		{
			g_theDebugRenderer->m_config.m_renderer->BindTexture(0, g_theDebugRenderer->m_messageInfo[i]->m_texture);

			Rgba8 color;

			if (g_theDebugRenderer->m_messageInfo[i]->m_lifeSpan == -1.f || g_theDebugRenderer->m_messageInfo[i]->m_lifeSpan == 0)
			{
				color = g_theDebugRenderer->m_messageInfo[i]->m_startColor;
			}
			else
			{
				color = Interpolate(g_theDebugRenderer->m_messageInfo[i]->m_startColor, g_theDebugRenderer->m_messageInfo[i]->m_endColor, g_theDebugRenderer->m_messageInfo[i]->m_timer->GetElapsedFraction());
			}

			float yPos = startY - (lineHeight * i);
			g_theDebugRenderer->m_messageInfo[i]->m_matrix.SetTranslation2D(Vec2(xPos, yPos));

			g_theDebugRenderer->m_config.m_renderer->SetBlendMode(BlendMode::ALPHA);
			g_theDebugRenderer->m_config.m_renderer->SetDepthMode(DepthMode::DISABLED);
			g_theDebugRenderer->m_config.m_renderer->SetRasterizerState(g_theDebugRenderer->m_messageInfo[i]->m_rasterizerMode);
			g_theDebugRenderer->m_config.m_renderer->SetModelConstants(g_theDebugRenderer->m_messageInfo[i]->m_matrix, color);
			g_theDebugRenderer->m_config.m_renderer->BindShader(nullptr);
			g_theDebugRenderer->m_config.m_renderer->DrawVertexArray(static_cast<int>(g_theDebugRenderer->m_messageInfo[i]->m_verts.size()), g_theDebugRenderer->m_messageInfo[i]->m_verts.data());
		}
	}

	g_theDebugRenderer->m_config.m_renderer->EndCamera(camera);
}

void DebugRenderEndFrame()
{
	std::lock_guard<std::mutex> lock(g_theDebugRenderer->m_renderMutex);
	for (size_t i = 0; i < g_theDebugRenderer->m_data.size(); i++)
	{
		if (g_theDebugRenderer->m_data[i]->m_lifeSpan != -1.f)
		{
			if (g_theDebugRenderer->m_data[i]->m_timer->HasPeriodElapsed() || g_theDebugRenderer->m_data[i]->m_lifeSpan == 0.f)
			{
				g_theDebugRenderer->m_data.erase(g_theDebugRenderer->m_data.begin() + i);
				i--;
			}
		}
	}

	for (size_t i = 0; i < g_theDebugRenderer->m_textInfo.size(); i++)
	{
		if (g_theDebugRenderer->m_textInfo[i]->m_lifeSpan != -1.f)
		{
			if (g_theDebugRenderer->m_textInfo[i]->m_timer->HasPeriodElapsed() || g_theDebugRenderer->m_textInfo[i]->m_lifeSpan == 0.f)
			{
				g_theDebugRenderer->m_textInfo.erase(g_theDebugRenderer->m_textInfo.begin() + i);
				i--;
			}
		}
	}

	for (size_t i = 0; i < g_theDebugRenderer->m_messageInfo.size(); i++)
	{
		if (g_theDebugRenderer->m_messageInfo[i]->m_lifeSpan != -1.f)
		{
			if (g_theDebugRenderer->m_messageInfo[i]->m_timer->HasPeriodElapsed() || g_theDebugRenderer->m_messageInfo[i]->m_lifeSpan == 0.f)
			{
				g_theDebugRenderer->m_messageInfo.erase(g_theDebugRenderer->m_messageInfo.begin() + i);
				i--;
			}
		}
	}
}

void DebugAddWorld3DTriangle(const Vec3& position1, const Vec3& position2, const Vec3& position3, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	std::lock_guard<std::mutex> lock(g_theDebugRenderer->m_renderMutex);

	RenderEntity* triangleProp = new RenderEntity();

	triangleProp->m_lifeSpan = duration;
	triangleProp->m_startColor = startColor;
	triangleProp->m_endColor = endColor;
	triangleProp->m_renderMode = mode;

	if (duration != -1.f)
	{
		triangleProp->m_timer = new Timer(duration, g_theDebugRenderer->m_clock);
		triangleProp->m_timer->Start();
	}

	Rgba8 color;
	if (triangleProp->m_lifeSpan == -1.f || triangleProp->m_lifeSpan == 0.f)
	{
		color = triangleProp->m_startColor;
	}
	else
	{
		color = Interpolate(triangleProp->m_startColor, triangleProp->m_endColor, triangleProp->m_timer->GetElapsedFraction());
	}

	AddVertsFor3DTriangle(triangleProp->m_verts, position1, position2, position3, color);

	triangleProp->m_rasterizerMode = RasterizerMode::SOLID_CULL_BACK;
	g_theDebugRenderer->m_data.emplace_back(triangleProp);
}

void DebugAddWorld3DWireTriangle(const Vec3& position1, const Vec3& position2, const Vec3& position3, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	std::lock_guard<std::mutex> lock(g_theDebugRenderer->m_renderMutex);

	RenderEntity* triangleProp = new RenderEntity();

	triangleProp->m_lifeSpan = duration;
	triangleProp->m_startColor = startColor;
	triangleProp->m_endColor = endColor;
	triangleProp->m_renderMode = mode;

	if (duration != -1.f)
	{
		triangleProp->m_timer = new Timer(duration, g_theDebugRenderer->m_clock);
		triangleProp->m_timer->Start();
	}

	Rgba8 color;
	if (triangleProp->m_lifeSpan == -1.f || triangleProp->m_lifeSpan == 0.f)
	{
		color = triangleProp->m_startColor;
	}
	else
	{
		color = Interpolate(triangleProp->m_startColor, triangleProp->m_endColor, triangleProp->m_timer->GetElapsedFraction());
	}

	AddVertsFor3DTriangle(triangleProp->m_verts, position1, position2, position3, color);

	triangleProp->m_rasterizerMode = RasterizerMode::WIREFRAME_CULL_BACK;
	g_theDebugRenderer->m_data.emplace_back(triangleProp);
}

void DebugAddWorld3DTriangle(const Mat44& transform, const Vec3& direction, float zPosition, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	std::lock_guard<std::mutex> lock(g_theDebugRenderer->m_renderMutex);

	RenderEntity* triangleProp = new RenderEntity();

	triangleProp->m_lifeSpan = duration;
	triangleProp->m_startColor = startColor;
	triangleProp->m_endColor = endColor;
	triangleProp->m_renderMode = mode;

	if (duration != -1.f)
	{
		triangleProp->m_timer = new Timer(duration, g_theDebugRenderer->m_clock);
		triangleProp->m_timer->Start();
	}

	Rgba8 color;
	if (triangleProp->m_lifeSpan == -1.f || triangleProp->m_lifeSpan == 0.f)
	{
		color = triangleProp->m_startColor;
	}
	else
	{
		color = Interpolate(triangleProp->m_startColor, triangleProp->m_endColor, triangleProp->m_timer->GetElapsedFraction());
	}

	Vec3 startPos = transform.TransformPosition3D(Vec3(0.f, 0.f, zPosition));
	AddVertsFor3DTriangle(triangleProp->m_verts, startPos, -direction, radius, color);

	triangleProp->m_rasterizerMode = RasterizerMode::SOLID_CULL_BACK;
	g_theDebugRenderer->m_data.emplace_back(triangleProp);
}

void DebugAddWorld3DWireTriangle(const Mat44& transform, const Vec3& direction, float zPosition, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	std::lock_guard<std::mutex> lock(g_theDebugRenderer->m_renderMutex);

	RenderEntity* triangleProp = new RenderEntity();

	triangleProp->m_lifeSpan = duration;
	triangleProp->m_startColor = startColor;
	triangleProp->m_endColor = endColor;
	triangleProp->m_renderMode = mode;

	if (duration != -1.f)
	{
		triangleProp->m_timer = new Timer(duration, g_theDebugRenderer->m_clock);
		triangleProp->m_timer->Start();
	}

	Rgba8 color;
	if (triangleProp->m_lifeSpan == -1.f || triangleProp->m_lifeSpan == 0.f)
	{
		color = triangleProp->m_startColor;
	}
	else
	{
		color = Interpolate(triangleProp->m_startColor, triangleProp->m_endColor, triangleProp->m_timer->GetElapsedFraction());
	}

	Vec3 startPos = transform.TransformPosition3D(Vec3(0.f, 0.f, zPosition));
	AddVertsFor3DTriangle(triangleProp->m_verts, startPos, -direction, radius, color);

	triangleProp->m_rasterizerMode = RasterizerMode::WIREFRAME_CULL_BACK;
	g_theDebugRenderer->m_data.emplace_back(triangleProp);
}

void DebugAddWorld2DRing(const Vec2& center, float radius, int sides, float thickness, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	std::lock_guard<std::mutex> lock(g_theDebugRenderer->m_renderMutex);

	RenderEntity* ringProp = new RenderEntity();

	ringProp->m_lifeSpan = duration;
	ringProp->m_startColor = startColor;
	ringProp->m_endColor = endColor;
	ringProp->m_renderMode = mode;

	if (duration != -1.f)
	{
		ringProp->m_timer = new Timer(duration, g_theDebugRenderer->m_clock);
		ringProp->m_timer->Start();
	}

	Rgba8 color;
	if (ringProp->m_lifeSpan == -1.f || ringProp->m_lifeSpan == 0.f)
	{
		color = ringProp->m_startColor;
	}
	else
	{
		color = Interpolate(ringProp->m_startColor, ringProp->m_endColor, ringProp->m_timer->GetElapsedFraction());
	}

	AddVertsForRing2D(ringProp->m_verts, center, radius, thickness, sides, color);

	ringProp->m_rasterizerMode = RasterizerMode::SOLID_CULL_BACK;
	g_theDebugRenderer->m_data.emplace_back(ringProp);
}

void DebugAddWorld3DRing(const Vec3& center, float radius, int sides, float thickness, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	std::lock_guard<std::mutex> lock(g_theDebugRenderer->m_renderMutex);

	RenderEntity* ringProp = new RenderEntity();

	ringProp->m_lifeSpan = duration;
	ringProp->m_startColor = startColor;
	ringProp->m_endColor = endColor;
	ringProp->m_renderMode = mode;

	if (duration != -1.f)
	{
		ringProp->m_timer = new Timer(duration, g_theDebugRenderer->m_clock);
		ringProp->m_timer->Start();
	}

	Rgba8 color;
	if (ringProp->m_lifeSpan == -1.f || ringProp->m_lifeSpan == 0.f)
	{
		color = ringProp->m_startColor;
	}
	else
	{
		color = Interpolate(ringProp->m_startColor, ringProp->m_endColor, ringProp->m_timer->GetElapsedFraction());
	}

	AddVertsFor3DRing(ringProp->m_verts, center, radius, sides, thickness, color);

	ringProp->m_rasterizerMode = RasterizerMode::SOLID_CULL_BACK;
	g_theDebugRenderer->m_data.emplace_back(ringProp);
}

void DebugAddWorldQuad(float duration, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	std::lock_guard<std::mutex> lock(g_theDebugRenderer->m_renderMutex);

	RenderEntity* quad = new RenderEntity();

	quad->m_lifeSpan = duration;
	quad->m_startColor = startColor;
	quad->m_endColor = endColor;
	quad->m_renderMode = mode;

	if (duration != -1.f)
	{
		quad->m_timer = new Timer(duration, g_theDebugRenderer->m_clock);
		quad->m_timer->Start();
	}

	Rgba8 color;
	if (quad->m_lifeSpan == -1.f || quad->m_lifeSpan == 0.f)
	{
		color = quad->m_startColor;
	}
	else
	{
		color = Interpolate(quad->m_startColor, quad->m_endColor, quad->m_timer->GetElapsedFraction());
	}

	AddVertsForQuad3D(quad->m_verts, bottomLeft, bottomRight, topRight, topLeft, color);

	quad->m_rasterizerMode = RasterizerMode::SOLID_CULL_BACK;
	g_theDebugRenderer->m_data.emplace_back(quad);
}

void DebugAddWorldLineSegmentedQuad(float duration, float lineThickness, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	std::lock_guard<std::mutex> lock(g_theDebugRenderer->m_renderMutex);

	RenderEntity* quad = new RenderEntity();

	quad->m_lifeSpan = duration;
	quad->m_startColor = startColor;
	quad->m_endColor = endColor;
	quad->m_renderMode = mode;

	if (duration != -1.f)
	{
		quad->m_timer = new Timer(duration, g_theDebugRenderer->m_clock);
		quad->m_timer->Start();
	}

	Rgba8 color;
	if (quad->m_lifeSpan == -1.f || quad->m_lifeSpan == 0.f)
	{
		color = quad->m_startColor;
	}
	else
	{
		color = Interpolate(quad->m_startColor, quad->m_endColor, quad->m_timer->GetElapsedFraction());
	}

	AddVertsForQuad3D(quad->m_verts, bottomLeft, bottomRight, topRight, topLeft, lineThickness, color);

	quad->m_rasterizerMode = RasterizerMode::SOLID_CULL_BACK;
	g_theDebugRenderer->m_data.emplace_back(quad);
}

void DebugAddWorldWireQuad(float duration, const Vec3& bottomLeft, const Vec3& bottomRight, const Vec3& topRight, const Vec3& topLeft, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	std::lock_guard<std::mutex> lock(g_theDebugRenderer->m_renderMutex);

	RenderEntity* quad = new RenderEntity();

	quad->m_lifeSpan = duration;
	quad->m_startColor = startColor;
	quad->m_endColor = endColor;
	quad->m_renderMode = mode;

	if (duration != -1.f)
	{
		quad->m_timer = new Timer(duration, g_theDebugRenderer->m_clock);
		quad->m_timer->Start();
	}

	Rgba8 color;
	if (quad->m_lifeSpan == -1.f || quad->m_lifeSpan == 0.f)
	{
		color = quad->m_startColor;
	}
	else
	{
		color = Interpolate(quad->m_startColor, quad->m_endColor, quad->m_timer->GetElapsedFraction());
	}

	AddVertsForQuad3D(quad->m_verts, bottomLeft, bottomRight, topRight, topLeft, color);

	quad->m_rasterizerMode = RasterizerMode::WIREFRAME_CULL_BACK;
	g_theDebugRenderer->m_data.emplace_back(quad);
}

void DebugAddWorldWireAABB3D(float duration, const AABB3& bounds, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	std::lock_guard<std::mutex> lock(g_theDebugRenderer->m_renderMutex);

	RenderEntity* aabb3d = new RenderEntity();

	aabb3d->m_lifeSpan = duration;
	aabb3d->m_startColor = startColor;
	aabb3d->m_endColor = endColor;
	aabb3d->m_renderMode = mode;

	if (duration != -1.f)
	{
		aabb3d->m_timer = new Timer(duration, g_theDebugRenderer->m_clock);
		aabb3d->m_timer->Start();
	}

	Rgba8 color;
	if (aabb3d->m_lifeSpan == -1.f || aabb3d->m_lifeSpan == 0.f)
	{
		color = aabb3d->m_startColor;
	}
	else
	{
		color = Interpolate(aabb3d->m_startColor, aabb3d->m_endColor, aabb3d->m_timer->GetElapsedFraction());
	}

	AddVertsForAABB3D(aabb3d->m_verts, bounds, color);

	aabb3d->m_rasterizerMode = RasterizerMode::WIREFRAME_CULL_BACK;
	g_theDebugRenderer->m_data.emplace_back(aabb3d);
}

void DebugAddWorldPoint(const Vec3& pos, float radius, int numSlices, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	std::lock_guard<std::mutex> lock(g_theDebugRenderer->m_renderMutex);

	RenderEntity* pointProp = new RenderEntity();
	
	pointProp->m_lifeSpan = duration;
	pointProp->m_startColor = startColor;
	pointProp->m_endColor = endColor;
	pointProp->m_renderMode = mode;

	if (duration != -1.f)
	{
		pointProp->m_timer = new Timer(duration, g_theDebugRenderer->m_clock);
		pointProp->m_timer->Start();
	}

	Rgba8 color;
	if (pointProp->m_lifeSpan == -1.f || pointProp->m_lifeSpan == 0.f)
	{
		color = pointProp->m_startColor;
	}
	else
	{
		color = Interpolate(pointProp->m_startColor, pointProp->m_endColor, pointProp->m_timer->GetElapsedFraction());
	}

	AddVertsForZSphere(pointProp->m_verts, pos, radius, color, AABB2::ZERO_TO_ONE, numSlices);

	pointProp->m_rasterizerMode = RasterizerMode::SOLID_CULL_BACK;
	g_theDebugRenderer->m_data.emplace_back(pointProp);
}

void DebugAddWorldLine(const Vec3& start, const Vec3& end, float lineThickness, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	std::lock_guard<std::mutex> lock(g_theDebugRenderer->m_renderMutex);

	RenderEntity* lineProp = new RenderEntity();

	lineProp->m_lifeSpan = duration;
	lineProp->m_startColor = startColor;
	lineProp->m_endColor = endColor;
	lineProp->m_renderMode = mode;

	if (duration != -1.f)
	{
		lineProp->m_timer = new Timer(duration, g_theDebugRenderer->m_clock);
		lineProp->m_timer->Start();
	}

	Rgba8 color;
	if (lineProp->m_lifeSpan == -1.f || lineProp->m_lifeSpan == 0.f)
	{
		color = lineProp->m_startColor;
	}
	else
	{
		color = Interpolate(lineProp->m_startColor, lineProp->m_endColor, lineProp->m_timer->GetElapsedFraction());
	}

	AddVertsForLine3D(lineProp->m_verts, start, end, lineThickness, color);

	lineProp->m_rasterizerMode = RasterizerMode::SOLID_CULL_BACK;
	g_theDebugRenderer->m_data.emplace_back(lineProp);
}

void DebugAddWorldLine(const Mat44& transform, float lineLength, float radius, int slices, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	std::lock_guard<std::mutex> lock(g_theDebugRenderer->m_renderMutex);

	RenderEntity* lineProp = new RenderEntity();

	lineProp->m_lifeSpan = duration;
	lineProp->m_startColor = startColor;
	lineProp->m_endColor = endColor;
	lineProp->m_renderMode = mode;

	if (duration != -1.f)
	{
		lineProp->m_timer = new Timer(duration, g_theDebugRenderer->m_clock);
		lineProp->m_timer->Start();
	}

	Rgba8 color;
	if (lineProp->m_lifeSpan == -1.f || lineProp->m_lifeSpan == 0.f)
	{
		color = lineProp->m_startColor;
	}
	else
	{
		color = Interpolate(lineProp->m_startColor, lineProp->m_endColor, lineProp->m_timer->GetElapsedFraction());
	}

	Vec3 start = transform.TransformPosition3D(Vec3::ZERO);
	Vec3 end = transform.TransformPosition3D(Vec3(lineLength, 0.f, 0.f));

	AddVertsForZCylinder3D(lineProp->m_verts, start, end, radius, slices, color);

	lineProp->m_rasterizerMode = RasterizerMode::SOLID_CULL_BACK;
	g_theDebugRenderer->m_data.emplace_back(lineProp);
}

void DebugAddWorldWireCylinder(const Vec3& base, const Vec3& top, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	std::lock_guard<std::mutex> lock(g_theDebugRenderer->m_renderMutex);

	RenderEntity* cylinderProp = new RenderEntity();
	
	const int numSlices = 8;
;
	cylinderProp->m_lifeSpan = duration;
	cylinderProp->m_startColor = startColor;
	cylinderProp->m_endColor = endColor;
	cylinderProp->m_renderMode = mode;

	if (duration != -1.f)
	{
		cylinderProp->m_timer = new Timer(duration, g_theDebugRenderer->m_clock);
		cylinderProp->m_timer->Start();
	}

	Rgba8 color;
	if (cylinderProp->m_lifeSpan == -1.f || cylinderProp->m_lifeSpan == 0.f)
	{
		color = cylinderProp->m_startColor;
	}
	else
	{
		color = Interpolate(cylinderProp->m_startColor, cylinderProp->m_endColor, cylinderProp->m_timer->GetElapsedFraction());
	}
	
	AddVertsForZCylinder3D(cylinderProp->m_verts, top, base, radius, numSlices, color);

	cylinderProp->m_rasterizerMode = RasterizerMode::WIREFRAME_CULL_BACK;
	g_theDebugRenderer->m_data.emplace_back(cylinderProp);
}

void DebugAddWorld2DDisc(const Vec2& center, float radius, int sides, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	std::lock_guard<std::mutex> lock(g_theDebugRenderer->m_renderMutex);

	RenderEntity* discProp = new RenderEntity();

	discProp->m_lifeSpan = duration;
	discProp->m_startColor = startColor;
	discProp->m_endColor = endColor;
	discProp->m_renderMode = mode;

	if (duration != -1.f)
	{
		discProp->m_timer = new Timer(duration, g_theDebugRenderer->m_clock);
		discProp->m_timer->Start();
	}

	Rgba8 color;
	if (discProp->m_lifeSpan == -1.f || discProp->m_lifeSpan == 0.f)
	{
		color = discProp->m_startColor;
	}
	else
	{
		color = Interpolate(discProp->m_startColor, discProp->m_endColor, discProp->m_timer->GetElapsedFraction());
	}

	AddVertsForDisc2D(discProp->m_verts, center, radius, sides, color);

	discProp->m_rasterizerMode = RasterizerMode::SOLID_CULL_BACK;
	g_theDebugRenderer->m_data.emplace_back(discProp);
}

void DebugAddWorld3DDisc(const Vec3& center, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	std::lock_guard<std::mutex> lock(g_theDebugRenderer->m_renderMutex);

	RenderEntity* discProp = new RenderEntity();

	discProp->m_lifeSpan = duration;
	discProp->m_startColor = startColor;
	discProp->m_endColor = endColor;
	discProp->m_renderMode = mode;

	if (duration != -1.f)
	{
		discProp->m_timer = new Timer(duration, g_theDebugRenderer->m_clock);
		discProp->m_timer->Start();
	}

	Rgba8 color;
	if (discProp->m_lifeSpan == -1.f || discProp->m_lifeSpan == 0.f)
	{
		color = discProp->m_startColor;
	}
	else
	{
		color = Interpolate(discProp->m_startColor, discProp->m_endColor, discProp->m_timer->GetElapsedFraction());
	}

	AddVertsForDisc3D(discProp->m_verts, center, radius, color);

	discProp->m_rasterizerMode = RasterizerMode::SOLID_CULL_BACK;
	g_theDebugRenderer->m_data.emplace_back(discProp);
}

void DebugAddWorldWireSphere(const Vec3& center, float radius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	std::lock_guard<std::mutex> lock(g_theDebugRenderer->m_renderMutex);

	RenderEntity* wireSphereProp = new RenderEntity();
	
	const int numSlices = 8;

	wireSphereProp->m_lifeSpan = duration;
	wireSphereProp->m_startColor = startColor;
	wireSphereProp->m_endColor = endColor;
	wireSphereProp->m_renderMode = mode;

	if (duration != -1.f)
	{
		wireSphereProp->m_timer = new Timer(duration, g_theDebugRenderer->m_clock);
		wireSphereProp->m_timer->Start();
	}

	Rgba8 color;
	if (wireSphereProp->m_lifeSpan == -1.f || wireSphereProp->m_lifeSpan == 0.f)
	{
		color = wireSphereProp->m_startColor;
	}
	else
	{
		color = Interpolate(wireSphereProp->m_startColor, wireSphereProp->m_endColor, wireSphereProp->m_timer->GetElapsedFraction());
	}

	AddVertsForZSphere(wireSphereProp->m_verts, center, radius, color, AABB2::ZERO_TO_ONE, numSlices);

	wireSphereProp->m_rasterizerMode = RasterizerMode::WIREFRAME_CULL_BACK;
	g_theDebugRenderer->m_data.emplace_back(wireSphereProp);
}

void DebugAddWorld2DCone(float duration, const Mat44& transform, float radius, float coneAngleDegrees, float startAngleDegrees, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	std::lock_guard<std::mutex> lock(g_theDebugRenderer->m_renderMutex);

	RenderEntity* coneProp = new RenderEntity();

	coneProp->m_lifeSpan = duration;
	coneProp->m_startColor = startColor;
	coneProp->m_endColor = endColor;
	coneProp->m_renderMode = mode;

	Vec2 center = transform.TransformPosition2D(Vec2::ZERO);

	if (duration != -1.f)
	{
		coneProp->m_timer = new Timer(duration, g_theDebugRenderer->m_clock);
		coneProp->m_timer->Start();
	}

	Rgba8 color;
	if (coneProp->m_lifeSpan == -1.f || coneProp->m_lifeSpan == 0.f)
	{
		color = coneProp->m_startColor;
	}
	else
	{
		color = Interpolate(coneProp->m_startColor, coneProp->m_endColor, coneProp->m_timer->GetElapsedFraction());
	}

	AddVertsForCone2D(coneProp->m_verts, center, radius, coneAngleDegrees, startAngleDegrees, color);

	coneProp->m_rasterizerMode = RasterizerMode::SOLID_CULL_BACK;
	g_theDebugRenderer->m_data.emplace_back(coneProp);
}

void DebugAddWorld3DCone(float duration, const Mat44& transform, float radius, float coneLength, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	std::lock_guard<std::mutex> lock(g_theDebugRenderer->m_renderMutex);

	RenderEntity* coneProp = new RenderEntity();

	const int numConeSlices = 32;

	coneProp->m_lifeSpan = duration;
	coneProp->m_startColor = startColor;
	coneProp->m_endColor = endColor;
	coneProp->m_renderMode = mode;

	Vec3 end = transform.TransformPosition3D(Vec3::ZERO);
	Vec3 start = transform.TransformPosition3D(Vec3(coneLength, 0.f, 0.f));

	if (duration != -1.f)
	{
		coneProp->m_timer = new Timer(duration, g_theDebugRenderer->m_clock);
		coneProp->m_timer->Start();
	}

	Rgba8 color;
	if (coneProp->m_lifeSpan == -1.f || coneProp->m_lifeSpan == 0.f)
	{
		color = coneProp->m_startColor;
	}
	else
	{
		color = Interpolate(coneProp->m_startColor, coneProp->m_endColor, coneProp->m_timer->GetElapsedFraction());
	}

	AddVertsForCone3D(coneProp->m_verts, start, end, radius, color, AABB2::ZERO_TO_ONE, numConeSlices);

	coneProp->m_rasterizerMode = RasterizerMode::SOLID_CULL_BACK;
	g_theDebugRenderer->m_data.emplace_back(coneProp);
}

void DebugAddWorldArrow(const Vec3& start, const Vec3& end, float interpolate, float coneRadius, float cylinderRadius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	std::lock_guard<std::mutex> lock(g_theDebugRenderer->m_renderMutex);

	RenderEntity* arrowProp = new RenderEntity();

	const int numCylinderSlices = 64;
	const int numConeSlices = 32;

	arrowProp->m_lifeSpan = duration;
	arrowProp->m_startColor = startColor;
	arrowProp->m_endColor = endColor;
	arrowProp->m_renderMode = mode;

	if (duration != -1.f)
	{
		arrowProp->m_timer = new Timer(duration, g_theDebugRenderer->m_clock);
		arrowProp->m_timer->Start();
	}

// 	Rgba8 color;
// 	if (arrowProp->m_lifeSpan == -1.f || arrowProp->m_lifeSpan == 0.f)
// 	{
// 		color = arrowProp->m_startColor;
// 	}
// 	else
// 	{
// 		color = Interpolate(arrowProp->m_startColor, arrowProp->m_endColor, arrowProp->m_timer->GetElapsedFraction());
// 	}
	
	AddVertsForArrow3D(arrowProp->m_verts, start, end, interpolate, cylinderRadius, coneRadius, endColor, startColor, AABB2::ZERO_TO_ONE, numCylinderSlices, numConeSlices);

	arrowProp->m_rasterizerMode = RasterizerMode::SOLID_CULL_BACK;
	g_theDebugRenderer->m_data.emplace_back(arrowProp);
}

void DebugAddWorldWireArrow(const Vec3& start, const Vec3& end, float interpolate, float coneRadius, float cylinderRadius, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	std::lock_guard<std::mutex> lock(g_theDebugRenderer->m_renderMutex);

	RenderEntity* arrowProp = new RenderEntity();

	const int numCylinderSlices = 64;
	const int numConeSlices = 32;

	arrowProp->m_lifeSpan = duration;
	arrowProp->m_startColor = startColor;
	arrowProp->m_endColor = endColor;
	arrowProp->m_renderMode = mode;

	if (duration != -1.f)
	{
		arrowProp->m_timer = new Timer(duration, g_theDebugRenderer->m_clock);
		arrowProp->m_timer->Start();
	}

// 	Rgba8 color;
// 	if (arrowProp->m_lifeSpan == -1.f || arrowProp->m_lifeSpan == 0.f)
// 	{
// 		color = arrowProp->m_startColor;
// 	}
// 	else
// 	{
// 		color = Interpolate(arrowProp->m_startColor, arrowProp->m_endColor, arrowProp->m_timer->GetElapsedFraction());
// 	}

	AddVertsForArrow3D(arrowProp->m_verts, start, end, interpolate, cylinderRadius, coneRadius, endColor, startColor, AABB2::ZERO_TO_ONE, numCylinderSlices, numConeSlices);

	arrowProp->m_rasterizerMode = RasterizerMode::WIREFRAME_CULL_BACK;
	g_theDebugRenderer->m_data.emplace_back(arrowProp);
}

void DebugAddWorldText(const std::string& text, const Mat44& transform, float textHeight, const Vec2& alignment, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	std::lock_guard<std::mutex> lock(g_theDebugRenderer->m_renderMutex);

	RenderEntity* worldText = new RenderEntity();

	worldText->m_lifeSpan = duration;
	worldText->m_startColor = startColor;
	worldText->m_endColor = endColor;
	worldText->m_renderMode = mode;

	Vec3 axisStart = transform.TransformPosition3D(Vec3::ZERO);
	Vec3 zAxisEnd = transform.TransformPosition3D(Vec3(0.f, 0.f, 1.f));
	Vec3 yAxisEnd = transform.TransformPosition3D(Vec3(0.f, 1.f, 0.f));
	Vec3 xAxisEnd = transform.TransformPosition3D(Vec3(1.f, 0.f, 0.f));

	if (duration != -1.f)
	{
		worldText->m_timer = new Timer(duration, g_theDebugRenderer->m_clock);
		worldText->m_timer->Start();
	}

	Rgba8 color;
	if (worldText->m_lifeSpan == -1.f || worldText->m_lifeSpan == 0.f)
	{
		color = worldText->m_startColor;
	}
	else
	{
		color = Interpolate(worldText->m_startColor, worldText->m_endColor, worldText->m_timer->GetElapsedFraction());
	}

	g_bitmapFont->AddVertsForText3DAtOriginXForward(worldText->m_verts, textHeight, text, color, 1.f, alignment);
	worldText->m_texture = &g_bitmapFont->GetTexture();

	worldText->m_rasterizerMode = RasterizerMode::SOLID_CULL_BACK;
	g_theDebugRenderer->m_data.emplace_back(worldText);
}

void DebugAddWorldBillboardText(const std::string& text, const Vec3& origin, float textHeight, const Vec2& alignment, float duration, const Rgba8& startColor, const Rgba8& endColor, DebugRenderMode mode)
{
	std::lock_guard<std::mutex> lock(g_theDebugRenderer->m_renderMutex);

	RenderEntity* billboardText = new RenderEntity();

	billboardText->m_isBillboardText = true;
	billboardText->m_lifeSpan = duration;
	billboardText->m_startColor = startColor;
	billboardText->m_endColor = endColor;
	billboardText->m_renderMode = mode;

	billboardText->m_matrix = GetBillboardMatrix(BillBoardType::FULL_CAMERA_OPPOSING, g_theDebugRenderer->m_worldCamera->GetModelMatrix(), origin);
	
	if (duration != -1.f)
	{
		billboardText->m_timer = new Timer(duration, g_theDebugRenderer->m_clock);
		billboardText->m_timer->Start();
	}

	Rgba8 color;
	if (billboardText->m_lifeSpan == -1.f || billboardText->m_lifeSpan == 0.f)
	{
		color = billboardText->m_startColor;
	}
	else
	{
		color = Interpolate(billboardText->m_startColor, billboardText->m_endColor, billboardText->m_timer->GetElapsedFraction());
	}

	g_bitmapFont->AddVertsForText3DAtOriginXForward(billboardText->m_verts, textHeight, text, color, 0.7f, alignment);
	billboardText->m_texture = &g_bitmapFont->GetTexture();

	billboardText->m_rasterizerMode = RasterizerMode::SOLID_CULL_BACK;
	g_theDebugRenderer->m_data.emplace_back(billboardText);
}

void DebugAdd2DWorldBasis(const Mat44& transform, float interpolate, float cylinderRadius, float coneRadius, float basisLength, float duration, DebugRenderMode mode)
{
	std::lock_guard<std::mutex> lock(g_theDebugRenderer->m_renderMutex);

	RenderEntity* basis = new RenderEntity();

	const int numCylinderSlices = 64;
	const int numConeSlices = 32;

	basis->m_lifeSpan = duration;
	basis->m_renderMode = mode;

	if (duration != -1.f)
	{
		basis->m_timer = new Timer(duration, g_theDebugRenderer->m_clock);
		basis->m_timer->Start();
	}

	Vec3 axisStart = transform.TransformPosition3D(Vec3::ZERO);
	Vec3 yAxisEnd = transform.TransformPosition3D(Vec3(0.f, basisLength, 0.f));
	Vec3 xAxisEnd = transform.TransformPosition3D(Vec3(basisLength, 0.f, 0.f));

	// Green Y right axis arrow		   
	AddVertsForArrow3D(basis->m_verts, axisStart, yAxisEnd, interpolate, cylinderRadius, coneRadius, Rgba8::GREEN, Rgba8::GREEN, AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f)), numCylinderSlices, numConeSlices);

	// Red X forward axis arrow		   
	AddVertsForArrow3D(basis->m_verts, axisStart, xAxisEnd, interpolate, cylinderRadius, coneRadius, Rgba8::RED, Rgba8::RED, AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f)), numCylinderSlices, numConeSlices);

	basis->m_rasterizerMode = RasterizerMode::SOLID_CULL_BACK;
	g_theDebugRenderer->m_data.emplace_back(basis);
}

void DebugAdd3DWorldBasis(const Mat44& transform, float interpolate, float cylinderRadius, float coneRadius, float basisLength, float duration, DebugRenderMode mode)
{
	std::lock_guard<std::mutex> lock(g_theDebugRenderer->m_renderMutex);

 	RenderEntity* basis = new RenderEntity();

	const int numCylinderSlices = 64;
	const int numConeSlices = 32;

	basis->m_lifeSpan = duration;
	basis->m_renderMode = mode;

	if (duration != -1.f)
	{
		basis->m_timer = new Timer(duration, g_theDebugRenderer->m_clock);
		basis->m_timer->Start();
	}

	Vec3 axisStart = transform.TransformPosition3D(Vec3::ZERO);
	Vec3 zAxisEnd = transform.TransformPosition3D(Vec3(0.f, 0.f, basisLength));
	Vec3 yAxisEnd = transform.TransformPosition3D(Vec3(0.f, basisLength, 0.f));
	Vec3 xAxisEnd = transform.TransformPosition3D(Vec3(basisLength, 0.f, 0.f));

	// Blue Z up axis arrow
	AddVertsForArrow3D(basis->m_verts, axisStart, zAxisEnd, interpolate, cylinderRadius, coneRadius, Rgba8::BLUE, Rgba8::BLUE, AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f)), numCylinderSlices, numConeSlices);
	
	// Green Y right axis arrow		   
	AddVertsForArrow3D(basis->m_verts, axisStart, yAxisEnd, interpolate, cylinderRadius, coneRadius, Rgba8::GREEN, Rgba8::GREEN, AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f)), numCylinderSlices, numConeSlices);
	
	// Red X forward axis arrow		   
	AddVertsForArrow3D(basis->m_verts, axisStart, xAxisEnd, interpolate, cylinderRadius, coneRadius, Rgba8::RED, Rgba8::RED, AABB2(Vec2(0.f, 0.f), Vec2(1.f, 1.f)), numCylinderSlices, numConeSlices);

	basis->m_rasterizerMode = RasterizerMode::SOLID_CULL_BACK;
	g_theDebugRenderer->m_data.emplace_back(basis);
}

void DebugAddScreenText(const std::string& text, const Vec2& position, float size, const Vec2& alignment, float duration, const Rgba8& startColor, const Rgba8& endColor)
{
	std::lock_guard<std::mutex> lock(g_theDebugRenderer->m_renderMutex);

	UNUSED(alignment);

	RenderScreenInformation* renderText = new RenderScreenInformation();

	renderText->m_textPosition = position;
	renderText->m_lifeSpan = duration;
	renderText->m_startColor = startColor;
	renderText->m_endColor = endColor;

	if (duration != -1.f)
	{
		renderText->m_timer = new Timer(duration, g_theDebugRenderer->m_clock);
		renderText->m_timer->Start();
	}

	Rgba8 color;
	if (renderText->m_lifeSpan == -1.f || renderText->m_lifeSpan == 0.f)
	{
		color = renderText->m_startColor;
	}
	else
	{
		color = Interpolate(renderText->m_startColor, renderText->m_endColor, renderText->m_timer->GetElapsedFraction());
	}

	g_bitmapFont->AddVertsForText2D(renderText->m_verts, Vec2::ZERO, size, text, color, 1.f);
	renderText->m_texture = &g_bitmapFont->GetTexture();

	renderText->m_rasterizerMode = RasterizerMode::SOLID_CULL_BACK;
	g_theDebugRenderer->m_textInfo.emplace_back(renderText);
}

void DebugAddMessage(const std::string& text, float duration, const Rgba8& startColor, const Rgba8& endColor)
{
	std::lock_guard<std::mutex> lock(g_theDebugRenderer->m_renderMutex);

	RenderScreenInformation* renderMessage = new RenderScreenInformation();

	renderMessage->m_lifeSpan = duration;
	renderMessage->m_startColor = startColor;
	renderMessage->m_endColor = endColor;

	if (duration != -1.f)
	{
		renderMessage->m_timer = new Timer(duration, g_theDebugRenderer->m_clock);
		renderMessage->m_timer->Start();
	}

	Rgba8 color;
	if (renderMessage->m_lifeSpan != -1.f || renderMessage->m_lifeSpan == 0.f)
	{
		color = renderMessage->m_startColor;
	}
	else
	{
		color = Interpolate(renderMessage->m_startColor, renderMessage->m_endColor, renderMessage->m_timer->GetElapsedFraction());
	}

	g_bitmapFont->AddVertsForText2D(renderMessage->m_verts, Vec2::ZERO, 10.f, text, color, 1.f);
	renderMessage->m_texture = &g_bitmapFont->GetTexture();

	renderMessage->m_rasterizerMode = RasterizerMode::SOLID_CULL_BACK;
	g_theDebugRenderer->m_messageInfo.emplace_back(renderMessage);
}

bool Command_DebugRenderClear(EventArgs& args)
{
	UNUSED(args);
	DebugRenderClear();
	return true;
}

bool Command_DebugRenderToggle(EventArgs& args)
{
	UNUSED(args);

	if (g_theDebugRenderer->m_isVisible)
	{
		DebugRenderSetHidden();
	}
	else
	{
		DebugRenderSetVisible();
	}


	return true;
}
