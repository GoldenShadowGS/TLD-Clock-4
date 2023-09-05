#include "PCH.h"
#include "UIElements.h"
#include "Renderer.h"

void UIElements::Set(const std::vector<UIElementBase*>& elements)
{
	m_Elements = elements;
}

void UIElements::Draw(ID2D1DeviceContext* dc)
{
	if (m_UI_State.GrabbedElement)
	{
		for (UIElementBase* element : m_Elements)
		{
			if (element == m_UI_State.GrabbedElement)
				element->Draw(dc, STATE_GRABBED, (element == m_UI_State.FocusedElement));
			else
				element->Draw(dc, STATE_NORMAL, (element == m_UI_State.FocusedElement));
		}
	}
	else
	{
		for (UIElementBase* element : m_Elements)
		{
			if (element == m_UI_State.HoverElement)
				element->Draw(dc, STATE_HIGHLIGHTED, (element == m_UI_State.FocusedElement));
			else
				element->Draw(dc, STATE_NORMAL, (element == m_UI_State.FocusedElement));
		}
	}
}

BOOL UIElements::UIProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	BOOL Redraw = FALSE;
	switch (message)
	{
	case WM_KILLFOCUS:
	case WM_MOUSELEAVE:
	{
		m_UI_State.LeaveWindow(hWnd);
	}
	break;
	case WM_MOUSEMOVE:
	{
		Redraw = m_UI_State.MouseMoved(hWnd, m_Elements, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	}
	break;
	case WM_LBUTTONDOWN:
	{
		Redraw = m_UI_State.ButtonDown(hWnd, LMB, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
	}
	break;
	case WM_LBUTTONUP:
	{
		Redraw = m_UI_State.ButtonUp(hWnd, LMB);
	}
	break;
	case WM_RBUTTONDOWN:
	{
		Redraw = m_UI_State.ButtonDown(hWnd, RMB, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));

	}
	break;
	case WM_RBUTTONUP:
	{
		Redraw = m_UI_State.ButtonUp(hWnd, RMB);
	}
	break;
	case WM_KEYDOWN:
	{
		Redraw = m_UI_State.KeyDown((int)wParam);
	}
	break;
	case WM_CHAR:
	{
		Redraw = m_UI_State.AddChar((char)wParam);
	}
	break;
	case WM_APP + 1:
	{
		Redraw = TRUE;
	}
	break;
	}
	return Redraw;
}