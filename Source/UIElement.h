#pragma once
#include "PCH.h"

enum ElementState
{
	STATE_NORMAL,
	STATE_HIGHLIGHTED,
	STATE_GRABBED
};

class UIElementBase
{
public:
	UIElementBase(const RECT& rect, BOOL GrabLock);
	virtual void Draw(ID2D1DeviceContext* dc, ElementState state, BOOL focused) = 0;
	virtual BOOL MouseMoved(int x, int y) { return FALSE; }
	virtual BOOL Press(int x, int y, BOOL LMB) { return FALSE; }
	virtual void Release() {}
	virtual BOOL AddChar(char value) { return FALSE; }
	virtual BOOL KeyDown(int value) { return FALSE; }
	virtual BOOL HitTest(int x, int y);
	virtual void LoseFocus();
	inline BOOL isGrabLocked() { return m_GrabLock; }
protected:
	BOOL m_GrabLock = FALSE;
	RECT m_Rect {};
	D2D1_POINT_2F m_Center {};
};
