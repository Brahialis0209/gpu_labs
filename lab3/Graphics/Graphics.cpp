#include "Graphics.h"
#include "../Loger.h"
#include "../Mode.h"

#include "../DDSTextureLoader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"

constexpr size_t CUBE_MAP_SIZE = 512;
constexpr size_t IRRADIANCE_SIZE = 32;

bool Graphics::Init(HWND hwnd, size_t width, size_t height) {
	wndWidth = width;
	wndHeight = height;

	if (!InitDirectX(hwnd, wndWidth, wndHeight))
		return false;
	if (!InitShader())
		return false;
	if (!InitScene())
		return false;
	if (!sphere.Initialize(pDevice))
		return false;
	if (!toneMap.Init(pDevice, pContext, wndWidth, wndHeight))
		return false;
	if (!InitLight())
		return false;
	if (!rendText.Init(pDevice, wndWidth, wndHeight))
		return false;
	if (!CreateDeepBuf(wndWidth, wndHeight))
		return false;

	return true;
}

void Graphics::RenderFrame() {
	static bool flag = true;
	if (flag) {
		Mode::GetUserAnnotation().BegEvent(L"Start Render Env Cubemap.");
		CreateCubText();
		Mode::GetUserAnnotation().EndEvent();
		Mode::GetUserAnnotation().BegEvent(L"Start Render irradiance texture.");
		CreateIrradTextFromCub();
		Mode::GetUserAnnotation().EndEvent();
		flag = false;
	}

	Mode::GetUserAnnotation().BegEvent(L"Start Render.");
	float backColor[] = { 0.0f, 0.0f, 1.0f, 1.0f };
	ID3D11RenderTargetView* rendTargViewPtr = rendText.GetTextTargView();
	D3D11_VIEWPORT viewport = rendText.GetViewPort();
	pContext->ClearRenderTargetView(rendTargViewPtr, backColor);
	pContext->ClearRenderTargetView(pRendTargView.Get(), backColor);
	pContext->ClearDepthStencilView(pDepthDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	if (ToneMapOn)
		pContext->OMSetRenderTargets(1, &rendTargViewPtr, pDepthDSV.Get());
	else
		pContext->OMSetRenderTargets(1, pRendTargView.GetAddressOf(), pDepthDSV.Get());

	pContext->RSSetViewports(1, &viewport);
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ConstBuf constBuf;
	constBuf.projection = XMMatrixTranspose(matrProj);
	constBuf.world = XMMatrixTranspose(matrWorld);
	constBuf.view = XMMatrixTranspose(matrView);
	constBuf.eye = XMFLOAT4(wrldCamPos.pos_x, wrldCamPos.pos_y, wrldCamPos.pos_z, 0);
	pContext->UpdateSubresource(constBuf1.Get(), 0, nullptr, &constBuf, 0, 0);

	LConstBuf lb;
	for (int m = 0; m < NUM_OF_LIGHT; ++m) {
		lb.vLightColor[m] = vLColors[m];
		lb.vLDir[m] = vLDirs[m];
		lb.vLightIntencity[m] = XMFLOAT4(intences[m], 0.0f, 0.0f, 0.0f);
	}
	//XMStoreFloat4(&constBuf.eye, camPos.eye);

	pContext->UpdateSubresource(lightBuf.Get(), 0, nullptr, &lb, 0, 0);
	pContext->IASetInputLayout(vertShader.GetInputLayoutPtr());

	UINT shift = 0;
	UINT step = sizeof(Vertex);
	pContext->IASetVertexBuffers(0, 1, vBuf.GetAddressOf(), &step, &shift);
	pContext->IASetIndexBuffer(idxBuf.Get(), DXGI_FORMAT_R16_UINT, 0);
	pContext->VSSetShader(vertShader.GetShaderPtr(), NULL, 0);
	pContext->VSSetConstantBuffers(0, 1, constBuf1.GetAddressOf());
	pContext->PSSetShader(pixShader.GetShaderPtr(), NULL, 0);
	pContext->PSSetConstantBuffers(0, 1, constBuf1.GetAddressOf());

	pContext->PSSetConstantBuffers(1, 1, lightBuf.GetAddressOf());
	pContext->PSSetConstantBuffers(2, 1, matBuf.GetAddressOf());
	pContext->PSSetShaderResources(0, 1, textResourceView.GetAddressOf());
	pContext->PSSetSamplers(0, 1, linSampl.GetAddressOf());
	pContext->DrawIndexed(indSphere.size(), 0, 0);

	switch (typeShader) {
	case SHADER_TYPE::BRDF:
		pContext->PSSetShaderResources(1, 1, irTextResourceView.GetAddressOf());
		pContext->PSSetShader(brdfPixelShader.GetShaderPtr(), NULL, 0);
		break;
	case SHADER_TYPE::NDF:
		pContext->PSSetShader(ndfPixelShader.GetShaderPtr(), NULL, 0);
		break;
	case SHADER_TYPE::GEOMETRY:
		pContext->PSSetShader(geomPixelShader.GetShaderPtr(), NULL, 0);
		break;
	case SHADER_TYPE::FRESNEL:
		pContext->PSSetShader(fresnelPixelShader.GetShaderPtr(), NULL, 0);
	}

	MaterialConstBuf mb;
	int constexpr SPHERES_COUNT = 8;
	for (int i = 0; i < SPHERES_COUNT; ++i) {
		for (int j = 0; j < SPHERES_COUNT; ++j)	{
			constBuf.world = XMMatrixTranspose(XMMatrixTranslation(
				(i - SPHERES_COUNT / 2) * Sphere::RADIUS * 3,
				(j - SPHERES_COUNT / 2) * Sphere::RADIUS * 3,
				0
			));
			mb.metalness = static_cast<float>(j) / (SPHERES_COUNT - 1);
			mb.roughness = static_cast<float>(i) / (SPHERES_COUNT - 1);
			static const XMFLOAT4 default_albedo = { 0.85f, 0.6f, 0.56f, 1.0f };  //color
			mb.albedo = default_albedo;

			pContext->IASetVertexBuffers(0, 1, sphere.GetAddressVertBuf(), &step, &shift);
			pContext->IASetIndexBuffer(sphere.GetIdxBuf(), DXGI_FORMAT_R16_UINT, 0);
			pContext->UpdateSubresource(constBuf1.Get(), 0, nullptr, &constBuf, 0, 0);
			pContext->UpdateSubresource(matBuf.Get(), 0, nullptr, &mb, 0, 0);
			pContext->DrawIndexed(sphere.GetSphereSize(), 0, 0);
		}
	}

	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<FLOAT>(wndWidth);
	viewport.Height = static_cast<FLOAT>(wndHeight);

	if (ToneMapOn)
		toneMap.Process(pContext, rendText.GetTextResView(), pRendTargView, viewport);
	pSwapChain->Present(1, 0);
	Mode::GetUserAnnotation().EndEvent();
}

void Graphics::ChangeLightIntense(size_t ind) {
	static constexpr float arrIntences[] = {0.0f, 1.0f, 10.0f, 50.0f, 100.0f};
	static constexpr size_t INTENCITY_SEQUENCE_SIZE = sizeof(arrIntences) / sizeof(float);
	//float constexpr MAX_INTENCITY_VALUE = 100.0f;
	if (ind < NUM_OF_LIGHT) {
		++vLigIntenIdx[ind];
		vLigIntenIdx[ind] %= INTENCITY_SEQUENCE_SIZE;
		intences[ind] = arrIntences[vLigIntenIdx[ind]];
	}
}

bool Graphics::ResizeWnd(size_t width, size_t height) {
	if (pSwapChain) {
		pContext->OMSetRenderTargets(0, 0, 0);
		pRendTargView->Release();
		pContext->Flush();

		wndWidth = width;
		wndHeight = height;
		matrProj = XMMatrixPerspectiveFovLH(XM_PIDIV2, static_cast<FLOAT>(wndWidth) / static_cast<FLOAT>(wndHeight), 0.01f, 100.0f);
		HRESULT hResult = pSwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
		if (FAILED(hResult)) {
			utils::Loger::Log(hResult, "Falied to resize buffers");
			return false;
		}

		viewport.Height = static_cast<FLOAT>(height);
		viewport.Width = static_cast<FLOAT>(width);
		ID3D11Texture2D* backBuf = nullptr;
		hResult = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuf));
		if (FAILED(hResult)) {
			utils::Loger::Log(hResult, "Failed create swap chain");
			return false;
		}
		hResult = pDevice->CreateRenderTargetView(backBuf, nullptr, pRendTargView.GetAddressOf());
		if (FAILED(hResult)) {
			utils::Loger::Log(hResult, "Failed create render target view");
			return false;
		}
		backBuf->Release();
		if (!CreateDeepBuf(width, height))
			return false;
		return toneMap.ResizeWnd(width, height) && rendText.Init(pDevice, width, height);
	}
	else
		return true;
}

