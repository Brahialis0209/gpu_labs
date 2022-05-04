#include "Mouse.h"

EventMouse::EventMouse(): ev(EVENTS::INVALID), xCord(0), yCord(0) {}
EventMouse::EventMouse(EVENTS event, int x, int y): ev(event), xCord(x), yCord(y) {}

bool EventMouse::IsValid() const {
	return ev != EVENTS::INVALID;
}

EVENTS EventMouse::GetEvent() const {
	return ev;
}

Dot EventMouse::GetPos() const {
	return { xCord, yCord };
}

int EventMouse::GetX() const {
	return xCord;
}

int EventMouse::GetY() const {
	return yCord;
}


void MouseButtons::LeftPressed(int x, int y) {
	leftDown = true;
	bufferEvents.emplace(EVENTS::LEFT_BUTTON_PRESS, x, y);
}

void MouseButtons::LeftReleased(int x, int y) {
	leftDown = false;
	bufferEvents.emplace(EVENTS::LEFT_BUTTON_RELEASE, x, y);
}

void MouseButtons::RightPressed(int x, int y) {
	rightDown = true;
	bufferEvents.emplace(EVENTS::RIGHT_BUTTON_PRESS, x, y);
}

void MouseButtons::RightReleased(int x, int y) {
	rightDown = false;
	bufferEvents.emplace(EVENTS::RIGHT_BUTTON_RELEASE, x, y);
}

void MouseButtons::MiddlePressed(int x, int y) {
	middleDown = true;
	bufferEvents.emplace(EVENTS::MIDDLE_BUTTON_PRESS, x, y);
}

void MouseButtons::MiddleReleased(int x, int y) {
	middleDown = false;
	bufferEvents.emplace(EVENTS::MIDDLE_BUTTON_RELEASE, x, y);
}

void MouseButtons::WHEEL_UP(int x, int y) {
	bufferEvents.emplace(EVENTS::WHEEL_UP, x, y);
}

void MouseButtons::WHEEL_DOWN(int x, int y) {
	bufferEvents.emplace(EVENTS::WHEEL_DOWN, x, y);
}

void MouseButtons::MouseMove(int x, int y) {
	xCord = x;
	yCord = y;
	bufferEvents.emplace(EVENTS::MOVE, x, y);
}

void MouseButtons::MouseMoveRaw(int x, int y) {
	bufferEvents.emplace(EVENTS::RAW_MODE, x, y);
}

bool MouseButtons::IsLeftDown() const {
	return leftDown;
}

bool MouseButtons::IsMiddleDown() const {
	return middleDown;
}

bool MouseButtons::IsRightDown() const {
	return rightDown;
}

int MouseButtons::GetX() const {
	return xCord;
}

int MouseButtons::GetY() const {
	return yCord;
}

Dot MouseButtons::GetPos() const {
	return { xCord, yCord };
}

bool MouseButtons::IsEventBufferEmpty() const {
	return bufferEvents.empty();
}

EventMouse MouseButtons::ReadEvent() {
	EventMouse e;
	if (bufferEvents.empty())
		e = EventMouse();
	else {
		e = bufferEvents.front();
		bufferEvents.pop();
	}
	return e;
}


bool Mouse::Init(MouseButtons* pMouseBut, Graphics* pGrap) {
	pMouse = pMouseBut;
	pGraphics = pGrap;
	return pMouse && pGraphics;
}

void Mouse::Process() {
	if (pMouse && pGraphics) {
		while (!pMouse->IsEventBufferEmpty()) {
			float const speed = 1.0f;
			EventMouse mouse_event = pMouse->ReadEvent();

			if (mouse_event.GetEvent() == EVENTS::LEFT_BUTTON_PRESS)
				mousePressed = true;
			if (mouse_event.GetEvent() == EVENTS::LEFT_BUTTON_RELEASE)
				mousePressed = false;

			auto& cameraPos = pGraphics->RefWorldCameraPosition();
			float& lon = cameraPos.lon;
			float& lat = cameraPos.lat;
			float& pos_x = cameraPos.pos_x;
			float& pos_y = cameraPos.pos_y;
			float& pos_z = cameraPos.pos_z;
			DirectX::XMMATRIX& view = pGraphics->RefView();

			if (mouse_event.GetEvent() == EVENTS::WHEEL_UP) {
				pos_z += speed;
				view = DirectX::XMMatrixInverse(NULL, DirectX::XMMatrixRotationAxis({ 1,0,0 }, lat) * DirectX::XMMatrixRotationAxis({ 0,1,0 }, lon) * DirectX::XMMatrixTranslation(pos_x, pos_y, pos_z));
			}
			if (mouse_event.GetEvent() == EVENTS::WHEEL_DOWN) {
				pos_z -= speed;
				view = DirectX::XMMatrixInverse(NULL, DirectX::XMMatrixRotationAxis({ 1,0,0 }, lat) * DirectX::XMMatrixRotationAxis({ 0,1,0 }, lon) * DirectX::XMMatrixTranslation(pos_x, pos_y, pos_z));
			}
			if (mousePressed && mouse_event.GetEvent() == EVENTS::RAW_MODE) {
				int dx = mouse_event.GetX();
				int dy = mouse_event.GetY();

				lon += (float)dx / 100.f;
				lat += (float)dy / 100.f;

				if (lat <= -(float)DirectX::XM_PI / 2) {
					lat = -(float)DirectX::XM_PI / 2;
				}
				if (lat >= (float)DirectX::XM_PI / 2) {
					lat = (float)DirectX::XM_PI / 2;
				}
				view = DirectX::XMMatrixInverse(NULL, DirectX::XMMatrixRotationAxis({ 1,0,0 }, lat) * DirectX::XMMatrixRotationAxis({ 0,1,0 }, lon) * DirectX::XMMatrixTranslation(pos_x, pos_y, pos_z));
			}
		}
	}
}