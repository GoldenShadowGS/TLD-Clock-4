#pragma once
#include "PCH.h"
#include "UIElement.h"
#include "Image.h"
#include "SevenSegment.h"
#include "Timer.h"
#include "Sound.h"

class UIBLocaltime : public UIElementBase
{
public:
	UIBLocaltime(const RECT& buttonrect, ID2D1Factory2* factory, ID2D1DeviceContext* dc, Timer* timer, std::function <void()> sound);
	void Draw(ID2D1DeviceContext* dc, ElementState state, BOOL focused) override;
	BOOL MouseMoved(int x, int y) override;
	BOOL Press(int x, int y, BOOL LMB) override;
	BOOL HitTest(int x, int y) override;
	INT64 GetLocaltime();
private: 
	void Drugs();
	Image m_SunImage;
	Image m_MoonImage;
	int m_Timeoffset = 0;
	int m_MousePressOffset = 0;
	SevenSegment m_Localtime;
	Timer* m_Timer;
	std::function <void()> m_PlaySound;
	ComPtr<ID2D1SolidColorBrush> Brush;
};