bool Graphics::InitDirectX(HWND hwnd, size_t width, size_t height) {
	vector<AdapterData> adapters = AdapterFinder::GetAdapters();
	if (adapters.size() < 1) {
		utils::Loger::Log("IDXGI Adapter has not found");
		return false;
	}

	DXGI_SWAP_CHAIN_DESC swapChain;
	ZeroMemory(&swapChain, sizeof(swapChain));

	swapChain.BufferDesc.Width = 0;
	swapChain.BufferDesc.Height = 0;
	swapChain.BufferDesc.RefreshRate.Numerator = 60;
	swapChain.BufferDesc.RefreshRate.Denominator = 1;
	swapChain.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChain.SampleDesc.Count = 1;
	swapChain.SampleDesc.Quality = 0;
	swapChain.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChain.BufferCount = 2;
	swapChain.OutputWindow = hwnd;
	swapChain.Windowed = TRUE;
	swapChain.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	D3D_FEATURE_LEVEL featureLvls[] =
	{
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
	};
	UINT numFeatureLvls = ARRAYSIZE(featureLvls);

	UINT devFlag = 0;
	if (Mode::GetDebug())
		devFlag |= D3D11_CREATE_DEVICE_DEBUG;

	HRESULT hResult = D3D11CreateDeviceAndSwapChain(
		adapters[0].pAdapter, // adapter pointer
		D3D_DRIVER_TYPE_UNKNOWN,
		NULL, // For softfare driver
		devFlag, // flags for runtime layers
		&featureLvls[1], // feature levels array
		numFeatureLvls - 1, //  number of feature levels
		D3D11_SDK_VERSION,
		&swapChain, //swapchain description
		pSwapChain.GetAddressOf(), // swapchain address
		pDevice.GetAddressOf(), // device address
		NULL, // supported feature level
		pContext.GetAddressOf() // device context address
	);

	if (FAILED(hResult)) {
		utils::Loger::Log(hResult, "Failed create device and swapchain");
		return false;
	}

	Mode::GetUserAnnotation().SetDeviceContext(pContext);

	ComPtr<ID3D11Texture2D> backBuf;
	hResult = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuf.GetAddressOf()));
	if (FAILED(hResult)) {
		utils::Loger::Log(hResult, "GetBuffer failed");
		return false;
	}

	hResult = pDevice->CreateRenderTargetView(backBuf.Get(), nullptr, pRendTargView.GetAddressOf());
	if (FAILED(hResult)) {
		utils::Loger::Log(hResult, "Failed create render target view");
		return false;
	}

	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.Width = static_cast<FLOAT>(width);
	viewport.Height = static_cast<FLOAT>(height);
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MaxDepth = 1;
	viewport.MinDepth = 0;

	pContext->RSSetViewports(1, &viewport);

	return true;
}

