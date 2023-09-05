#include "PCH.h"
#include "UIButton.h"
#include "Renderer.h"

void UIButton::InitResources(ID2D1DeviceContext* dc)
{
	HR(dc->CreateSolidColorBrush(D2D1::ColorF(0.8f, 0.8f, 0.8f, 1.0f), Brush.ReleaseAndGetAddressOf()));
	HR(dc->CreateSolidColorBrush(D2D1::ColorF(0.82f, 0.61f, 0.91f, 1.0f), HoverBrush.ReleaseAndGetAddressOf()));
	HR(dc->CreateSolidColorBrush(D2D1::ColorF(0.5f, 0.0f, 0.4f, 1.0f), FocusedBrush.ReleaseAndGetAddressOf()));
	HR(dc->CreateSolidColorBrush(D2D1::ColorF(0.71f, 0.39f, 0.76f, 1.0f), GrabbedBrush.ReleaseAndGetAddressOf()));
}

UIButton::UIButton(const Image& image, const RECT& buttonrect, BOOL GrabLock, std::function <BOOL(BOOL LMB)> activation) :
	UIElementBase(buttonrect, GrabLock),
	TOGGLEBUTTON(FALSE)
{
	m_ImageOn = image;
	m_ActivateFunction = activation;
}

UIButton::UIButton(const Image& imageon, const Image& imageoff, BOOL initialState, const RECT& buttonrect, BOOL GrabLock, std::function <BOOL(BOOL LMB)> activation) :
	UIElementBase(buttonrect, GrabLock),
	TOGGLEBUTTON(TRUE),
	m_state(initialState)
{
	m_ImageOn = imageon;
	m_ImageOff = imageoff;
	m_ActivateFunction = activation;
}

void UIButton::Draw(ID2D1DeviceContext* dc, ElementState state, BOOL focused)
{
	dc->SetTransform(D2D1::Matrix3x2F::Translation(m_Center.x, m_Center.y));
	float widthhalf = (float)(m_Rect.right - m_Rect.left) / 2.0f;
	float heighthalf = (float)(m_Rect.bottom - m_Rect.top) / 2.0f;
	D2D_RECT_F UI_Rect = { -widthhalf, -heighthalf, widthhalf, heighthalf };

	dc->SetTransform(D2D1::Matrix3x2F::Translation(m_Center.x, m_Center.y));
	//D2D_RECT_F UI_Rect = { (float)m_Rect.left, (float)m_Rect.top, (float)m_Rect.right, (float)m_Rect.bottom };
	switch (state)
	{
	case STATE_NORMAL:
		dc->FillRectangle(UI_Rect, Brush.Get());
		break;
	case STATE_HIGHLIGHTED:
		dc->FillRectangle(UI_Rect, HoverBrush.Get());
		break;
	case STATE_GRABBED:
		dc->FillRectangle(UI_Rect, GrabbedBrush.Get());
		break;
	}
	if (focused)
		dc->DrawRectangle(UI_Rect, FocusedBrush.Get(), 5.0f);
	//dc->DrawLine({ 0.0f, 0.0f }, m_EndPoint, FocusedBrush.Get());
	if (TOGGLEBUTTON)
	{
		if (m_state)
		{
			float ihw = m_ImageOn.m_Size.width / 2.0f;
			float ihh = m_ImageOn.m_Size.height / 2.0f;
			D2D_RECT_F Image_Rect = { -ihw, -ihh, ihw, ihh };
			dc->DrawBitmap(m_ImageOn.m_Bitmap.Get(), Image_Rect, 1.0f);
		}
		else
		{
			float ihw = m_ImageOff.m_Size.width / 2.0f;
			float ihh = m_ImageOff.m_Size.height / 2.0f;
			D2D_RECT_F Image_Rect = { -ihw, -ihh, ihw, ihh };
			dc->DrawBitmap(m_ImageOff.m_Bitmap.Get(), Image_Rect, 1.0f);
		}
	}
	else
	{
		float ihw = m_ImageOn.m_Size.width / 2.0f;
		float ihh = m_ImageOn.m_Size.height / 2.0f;
		D2D_RECT_F Image_Rect = { -ihw, -ihh, ihw, ihh };
		dc->DrawBitmap(m_ImageOn.m_Bitmap.Get(), Image_Rect, 1.0f);
	}
}

BOOL UIButton::MouseMoved(int x, int y)
{
	m_EndPoint = { float(x) - m_Center.x, float(y) - m_Center.y };
	return TRUE;
}

BOOL UIButton::Press(int x, int y, BOOL LMB)
{
	m_EndPoint = { float(x) - m_Center.x, float(y) - m_Center.y };
	m_state = m_ActivateFunction(LMB);
	return TRUE;
}

BOOL UIButton::KeyDown(int value)
{
	if (value == VK_SPACE)
	{
		m_state = m_ActivateFunction(TRUE);
		return TRUE;
	}
	return FALSE;
}
