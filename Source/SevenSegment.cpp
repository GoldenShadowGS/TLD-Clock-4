#include "PCH.h"
#include "Renderer.h"
#include "SevenSegment.h"
#include "TimeString.h"

void SevenSegment::Init(ID2D1Factory2* pD2DFactory, ID2D1DeviceContext* dc, float size, float skew, D2D1::ColorF color1, D2D1::ColorF color2)
{
	constexpr float padding = 1.1f;

	m_Length = size / 2.0f;
	m_Width = m_Length * 0.2f;
	m_CornerLength = m_Length - m_Width;

	m_DigitWidth = (m_Length * 2.0f + m_Width * 2.0f) * padding;
	m_DigitHeight = (m_Length * 4.0f + m_Width * 2.0f) * padding;
	//m_Scale = size / 20.0f;

	float shadowoffset = m_Width * 1.2f;

	//pulls in segment so there is a gap between them.
	const float pull = m_Length * 0.22f;
	const float length = m_Length - pull;
	const float cornerlength = m_CornerLength - pull;
	const float width = m_Width;
	ComPtr<ID2D1PathGeometry> Geometry;
	ComPtr<ID2D1GeometrySink> Sink;
	HR(pD2DFactory->CreatePathGeometry(Geometry.ReleaseAndGetAddressOf()));
	HR(Geometry->Open(Sink.ReleaseAndGetAddressOf()));
	Sink->SetFillMode(D2D1_FILL_MODE_WINDING);
	D2D1_POINT_2F p1 = { -length, 0.0f };
	D2D1_POINT_2F p2 = { -cornerlength,-width };
	D2D1_POINT_2F p3 = { cornerlength  ,-width };
	D2D1_POINT_2F p4 = { length, 0.0f };
	D2D1_POINT_2F p5 = { cornerlength, width };
	D2D1_POINT_2F p6 = { -cornerlength, width };

	Sink->BeginFigure(p1, D2D1_FIGURE_BEGIN_FILLED);
	Sink->AddLine(p2);
	Sink->AddLine(p3);
	Sink->AddLine(p4);
	Sink->AddLine(p5);
	Sink->AddLine(p6);
	Sink->EndFigure(D2D1_FIGURE_END_CLOSED);
	HR(Sink->Close());

	ComPtr<ID2D1SolidColorBrush> FillBrush;
	HR(dc->CreateSolidColorBrush(color1, FillBrush.ReleaseAndGetAddressOf()));

	ComPtr<ID2D1SolidColorBrush> OutlineBrush;
	HR(dc->CreateSolidColorBrush(color2, OutlineBrush.ReleaseAndGetAddressOf()));

	D2D1_SIZE_F BitmapSize = { GetDigitWidth() + 2.0f, GetDigitHeight() + 2.0f }; // Adding a 1 pixel border
	auto CreateBitmaps = [&] (BYTE digit, ID2D1SolidColorBrush* fillbrush, ID2D1SolidColorBrush* outlinebrush)
		{
			ComPtr<ID2D1BitmapRenderTarget> BitmapRenderTarget;
			ComPtr<ID2D1Bitmap> Bitmap;
			HR(dc->CreateCompatibleRenderTarget(BitmapSize, &BitmapRenderTarget));
			BitmapRenderTarget->BeginDraw();

			D2D1::Matrix3x2F transform = D2D1::Matrix3x2F::Translation(BitmapSize.width / 2.0f, BitmapSize.height / 2.0f);
			RasterizeDigit(BitmapRenderTarget.Get(), Geometry.Get(), digit, transform, fillbrush, outlinebrush);

			HR(BitmapRenderTarget->EndDraw());
			HR(BitmapRenderTarget->GetBitmap(Bitmap.ReleaseAndGetAddressOf()));

			ComPtr<ID2D1Effect> ShadowEffect;
			ComPtr<ID2D1Effect> AffineTransformEffect;
			ComPtr<ID2D1Effect> CompositeEffect;
			HR(dc->CreateEffect(CLSID_D2D1Shadow, ShadowEffect.ReleaseAndGetAddressOf()));
			HR(dc->CreateEffect(CLSID_D2D12DAffineTransform, AffineTransformEffect.ReleaseAndGetAddressOf()));
			HR(dc->CreateEffect(CLSID_D2D1Composite, CompositeEffect.ReleaseAndGetAddressOf()));
			ShadowEffect->SetInput(0, Bitmap.Get());

			AffineTransformEffect->SetInputEffect(0, ShadowEffect.Get());
			D2D1_MATRIX_3X2_F matrix = D2D1::Matrix3x2F::Translation(shadowoffset, shadowoffset);
			AffineTransformEffect->SetValue(D2D1_2DAFFINETRANSFORM_PROP_TRANSFORM_MATRIX, matrix);

			CompositeEffect->SetInputEffect(0, AffineTransformEffect.Get());
			CompositeEffect->SetInput(1, Bitmap.Get());
			return CompositeEffect;
		};
	for (int i = 0; i < DIGITCOUNT; i++)
	{
		Bitmaps[i] = CreateBitmaps(i, FillBrush.Get(), OutlineBrush.Get());
	}

	D2D1::Matrix3x2F translationdown = D2D1::Matrix3x2F::Translation(0.0f, GetDigitHeight() / 2.0f);
	D2D1::Matrix3x2F skewmatrix = D2D1::Matrix3x2F::Skew(skew, 0.0f);
	D2D1::Matrix3x2F translationup = D2D1::Matrix3x2F::Translation(0.0f, -GetDigitHeight() / 2.0f);
	m_Skew = translationup * skewmatrix * translationdown;
}

