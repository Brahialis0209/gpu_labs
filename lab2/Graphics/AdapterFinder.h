#pragma once
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "DXGI.lib")

#include <vector>
#include <wrl/client.h>
#include <d3d11.h>

using namespace std;

class AdapterData {
public:
	AdapterData(IDXGIAdapter* pAdapter);
	IDXGIAdapter* pAdapter = nullptr;
	DXGI_ADAPTER_DESC desc;

};

class AdapterFinder {
public:
	static vector<AdapterData> GetAdapters();

private:
	static vector<AdapterData> vecAdapter;
};
