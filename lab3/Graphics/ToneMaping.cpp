#include "ToneMaping.h"
#include "../Mode.h"

bool ToneMaping::Init(ComPtr<ID3D11Device> device, ComPtr<ID3D11DeviceContext> device_context, size_t width, size_t height) {
	wndWidth = width;
	wndHeight = height;

	if (!vShader.Init(device, L"tone_v_shader.cso", NULL, 0)) {
		return false;
	}

	if (!pShader.Init(device, L"tone_p_shader.cso")) {
		return false;
	}
	D3D11_SAMPLER_DESC samplDesc;
	ZeroMemory(&samplDesc, sizeof(samplDesc));
	samplDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
	samplDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplDesc.MinLOD = 0;
	samplDesc.MaxLOD = D3D11_FLOAT32_MAX;
	samplDesc.MaxAnisotropy = D3D11_MAX_MAXANISOTROPY;
	HRESULT hResult = device->CreateSamplerState(&samplDesc, toneSampler.GetAddressOf());
	if (FAILED(hResult)) {
		utils::Loger::Log(hResult, "Failed create smapler state");
		return false;
	}
	calc = make_unique<LuminanceCalc>();
	if (!calc->Init(device, width, height)) {
		return false;
	}

	CD3D11_BUFFER_DESC tbd(sizeof(LumConstBuf), D3D11_BIND_CONSTANT_BUFFER);
	hResult = device->CreateBuffer(&tbd, nullptr, lumBuffer.GetAddressOf());
	if (FAILED(hResult)) {
		utils::Loger::Log(hResult, "Failed create constant time buffer.");
		return false;
	}

	return true;
}

void ToneMaping::Process(ComPtr<ID3D11DeviceContext> ctx, ComPtr<ID3D11ShaderResourceView> srcText,
	ComPtr<ID3D11RenderTargetView> renderTarget, D3D11_VIEWPORT viewport) {
	Mode::GetUserAnnotation().BegEvent(L"Start ToneMaping.");
	float averageLuminance = calc->process(ctx, srcText);

	LumConstBuf luminanceBufferData = { averageLuminance };

	ctx->UpdateSubresource(lumBuffer.Get(), 0, nullptr, &luminanceBufferData, 0, 0);
	ctx->PSSetShaderResources(0, 1, srcText.GetAddressOf());
	ctx->OMSetRenderTargets(1, renderTarget.GetAddressOf(), nullptr);
	ctx->RSSetViewports(1, &viewport);

	ctx->IASetInputLayout(nullptr);
	ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	ctx->VSSetShader(vShader.GetShaderPtr(), nullptr, 0);
	ctx->PSSetShader(pShader.GetShaderPtr(), nullptr, 0);
	ctx->PSSetConstantBuffers(0, 1, lumBuffer.GetAddressOf());
	ctx->PSSetSamplers(0, 1, toneSampler.GetAddressOf());

	ctx->Draw(4, 0);

	ID3D11ShaderResourceView* nullsrv[] = { nullptr };
	ctx->PSSetShaderResources(0, 1, nullsrv);
	Mode::GetUserAnnotation().EndEvent();
}

bool ToneMaping::ResizeWnd(size_t width, size_t height) {
	wndWidth = width;
	wndHeight = height;
	return calc->ResizeWnd(width, height);
}
