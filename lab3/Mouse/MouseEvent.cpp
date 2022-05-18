#include "MouseEvent.h"

MouseEvent::MouseEvent()
	: m_type(EventType::Invalid)
	, m_x(0)
	, m_y(0)
{
}

MouseEvent::MouseEvent(EventType type, int x, int y)
	: m_type(type)
	, m_x(x)
	, m_y(y)
{
}

bool MouseEvent::IsValid() const
{
	return m_type != EventType::Invalid;
}

MouseEvent::EventType MouseEvent::GetType() const
{
	return m_type;
}

MousePoint MouseEvent::GetPos() const
{
	return { m_x, m_y };
}

int MouseEvent::GetPosX() const
{
	return m_x;
}

int MouseEvent::GetPosY() const
{
	return m_y;
}
