#pragma once

#include <vector>
#include "Render.h"
#include "Shaders.h"

using namespace Microsoft;
using namespace WRL;

class LuminanceCalc {
public:
    LuminanceCalc() {
        QueryPerformanceFrequency(&freq);
        QueryPerformanceCounter(&lastTime);
    }

    bool Init(ComPtr<ID3D11Device> pDev, size_t width, size_t height);
    bool ResizeWnd(size_t width, size_t height);
    ID3D11ShaderResourceView* ResShaderView() const { return vecTextures.back().GetTextResView(); }
    float process(ComPtr<ID3D11DeviceContext> devCtx, ComPtr<ID3D11ShaderResourceView> sourceTexture);

private:
    void rendText(ComPtr<ID3D11DeviceContext> devCtx, ComPtr<ID3D11ShaderResourceView> sourceTexture,
        Render& render, ComPtr<ID3D11PixelShader> pShader);

    vector<Render> vecTextures;

    VertexShader vertexShader;
    PixelShader pixelShader;
    PixelShader lumPixelShader;

    ComPtr<ID3D11SamplerState> samplState;
    ComPtr<ID3D11Texture2D> lumTexture;
    ComPtr<ID3D11Device> pDevice;

    float lum{ 0.0f };
    LARGE_INTEGER lastTime;
    LARGE_INTEGER freq;
};