bool Graphics::InitShader() {
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA}
	};

	UINT numElem = ARRAYSIZE(layout);
	if (!vertShader.Init(pDevice, L"v_shader.cso", layout, numElem))
		return false;

	if (!brdfPixelShader.Init(pDevice, L"brdf_p_shader.cso"))
		return false;

	if (!geomPixelShader.Init(pDevice, L"geom_p_shader.cso"))
		return false;

	if (!ndfPixelShader.Init(pDevice, L"ndf_p_shader.cso"))
		return false;

	if (!fresnelPixelShader.Init(pDevice, L"fresnel_p_shader.cso"))
		return false;

	if (!pixShader.Init(pDevice, L"p_shader.cso"))
		return false;

	if (!cubPixelShader.Init(pDevice, L"cub_p_shader.cso"))
		return false;

	if (!cubVertexShader.Init(pDevice, L"cub_v_shader.cso", layout, numElem))
		return false;

	if (!irPixelShader.Init(pDevice, L"ir_p_shader.cso"))
		return false;

	if (!irVertexShader.Init(pDevice, L"ir_v_shader.cso", layout, numElem))
		return false;

	return true;
}

bool Graphics::UpText() {
	return true;
}

bool Graphics::InitToneMap() {
	return true;
}

bool Graphics::InitLight() {
	// Setup our lighting parameters
	XMFLOAT4 vLightDirs[NUM_OF_LIGHT] =
	{
		XMFLOAT4(0.0f, 0.0f, -10.0f, 1.0f),
		XMFLOAT4(0.0f, 0.0f, 10.0f, 1.0f),
		XMFLOAT4(5.0f, 5.0f, -5.0f, 1.0f),
	};
	XMFLOAT4 vLightColors[NUM_OF_LIGHT] =
	{
		XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f),
		XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f),
		XMFLOAT4(0.0f, 0.0f, 0.5f, 1.0f)
	};

	float vLightIntencitys[NUM_OF_LIGHT] =
	{
		1.0f, 1.0f, 1.0f
	};

	for (int m = 0; m < NUM_OF_LIGHT; ++m) {
		vLDirs[m] = vLightDirs[m];
		vLColors[m] = vLightColors[m];
		intences[m] = vLightIntencitys[m];
		vLigIntenIdx[m] = 0;
	}

	return true;
}

