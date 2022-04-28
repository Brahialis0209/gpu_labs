#pragma once
#include <queue>
#include "Events.h"
#include "Graphics/Graphics.h"

struct Dot {
	int x;
	int y;
};


class EventMouse {
public:
	EventMouse();
	EventMouse(EVENTS event, int x, int y);

	int GetX() const;
	int GetY() const;
	EVENTS GetEvent() const;
	Dot GetPos() const;
	bool IsValid() const;

private:
	EVENTS ev;
	int xCord;
	int yCord;
};



class MouseButtons {
public:
	void LeftPressed(int x, int y);
	void LeftReleased(int x, int y);
	void RightPressed(int x, int y);
	void RightReleased(int x, int y);
	void MiddlePressed(int x, int y);
	void MiddleReleased(int x, int y);
	void WHEEL_UP(int x, int y);
	void WHEEL_DOWN(int x, int y);
	void MouseMove(int x, int y);
	void MouseMoveRaw(int x, int y);

	bool IsLeftDown() const;
	bool IsMiddleDown() const;
	bool IsRightDown() const;

	int GetX() const;
	int GetY() const;
	Dot GetPos() const;

	bool IsEventBufferEmpty() const;
	EventMouse ReadEvent();

private:
	queue<EventMouse> bufferEvents;
	bool leftDown = false;
	bool middleDown = false;
	bool rightDown = false;
	int xCord = 0;
	int yCord = 0;
};


class Mouse {
public:
	Mouse() = default;
	bool Init(MouseButtons* pMouseBut, Graphics* pGrap);
	void Process();
private:
	MouseButtons* pMouse{ nullptr };
	Graphics* pGraphics{ nullptr };
	bool mousePressed{ false };
};
