#include "Loger.h"
#include "Mode.h"

UserAnnotation::~UserAnnotation() {
    delAnnot();
}

void UserAnnotation::delAnnot() {
    if (inst) {
        inst->Release();
    }
}

void UserAnnotation::BegEvent(LPCWSTR event) {
    if (inst) {
        inst->BeginEvent(event);
    }
}

void UserAnnotation::EndEvent() {
    if (inst) {
        inst->EndEvent();
    }
}

bool UserAnnotation::SetDeviceContext(ComPtr<ID3D11DeviceContext> devCtx) {
    HRESULT hResult = devCtx->QueryInterface(__uuidof(inst), reinterpret_cast<void**>(&inst));
    if (FAILED(hResult)) {
        utils::Loger::Log(hResult, "Failed create annot");
        return false;
    }
    return true;
}

Mode& Mode::ReturnExample() {
    static Mode instance;
    return instance;
}

UserAnnotation& Mode::GetUserAnnotation() {
    return ReturnExample().annot;
}

bool Mode::GetDebug() {
    return ReturnExample().dbgMode;
}
