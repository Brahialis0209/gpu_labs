#pragma once

#define _XM_NO_INTRINSICS_
#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>

#include "ToneMaping.h"
#include "AdapterFinder.h"
#include "Shaders.h"


struct Vertex {
	DirectX::XMFLOAT4 pos;
	DirectX::XMFLOAT3 norm;
};


class Graphics {
public:
	bool Init(HWND hwnd, size_t width, size_t height);
	void RenderFrame();

	XMMATRIX& RefWorld() { return matrWorld; }
	XMMATRIX& RefView() { return matrView; }
	XMMATRIX& RefProjection() { return matrProj; }
	CamPos& RefCamera() { return camPos; }
	WrlCamPos& RefWorldCameraPosition() { return wrldCamPos; }

	void ChangeLightsIntencity(size_t ind);
	void ChangeToneMaping() { toneMapOn ^= true; }
	bool ResizeWnd(size_t width, size_t height);

private:
	ComPtr<ID3D11RasterizerState> rasterState;
	ComPtr<ID3D11Device> pDevice;
	ComPtr<ID3D11DeviceContext> pContext;
	ComPtr<ID3D11RenderTargetView> pRendTargView;
	ComPtr<IDXGISwapChain> pSwapChain;
	ComPtr<ID3D11Buffer> vBuf;
	ComPtr<ID3D11Buffer> idxBuf;
	ComPtr<ID3D11Buffer> constBuf1;
	ComPtr<ID3D11Buffer> constBuf2;
	//	ComPtr<ID3D11Texture2D> depthBuf;
	//	ComPtr<ID3D11DepthStencilView> depthView;
	//	ComPtr<ID3D11DepthStencilState> depthState;

	bool initLight();
	bool initDirectX(HWND hwnd, size_t width, size_t height);
	bool initShader();
	bool toneMapOn{ true };
	bool initToneMap();
	bool initScene();
	bool upText();
	
	ToneMaping toneMap;
	Render rendText{ DXGI_FORMAT_R32G32B32A32_FLOAT };
	VertexShader vShader;
	PixelShader pShader;

	XMFLOAT4 vLDirs[NUM_OF_LIGHT];
	XMFLOAT4 vLColors[NUM_OF_LIGHT];
	float intences[NUM_OF_LIGHT];

	D3D11_VIEWPORT viewport;
	XMMATRIX matrWorld;
	XMMATRIX matrView;
	XMMATRIX matrProj;
	CamPos camPos;
	WrlCamPos wrldCamPos;

	size_t wndWidth;
	size_t wndHeight;
//	D3D_DRIVER_TYPE typeDriver;
};