float SevenSegment::GetStringWidth(const TimeString& timeString)
{
	const int size = timeString.GetSize();
	float stringwidth = 0;
	for (int i = 0; i < size; i++)
	{
		char value = timeString.GetChar(i);
		char nextvalue = timeString.GetChar(i + 1);
		stringwidth += GetDigitSpacing(value, nextvalue);
	}
	return stringwidth;
}

float SevenSegment::GetStringWidth(const TimeStringSmall& timeString)
{
	const int size = timeString.GetSize();
	float stringwidth = 0;
	for (int i = 0; i < size; i++)
	{
		char value = timeString.GetChar(i);
		char nextvalue = timeString.GetChar(i + 1);
		stringwidth += GetDigitSpacing(value, nextvalue);
	}
	return stringwidth;
}

void SevenSegment::DrawDigits(ID2D1DeviceContext* dc, const TimeString& timeString, D2D1::Matrix3x2F transform)
{
	D2D1::Matrix3x2F maintransform = m_Skew * transform;
	D2D1::Matrix3x2F digittransform = D2D1::Matrix3x2F::Identity();
	D2D1_RECT_F rect = D2D1::RectF(0.0f, 0.0f, GetDigitWidth(), GetDigitHeight());
	const int size = timeString.GetSize();

	for (int i = 0; i < size; i++)
	{
		char value = timeString.GetChar(i);
		char nextvalue = timeString.GetChar(i + 1);
		ID2D1Effect* Digit = GetBitmap(value);
		if (i == size - 1)
			dc->SetTransform(D2D1::Matrix3x2F::Scale(0.7f, 0.7f) * D2D1::Matrix3x2F::Translation(0.0f, GetDigitHeight() * 0.3f) * digittransform * maintransform);
		else if (value == 'd')
			dc->SetTransform(D2D1::Matrix3x2F::Scale(0.7f, 0.7f) * D2D1::Matrix3x2F::Translation(0.0f, GetDigitHeight() * 0.3f) * digittransform * maintransform);
		else
			dc->SetTransform(digittransform * maintransform);
		digittransform = D2D1::Matrix3x2F::Translation(GetDigitSpacing(value, nextvalue), 0.0f) * digittransform;
		if (Digit)
		{
			dc->DrawImage(Digit);
		}
	}
}

