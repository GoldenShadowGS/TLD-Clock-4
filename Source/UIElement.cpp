#include "PCH.h"
#include "UIElement.h"

UIElementBase::UIElementBase(const RECT& rect, BOOL GrabLock)
{
	m_Rect = rect;
	m_GrabLock = GrabLock;
	float width = float(rect.right - rect.left) / 2.0f;
	float height = float(rect.bottom - rect.top) / 2.0f;
	m_Center = { float(rect.left) + width, float(rect.top) + height };
}

BOOL UIElementBase::HitTest(int x, int y)
{
	return (x > m_Rect.left && x < m_Rect.right && y > m_Rect.top && y < m_Rect.bottom);
}

void UIElementBase::LoseFocus()
{

}
