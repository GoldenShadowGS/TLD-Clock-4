#pragma once
#include "PCH.h"
#include "UIElement.h"
#include "Image.h"
#include "SevenSegment.h"
#include "Timer.h"
#include "Sound.h"
#include "TimeString.h"

class UIAlarmDisplay : public UIElementBase
{
public:
	UIAlarmDisplay(const RECT& buttonrect, ID2D1Factory2* factory, ID2D1DeviceContext* dc, Timer* timer, std::function <void()> sound);
	void Draw(ID2D1DeviceContext* dc, ElementState state, BOOL focused) override;
	INT64 GetRemainingtime();
	void SetTime(INT64 time);
private:
	SevenSegment m_Alarmtime;
	TimeString m_TimeString;
	Timer* m_Timer;
	BOOL AlarmBeeped = TRUE;
	BOOL m_AlarmSet = FALSE;
	std::function <void()> m_PlaySound;
	ComPtr<ID2D1SolidColorBrush> Brush;
	INT64 m_AlarmTime = 0;
};