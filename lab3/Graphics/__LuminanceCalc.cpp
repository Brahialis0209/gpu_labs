#include "LuminanceCalc.h"
#include <cmath>
#include "../Mode.h"

using namespace Microsoft;
using namespace WRL;

bool LuminanceCalc::Init(ComPtr<ID3D11Device> ptrDevice, size_t width, size_t height) {
    if (!vertexShader.Init(ptrDevice, L"colorvertexshader.cso", nullptr, 0))
        return false;
    if (!pixelShader.Init(ptrDevice, L"colorpixelshader.cso"))
        return false;
    if (!lumPixelShader.Init(ptrDevice, L"luminancepixelshader.cso"))
        return false;

    pDevice = ptrDevice;
    // Create the sampler state
    D3D11_SAMPLER_DESC samplDesc;
    ZeroMemory(&samplDesc, sizeof(samplDesc));
    samplDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplDesc.MaxAnisotropy = D3D11_MAX_MAXANISOTROPY;
    samplDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplDesc.MinLOD = 0;
    samplDesc.MaxLOD = D3D11_FLOAT32_MAX;

    HRESULT hResult = ptrDevice->CreateSamplerState(&samplDesc, samplState.GetAddressOf());
    if (FAILED(hResult)) {
        utils::Loger::Log(hResult, "Failed create sampler state");
        return false;
    }

    CD3D11_TEXTURE2D_DESC tx2dDesc(
        DXGI_FORMAT_R32G32B32A32_FLOAT,
        1,
        1,
        1,
        1,
        0,
        D3D11_USAGE_STAGING,
        D3D11_CPU_ACCESS_READ
    );

    hResult = ptrDevice->CreateTexture2D(&tx2dDesc, nullptr, &lumTexture);
    if (FAILED(hResult)) {
        utils::Loger::Log(hResult, "Failed create 2d texture");
        return false;
    }

    return ResizeWnd(width, height);
}

bool LuminanceCalc::ResizeWnd(size_t width, size_t height) {
    size_t minHW = static_cast<size_t>(min(width, height));
    size_t numTx = static_cast<size_t>(log2(minHW));
    vecTextures.clear();
    try {
        for (size_t i = 0; i <= numTx; ++i) {
            size_t sizeTx = static_cast<size_t>(1) << (numTx - i);
            vecTextures.emplace_back(DXGI_FORMAT_R32G32B32A32_FLOAT, pDevice, sizeTx, sizeTx);
        }
    }
    catch (...) {
        return false;
    }
    return true;
}

void LuminanceCalc::rendText(ComPtr<ID3D11DeviceContext> ctx, ComPtr<ID3D11ShaderResourceView> srcTx, Render& render, ComPtr<ID3D11PixelShader> pShader) {
    ID3D11RenderTargetView* renderTarget = render.GetTextTargView();
    D3D11_VIEWPORT viewport = render.GetViewPort();
    ctx->OMSetRenderTargets(1, &renderTarget, nullptr);
    ctx->RSSetViewports(1, &viewport);
    ctx->PSSetShader(pShader.Get(), nullptr, 0);
    ctx->PSSetShaderResources(0, 1, srcTx.GetAddressOf());
    ctx->Draw(4, 0);
    ID3D11ShaderResourceView* nullsrv[] = { nullptr };
    ctx->PSSetShaderResources(0, 1, nullsrv);
}

float LuminanceCalc::process(ComPtr<ID3D11DeviceContext> devCtx, ComPtr<ID3D11ShaderResourceView> srcTx) {
    Mode::GetUserAnnotation().BegEvent(L"Luminance calculating.");
    float colors[] = { 0.0f, 0.0f, 0.0f, 1.0f };
    for (size_t i = 0; i < vecTextures.size(); ++i)
        devCtx->ClearRenderTargetView(vecTextures[i].GetTextTargView(), colors);

    devCtx->IASetInputLayout(nullptr);
    devCtx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
    devCtx->PSSetSamplers(0, 1, samplState.GetAddressOf());
    devCtx->VSSetShader(vertexShader.GetShaderPtr(), nullptr, 0);    
    rendText(devCtx, srcTx, vecTextures[0], pixelShader.GetShaderPtr());

    for (size_t i = 1; i < vecTextures.size(); ++i)
        rendText(devCtx, vecTextures[i - 1].GetTextResView(), vecTextures[i], lumPixelShader.GetShaderPtr());

    ID3D11ShaderResourceView* nullsrv[] = { nullptr };
    devCtx->PSSetShaderResources(0, 1, nullsrv);
    LARGE_INTEGER curTime;
    QueryPerformanceCounter(&curTime);

    size_t deltaT = static_cast<size_t>(curTime.QuadPart - lastTime.QuadPart);
    lastTime = curTime;
    double delta = static_cast<double>(deltaT) / freq.QuadPart;
    D3D11_MAPPED_SUBRESOURCE lumAcc;
    devCtx->CopyResource(lumTexture.Get(), vecTextures[vecTextures.size() - 1].GetTextRendTarg());
    devCtx->Map(lumTexture.Get(), 0, D3D11_MAP_READ, 0, &lumAcc);
    
    float luminance = *(float*)lumAcc.pData;
    devCtx->Unmap(lumTexture.Get(), 0);
    float sigma = 0.04f / (0.04f + luminance);
    float tau = sigma * 0.4f + (1 - sigma) * 0.1f;
    lum += (luminance - lum) * (1 - exp(-delta * tau));
    Mode::GetUserAnnotation().EndEvent();
    return lum;
}