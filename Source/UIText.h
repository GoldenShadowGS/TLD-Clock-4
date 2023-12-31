#pragma once
#include "PCH.h"
#include "UIElement.h"
#include "TimeString.h"
#include "SevenSegment.h"

class Timer;
class UIAlarmDisplay;

class UIText : public UIElementBase
{
public:
	UIText(ID2D1Factory2* pD2DFactory, ID2D1DeviceContext* dc, Timer* pTimer, const RECT& buttonrect, BOOL GrabLock, float skew, const D2D1::ColorF& color1, const D2D1::ColorF& color2, std::function <BOOL()> activation);
	void Draw(ID2D1DeviceContext* dc, ElementState state, BOOL focused) override;
	BOOL AddChar(char value) override;
	BOOL KeyDown(int value) override;
	BOOL HitTest(int x, int y) override;
	void AddTime(INT64 time);
	float GetStringWidth();
	void SetTime(INT64 time);
	void LoseFocus() override;
	inline void SetAlarmPtr1(UIAlarmDisplay* alarmptr) { m_pAlarm1 = alarmptr; }
	inline void SetAlarmPtr2(UIAlarmDisplay* alarmptr) { m_pAlarm2 = alarmptr; }
private:
	SevenSegment m_SevenSegment;
	TimeString m_TimeString;
	Timer* m_pTimer = nullptr;
	std::function <BOOL()> m_ActivateFunction;
	ComPtr<ID2D1SolidColorBrush> Brush;
	static const INT64 MAXTIME = 719999995000;
	UIAlarmDisplay* m_pAlarm1 = nullptr;
	UIAlarmDisplay* m_pAlarm2 = nullptr;
};