#include "Render.h"
#include "../Loger.h"

bool Render::Init(ComPtr<ID3D11Device> pDev, size_t width, size_t height) {
    CD3D11_TEXTURE2D_DESC text2d;
    ZeroMemory(&text2d, sizeof(text2d));

    text2d.Format = format;
    text2d.Width = width;
    text2d.Height = height;
    text2d.ArraySize = 1;
    text2d.MipLevels = 1;
    text2d.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    text2d.Usage = D3D11_USAGE_DEFAULT;
    text2d.CPUAccessFlags = 0;
    text2d.SampleDesc.Count = 1;
    text2d.SampleDesc.Quality = 0;
    text2d.MiscFlags = 0;

    HRESULT hResult = pDev->CreateTexture2D(&text2d, nullptr, textRendTarg.ReleaseAndGetAddressOf());
    if (FAILED(hResult)) {
        utils::Loger::Log(hResult, "Failed create render target texture.");
        return false;
    }

    CD3D11_RENDER_TARGET_VIEW_DESC rendTarg;
    ZeroMemory(&rendTarg, sizeof(rendTarg));
    rendTarg.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rendTarg.Format = text2d.Format;

    hResult = pDev->CreateRenderTargetView(textRendTarg.Get(), &rendTarg, textTargView.ReleaseAndGetAddressOf());
    if (FAILED(hResult)) {
        utils::Loger::Log(hResult, "Failed create render target view texture.");
        return false;
    }

    CD3D11_SHADER_RESOURCE_VIEW_DESC srvd(D3D11_SRV_DIMENSION_TEXTURE2D, text2d.Format, 0, 1);
    hResult = pDev->CreateShaderResourceView(textRendTarg.Get(), &srvd, textResView.ReleaseAndGetAddressOf());
    if (FAILED(hResult)) {
        utils::Loger::Log(hResult, "Failed create shader resource view texture.");
        return false;
    }

    ZeroMemory(&viewPort, sizeof(viewPort));
    viewPort.Width = static_cast<FLOAT>(width);
    viewPort.Height = static_cast<FLOAT>(height);
    viewPort.MinDepth = 0.0f;
    viewPort.MaxDepth = 1.0f;
    viewPort.TopLeftX = 0;
    viewPort.TopLeftY = 0;

    return true;
}