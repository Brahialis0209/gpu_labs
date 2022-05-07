#include "Graphics.h"
#include "../Loger.h"
#include "../Mode.h"


bool Graphics::Init(HWND hwnd, size_t width, size_t height) {
	wndWidth = width;
	wndHeight = height;

	if (!(initDirectX(hwnd, width, height) && initShader() && initScene() && toneMap.Init(pDevice, pContext, wndWidth, wndHeight) &&
		initLight() && rendText.Init(pDevice, wndWidth, wndHeight))) {
		return false;
	}
	return true;
}

void Graphics::RenderFrame() {
	Mode::GetUserAnnotation().BegEvent(L"Start Render.");
	float backColor[] = { 0.0f, 0.0f, 1.0f, 1.0f };
	ID3D11RenderTargetView* rendTargViewPtr = rendText.GetTextTargView();
	D3D11_VIEWPORT viewport = rendText.GetViewPort();
	pContext->ClearRenderTargetView(rendTargViewPtr, backColor);
	pContext->ClearRenderTargetView(pRendTargView.Get(), backColor);

	if (toneMapOn)
		pContext->OMSetRenderTargets(1, &rendTargViewPtr, nullptr);
	else
		pContext->OMSetRenderTargets(1, pRendTargView.GetAddressOf(), nullptr);

	pContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pContext->RSSetViewports(1, &viewport);
	ConstBuf constBuf;
	constBuf.projection = XMMatrixTranspose(matrProj);
	constBuf.world = XMMatrixTranspose(matrWorld);
	constBuf.view = XMMatrixTranspose(matrView);

	for (int m = 0; m < NUM_OF_LIGHT; ++m) {
		constBuf.vLightColor[m] = vLColors[m];
		constBuf.vLightDir[m] = vLDirs[m];
		constBuf.vLightIntencity[m] = XMFLOAT4(intences[m], 0.0f, 0.0f, 0.0f);
	}
	XMStoreFloat4(&constBuf.eye, camPos.eye);

	pContext->UpdateSubresource(constBuf1.Get(), 0, nullptr, &constBuf, 0, 0);
	pContext->IASetInputLayout(vShader.GetInputLayoutPtr());

	UINT shift = 0;
	UINT step = sizeof(Vertex);
	pContext->IASetVertexBuffers(0, 1, vBuf.GetAddressOf(), &step, &shift);
	pContext->IASetIndexBuffer(idxBuf.Get(), DXGI_FORMAT_R16_UINT, 0);
	pContext->VSSetShader(vShader.GetShaderPtr(), NULL, 0);
	pContext->VSSetConstantBuffers(0, 1, constBuf1.GetAddressOf());
	pContext->PSSetShader(pShader.GetShaderPtr(), NULL, 0);
	pContext->PSSetConstantBuffers(0, 1, constBuf1.GetAddressOf());

	pContext->DrawIndexed(6, 0, 0);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<FLOAT>(wndWidth);
	viewport.Height = static_cast<FLOAT>(wndHeight);
	
	if (toneMapOn)
		toneMap.Process(pContext, rendText.GetTextResView(), pRendTargView, viewport);
	pSwapChain->Present(1, 0);
	Mode::GetUserAnnotation().EndEvent();
}

void Graphics::ChangeLightsIntencity(size_t ind) {
	float constexpr MAX_INTENCITY_VALUE = 100.0f;
	if (ind < NUM_OF_LIGHT) {
		intences[ind] *= 10.0f;
		if (intences[ind] > MAX_INTENCITY_VALUE)
			intences[ind] = 1.0f;
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
		return toneMap.ResizeWnd(width, height) && rendText.Init(pDevice, width, height);
	}
	else
		return true;
}

