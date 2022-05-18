#include "Render.h"
#include "../Loger.h"

bool Render::Init(ComPtr<ID3D11Device> pDev, size_t width, size_t height) {
    //CD3D11_TEXTURE2D_DESC text2d;
    ZeroMemory(&m_rtd, sizeof(m_rtd));

    m_rtd.Format = format;
    m_rtd.Width = width;
    m_rtd.Height = height;
    m_rtd.ArraySize = 1;
    m_rtd.MipLevels = 1;
    m_rtd.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
    m_rtd.Usage = D3D11_USAGE_DEFAULT;
    m_rtd.CPUAccessFlags = 0;
    m_rtd.SampleDesc.Count = 1;
    m_rtd.SampleDesc.Quality = 0;
    m_rtd.MiscFlags = 0;

    HRESULT hResult = pDev->CreateTexture2D(&m_rtd, nullptr, textRendTarg.ReleaseAndGetAddressOf());
    if (FAILED(hResult)) {
        utils::Loger::Log(hResult, "Failed create render target texture.");
        return false;
    }

    CD3D11_RENDER_TARGET_VIEW_DESC rendTarg;
    ZeroMemory(&rendTarg, sizeof(rendTarg));
    rendTarg.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
    rendTarg.Format = m_rtd.Format;

    hResult = pDev->CreateRenderTargetView(textRendTarg.Get(), &rendTarg, textTargView.ReleaseAndGetAddressOf());
    if (FAILED(hResult)) {
        utils::Loger::Log(hResult, "Failed create render target view texture.");
        return false;
    }

    CD3D11_SHADER_RESOURCE_VIEW_DESC srvd(D3D11_SRV_DIMENSION_TEXTURE2D, m_rtd.Format, 0, 1);
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