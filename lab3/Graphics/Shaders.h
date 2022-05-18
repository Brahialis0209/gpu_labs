#pragma once
#pragma comment(lib, "D3DCompiler.lib")

#include "../Loger.h"
#include <d3d11.h>
#include <wrl/client.h>
#include <d3dcompiler.h>

using namespace Microsoft;
using namespace WRL;
using namespace std;

class PixelShader {
public:
	using DeviceComPtr = ComPtr<ID3D11Device>;
	bool Init(DeviceComPtr pDev, wstring pathSh);
	ID3D11PixelShader* GetShaderPtr();
	ID3D10Blob* GetBufferPtr();
	ComPtr<ID3D11PixelShader> GetShPtr() { return pShader; }
	ComPtr<ID3D10Blob> GetBufPtr() { return pShaderBuf; }
private:
	ComPtr<ID3D11PixelShader> pShader;
	ComPtr<ID3D10Blob> pShaderBuf;
};


class VertexShader {
public:
	using DeviceComPtr = ComPtr<ID3D11Device>;
	bool Init(DeviceComPtr pDev, wstring pathSh, D3D11_INPUT_ELEMENT_DESC* layoutDesc, UINT numElem);
	ID3D11VertexShader* GetShaderPtr();
	ID3D10Blob* GetBufferPtr();
	ID3D11InputLayout* GetInputLayoutPtr() { return pLayout.Get(); }
private:
	ComPtr<ID3D11VertexShader> pShader;
	ComPtr<ID3D10Blob> pShaderBuf;
	ComPtr<ID3D11InputLayout> pLayout;
};