void SevenSegment::DrawDigits(ID2D1DeviceContext* dc, const TimeStringSmall& timeString, D2D1::Matrix3x2F transform)
{
	D2D1::Matrix3x2F maintransform = m_Skew * transform;
	D2D1::Matrix3x2F digittransform = D2D1::Matrix3x2F::Identity();
	D2D1_RECT_F rect = D2D1::RectF(0.0f, 0.0f, GetDigitWidth(), GetDigitHeight());
	const int size = timeString.GetSize();

	for (int i = 0; i < size; i++)
	{
		char value = timeString.GetChar(i);
		char nextvalue = timeString.GetChar(i + 1);
		ID2D1Effect* Digit = GetBitmap(value);
		if (i == size - 1)
			dc->SetTransform(D2D1::Matrix3x2F::Scale(0.7f, 0.7f) * D2D1::Matrix3x2F::Translation(0.0f, GetDigitHeight() * 0.3f) * digittransform * maintransform);
		else if (value == 'd')
			dc->SetTransform(D2D1::Matrix3x2F::Scale(0.7f, 0.7f) * D2D1::Matrix3x2F::Translation(0.0f, GetDigitHeight() * 0.3f) * digittransform * maintransform);
		else
			dc->SetTransform(digittransform * maintransform);
		digittransform = D2D1::Matrix3x2F::Translation(GetDigitSpacing(value, nextvalue), 0.0f) * digittransform;
		if (Digit)
		{
			dc->DrawImage(Digit);
		}
	}
}

void SevenSegment::LocalDrawDigits(ID2D1DeviceContext* dc, int timeoffset, D2D1::Matrix3x2F transform)
{
	D2D1::Matrix3x2F maintransform = m_Skew * transform;
	D2D1::Matrix3x2F digittransform = D2D1::Matrix3x2F::Identity();
	D2D1_RECT_F rect = D2D1::RectF(0.0f, 0.0f, GetDigitWidth(), GetDigitHeight());
	int hours = (timeoffset / 60) % 12;
	if (hours == 0)
		hours = 12;
	int minutes = timeoffset % 60;
	char digits[6] =
	{
		char((hours / 10) + 48),
		char((hours % 10) + 48),
		':',
		char((minutes / 10) + 48),
		char((minutes % 10) + 48),
		'0'
	};

	if (digits[0] == '0')
	{
		digits[0] = 0;
		digittransform = digittransform * D2D1::Matrix3x2F::Translation(-GetDigitWidth() / 2.0f, 0.0f);
	}

	for (int i = 0; i < 5; i++)
	{
		ID2D1Effect* Digit = GetBitmap(digits[i]);
		dc->SetTransform(digittransform * maintransform);
		digittransform = digittransform * D2D1::Matrix3x2F::Translation(GetDigitWidth(), 0.0f);
		if (Digit)
		{
			dc->DrawImage(Digit);
		}
	}
}

ID2D1Effect* SevenSegment::GetBitmap(char value)
{
	switch (value)
	{
	case '0':
		return Bitmaps[0].Get();
	case '1':
		return Bitmaps[1].Get();
	case '2':
		return Bitmaps[2].Get();
	case '3':
		return Bitmaps[3].Get();
	case '4':
		return Bitmaps[4].Get();
	case '5':
		return Bitmaps[5].Get();
	case '6':
		return Bitmaps[6].Get();
	case '7':
		return Bitmaps[7].Get();
	case '8':
		return Bitmaps[8].Get();
	case '9':
		return Bitmaps[9].Get();
	case ':':
		return Bitmaps[10].Get();
	case '.':
		return Bitmaps[11].Get();
	case 'd':
		return Bitmaps[12].Get();
	default:
		return nullptr;
	}
}

float SevenSegment::GetDigitSpacing(char value, char nextvalue)
{
	float digitWidth = GetDigitWidth() * 1.1f;
	float periodwidth = GetDigitWidth() * 0.75f;
	if (value == ':' || value == '.' || nextvalue == ':' || nextvalue == '.')
		return periodwidth;
	else
		return digitWidth;
}

