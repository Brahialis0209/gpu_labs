#pragma once
#include <Windows.h>
#include <string>

using namespace std;

namespace utils {
	class Loger {
	public:
		void static Log(string message);
		void static Log(HRESULT hResult, wstring message);
		void static Log(HRESULT hResult, string message);
	};
}