bool Graphics::InitScene() {
	XMFLOAT3 curPos{ 0.0f, 0.0f, 0.0f };
	static size_t constexpr SPHERE_PARTS = 100;
	static float constexpr RADIUS = 50.f;
	float deltaTetha = XM_PI / SPHERE_PARTS; //thetta
	float deltaPhi = 2 * XM_PI / SPHERE_PARTS;
	size_t layer = 0;
	size_t curLayerSize = 0;
	size_t prevLayerSize = 0;
	float tetha = 0;
	for (; layer <= SPHERE_PARTS; tetha += deltaTetha, ++layer) {
		prevLayerSize = curLayerSize;
		if (tetha > XM_PI)
			tetha = XM_PI;
		curLayerSize = SPHERE_PARTS + 1;
		int ind = 0;
		for (float phi = 0; phi < 2 * XM_PI; phi += deltaPhi) {
			float n_x = sin(tetha) * sin(phi);
			float n_y = cos(tetha);
			float n_z = sin(tetha) * cos(phi);
			float x = RADIUS * n_x + curPos.x;
			float y = RADIUS * n_y + curPos.y;
			float z = RADIUS * n_z + curPos.z;
			vertSphere.push_back({ XMFLOAT4(x, y, z, 1.0f), XMFLOAT3(n_x, n_y, n_z), 
				XMFLOAT2(static_cast<float>(ind) / (SPHERE_PARTS), static_cast<float>(layer) / (SPHERE_PARTS)) });
			++ind;
		}
		{
			float n_x = sin(tetha) * sin(0.0f);
			float n_y = cos(tetha);
			float n_z = sin(tetha) * cos(0.0f);
			float x = RADIUS * n_x + curPos.x;
			float y = RADIUS * n_y + curPos.y;
			float z = RADIUS * n_z + curPos.z;

			vertSphere.push_back({ XMFLOAT4(x, y, z, 1.0f), XMFLOAT3(n_x, n_y, n_z), 
				XMFLOAT2(static_cast<float>(ind) / (SPHERE_PARTS), static_cast<float>(layer) / (SPHERE_PARTS)) });
		}
		if (layer > 0) {
			size_t cur_layer_start = vertSphere.size() - curLayerSize;
			size_t prev_layer_start = cur_layer_start - prevLayerSize;
			for (size_t i = 0; i < SPHERE_PARTS; ++i) {
				indSphere.push_back(cur_layer_start + i + 1);
				indSphere.push_back(cur_layer_start + i);
				indSphere.push_back(prev_layer_start + i);

				indSphere.push_back(prev_layer_start + i + 1);
				indSphere.push_back(cur_layer_start + i + 1);
				indSphere.push_back(prev_layer_start + i);
			}

		}
	}

	D3D11_BUFFER_DESC vertBuf;
	ZeroMemory(&vertBuf, sizeof(vertBuf));
	vertBuf.Usage = D3D11_USAGE_DEFAULT;
	vertBuf.ByteWidth = sizeof(Vertex) * vertSphere.size();
	vertBuf.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertBuf.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA vBufData;
	ZeroMemory(&vBufData, sizeof(vBufData));
	vBufData.pSysMem = vertSphere.data();

	//	vBufData.pSysMem = vert;
	HRESULT hResult = pDevice->CreateBuffer(&vertBuf, &vBufData, vBuf.GetAddressOf());
	if (FAILED(hResult)) {
		utils::Loger::Log(hResult, "Failed create vertex buffer");
		return false;
	}

	D3D11_BUFFER_DESC idxBufDesc;
	ZeroMemory(&idxBufDesc, sizeof(idxBufDesc));
	idxBufDesc.Usage = D3D11_USAGE_DEFAULT;
	idxBufDesc.ByteWidth = sizeof(WORD) * indSphere.size();
	idxBufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	idxBufDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA idxSubData;
	ZeroMemory(&idxSubData, sizeof(idxSubData));
	idxSubData.pSysMem = indSphere.data();

	hResult = pDevice->CreateBuffer(&idxBufDesc, &idxSubData, idxBuf.GetAddressOf());
	if (FAILED(hResult)) {
		utils::Loger::Log(hResult, "Failed create index buffer");
		return false;
	}

	D3D11_BUFFER_DESC constBufDesc;
	ZeroMemory(&constBufDesc, sizeof(constBufDesc));
	constBufDesc.Usage = D3D11_USAGE_DEFAULT;
	constBufDesc.ByteWidth = sizeof(ConstBuf);
	constBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBufDesc.CPUAccessFlags = 0;
	hResult = pDevice->CreateBuffer(&constBufDesc, nullptr, constBuf1.GetAddressOf());
	if (FAILED(hResult)) {
		utils::Loger::Log(hResult, "Failed create consatnt buffer.");
		return false;
	}
	
	ZeroMemory(&constBufDesc, sizeof(constBufDesc));
	constBufDesc.Usage = D3D11_USAGE_DEFAULT;
	constBufDesc.ByteWidth = sizeof(ConstBuf);
	constBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBufDesc.CPUAccessFlags = 0;
	hResult = pDevice->CreateBuffer(&constBufDesc, nullptr, lightBuf.GetAddressOf());
	if (FAILED(hResult)) {
		utils::Loger::Log(hResult, "Failed to create consatnt buffer.");
		return false;
	}

	ZeroMemory(&constBufDesc, sizeof(constBufDesc));
	constBufDesc.Usage = D3D11_USAGE_DEFAULT;
	constBufDesc.ByteWidth = sizeof(ConstBuf);
	constBufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	constBufDesc.CPUAccessFlags = 0;
	hResult = pDevice->CreateBuffer(&constBufDesc, nullptr, matBuf.GetAddressOf());
	if (FAILED(hResult)) {
		utils::Loger::Log(hResult, "Failed to create consatnt buffer.");
		return false;
	}

	matrWorld = XMMatrixIdentity();
	matrView = XMMatrixLookAtLH(camPos.eye, camPos.at, camPos.up);
	matrProj = XMMatrixPerspectiveFovLH(XM_PIDIV2, wndWidth / (FLOAT)wndHeight, 0.01f, 100.0f);

	if (!LoadTexture("pink_sky.hdr"))
		return false;

	if (!rendForCub.Init(pDevice, CUBE_MAP_SIZE, CUBE_MAP_SIZE))
		return false;

	D3D11_SAMPLER_DESC sampDesc;
	ZeroMemory(&sampDesc, sizeof(sampDesc));
	sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampDesc.MinLOD = 0;
	sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hResult = pDevice->CreateSamplerState(&sampDesc, linSampl.GetAddressOf());
	if (FAILED(hResult))
		return false;


	return true;
}

