#include "Shaders.h"

using namespace std;
using namespace utils;

bool PixelShader::Init(DeviceComPtr pDev, wstring pathSh) {
	HRESULT hResult = D3DReadFileToBlob(pathSh.c_str(), pShaderBuf.GetAddressOf());
	if (FAILED(hResult)) {
		wstring error_msg = L"Failed load shader: " + pathSh;
		Loger::Log(hResult, error_msg);
		return false;
	}

	hResult = pDev->CreatePixelShader(pShaderBuf->GetBufferPointer(), pShaderBuf->GetBufferSize(), NULL, pShader.GetAddressOf());
	if (FAILED(hResult)) {
		wstring error_msg = L"Failed create pixel shader: " + pathSh;
		Loger::Log(hResult, error_msg);
		return false;
	}
	return true;
}

ID3D11PixelShader* PixelShader::GetShaderPtr() {
	return pShader.Get();
}

ID3D10Blob* PixelShader::GetBufferPtr() {
	return pShaderBuf.Get();
}


bool VertexShader::Init(DeviceComPtr pDev, wstring pathSh, D3D11_INPUT_ELEMENT_DESC* layoutDesc, UINT numElem)
{
	HRESULT hResult = D3DReadFileToBlob(pathSh.c_str(), pShaderBuf.GetAddressOf());
	if (FAILED(hResult)) {
		wstring error_msg = L"Failed load shader: " + pathSh;
		Loger::Log(hResult, error_msg);
		return false;
	}

	hResult = pDev->CreateVertexShader(pShaderBuf->GetBufferPointer(), pShaderBuf->GetBufferSize(), NULL, pShader.GetAddressOf());
	if (FAILED(hResult)) {
		wstring error_msg = L"Failed create vertex shader: " + pathSh;
		Loger::Log(hResult, error_msg);
		return false;
	}
	if (layoutDesc != NULL){
		hResult = pDev->CreateInputLayout(layoutDesc, numElem, pShaderBuf->GetBufferPointer(), 
			pShaderBuf->GetBufferSize(), pLayout.GetAddressOf());
		if (FAILED(hResult)) {
			Loger::Log(hResult, "Failed create input layout");
			return false;
		}
	}

	return true;
}

ID3D11VertexShader* VertexShader::GetShaderPtr() {
	return pShader.Get();
}

ID3D10Blob* VertexShader::GetBufferPtr() {
	return pShaderBuf.Get();
}