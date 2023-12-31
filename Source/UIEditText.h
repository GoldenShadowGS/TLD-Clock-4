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
	UIEditText(ID2D1Factory2* pD2DFactory, ID2D1DeviceContext* dc, const RECT& buttonrect, BOOL GrabLock, float skew, const D2D1::ColorF& color1, const D2D1::ColorF& color2, std::function <BOOL()> activation);
	void Draw(ID2D1DeviceContext* dc, ElementState state, BOOL focused) override;
	BOOL AddChar(char value) override;
	BOOL KeyDown(int value) override;
	BOOL HitTest(int x, int y) override;
	BOOL Press(int x, int y, BOOL LMB) override;
	float GetStringWidth();
	INT64 GetTime();
	void SetTime(INT64 time);
	void LoseFocus() override;
private:
	SevenSegment m_SevenSegment;
	TimeStringSmall m_TimeString;
	std::function <BOOL()> m_ActivateFunction;
	static inline ComPtr<ID2D1SolidColorBrush> Brush;
	static const INT64 MAXTIME = 71995000;
};