#pragma once

#include "Config.h"
#include <stb_truetype.h>
#include "Texture.h"

namespace UE {

    class UE_API Font{
    public:
        Font(const std::string& path, float pixelHeight = 48.0f);
        ~Font() = default;
        stbtt_bakedchar* GetCharData() { return m_CharData;}
        Ref<Texture2D> GetTexture() const { return m_Texture;}
    private:    
        std::vector<uint8_t> m_FontBuffer;    
        stbtt_bakedchar m_CharData[96];
        unsigned char m_RGBA[512 * 512];
        Ref<Texture2D> m_Texture;
        
    };
}