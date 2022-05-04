#pragma once
#include <wrl/client.h>
#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <memory>

using namespace Microsoft;
using namespace WRL;

class UserAnnotation {
private:
    UserAnnotation(UserAnnotation const&) = delete;
    UserAnnotation(UserAnnotation&&) = delete;

    void delAnnot();
public:
    UserAnnotation() = default;
    ~UserAnnotation();

    void BegEvent(LPCWSTR name);
    void EndEvent();
    bool SetDeviceContext(ComPtr<ID3D11DeviceContext> devCtx);

private:
    ID3DUserDefinedAnnotation* inst{ nullptr };
};


class Mode {
private:
    Mode() = default;
    Mode(Mode&&) = delete;
    Mode(Mode const&) = delete;

public:
    UserAnnotation annot;
    bool dbgMode{ false };
    static UserAnnotation& GetUserAnnotation();
    static bool GetDebug();
    static Mode& ReturnExample();
    
};
