#pragma once

#include <queue>
#include "Graphics/Graphics.h"
#include "Events.h"
#include "Mode.h"

class EventKeyboard {
public:
	EventKeyboard();
	EventKeyboard(EVENTS event, unsigned char key);

	bool IsValid() const;
	bool IsPress() const;
	bool IsRelease() const;
	
	unsigned char GetKeyCode() const;

private:
	EVENTS ev;
	unsigned char symb;
};


class KeyboardKeys {
public:
	static size_t const MAX_KEYS_COUNT = 256;

	KeyboardKeys();

	bool keyPressed(unsigned char keycode);
	bool keyBufferEmpty();
	bool symbBufferEmpty();

	EventKeyboard ReadKey();
	unsigned char ReadChar();

	void OnKeyPressed(unsigned char key);
	void OnKeyReleased(unsigned char key);
	void OnChar(unsigned char key);

	void EnableAutoRepeatKeys();
	void DisableAutoRepeatKeys();
	void EnableAutoRepeatChars();
	void DisableAutoRepeatChars();
	bool IsKeysAutoRepeat();
	bool IsCharsAutoRepeat();

private:
	bool m_auto_repeat_keys = false;
	bool m_auto_repeat_chars = false;
	bool m_key_states[MAX_KEYS_COUNT];
	queue<EventKeyboard> keyEventBuffer;
	queue<unsigned char> symbBuffer;
};


class KeyboardProcessor {
public:
	KeyboardProcessor() = default;
	bool Init(KeyboardKeys* pKbrd, Graphics* pGrap);
	void Process();
private:
	KeyboardKeys* pKey{ nullptr };
	Graphics* pGraph{ nullptr };
};