bool Graphics::initDirectX(HWND hwnd, size_t width, size_t height) {
	vector<AdapterData> adapters = AdapterFinder::GetAdapters();
	if (adapters.size() < 1) {
		utils::Loger::Log("IDXGI Adapters not found");
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

	// Create Viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.Width = static_cast<FLOAT>(width);
	viewport.Height = static_cast<FLOAT>(height);
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.MaxDepth = 1;
	viewport.MinDepth = 0;

	pContext->RSSetViewports(1, &viewport);

	// Create rasterizer stae

	D3D11_RASTERIZER_DESC rasterizer_desc;
	ZeroMemory(&rasterizer_desc, sizeof(D3D11_RASTERIZER_DESC));

	rasterizer_desc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
	rasterizer_desc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
	hResult = pDevice->CreateRasterizerState(&rasterizer_desc, rasterState.GetAddressOf());
	
	if (FAILED(hResult)) {
		utils::Loger::Log(hResult, "Failed create rasterizer state.");
		return false;
	}
	return true;
}

bool Graphics::initShader()
{
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	UINT numElem = ARRAYSIZE(layout);

	if (!vShader.Init(pDevice, L"vertexshader.cso", layout, numElem)) {
		return false;
	}

	if (!pShader.Init(pDevice, L"pixelshader.cso")) {
		return false;
	}

	return true;
}

bool Graphics::upText() {
	return true;
}

bool Graphics::initToneMap() {
	return true;
}

bool Graphics::initLight()
{
	float a = 2.0f;
	// Setup our lighting parameters
	XMFLOAT4 vLightDirs[NUM_OF_LIGHT] =
	{
		XMFLOAT4(-a, 0.0f, 10.0f, 1.0f),
		XMFLOAT4(a, 0.0f, 10.0f, 1.0f),
		XMFLOAT4(0.0f, static_cast<float>(5 * a * sin(60)), 10.0f, 1.0f),
	};
	XMFLOAT4 vLightColors[NUM_OF_LIGHT] =
	{
		XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f),
		XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f),
		XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f)
	};

	float vLightIntencitys[NUM_OF_LIGHT] =
	{
		1.0f, 1.0f, 1.0f
	};

	for (int m = 0; m < NUM_OF_LIGHT; ++m) {
		vLDirs[m] = vLightDirs[m];
		vLColors[m] = vLightColors[m];
		intences[m] = vLightIntencitys[m];
	}

	return true;
}

bool Graphics::initScene() {
	Vertex vert[] =
	{
		{ XMFLOAT4(-10.0f, -10.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT4(10.0f, -10.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT4(10.0f, 10.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
		{ XMFLOAT4(-10.0f, 10.0f, 0.0f, 1.0f), XMFLOAT3(0.0f, 0.0f, 1.0f) },
	};

	D3D11_BUFFER_DESC vertBuf;
	ZeroMemory(&vertBuf, sizeof(vertBuf));
	vertBuf.Usage = D3D11_USAGE_DEFAULT;
	vertBuf.ByteWidth = sizeof(Vertex) * ARRAYSIZE(vert);
	vertBuf.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertBuf.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA vBufData;
	ZeroMemory(&vBufData, sizeof(vBufData));

	vBufData.pSysMem = vert;
	HRESULT hResult = pDevice->CreateBuffer(&vertBuf, &vBufData, vBuf.GetAddressOf());
	if (FAILED(hResult)) {
		utils::Loger::Log(hResult, "Failed create vertex buffer");
		return false;
	}

	WORD indices[] =
	{
		2,1,3,
		3,1,0,
	};

	D3D11_BUFFER_DESC idxBufDesc;
	ZeroMemory(&idxBufDesc, sizeof(idxBufDesc));
	idxBufDesc.Usage = D3D11_USAGE_DEFAULT;
	idxBufDesc.ByteWidth = sizeof(WORD) * ARRAYSIZE(indices);
	idxBufDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	idxBufDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA idxSubData;
	ZeroMemory(&idxSubData, sizeof(idxSubData));
	idxSubData.pSysMem = indices;

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
	matrWorld = XMMatrixIdentity();
	matrView = XMMatrixLookAtLH(camPos.eye, camPos.at, camPos.up);
	matrProj = XMMatrixPerspectiveFovLH(XM_PIDIV2, wndWidth / (FLOAT)wndHeight, 0.01f, 100.0f);
	return true;
}