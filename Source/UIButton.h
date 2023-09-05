#pragma once
#include "PCH.h"
#include "UIElement.h"
#include "Image.h"

class UIButton : public UIElementBase
{
public:
	UIButton(const Image& image, const RECT& buttonrect, BOOL GrabLock, std::function <BOOL(BOOL LMB)> activation);
	UIButton(const Image& imageon, const Image& imageoff, BOOL initialState, const RECT& buttonrect, BOOL GrabLock, std::function <BOOL(BOOL LMB)> activation);
	static void InitResources(ID2D1DeviceContext* dc);
	void Draw(ID2D1DeviceContext* dc, ElementState state, BOOL focused) override;
	BOOL MouseMoved(int x, int y) override;
	BOOL Press(int x, int y, BOOL LMB) override;
	BOOL KeyDown(int value) override;
private:
	static inline ComPtr<ID2D1SolidColorBrush> Brush;
	static inline ComPtr<ID2D1SolidColorBrush> HoverBrush;
	static inline ComPtr<ID2D1SolidColorBrush> FocusedBrush;
	static inline ComPtr<ID2D1SolidColorBrush> GrabbedBrush;
	Image m_ImageOn;
	Image m_ImageOff;
	D2D1_POINT_2F m_EndPoint = { };
	std::function <BOOL(BOOL LMB)> m_ActivateFunction;
	const BOOL TOGGLEBUTTON;
	BOOL m_state;
};
