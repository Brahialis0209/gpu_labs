#include "Sphere.h"
#include "../Loger.h"

Sphere::Sphere(Sphere const& other){}

bool Sphere::Initialize(ComPtr<ID3D11Device> device) {
	CreateIdxVert();
	return CreateBuf(device);
}

void Sphere::MoveVertexes(XMFLOAT3 direction) {
	for (auto& [position, _, tmp] : vertSphere) {
		position.x += direction.x;
		position.y += direction.y;
		position.z += direction.z;
	}
}

bool Sphere::CreateBuf(ComPtr<ID3D11Device> device) {
	D3D11_BUFFER_DESC vBufDesc;
	ZeroMemory(&vBufDesc, sizeof(vBufDesc));

	vBufDesc.Usage = D3D11_USAGE_DEFAULT;
	vBufDesc.ByteWidth = sizeof(Vertex) * vertSphere.size();
	vBufDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vBufDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA vertBufData;
	ZeroMemory(&vertBufData, sizeof(vertBufData));
	vertBufData.pSysMem = vertSphere.data();

	if (vertBuf.Get()) {
		vertBuf->Release();
		vertBuf.Reset();
	}
	HRESULT hResult = device->CreateBuffer(&vBufDesc, &vertBufData, vertBuf.GetAddressOf());
	if (FAILED(hResult)) {
		utils::Loger::Log(hResult, "Failed to create sphere vertex buffer");
		return false;
	}

	D3D11_BUFFER_DESC idxBufDesc;
	ZeroMemory(&idxBufDesc, sizeof(idxBufDesc));
	idxBufDesc.Usage = D3D11_USAGE_DEFAULT;
	idxBufDesc.ByteWidth = sizeof(WORD) * indSphere.size();
	idxBufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	idxBufDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA sbrsData;
	ZeroMemory(&sbrsData, sizeof(sbrsData));
	sbrsData.pSysMem = indSphere.data();
	if (idxBuf.Get()) {
		idxBuf->Release();
		idxBuf.Reset();
	}
	hResult = device->CreateBuffer(&idxBufDesc, &sbrsData, idxBuf.GetAddressOf());
	if (FAILED(hResult)) {
		utils::Loger::Log(hResult, "Failed to create sphere index buffer");
		return false;
	}

	return true;
}

void Sphere::CreateIdxVert() {
	using namespace DirectX;
	float deltaTetha = XM_PI / SPHERE_PARTS; //thetta
	float deltaPhi = 2 * XM_PI / SPHERE_PARTS;
	size_t layer = 0;
	size_t curLayerSize = 0;
	size_t prevLayerSize = 0;
	for (float tetha = 0; tetha < XM_PI; tetha += deltaTetha) {
		prevLayerSize = curLayerSize;
		if (layer == 0 || layer == SPHERE_PARTS - 1) {
			float n_x = sin(tetha) * sin(0);
			float n_y = cos(tetha);
			float n_z = sin(tetha) * cos(0);
			float x = RADIUS * n_x + curPos.x;
			float y = RADIUS * n_y + curPos.y;
			float z = RADIUS * n_z + curPos.z;
			vertSphere.push_back({ XMFLOAT4(x, y, z, 1.0f), XMFLOAT3(n_x, n_y, n_z) });
			curLayerSize = 1;
		}
		else {
			curLayerSize = SPHERE_PARTS;
			for (float phi = 0; phi < 2 * XM_PI; phi += deltaPhi) {
				float n_x = sin(tetha) * sin(phi);
				float n_y = cos(tetha);
				float n_z = sin(tetha) * cos(phi);
				float x = RADIUS * n_x + curPos.x;
				float y = RADIUS * n_y + curPos.y;
				float z = RADIUS * n_z + curPos.z;
				vertSphere.push_back({ XMFLOAT4(x, y, z, 1.0f), XMFLOAT3(n_x, n_y, n_z) });
			}
		}
		if (layer > 0) {
			size_t start = vertSphere.size() - curLayerSize;
			size_t prev = start - prevLayerSize;
			for (size_t i = 0; i < SPHERE_PARTS; ++i) {
				indSphere.push_back(prev + i % prevLayerSize);
				indSphere.push_back(start + i % curLayerSize);
				indSphere.push_back(start + (i + 1) % curLayerSize);
				indSphere.push_back(prev + i % prevLayerSize);
				indSphere.push_back(start + (i + 1) % curLayerSize);
				indSphere.push_back(prev + (i + 1) % prevLayerSize);
			}

		}
		++layer;
	}
}