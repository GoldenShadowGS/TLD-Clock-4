#include "PCH.h"
#include "UILocaltime.h"
#include "Renderer.h"
#include "Math.h"
#include "Resource.h"

UIBLocaltime::UIBLocaltime(const RECT& buttonrect, ID2D1Factory2* factory, ID2D1DeviceContext* dc, Timer* timer, std::function <void()> sound) :
	UIElementBase(buttonrect, TRUE),
	m_Timer(timer)
{
	m_PlaySound = sound;
	m_SunImage = CreateSunImage(factory, dc, 30);
	m_MoonImage = CreateMoonImage(factory, dc, 30);
	m_Localtime.Init(factory, dc, 10.0f, -10.0f, D2D1::ColorF(0.48f, 0.43f, 0.54f, 1.0f), D2D1::ColorF(0.86f, 0.81f, 0.97f, 1.0f));
	HR(dc->CreateSolidColorBrush(D2D1::ColorF(0.75f, 0.79f, 0.75f, 1.0f), Brush.ReleaseAndGetAddressOf()));
}

void UIBLocaltime::Draw(ID2D1DeviceContext* dc, ElementState state, BOOL focused)
{
	Drugs();
	dc->SetTransform(D2D1::Matrix3x2F::Translation(m_Center.x, m_Center.y));

	D2D_RECT_F halfRect = D2D1::RectF(-100.0f, -100.0f, 100.0f, 0.0f);
	dc->PushAxisAlignedClip(halfRect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);

	INT64 currentTime = m_Timer->GetTime() / 1000;
	int timeoffset = (currentTime + m_Timeoffset) % MINUTESPERDAY;
	float widthhalf = (float)(m_Rect.right - m_Rect.left) / 2.0f;
	float heighthalf = (float)(m_Rect.bottom - m_Rect.top) / 2.0f;
	D2D_RECT_F UI_Rect = { -widthhalf, -heighthalf, widthhalf, heighthalf };

	auto Drawshape = [&] (Image& image, float angle, float opacity)
		{
			float w = image.m_Size.width / 2.0f;
			float h = image.m_Size.height / 2.0f;
			D2D_RECT_F ImageRect = { -w, -h, w, h };
			D2D1_POINT_2F offset = rotate({ 0.0f, widthhalf * 0.44f }, angle);
			dc->SetTransform(D2D1::Matrix3x2F::Translation(m_Center.x + offset.x, m_Center.y + offset.y));
			dc->DrawBitmap(image.m_Bitmap.Get(), ImageRect, opacity);
		};
	float angle = float(timeoffset / (float)MINUTESPERDAY) * PI2;
	float angle2 = float((timeoffset + (MINUTESPERDAY / 2)) / (float)MINUTESPERDAY) * PI2;
	Drawshape(m_SunImage, angle, 0.5f);
	Drawshape(m_MoonImage, angle2, 0.5f);

	if (focused)
	{
		dc->SetTransform(D2D1::Matrix3x2F::Translation(m_Center.x, m_Center.y));
		float radius = float(m_Rect.right - m_Rect.left) * 0.45f;
		D2D1_ELLIPSE e = { { 0.0f, 0.0f }, radius, radius };
		dc->DrawEllipse(e, Brush.Get());
	}
	dc->PopAxisAlignedClip();
	m_Localtime.LocalDrawDigits(dc, timeoffset, D2D1::Matrix3x2F::Translation(m_Center.x - m_Localtime.GetDigitWidth() * 2.5f, m_Center.y));
}

BOOL UIBLocaltime::MouseMoved(int x, int y)
{
	D2D1_POINT_2F point = { float(x),float(y) };
	D2D1_POINT_2F localpoint = point - m_Center;
	float angle = atan2(localpoint.y, localpoint.x);
	m_Timeoffset = int((angle / PI2) * MINUTESPERDAY) - m_MousePressOffset;
	while (m_Timeoffset < 0)
		m_Timeoffset += MINUTESPERDAY;
	while (m_Timeoffset >= MINUTESPERDAY)
		m_Timeoffset -= MINUTESPERDAY;
	return TRUE;
}

BOOL UIBLocaltime::Press(int x, int y, BOOL LMB)
{
	D2D1_POINT_2F point = { float(x),float(y) };
	D2D1_POINT_2F localpoint = point - m_Center;
	float angle = atan2(localpoint.y, localpoint.x);
	m_MousePressOffset = int((angle / PI2) * MINUTESPERDAY) - m_Timeoffset;
	return TRUE;
}

BOOL UIBLocaltime::HitTest(int x, int y)
{
	INT64 currentTime = m_Timer->GetTime() / 1000;
	int timeoffset = (currentTime + m_Timeoffset) % MINUTESPERDAY;
	BOOL result = FALSE;
	if (UIElementBase::HitTest(x, y) && y <= (int)m_Center.y)
	{
		D2D1_POINT_2F point = { float(x),float(y) };
		float dist = GetLength(point - m_Center);
		float radius = float(m_Rect.right - m_Rect.left) * 0.5f;
		if (dist <= radius)
		{
			float widthhalf = (float)(m_Rect.right - m_Rect.left) / 2.0f;
			float angle = float(timeoffset / (float)MINUTESPERDAY) * PI2;
			D2D1_POINT_2F offset1 = rotate({ 0.0f, -widthhalf * 0.45f }, angle);
			D2D1_POINT_2F offset2 = rotate({ 0.0f, widthhalf * 0.45f }, angle);
			const float imageradius = m_SunImage.m_Size.width / 2.0f;
			float sundist = GetLength(point - m_Center + offset1);
			float moondist = GetLength(point - m_Center + offset2);
			if (sundist <= imageradius)
				result = TRUE;
			else if (moondist <= imageradius)
				result = TRUE;
		}
	}
	return result;
}

INT64 UIBLocaltime::GetLocaltime()
{
	return m_Timer->GetTime() / 1000 + m_Timeoffset;
}

void UIBLocaltime::Drugs()
{
	int CurrentDay = int(GetLocaltime() / 1440);
	static int PrevDay = CurrentDay;
	if (CurrentDay > PrevDay)
	{
		m_PlaySound();
	}
	PrevDay = CurrentDay;
}