void SevenSegment::RasterizeDigit(ID2D1BitmapRenderTarget* bitmapRT, ID2D1PathGeometry* geometry, BYTE digit, const D2D1::Matrix3x2F& transform, ID2D1SolidColorBrush* fillbrush, ID2D1SolidColorBrush* outlinebrush)
{
	const float Offset = m_Length;
	const float bigoffset = Offset * 2.0f;
	const float outlinewidth = m_Length * 0.25f;

	const D2D1::Matrix3x2F transforms[7] =
	{
		D2D1::Matrix3x2F::Translation(0.0f, -bigoffset),
		D2D1::Matrix3x2F::Rotation(90, { 0.0f, 0.0f }) * D2D1::Matrix3x2F::Translation(-Offset, -Offset),
		D2D1::Matrix3x2F::Rotation(90, { 0.0f, 0.0f }) * D2D1::Matrix3x2F::Translation(Offset, -Offset),
		D2D1::Matrix3x2F::Translation(0.0f, 0.0f),
		D2D1::Matrix3x2F::Rotation(90, { 0.0f, 0.0f }) * D2D1::Matrix3x2F::Translation(-Offset, Offset),
		D2D1::Matrix3x2F::Rotation(90, { 0.0f, 0.0f }) * D2D1::Matrix3x2F::Translation(Offset, Offset),
		D2D1::Matrix3x2F::Translation(0.0f, bigoffset)
	};

	auto drawsegment = [&] (int segmentindex)
		{
			bitmapRT->SetTransform(transforms[segmentindex] * transform);
			bitmapRT->DrawGeometry(geometry, outlinebrush, outlinewidth);
			bitmapRT->FillGeometry(geometry, fillbrush);
		};

	float dotradius = m_Width;
	switch (digit)
	{
	case 10: // colon
	{
		D2D1_ELLIPSE ellipse = { {0.0f, -m_Length}, dotradius, dotradius };
		bitmapRT->SetTransform(transform);
		bitmapRT->DrawEllipse(ellipse, outlinebrush, outlinewidth);
		bitmapRT->FillEllipse(ellipse, fillbrush);
		ellipse = { {0.0f, m_Length}, dotradius, dotradius };
		bitmapRT->DrawEllipse(ellipse, outlinebrush, outlinewidth);
		bitmapRT->FillEllipse(ellipse, fillbrush);
		break;
	}
	case 11: // dot
	{
		D2D1_ELLIPSE ellipse = { {0.0f, m_Length * 2.0f}, dotradius, dotradius };
		bitmapRT->SetTransform(transform);
		bitmapRT->DrawEllipse(ellipse, outlinebrush, outlinewidth);
		bitmapRT->FillEllipse(ellipse, fillbrush);
		break;
	}
	case 12: // 'd'
		drawsegment(2);
		drawsegment(3);
		drawsegment(4);
		drawsegment(5);
		drawsegment(6);
		break;
	case 0:
		drawsegment(0);
		drawsegment(1);
		drawsegment(2);
		drawsegment(4);
		drawsegment(5);
		drawsegment(6);
		break;
	case 1:
		drawsegment(2);
		drawsegment(5);
		break;
	case 2:
		drawsegment(0);
		drawsegment(2);
		drawsegment(3);
		drawsegment(4);
		drawsegment(6);
		break;
	case 3:
		drawsegment(0);
		drawsegment(2);
		drawsegment(3);
		drawsegment(5);
		drawsegment(6);
		break;
	case 4:
		drawsegment(1);
		drawsegment(2);
		drawsegment(3);
		drawsegment(5);
		break;
	case 5:
		drawsegment(0);
		drawsegment(1);
		drawsegment(3);
		drawsegment(5);
		drawsegment(6);
		break;
	case 6:
		drawsegment(0);
		drawsegment(1);
		drawsegment(3);
		drawsegment(4);
		drawsegment(5);
		drawsegment(6);
		break;
	case 7:
		drawsegment(0);
		drawsegment(2);
		drawsegment(5);
		break;
	case 8:
		drawsegment(0);
		drawsegment(1);
		drawsegment(2);
		drawsegment(3);
		drawsegment(4);
		drawsegment(5);
		drawsegment(6);
		break;
	case 9:
		drawsegment(0);
		drawsegment(1);
		drawsegment(2);
		drawsegment(3);
		drawsegment(5);
		drawsegment(6);
		break;
	}
}
