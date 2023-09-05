#pragma once
#include "PCH.h"
#include "UIElement.h"

enum MOUSEBUTTONS
{
	LMB,
	RMB
};

class UIElements
{
public:
	void Set(const std::vector<UIElementBase*>& elements);
	void Draw(ID2D1DeviceContext* dc);
	BOOL UIProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
private:
	struct UI_State
	{
		inline BOOL ButtonDown(HWND hwnd, MOUSEBUTTONS button, int x, int y)
		{
			BOOL redraw { FALSE };
			if (!GrabbedElement && HoverElement)
			{
				GrabbedElement = HoverElement;
				FocusedElement = HoverElement;
				GrabButton = button;
				redraw = TRUE;
				if (GrabbedElement->isGrabLocked())
					SetCapture(hwnd);
				if (GrabbedElement && GrabbedElement->Press(x, y, button == LMB ? TRUE : FALSE))
					redraw = TRUE;
			}
			else if (!HoverElement && FocusedElement)
			{
				FocusedElement = nullptr;
				redraw = TRUE;
			}
			return redraw;
		}
		inline BOOL ButtonUp(HWND hwnd, MOUSEBUTTONS button)
		{
			BOOL redraw { FALSE };
			if (GrabButton == button)
			{
				if (GrabbedElement)
				{
					if (GrabbedElement->isGrabLocked())
						ReleaseCapture();
					GrabbedElement->Release();
				}
				GrabbedElement = nullptr;
				redraw = TRUE;
			}
			return redraw;
		}
		inline BOOL MouseMoved(HWND hwnd, std::vector<UIElementBase*>& elements, int x, int y)
		{
			BOOL redraw { FALSE };
			if (!MouseinWindow)
			{
				MouseinWindow = TRUE;
				TRACKMOUSEEVENT mouseEvent = { sizeof(TRACKMOUSEEVENT), TME_LEAVE, hwnd, HOVER_DEFAULT };
				TrackMouseEvent(&mouseEvent);
			}
			HoverElement = nullptr;
			for (UIElementBase* element : elements)
			{
				if (element->HitTest(x, y))
				{
					HoverElement = element;
					break;
				}
			}
			if (HoverElement != PrevHoverElement)
				redraw = TRUE;
			PrevHoverElement = HoverElement;
			if (GrabbedElement && GrabbedElement != HoverElement && !GrabbedElement->isGrabLocked())
				GrabbedElement = nullptr;
			if (GrabbedElement)
				if (GrabbedElement->MouseMoved(x, y))
					redraw = TRUE;
			return redraw;
		}
		inline BOOL KeyDown(int value)
		{
			if (FocusedElement)
				return	FocusedElement->KeyDown(value);
			return FALSE;
		}
		inline BOOL AddChar(char value)
		{
			if (FocusedElement)
				return	FocusedElement->AddChar(value);
			else
				return FALSE;
		}
		inline void LeaveWindow(HWND hwnd)
		{
			RECT rc;
			GetClientRect(hwnd, &rc);
			InvalidateRect(hwnd, &rc, TRUE);
			PrevHoverElement = nullptr;
			HoverElement = nullptr;
			GrabbedElement = nullptr;
			MouseinWindow = FALSE;
		}
		UIElementBase* PrevHoverElement = nullptr;
		UIElementBase* HoverElement = nullptr;
		UIElementBase* GrabbedElement = nullptr;
		UIElementBase* FocusedElement = nullptr;
		MOUSEBUTTONS GrabButton = LMB;
		BOOL MouseinWindow = FALSE;
	}m_UI_State;
	std::vector<UIElementBase*> m_Elements;
};