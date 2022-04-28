#include "AdapterFinder.h"
#include "../Loger.h"

using namespace std;
using namespace Microsoft;
using namespace WRL;

vector<AdapterData> AdapterFinder::vecAdapter;

AdapterData::AdapterData(IDXGIAdapter* pAdapter) {
	this->pAdapter = pAdapter;
	HRESULT hResult = pAdapter->GetDesc(&desc);

	if (FAILED(hResult)) {
		utils::Loger::Log(hResult, "Failed get description for IDXGIAdapter");
	}
}

vector<AdapterData> AdapterFinder::GetAdapters() {
	if (!vecAdapter.empty())
		return vecAdapter;
	ComPtr<IDXGIFactory> factory;
		HRESULT hResult = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)factory.GetAddressOf());
	if (FAILED(hResult)) {
		utils::Loger::Log(hResult, "Failed create DXGIFactory for enumerating adapters.");
		exit(-1);
	}

	IDXGIAdapter* pAdapter;
	UINT idx = 0;

	while (SUCCEEDED(factory->EnumAdapters(idx, &pAdapter))) {
		vecAdapter.push_back(AdapterData(pAdapter));
		idx++;
	}

	return vecAdapter;
}