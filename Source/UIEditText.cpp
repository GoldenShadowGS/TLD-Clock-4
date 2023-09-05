#include "PCH.h"
#include "UIEditText.h"
#include "Renderer.h"
#include "Timer.h"

void UIEditText::InitResources(ID2D1DeviceContext* dc)
{
	HR(dc->CreateSolidColorBrush(D2D1::ColorF(0.79f, 0.5f, 0.8f, 0.3f), Brush.ReleaseAndGetAddressOf()));
}

UIEditText::UIEditText(ID2D1Factory2* pD2DFactory, ID2D1DeviceContext* dc, const RECT& buttonrect, BOOL GrabLock, float skew, std::function <BOOL()> activation) :
	UIElementBase(buttonrect, GrabLock)
{
	m_ActivateFunction = activation;
	float size = float(buttonrect.bottom - buttonrect.top) * 0.4f;
	m_SevenSegment.Init(pD2DFactory, dc, size, skew, D2D1::ColorF(0.1f, 0.1f, 0.1f, 1.0f), D2D1::ColorF(0.86f, 0.61f, 0.97f, 1.0f));
}

void UIEditText::Draw(ID2D1DeviceContext* dc, ElementState state, BOOL focused)
{
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

BOOL UIEditText::AddChar(char value)
{
	BOOL result = FALSE;
	if (value >= '0' && value <= '9')
	{
		result = m_TimeString.Add(value);
	}
	else if (value == 8)
	{
		result = m_TimeString.Back();
	}
	if (result)
		m_ActivateFunction();
	return result;
}

BOOL UIEditText::KeyDown(int value)
{
	if (value == VK_DELETE)
	{
		return m_TimeString.Clear();
	}
	return FALSE;
}

BOOL UIEditText::HitTest(int x, int y)
{
	const float widthhalf = GetStringWidth() * 0.5f;
	const int rectleft = int(m_Center.x - widthhalf);
	const int rectright = int(m_Center.x + widthhalf);
	return (x > rectleft && x < rectright && y > m_Rect.top && y < m_Rect.bottom);
}

float UIEditText::GetStringWidth()
{
	return m_SevenSegment.GetStringWidth(m_TimeString);
}

INT64 UIEditText::GetTime()
{
	return m_TimeString.GetTime();
}