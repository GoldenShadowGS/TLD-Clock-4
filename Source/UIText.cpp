#include "PCH.h"
#include "UIText.h"
#include "Renderer.h"
#include "Timer.h"
#include "UIAlarmDisplay.h"

UIText::UIText(ID2D1Factory2* pD2DFactory, ID2D1DeviceContext* dc, Timer* pTimer, const RECT& buttonrect, BOOL GrabLock, float skew, const D2D1::ColorF& color1, const D2D1::ColorF& color2, std::function <BOOL()> activation) :
	UIElementBase(buttonrect, GrabLock)
{
	m_ActivateFunction = activation;
	m_pTimer = pTimer;
	float size = float(buttonrect.bottom - buttonrect.top) * 0.4f;
	m_SevenSegment.Init(pD2DFactory, dc, size, skew, color1, color2);
	HR(dc->CreateSolidColorBrush(D2D1::ColorF(0.79f, 0.5f, 0.8f, 0.3f), Brush.ReleaseAndGetAddressOf()));
}

void UIText::Draw(ID2D1DeviceContext* dc, ElementState state, BOOL focused)
{
	if (m_pTimer->isStarted())
		m_TimeString.Set(m_pTimer->GetTime());

	float stringwidthhalf = m_SevenSegment.GetStringWidth(m_TimeString) / 2.0f;
	float heighthalf = (float)(m_Rect.bottom - m_Rect.top) / 2.0f;
	if (focused)
	{
		D2D_RECT_F UI_Rect = { -stringwidthhalf, -heighthalf, stringwidthhalf, heighthalf };
		dc->SetTransform(m_SevenSegment.GetSkewMatrix() * D2D1::Matrix3x2F::Translation(m_Center.x - m_SevenSegment.GetDigitWidth() * 0.3f, m_Center.y));
		dc->FillRectangle(UI_Rect, Brush.Get());
	}
	D2D1::Matrix3x2F transform = D2D1::Matrix3x2F::Translation(m_Center.x - stringwidthhalf, m_Center.y - m_SevenSegment.GetDigitHeight() / 2.0f);
	m_SevenSegment.DrawDigits(dc, m_TimeString, transform);
}

BOOL UIText::AddChar(char value)
{
	BOOL result = FALSE;
	{
		if (value >= '0' && value <= '9')
			result = m_TimeString.Add(value);
		else if (value == 8)
			result = m_TimeString.Back();
		if (result)
		{
			m_ActivateFunction();
			m_pTimer->Reset(m_TimeString.GetTime());
		}
	}
	return result;
}

BOOL UIText::KeyDown(int value)
{
	if (value == VK_DELETE)
	{
		m_ActivateFunction();
		m_pTimer->Reset(0);
		m_pAlarm1->SetTime(0);
		m_pAlarm2->SetTime(0);
		return m_TimeString.Clear();
	}
	return FALSE;
}

BOOL UIText::HitTest(int x, int y)
{
	const float widthhalf = GetStringWidth() * 0.5f;
	const int rectleft = int(m_Center.x - widthhalf);
	const int rectright = int(m_Center.x + widthhalf);
	return (x > rectleft && x < rectright && y > m_Rect.top && y < m_Rect.bottom);
}

float UIText::GetStringWidth()
{
	return m_SevenSegment.GetStringWidth(m_TimeString);
}

void UIText::AddTime(INT64 time)
{
	m_pTimer->AddTime(time);
	m_TimeString.Set(m_pTimer->GetTime());
}

void UIText::SetTime(INT64 time)
{
	if (!m_pTimer->isStarted())
	{
		m_pTimer->Reset(time);
		m_TimeString.Set(m_pTimer->GetTime());
	}
}

void UIText::LoseFocus()
{
	if (!m_pTimer->isStarted())
	{
		INT64 time = min(m_TimeString.GetTime(), MAXTIME);
		m_TimeString.Set(time);
		m_pTimer->Reset(time);
	}
}
