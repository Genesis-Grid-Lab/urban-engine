#include "Renderer/Font.h"
#include <glad/glad.h>


namespace UE {

    Font::Font(const std::string& path, float pixelHeight) {
        m_FontBuffer.resize(1 << 20); // 1 MB
        std::ifstream file(path, std::ios::binary);
        if (!file) {
            UE_CORE_ERROR("Failed to open font file: {}", path);
            return;
        }

        file.read(reinterpret_cast<char*>(m_FontBuffer.data()), m_FontBuffer.size());

        // Bake font
        stbtt_BakeFontBitmap(m_FontBuffer.data(), 0, pixelHeight, m_RGBA, 512, 512, 32, 96, m_CharData);

        // Convert grayscale to RGBA
        std::vector<uint32_t> rgbaData(512 * 512);
        for (int i = 0; i < 512 * 512; ++i) {
            uint8_t alpha = m_RGBA[i];
            rgbaData[i] = (alpha << 24) | (0xFFFFFF); // white with alpha
        }
        
        m_Texture = CreateRef<Texture2D>(512, 512);
        m_Texture->SetData(rgbaData.data(), rgbaData.size() * sizeof(uint32_t));
    }
}