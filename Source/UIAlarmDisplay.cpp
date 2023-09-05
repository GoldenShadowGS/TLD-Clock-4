#include "PCH.h"
#include "UIAlarmDisplay.h"
#include "Renderer.h"
#include "Math.h"
#include "Resource.h"

UIAlarmDisplay::UIAlarmDisplay(const RECT& buttonrect, ID2D1Factory2* factory, ID2D1DeviceContext* dc, Timer* timer, std::function <void()> sound) :
	UIElementBase(buttonrect, TRUE),
	m_Timer(timer)
{
	m_PlaySound = sound;
	m_Alarmtime.Init(factory, dc, 10.0f, -10.0f, D2D1::ColorF(0.48f, 0.13f, 0.14f, 1.0f), D2D1::ColorF(0.86f, 0.21f, 0.27f, 1.0f));
	HR(dc->CreateSolidColorBrush(D2D1::ColorF(0.85f, 0.69f, 0.55f, 0.4f), Brush.ReleaseAndGetAddressOf()));
}

void UIAlarmDisplay::Draw(ID2D1DeviceContext* dc, ElementState state, BOOL focused)
{
	INT64 remainingtime = GetRemainingtime();
	if (!AlarmBeeped)
	{
		if (remainingtime == 0)
		{
			m_AlarmSet = FALSE;
			AlarmBeeped = TRUE;
			m_PlaySound();
		}
	}

	if (remainingtime > 0)
	{
		m_TimeString.Set(GetRemainingtime());

		float stringwidthhalf = m_Alarmtime.GetStringWidth(m_TimeString) / 2.0f;
		D2D1::Matrix3x2F transform = D2D1::Matrix3x2F::Translation(m_Center.x - stringwidthhalf, m_Center.y - m_Alarmtime.GetDigitHeight() / 2.0f);
		m_Alarmtime.DrawDigits(dc, m_TimeString, transform);
	}
}

INT64 UIAlarmDisplay::GetRemainingtime()
{
	if (m_AlarmSet)
		return max(m_AlarmTime - m_Timer->GetTime(), 0);
	else return 0;
}

void UIAlarmDisplay::SetTime(INT64 time)
{
	if (time > 0)
	{
		m_AlarmSet = TRUE;
		m_AlarmTime = time + m_Timer->GetTime();
		m_TimeString.Set(GetRemainingtime());
		AlarmBeeped = FALSE;
	}
	else if (time == 0 && GetRemainingtime() > 0)
	{
		m_AlarmSet = FALSE;
		AlarmBeeped = TRUE;
	}
}
