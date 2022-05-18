#include <comdef.h>

#include "Loger.h"

namespace utils {
	void Loger::Log(string message) {
		string erMes = "Error: " + message;
		MessageBoxA(NULL, erMes.c_str(), "Error", MB_ICONERROR); 
	}

	void Loger::Log(HRESULT hResult, string str) {
		Log(hResult, wstring(str.begin(), str.end()));
	}

	void Loger::Log(HRESULT hResult, wstring message) {
		_com_error error(hResult);
		wstring erMes = L"Error: " + message + L"\n" + error.ErrorMessage();
		MessageBoxW(NULL, erMes.c_str(), L"Error", MB_ICONERROR);
	}
}