bool Graphics::LoadTexture(const char* path) {
	HRESULT hResult;
	int w, h, n;
	float* data = stbi_loadf(path, &w, &h, &n, STBI_rgb_alpha);
	if (data == nullptr)
		return false;
	ComPtr<ID3D11Texture2D> texture;
	ID3D11Device* device = pDevice.Get();

	CD3D11_TEXTURE2D_DESC textDesc(DXGI_FORMAT_R32G32B32A32_FLOAT, w, h, 1, 1, D3D11_BIND_SHADER_RESOURCE);
	D3D11_SUBRESOURCE_DATA init_data;
	init_data.pSysMem = data;
	init_data.SysMemPitch = 4 * w * sizeof(float);
	hResult = device->CreateTexture2D(&textDesc, &init_data, texture.GetAddressOf());
	stbi_image_free(data);
	if (FAILED(hResult))
		return false;

	CD3D11_SHADER_RESOURCE_VIEW_DESC srvd(D3D11_SRV_DIMENSION_TEXTURE2D, textDesc.Format);
	hResult = device->CreateShaderResourceView(texture.Get(), &srvd, textResourceView.GetAddressOf());
	if (FAILED(hResult))
		return false;

	return true;
}

bool Graphics::CreateCubText() {
	CD3D11_TEXTURE2D_DESC textDesc = CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_R32G32B32A32_FLOAT, CUBE_MAP_SIZE, CUBE_MAP_SIZE, 6, 0, D3D11_BIND_SHADER_RESOURCE | 
		D3D11_BIND_RENDER_TARGET, D3D11_USAGE_DEFAULT, 0, 1, 0, D3D11_RESOURCE_MISC_TEXTURECUBE | D3D11_RESOURCE_MISC_GENERATE_MIPS);
	HRESULT hResult = pDevice->CreateTexture2D(&textDesc, nullptr, cubText.GetAddressOf());
	if (FAILED(hResult))
		return false;

	if (!CreateCubFromText(CUBE_MAP_SIZE, cubText.Get(), textResourceView.Get(), &cubVertexShader, &cubPixelShader, 0))
		return false;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvd = CD3D11_SHADER_RESOURCE_VIEW_DESC(D3D11_SRV_DIMENSION_TEXTURECUBE, textDesc.Format);
	hResult = pDevice->CreateShaderResourceView(cubText.Get(), &srvd, cubTextResourceView.GetAddressOf());
	if (FAILED(hResult))
		return false;

	pContext->GenerateMips(textResourceView.Get());

	return false;

}

