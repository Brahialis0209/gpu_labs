#pragma once

#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl/client.h>

#include "ToneMaping.h"
#include "AdapterFinder.h"
#include "Shaders.h"
#include "Sphere.h"


using namespace Microsoft;
using namespace WRL;
using namespace std;
using namespace DirectX;

enum class SHADER_TYPE {
	BRDF,
	NDF,
	GEOMETRY,
	FRESNEL
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

	void ChangeLightIntense(size_t ind);
	void ChangeToneMaping() { ToneMapOn ^= true; }
	bool ResizeWnd(size_t width, size_t height);
	void SetToneMaping(bool enable) { ToneMapOn = enable; }
	void SetPbrShaderType(SHADER_TYPE type) { typeShader = type; }

private:
	ComPtr<ID3D11Device> pDevice;
	ComPtr<ID3D11DeviceContext> pContext;
	ComPtr<ID3D11RenderTargetView> pRendTargView;
	ComPtr<IDXGISwapChain> pSwapChain;
	ComPtr<ID3D11Buffer> vBuf;
	ComPtr<ID3D11Buffer> idxBuf;
	ComPtr<ID3D11Buffer> constBuf1;
	ComPtr<ID3D11ShaderResourceView> textResourceView;
	ComPtr<ID3D11SamplerState> linSampl;
	ComPtr<ID3D11Texture2D> cubText;
	ComPtr<ID3D11ShaderResourceView> cubTextResourceView;
	ComPtr<ID3D11Texture2D> irText;
	ComPtr<ID3D11ShaderResourceView> irTextResourceView;
	ComPtr<ID3D11Buffer> lightBuf;
	ComPtr<ID3D11Buffer> matBuf;
	ComPtr<ID3D11Texture2D> pDepth;
	ComPtr<ID3D11DepthStencilView> pDepthDSV;

	bool InitLight();
	bool InitDirectX(HWND hwnd, size_t width, size_t height);
	bool InitShader();
	bool ToneMapOn{ true };
	bool InitToneMap();
	bool InitScene();
	bool UpText();
	bool LoadTexture(const char* path);
	bool CreateCubText();
	bool CreateCubFromText(size_t cubemap_size, ID3D11Texture2D* dst, ID3D11ShaderResourceView* src, VertexShader* vs, PixelShader* ps, UINT mip_slice);
	bool CreateIrradTextFromCub();
	bool CreateDeepBuf(size_t width, size_t height);

	ToneMaping toneMap;
	Render rendText{ DXGI_FORMAT_R32G32B32A32_FLOAT };
	VertexShader vertShader;
	PixelShader pixShader;
	Render rendForCub{ DXGI_FORMAT_R32G32B32A32_FLOAT };
	PixelShader brdfPixelShader;
	PixelShader ndfPixelShader;
	PixelShader geomPixelShader;
	PixelShader fresnelPixelShader;

	VertexShader cubVertexShader;
	PixelShader cubPixelShader;
	VertexShader irVertexShader;
	PixelShader irPixelShader;
	SHADER_TYPE typeShader{ SHADER_TYPE::BRDF };

	Sphere sphere;
	vector<Vertex> vertSphere;
	vector<WORD> indSphere;
	size_t vLigIntenIdx[NUM_OF_LIGHT];

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
