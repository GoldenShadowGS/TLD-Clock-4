#pragma once
#include "PCH.h"

struct Image
{
	ComPtr<ID2D1Bitmap> m_Bitmap;
	D2D1_POINT_2F m_Pivot = {};
	D2D1_SIZE_F m_Size = {};
};

Image LoadBitmapImage(ID2D1DeviceContext* dc, int resource);
Image CreateTextImage(ID2D1DeviceContext* dc, const std::wstring& text, float width,float height, float fontsize, float verticaloffset);
Image CreateSunImage(ID2D1Factory2* factory, ID2D1DeviceContext* dc, float size);
Image CreateMoonImage(ID2D1Factory2* factory, ID2D1DeviceContext* dc, float size);
Image CreatePlayImage(ID2D1Factory2* factory, ID2D1DeviceContext* dc, float size);
Image CreatePauseImage(ID2D1Factory2* factory, ID2D1DeviceContext* dc, float size);