bool Graphics::CreateCubFromText(size_t cubemap_size, ID3D11Texture2D* dst, ID3D11ShaderResourceView* src, VertexShader* vs, PixelShader* ps, UINT mip_slice) {
	HRESULT hResult = S_OK;
	ComPtr<ID3D11Buffer> vertBuf;
	ComPtr<ID3D11Buffer> idxBuf;
	Vertex vertices[4];
	WORD indices[6] = {
		0, 1, 2,
		2, 3, 0
	};
	UINT indexCount = 6;


	D3D11_SUBRESOURCE_DATA initData;
	ZeroMemory(&initData, sizeof(D3D11_SUBRESOURCE_DATA));
	CD3D11_BUFFER_DESC vbd(sizeof(Vertex) * 4, D3D11_BIND_VERTEX_BUFFER);
	CD3D11_BUFFER_DESC ibd(sizeof(WORD) * indexCount, D3D11_BIND_INDEX_BUFFER);
	initData.pSysMem = indices;
	hResult = pDevice->CreateBuffer(&ibd, &initData, idxBuf.GetAddressOf());
	if (FAILED(hResult))
		return false;


	ComPtr<ID3D11Texture2D> texture;
	D3D11_TEXTURE2D_DESC dtd;
	dst->GetDesc(&dtd);
	D3D11_TEXTURE2D_DESC textDesc = CD3D11_TEXTURE2D_DESC(dtd.Format, cubemap_size, cubemap_size, 1, 1, D3D11_BIND_RENDER_TARGET);
	hResult = pDevice->CreateTexture2D(&textDesc, nullptr, &texture);
	if (FAILED(hResult))
		return false;

	// Create render target view
	CD3D11_RENDER_TARGET_VIEW_DESC rtvd(D3D11_RTV_DIMENSION_TEXTURE2D, textDesc.Format);
	ComPtr<ID3D11RenderTargetView> render_target_ptr;
	hResult = pDevice->CreateRenderTargetView(texture.Get(), &rtvd, &render_target_ptr);
	if (FAILED(hResult))
		return false;

	D3D11_VIEWPORT viewport;
	viewport.Width = static_cast<FLOAT>(cubemap_size);
	viewport.Height = static_cast<FLOAT>(cubemap_size);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;


	ConstBuf cb;
	cb.world = XMMatrixTranspose(XMMatrixIdentity());
	cb.projection = XMMatrixTranspose(XMMatrixPerspectiveFovLH(XM_PIDIV2, 1, 0.2f, 0.8f));

	float bgcolor[] = { 0.0f, 1.0f, 0.0f, 1.0f };

	ID3D11RenderTargetView* render_target = render_target_ptr.Get();

	pContext->ClearRenderTargetView(render_target, bgcolor);
	pContext->OMSetRenderTargets(1, &render_target, nullptr);
	pContext->RSSetViewports(1, &viewport);
	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	pContext->UpdateSubresource(constBuf1.Get(), 0, nullptr, &cb, 0, 0);
	pContext->IASetInputLayout(vs->GetInputLayoutPtr());

	UINT step = sizeof(Vertex);
	UINT shift = 0;

	pContext->IASetIndexBuffer(idxBuf.Get(), DXGI_FORMAT_R16_UINT, 0);

	pContext->VSSetShader(vs->GetShaderPtr(), NULL, 0);
	pContext->VSSetConstantBuffers(0, 1, constBuf1.GetAddressOf());

	pContext->PSSetShader(ps->GetShaderPtr(), NULL, 0);

	pContext->PSSetShaderResources(0, 1, &src);
	pContext->PSSetConstantBuffers(0, 1, constBuf1.GetAddressOf());
	pContext->PSSetSamplers(0, 1, linSampl.GetAddressOf());

	static const XMFLOAT3 squareBotLeft[6] = {
		XMFLOAT3(0.5f, -0.5f, 0.5f),   //  x+
		XMFLOAT3(-0.5f, -0.5f, -0.5f), //  x-   
		XMFLOAT3(-0.5f, 0.5f, 0.5f),   //  y+
		XMFLOAT3(-0.5f, -0.5f, -0.5f), //  y-
		XMFLOAT3(-0.5f, -0.5f, 0.5f),  //  z+
		XMFLOAT3(0.5f, -0.5f, -0.5f)   //  z-
	};

	static const XMFLOAT3 squareTopRight[6] = {
		XMFLOAT3(0.5f, 0.5f, -0.5f),	//  x+
		XMFLOAT3(-0.5f, 0.5f, 0.5f),	//  x-   
		XMFLOAT3(0.5f, 0.5f, -0.5f),	//  y+
		XMFLOAT3(0.5f, -0.5f, 0.5f),	//  y-
		XMFLOAT3(0.5f, 0.5f, 0.5f),		//  z+
		XMFLOAT3(-0.5f, 0.5f, -0.5f)	//  z-
	};

	static const XMVECTOR arrAt[6] = {
	XMVectorSet(1, 0, 0, 0),		//  x+
	XMVectorSet(-1, 0, 0, 0),		//  x-
	XMVectorSet(0, 1, 0, 0),		//  y+
	XMVectorSet(0, -1, 0, 0),		//  y-
	XMVectorSet(0, 0, 1, 0),		//  z+
	XMVectorSet(0, 0, -1, 0)		//  z-
	};

	static const XMVECTOR arrUp[6] = {
		XMVectorSet(0, 1, 0, 0),	//  x+
		XMVectorSet(0, 1, 0, 0),	//  x-
		XMVectorSet(0, 0, -1, 0),	//  y+
		XMVectorSet(0, 0, 1, 0),	//  y-
		XMVectorSet(0, 1, 0, 0),	//  z+
		XMVectorSet(0, 1, 0, 0)		//  z-
	};

	D3D11_BOX box = CD3D11_BOX(0, 0, 0, cubemap_size, cubemap_size, 1);

	for (UINT i = 0; i < 6; ++i)
	{
		if (squareBotLeft[i].x == squareTopRight[i].x)
		{
			vertices[0].pos = { squareBotLeft[i].x, squareBotLeft[i].y, squareBotLeft[i].z , 1.0 };
			vertices[1].pos = { squareBotLeft[i].x, squareTopRight[i].y, squareBotLeft[i].z , 1.0 };
			vertices[2].pos = { squareTopRight[i].x, squareTopRight[i].y, squareTopRight[i].z , 1.0 };
			vertices[3].pos = { squareTopRight[i].x, squareBotLeft[i].y, squareTopRight[i].z , 1.0 };
		}

		if (squareBotLeft[i].y == squareTopRight[i].y)
		{
			vertices[0].pos = { squareBotLeft[i].x, squareBotLeft[i].y, squareBotLeft[i].z , 1.0 };
			vertices[1].pos = { squareBotLeft[i].x, squareBotLeft[i].y, squareTopRight[i].z , 1.0 };
			vertices[2].pos = { squareTopRight[i].x, squareBotLeft[i].y, squareTopRight[i].z , 1.0 };
			vertices[3].pos = { squareTopRight[i].x, squareBotLeft[i].y, squareBotLeft[i].z, 1.0 };
		}

		if (squareBotLeft[i].z == squareTopRight[i].z)
		{
			vertices[0].pos = { squareBotLeft[i].x, squareBotLeft[i].y, squareBotLeft[i].z, 1.0 };
			vertices[1].pos = { squareBotLeft[i].x, squareTopRight[i].y, squareBotLeft[i].z, 1.0 };
			vertices[2].pos = { squareTopRight[i].x, squareTopRight[i].y, squareBotLeft[i].z, 1.0 };
			vertices[3].pos = { squareTopRight[i].x, squareBotLeft[i].y, squareBotLeft[i].z, 1.0 };
		}


		initData.pSysMem = vertices;
		hResult = pDevice->CreateBuffer(&vbd, &initData, vertBuf.ReleaseAndGetAddressOf());
		if (FAILED(hResult))
			return false;

		pContext->IASetVertexBuffers(0, 1, vertBuf.GetAddressOf(), &step, &shift);
		cb.view = XMMatrixTranspose(XMMatrixLookAtLH(XMVectorSet(0, 0, 0, 0), arrAt[i], arrUp[i]));
		pContext->UpdateSubresource(constBuf1.Get(), 0, nullptr, &cb, 0, 0);
		pContext->ClearRenderTargetView(render_target, bgcolor);
		pContext->DrawIndexed(indexCount, 0, 0);

		pContext->CopySubresourceRegion(dst, D3D11CalcSubresource(mip_slice, i, dtd.MipLevels), 0, 0, 0, texture.Get(), 0, &box);
	}

	ID3D11ShaderResourceView* nullsrv[] = { nullptr };
	pContext->PSSetShaderResources(0, 1, nullsrv);
	return true;
}

