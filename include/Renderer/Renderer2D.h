#pragma once

#include "Config.h"
#include "Camera.h"
#include "Texture.h"
#include "Scene/Components.h"
#include "Font.h"

namespace UE {

    class UE_API Renderer2D{
    public:
        static void Init();
        static void Shutdown();

        static void BeginCamera(const Camera& camera);        
        static void EndCamera();
        static void Flush();

        // Primitives
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));

		static void DrawQuad(const glm::mat4& transform, const glm::vec4& color, int entityID = -1);
		static void DrawQuad(const glm::mat4& transform, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f), int entityID = -1);

		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color);
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color);
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const Ref<Texture2D>& texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));

        static void DrawSprite(const glm::mat4& transform, SpriteRendererComponent& src, int entityID = -1);
		static void DrawUI(const glm::mat4& transform, UIElement& src, int entityID = -1);
		static void DrawUI(const glm::vec3& position, TextUIComponent& src, int entityID = -1);
		static void DrawScreen(Ref<Framebuffer>& buffer);
		static void DrawGlyph(const glm::mat4& transform,
			const glm::vec2& uv0, const glm::vec2& uv1,
			const Ref<Texture2D>& texture, float tilingFactor = 1.0f, 
			const glm::vec4& tintColor = glm::vec4(1.0f), int entityID = -1);
		static void DrawText(const std::string& text, const glm::vec3& position, Ref<Font> font, const glm::vec4& color);

        // Stats
		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t QuadCount = 0;

			uint32_t GetTotalVertexCount() const { return QuadCount * 4; }
			uint32_t GetTotalIndexCount() const { return QuadCount * 6; }
		};
		static void ResetStats();
		static Statistics GetStats();

	private:
		static void StartBatch();
		static void NextBatch();		
    };
}