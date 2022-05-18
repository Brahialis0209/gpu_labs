#pragma once
#define _XM_NO_INTRINSICS_

#include "LuminanceCalc.h"
#include "Shaders.h"
#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include <memory>

using namespace DirectX;
using namespace Microsoft;
using namespace WRL;

#define NUM_OF_LIGHT 3

XMFLOAT3 constexpr EYE{ 0.0f, 0.0f, -10.0f };
XMFLOAT3 constexpr AT{ 0.0f, 0.0f, 1.0f };
XMFLOAT3 constexpr UP{ 0.0f, 1.0f, 0.0f };

struct Vertex {
	XMFLOAT4 pos;
	XMFLOAT3 norm;
	XMFLOAT2 tex;
};

struct CamPos {
	CamPos() : eye(XMVectorSet(EYE.x, EYE.y, EYE.z, 0.0f)), at(XMVectorSet(AT.x, AT.y, AT.z, 0.0f)), up(XMVectorSet(UP.x, UP.y, UP.z, 0.0f)) {}
	CamPos(XMVECTOR eye, XMVECTOR at, XMVECTOR up) : eye(eye), at(at), up(up) {}

	XMVECTOR eye;
	XMVECTOR at;
	XMVECTOR up;
};

struct WrlCamPos {
	float lat{ 0.0f };
	float lon{ 0.0f };
	float pos_x{ EYE.x };
	float pos_y{ EYE.y };
	float pos_z{ EYE.z };
};

__declspec(align(16))
struct ConstBuf {
	XMMATRIX world; // 16
	XMMATRIX view; // 16
	XMMATRIX projection;// 
	XMFLOAT4 eye;
};

__declspec(align(16))
struct MaterialConstBuf {
	XMFLOAT4 albedo;
	float metalness;
	float roughness;
};

__declspec(align(16))
struct LConstBuf {
	XMFLOAT4 vLDir[NUM_OF_LIGHT]; //12
	XMFLOAT4 vLightColor[NUM_OF_LIGHT]; // 12
	XMFLOAT4 vLightIntencity[NUM_OF_LIGHT];
};

__declspec(align(16))
struct LumConstBuf {
	float meanLum;
};


class ToneMaping {
public:
	bool Init(ComPtr<ID3D11Device> device, ComPtr<ID3D11DeviceContext> device_context, size_t width, size_t height);
	bool ResizeWnd(size_t width, size_t height);
	void Process(ComPtr<ID3D11DeviceContext> ctx, ComPtr<ID3D11ShaderResourceView> srcText,
		ComPtr<ID3D11RenderTargetView> renderTarget, D3D11_VIEWPORT viewport);

private:
	unique_ptr<LuminanceCalc> calc;
	ComPtr<ID3D11SamplerState> toneSampler;
	ComPtr<ID3D11Buffer> lumBuffer;
	VertexShader vShader;
	PixelShader pShader;
	size_t wndWidth;
	size_t wndHeight;
};