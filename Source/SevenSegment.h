#pragma once
#include "PCH.h"

#define DIGITCOUNT 13

class TimeString;
class TimeStringSmall;

class SevenSegment
{
public:
	void Init(ID2D1Factory2* pD2DFactory, ID2D1DeviceContext* dc, float size, float skew, D2D1::ColorF color1, D2D1::ColorF color2);
	void DrawDigits(ID2D1DeviceContext* dc, const TimeString& timeString, D2D1::Matrix3x2F transform);
	void DrawDigits(ID2D1DeviceContext* dc, const TimeStringSmall& timeString, D2D1::Matrix3x2F transform);
	void LocalDrawDigits(ID2D1DeviceContext* dc, int timeoffset, D2D1::Matrix3x2F transform);
	float GetDigitSpacing(char value, char nextvalue);
	float GetDigitWidth() { return m_DigitWidth; }
	float GetDigitHeight() { return m_DigitHeight; }
	float GetStringWidth(const TimeString& timeString);
	float GetStringWidth(const TimeStringSmall& timeString);
	D2D1::Matrix3x2F GetSkewMatrix() { return m_Skew; }
private:
	ID2D1Effect* GetBitmap(char value);
	void RasterizeDigit(ID2D1BitmapRenderTarget* bitmapRT, ID2D1PathGeometry* geometry, BYTE digit, const D2D1::Matrix3x2F& transform, ID2D1SolidColorBrush* fillbrush, ID2D1SolidColorBrush* outlinebrush);
	ComPtr<ID2D1Effect> Bitmaps[DIGITCOUNT]; // Holds the 10 digit characters and colon and period
	float m_Length = 0.0f;
	float m_CornerLength = 0.0f;
	float m_Width = 0.0f;
	float m_DigitWidth = 0.0f;
	float m_DigitHeight = 0.0f;
	D2D1::Matrix3x2F m_Skew = D2D1::Matrix3x2F::Identity();
};