#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <vector>

#include "ToneMaping.h"

class Sphere {
public:
	static size_t constexpr SPHERE_PARTS = 100;
	static float constexpr RADIUS = 2.f;

	Sphere() = default;
	Sphere(Sphere const& other);

	bool Initialize(ComPtr<ID3D11Device> device);
	bool UpdateBuffers(ComPtr<ID3D11Device> device) { return CreateBuf(device); }
	void MoveVertexes(XMFLOAT3 direction);
		
	ID3D11Buffer* GetVertBuf() { return vertBuf.Get(); }
	ID3D11Buffer* GetIdxBuf() { return idxBuf.Get(); }
	ID3D11Buffer** GetAddressVertBuf() { return vertBuf.GetAddressOf(); }
	ID3D11Buffer** GetAddressIdxBuf() { return idxBuf.GetAddressOf(); }
	size_t GetSphereSize() const { return indSphere.size(); }

private:
	void CreateIdxVert();
	bool CreateBuf(ComPtr<ID3D11Device> device);

	std::vector<Vertex> vertSphere;
	std::vector<WORD> indSphere;

	ComPtr<ID3D11Buffer> vertBuf;
	ComPtr<ID3D11Buffer> idxBuf;

	XMFLOAT3 curPos{ 0.0f, 0.0f, 0.0f };
};