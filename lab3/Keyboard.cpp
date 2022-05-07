#include "Keyboard.h"

EventKeyboard::EventKeyboard() : ev(EVENTS::INVALID), symb(0u) {}

EventKeyboard::EventKeyboard(EVENTS event, unsigned char key) : ev(event), symb(key) {}

bool EventKeyboard::IsPress() const {
	return ev == EVENTS::BUTTON_PRESS;
}

bool EventKeyboard::IsRelease() const {
	return ev == EVENTS::BUTTON_RELEASE;
}

bool EventKeyboard::IsValid() const {
	return ev != EVENTS::INVALID;
}

unsigned char EventKeyboard::GetKeyCode() const {
	return symb;
}


KeyboardKeys::KeyboardKeys() {
	for (int i = 0; i < MAX_KEYS_COUNT; ++i) {
		m_key_states[i] = false;
	}
}

bool KeyboardKeys::keyPressed(unsigned char keycode) {
	return m_key_states[keycode];
}

bool KeyboardKeys::keyBufferEmpty() {
	return keyEventBuffer.empty();
}

bool KeyboardKeys::symbBufferEmpty() {
	return symbBuffer.empty();
}

EventKeyboard KeyboardKeys::ReadKey() {
	if (keyEventBuffer.empty())
		return EventKeyboard();
	else {
		EventKeyboard event = keyEventBuffer.front();
		keyEventBuffer.pop();
		return event;
	}
}

unsigned char KeyboardKeys::ReadChar() {
	if (symbBuffer.empty())
		return 0u;
	else {
		unsigned char symb = symbBuffer.front();
		symbBuffer.pop();
		return symb;
	}
}

void KeyboardKeys::OnKeyPressed(unsigned char key) {
	m_key_states[key] = true;
	keyEventBuffer.emplace(EVENTS::BUTTON_PRESS, key);
}

void KeyboardKeys::OnKeyReleased(unsigned char key) {
	m_key_states[key] = false;
	keyEventBuffer.emplace(EVENTS::BUTTON_RELEASE, key);
}

void KeyboardKeys::OnChar(unsigned char key) {
	symbBuffer.push(key);
}


void KeyboardKeys::EnableAutoRepeatKeys() {
	m_auto_repeat_keys = true;
}

void KeyboardKeys::DisableAutoRepeatKeys() {
	m_auto_repeat_keys = false;
}

void KeyboardKeys::EnableAutoRepeatChars() {
	m_auto_repeat_chars = true;
}

void KeyboardKeys::DisableAutoRepeatChars() {
	m_auto_repeat_chars = false;
}

bool KeyboardKeys::IsKeysAutoRepeat() {
	return m_auto_repeat_keys;
}

bool KeyboardKeys::IsCharsAutoRepeat() {
	return m_auto_repeat_chars;
}


bool KeyboardProcessor::Init(KeyboardKeys* pK, Graphics* pGrap) {
	pKey = pK;
	pGraph = pGrap;
	return pKey && pGraph;
}

void KeyboardProcessor::Process() {
	if (pKey && pGraph) {
		while (!pKey->keyBufferEmpty()) {
			EventKeyboard eventKey = pKey->ReadKey();
			float const speed = 0.5f;

			wstring msg = L"Event key: " + eventKey.GetKeyCode();
			msg += L"'";
			Mode::GetUserAnnotation().BegEvent(msg.c_str());

			auto& cameraPos = pGraph->RefWorldCameraPosition();
			float& lon = cameraPos.lon;
			float& lat = cameraPos.lat;
			float& pos_x = cameraPos.pos_x;
			float& pos_y = cameraPos.pos_y;
			float& pos_z = cameraPos.pos_z;
			XMMATRIX& view = pGraph->RefView();

			if (eventKey.IsPress())
				switch (eventKey.GetKeyCode()) {
					case 'W':
						pos_y += speed;
						view = XMMatrixInverse(NULL, XMMatrixRotationAxis({ 1,0,0 }, lat) *
							XMMatrixRotationAxis({ 0,1,0 }, lon) * XMMatrixTranslation(pos_x, pos_y, pos_z));
						break;
					case 'A':
						pos_x -= speed;
						view = XMMatrixInverse(NULL, XMMatrixRotationAxis({ 1,0,0 }, lat) * 
							XMMatrixRotationAxis({ 0,1,0 }, lon) * XMMatrixTranslation(pos_x, pos_y, pos_z));
						break;
					case 'S':
						pos_y -= speed;
						view = XMMatrixInverse(NULL, XMMatrixRotationAxis({ 1,0,0 }, lat) * 
							XMMatrixRotationAxis({ 0,1,0 }, lon) * XMMatrixTranslation(pos_x, pos_y, pos_z));
						break;
					case 'D':
						pos_x += speed;
						view = XMMatrixInverse(NULL, XMMatrixRotationAxis({ 1,0,0 }, lat) *
							XMMatrixRotationAxis({ 0,1,0 }, lon) * XMMatrixTranslation(pos_x, pos_y, pos_z));
						break;
					case '1':
						pGraph->ChangeLightsIntencity(0);
						break;
					case '2':
						pGraph->ChangeLightsIntencity(1);
						break;
					case '3':
						pGraph->ChangeLightsIntencity(2);
						break;
					case '4':
						pGraph->ChangeToneMaping();
						break;
					default:
						break;
				}

			Mode::GetUserAnnotation().EndEvent();
		}
	}
}