bool Graphics::CreateIrradTextFromCub() {
	CD3D11_TEXTURE2D_DESC textDesc = CD3D11_TEXTURE2D_DESC(DXGI_FORMAT_R32G32B32A32_FLOAT, IRRADIANCE_SIZE, IRRADIANCE_SIZE, 6, 1, 
		D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET,
		D3D11_USAGE_DEFAULT, 0, 1, 0, D3D11_RESOURCE_MISC_TEXTURECUBE | D3D11_RESOURCE_MISC_GENERATE_MIPS);
	HRESULT hResult = pDevice->CreateTexture2D(&textDesc, nullptr, irText.GetAddressOf());
	if (FAILED(hResult))
		return false;

	if (!CreateCubFromText(IRRADIANCE_SIZE, irText.Get(), cubTextResourceView.Get(), &irVertexShader, &irPixelShader, 0))
		return false;

	D3D11_SHADER_RESOURCE_VIEW_DESC srvd = CD3D11_SHADER_RESOURCE_VIEW_DESC(D3D11_SRV_DIMENSION_TEXTURECUBE, textDesc.Format, 0, 1);
	hResult = pDevice->CreateShaderResourceView(irText.Get(), &srvd, irTextResourceView.GetAddressOf());
	if (FAILED(hResult))
		return false;
	return false;

}

bool Graphics::CreateDeepBuf(size_t width, size_t height)
{
	HRESULT hResult;
	D3D11_TEXTURE2D_DESC depthDesc = {};
	depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthDesc.ArraySize = 1;
	depthDesc.MipLevels = 1;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.Height = height;
	depthDesc.Width = width;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = 0;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;

	hResult = pDevice->CreateTexture2D(&depthDesc, NULL, &pDepth);
	if (SUCCEEDED(hResult))
		hResult = pDevice->CreateDepthStencilView(pDepth.Get(), NULL, &pDepthDSV);

	return SUCCEEDED(hResult);
}