#pragma once
#include "PCH.h"
#include "UIElement.h"
#include "TimeString.h"
#include "SevenSegment.h"

class Timer;

class UIEditText : public UIElementBase
{
public:
	static void InitResources(ID2D1DeviceContext* dc);
	UIEditText(ID2D1Factory2* pD2DFactory, ID2D1DeviceContext* dc, const RECT& buttonrect, BOOL GrabLock, float skew, std::function <BOOL()> activation);
	void Draw(ID2D1DeviceContext* dc, ElementState state, BOOL focused) override;
	BOOL AddChar(char value) override;
	BOOL KeyDown(int value) override;
	BOOL HitTest(int x, int y) override;
	float GetStringWidth();
	INT64 GetTime();
	void SetTime(INT64 time);
private:
	SevenSegment m_SevenSegment;
	TimeStringSmall m_TimeString;
	std::function <BOOL()> m_ActivateFunction;
	static inline ComPtr<ID2D1SolidColorBrush> Brush;
};