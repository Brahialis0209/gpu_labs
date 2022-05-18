#pragma once
#include <stdexcept>
#include <d3d11.h>
#include <wrl/client.h>

using namespace std;
using namespace Microsoft;
using namespace WRL;

class Render {
public:
    Render() : format(DXGI_FORMAT_R16G16B16A16_FLOAT), viewPort() {}
    Render(DXGI_FORMAT frmt) : format(frmt), viewPort() {}
    Render(DXGI_FORMAT frmt, ComPtr<ID3D11Device> dvc, size_t width, size_t height) : format(frmt), viewPort() {
        if (!Init(dvc, width, height))
            throw new runtime_error("Failed create texture render.");
    }

    bool Init(ComPtr<ID3D11Device> dvc, size_t width, size_t height);

    D3D11_VIEWPORT GetViewPort() const { return viewPort; }
    ID3D11Texture2D* GetTextRendTarg() const { return textRendTarg.Get(); };
    ID3D11RenderTargetView* GetTextTargView() const { return textTargView.Get(); };
    ID3D11ShaderResourceView* GetTextResView() const { return textResView.Get(); };
    size_t GetMipLevels() const { return m_rtd.MipLevels; }

private:
    ComPtr<ID3D11ShaderResourceView> textResView;
    ComPtr<ID3D11Texture2D> textRendTarg;
    ComPtr<ID3D11RenderTargetView> textTargView;
    DXGI_FORMAT format;
    D3D11_VIEWPORT viewPort;

    CD3D11_TEXTURE2D_DESC m_rtd;
};