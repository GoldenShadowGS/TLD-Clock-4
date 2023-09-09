#include "PCH.h"
#include "Image.h"
#include "Renderer.h"
#include "Math.h"
#include "ResourceLoader.h"

static std::vector<BYTE> FlipImage32bit(std::vector<BYTE>& Image, int pitch, int height)
{
	std::vector<BYTE> FlippedPixels(Image.size());
	//FlippedPixels.resize(Image.size());
	for (UINT64 row = 0; row < height; row++)
	{
		BYTE* SrcRow = Image.data() + (row * pitch);
		BYTE* DestRow = FlippedPixels.data() + (((height - 1LL) - row) * pitch);
		memcpy(DestRow, SrcRow, pitch);
	}
	return FlippedPixels;
}

static std::vector<BYTE> PreMultiply(std::vector<BYTE>& Image, int width, int height)
{
	std::vector<BYTE> PreMultipliedPixels(Image.size());
	DWORD* pPixelData = (DWORD*)Image.data();
	DWORD* pDestPixel = (DWORD*)PreMultipliedPixels.data();

	for (int j = 0; j < height; ++j)
	{
		for (int i = 0; i < width; ++i)
		{
			int index = (height - j - 1) * width + i;

			DWORD d = pPixelData[index];

			BYTE a = d >> 24;
			BYTE pmR = static_cast<BYTE>(((d & 0x00FF0000) >> 16) * a / 255);
			BYTE pmG = static_cast<BYTE>(((d & 0x0000FF00) >> 8) * a / 255);
			BYTE pmB = static_cast<BYTE>(((d & 0x000000FF)) * a / 255);
			d = pmB | (pmG << 8) | (pmR << 16) | (a << 24);

			pDestPixel[index] = d;
		}
	}
	return PreMultipliedPixels;
}


inline UINT Align(UINT uLocation, UINT uAlign)
{
	return ((uLocation + (uAlign - 1)) & ~(uAlign - 1));
}

static std::vector<BYTE> FileLoader(_In_ int resource, _Out_ UINT& width, _Out_ UINT& height, _Out_ UINT& pitch)
{
	std::vector<BYTE> Pixels8Bit;
	struct BMPHeader
	{
		char bm[2];
		unsigned int bmpfilesize;
		short appspecific1;
		short appspecific2;
		unsigned int pixeldataoffset;
	} bmpHeader = {};
	struct DIBHeader
	{
		unsigned int dibSize;
		unsigned int width;
		unsigned int height;
		short colorplane;
		short bitsperpixel;
		unsigned int pixelcompression;
		unsigned int rawPixelDataSize;
	} dibHeader = {};

	ResourceLoader resLoader(resource, L"BITMAPDATA");
	resLoader.Read(bmpHeader.bm, 2);
	if (!(bmpHeader.bm[0] == 'B' && bmpHeader.bm[1] == 'M'))
		throw std::exception("Bad Bitmap");
	resLoader.Read(&bmpHeader.bmpfilesize, sizeof(bmpHeader.bmpfilesize));
	resLoader.Read(&bmpHeader.appspecific1, sizeof(bmpHeader.appspecific1));
	resLoader.Read(&bmpHeader.appspecific2, sizeof(bmpHeader.appspecific2));
	resLoader.Read(&bmpHeader.pixeldataoffset, sizeof(bmpHeader.pixeldataoffset));
	resLoader.Read(&dibHeader.dibSize, sizeof(dibHeader.dibSize));
	resLoader.Read(&dibHeader.width, sizeof(dibHeader.width));
	resLoader.Read(&dibHeader.height, sizeof(dibHeader.height));
	resLoader.Read(&dibHeader.colorplane, sizeof(dibHeader.colorplane));
	resLoader.Read(&dibHeader.bitsperpixel, sizeof(dibHeader.bitsperpixel));
	resLoader.Read(&dibHeader.pixelcompression, sizeof(dibHeader.pixelcompression));
	resLoader.Read(&dibHeader.rawPixelDataSize, sizeof(dibHeader.rawPixelDataSize));

	width = dibHeader.width;
	height = dibHeader.height;
	pitch = Align(dibHeader.width * 4, 16);

	const size_t buffersize = dibHeader.rawPixelDataSize;
	Pixels8Bit.resize(buffersize);
	resLoader.Seek(bmpHeader.pixeldataoffset);
	resLoader.Read(Pixels8Bit.data(), buffersize);

	return Pixels8Bit;
}

Image LoadBitmapImage(ID2D1DeviceContext* dc, int resource)
{
	Image image = {};

	UINT PixelWidth;
	UINT PixelHeight;
	UINT Pitch;
	std::vector<BYTE> Pixels32Bit = FileLoader(resource, PixelWidth, PixelHeight, Pitch);
	std::vector<BYTE> FlippedPixels32Bit = FlipImage32bit(Pixels32Bit, Pitch, PixelHeight);
	std::vector<BYTE> premultipliedpixels = PreMultiply(FlippedPixels32Bit, PixelWidth, PixelHeight);
	D2D1_SIZE_U bitmapsize = {};
	bitmapsize.width = PixelWidth;
	bitmapsize.height = PixelHeight;
	image.m_Size = { (float)PixelWidth, (float)PixelHeight };

	// m_Pivot is the Center of the bitmap for positioning and rotation
	image.m_Pivot = { image.m_Size.width / 2.0f, image.m_Size.height / 2.0f };

	// Resource Bitmap
	D2D1_BITMAP_PROPERTIES bitmapprops = {};
	bitmapprops.pixelFormat.format = DXGI_FORMAT_B8G8R8A8_UNORM;
	bitmapprops.pixelFormat.alphaMode = D2D1_ALPHA_MODE_PREMULTIPLIED;
	bitmapprops.dpiX = 96.0f;
	bitmapprops.dpiY = 96.0f;

	HR(dc->CreateBitmap(bitmapsize, premultipliedpixels.data(), Pitch, bitmapprops, image.m_Bitmap.ReleaseAndGetAddressOf()));

	return image;
}

Image CreateTextImage(ID2D1DeviceContext* dc, const std::wstring& text, float width, float height, float fontsize, float verticaloffset)
{
	Image image = {};
	image.m_Size = { width, height };
	image.m_Pivot = { width / 2.0f, height / 2.0f };
	image.m_Bitmap;
	{
		ComPtr<IDWriteFactory> DWriteFactory;
		ComPtr<IDWriteTextFormat> TextFormat;

		HR(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(DWriteFactory.ReleaseAndGetAddressOf())));

		HR(DWriteFactory->CreateTextFormat(L"Arial", NULL, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontsize, L"en-us", TextFormat.ReleaseAndGetAddressOf()));

		TextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		ComPtr<ID2D1BitmapRenderTarget> BitmapRenderTarget;
		HR(dc->CreateCompatibleRenderTarget(image.m_Size, &BitmapRenderTarget));

		ComPtr<ID2D1SolidColorBrush> Brush;
		HR(BitmapRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f), Brush.ReleaseAndGetAddressOf()));

		BitmapRenderTarget->BeginDraw();

		BitmapRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(image.m_Pivot.x, image.m_Pivot.y));

		D2D1_RECT_F rect = { -image.m_Pivot.x, -image.m_Pivot.y + verticaloffset, image.m_Pivot.x, image.m_Pivot.y };
		//D2D1_RECT_F rect = { 0, 0, image.m_Size.width, image.m_Size.height };
		BitmapRenderTarget->DrawTextW(text.c_str(), (UINT)text.size(), TextFormat.Get(), rect, Brush.Get());

		HR(BitmapRenderTarget->EndDraw());

		HR(BitmapRenderTarget->GetBitmap(image.m_Bitmap.ReleaseAndGetAddressOf()));
	}
	return image;
}

Image CreateSunImage(ID2D1Factory2* factory, ID2D1DeviceContext* dc, float size)
{
	Image image = {};
	image.m_Size = { size, size };
	image.m_Pivot = { size / 2.0f, size / 2.0f };

	// Create Shape Geometry 

	ComPtr<ID2D1PathGeometry> Geometry;
	{
		ComPtr<ID2D1GeometrySink> Sink;
		HR(factory->CreatePathGeometry(Geometry.ReleaseAndGetAddressOf()));
		HR(Geometry->Open(Sink.ReleaseAndGetAddressOf()));
		Sink->SetFillMode(D2D1_FILL_MODE_WINDING);

		float sunradius = size / 4.0f;
		float trianglesize = sunradius * 0.3f;
		float angle = 0.0f;
		D2D1_POINT_2F p1 = { -trianglesize, 0.0f };
		D2D1_POINT_2F p2 = { trianglesize, 0.0f };
		D2D1_POINT_2F p3 = { 0.0f, -trianglesize * 2.0f };
		D2D1_POINT_2F baseoffset = { 0.0f, -sunradius * 1.1f };
		for (int i = 0; i < 8; i++)
		{
			angle = PI2 * ((float)i / 8.0f);
			D2D1_POINT_2F offset = rotate(baseoffset, angle);
			Sink->BeginFigure(translate(rotate(p1, angle), offset), D2D1_FIGURE_BEGIN_FILLED);
			Sink->AddLine(translate(rotate(p2, angle), offset));
			Sink->AddLine(translate(rotate(p3, angle), offset));
			Sink->EndFigure(D2D1_FIGURE_END_CLOSED);
		}
		HR(Sink->Close());
	}
	// Render Geometry to Bitmap Target
	{
		ComPtr<ID2D1GradientStopCollection> StopsCollection;
		ComPtr<ID2D1RadialGradientBrush> RadialBackGroundBrush;
		ComPtr<ID2D1BitmapRenderTarget> BitmapRenderTarget;
		HR(dc->CreateCompatibleRenderTarget(image.m_Size, &BitmapRenderTarget));

		ComPtr<ID2D1SolidColorBrush> Brush;
		HR(BitmapRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.95f, 0.95f, 0.95f, 0.8f), Brush.ReleaseAndGetAddressOf()));

		D2D1_GRADIENT_STOP gstop[3] = {};
		gstop[0].color = { 0.8f, 0.2f, 0.8f, 0.6f };
		gstop[0].position = { 0.0f };
		gstop[1].color = { 0.8f, 0.2f, 0.8f, 0.6f };
		gstop[1].position = { 0.5f };
		gstop[2].color = { 0.8f, 0.2f, 0.8f, 0.0f };
		gstop[2].position = { 1.0f };
		HR(BitmapRenderTarget->CreateGradientStopCollection(gstop, 3, StopsCollection.ReleaseAndGetAddressOf()));

		D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES radialprops = {};
		radialprops.center = D2D1::Point2F(0.0f, 0.0f);
		radialprops.radiusX = size / 2.0f;
		radialprops.radiusY = size / 2.0f;
		radialprops.gradientOriginOffset = D2D1::Point2F(0.0f, 0.0f);
		HR(BitmapRenderTarget->CreateRadialGradientBrush(radialprops, StopsCollection.Get(), RadialBackGroundBrush.ReleaseAndGetAddressOf()));

		BitmapRenderTarget->BeginDraw();

		BitmapRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(image.m_Pivot.x, image.m_Pivot.y));

		const float bkgRadius = 8.0f;

		D2D1_ELLIPSE bkgellipse = { { 0.0f, 0.0f }, bkgRadius, bkgRadius };
		BitmapRenderTarget->DrawEllipse(bkgellipse, RadialBackGroundBrush.Get(), size);

		D2D1_ELLIPSE sunellipse = { { 0.0f, 0.0f }, size / 4.0f, size / 4.0f };
		BitmapRenderTarget->FillEllipse(sunellipse, Brush.Get());

		BitmapRenderTarget->FillGeometry(Geometry.Get(), Brush.Get());

		HR(BitmapRenderTarget->EndDraw());

		HR(BitmapRenderTarget->GetBitmap(image.m_Bitmap.ReleaseAndGetAddressOf()));
	}

	return image;
}

Image CreateMoonImage(ID2D1Factory2* factory, ID2D1DeviceContext* dc, float size)
{
	Image image = {};
	image.m_Size = { size, size };
	image.m_Pivot = { size / 2.0f, size / 2.0f };

	// Create Shape Geometry 

	ComPtr<ID2D1PathGeometry> Geometry;
	{
		ComPtr<ID2D1GeometrySink> Sink;
		HR(factory->CreatePathGeometry(Geometry.ReleaseAndGetAddressOf()));
		HR(Geometry->Open(Sink.ReleaseAndGetAddressOf()));
		Sink->SetFillMode(D2D1_FILL_MODE_WINDING);

		float moonradius = size / 3.0f;
		float angle1 = 0.95f;
		float angle2 = -0.95f;
		D2D1_POINT_2F p1 = { cos(angle1) * moonradius, sin(angle1) * moonradius };
		D2D1_POINT_2F p2 = { cos(angle2) * moonradius, sin(angle2) * moonradius };
		Sink->BeginFigure(p1, D2D1_FIGURE_BEGIN_FILLED);
		Sink->AddArc(D2D1::ArcSegment(p2, D2D1::SizeF(moonradius, moonradius), 0.0f, D2D1_SWEEP_DIRECTION_CLOCKWISE, D2D1_ARC_SIZE_LARGE));
		Sink->AddArc(D2D1::ArcSegment(p1, D2D1::SizeF(moonradius * 0.7f, moonradius * 0.7f), 0.0f, D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE, D2D1_ARC_SIZE_LARGE));

		Sink->EndFigure(D2D1_FIGURE_END_CLOSED);
		HR(Sink->Close());
	}
	// Render Geometry to Bitmap Target
	{
		ComPtr<ID2D1GradientStopCollection> StopsCollection;
		ComPtr<ID2D1RadialGradientBrush> RadialBackGroundBrush;
		ComPtr<ID2D1BitmapRenderTarget> BitmapRenderTarget;
		HR(dc->CreateCompatibleRenderTarget(image.m_Size, &BitmapRenderTarget));

		ComPtr<ID2D1SolidColorBrush> Brush;
		HR(BitmapRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.95f, 0.95f, 0.95f, 0.8f), Brush.ReleaseAndGetAddressOf()));

		D2D1_GRADIENT_STOP gstop[3] = {};
		gstop[0].color = { 0.8f, 0.2f, 0.8f, 0.6f };
		gstop[0].position = { 0.0f };
		gstop[1].color = { 0.8f, 0.2f, 0.8f, 0.6f };
		gstop[1].position = { 0.5f };
		gstop[2].color = { 0.8f, 0.2f, 0.8f, 0.0f };
		gstop[2].position = { 1.0f };
		HR(BitmapRenderTarget->CreateGradientStopCollection(gstop, 3, StopsCollection.ReleaseAndGetAddressOf()));

		D2D1_RADIAL_GRADIENT_BRUSH_PROPERTIES radialprops = {};
		radialprops.center = D2D1::Point2F(0.0f, 0.0f);
		radialprops.radiusX = size / 2.0f;
		radialprops.radiusY = size / 2.0f;
		radialprops.gradientOriginOffset = D2D1::Point2F(0.0f, 0.0f);
		HR(BitmapRenderTarget->CreateRadialGradientBrush(radialprops, StopsCollection.Get(), RadialBackGroundBrush.ReleaseAndGetAddressOf()));

		BitmapRenderTarget->BeginDraw();

		BitmapRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(image.m_Pivot.x, image.m_Pivot.y));

		const float bkgRadius = 8.0f;

		D2D1_ELLIPSE bkgellipse = { { 0.0f, 0.0f }, bkgRadius, bkgRadius };
		BitmapRenderTarget->DrawEllipse(bkgellipse, RadialBackGroundBrush.Get(), size);
		BitmapRenderTarget->FillGeometry(Geometry.Get(), Brush.Get());
		HR(BitmapRenderTarget->EndDraw());
		HR(BitmapRenderTarget->GetBitmap(image.m_Bitmap.ReleaseAndGetAddressOf()));
	}

	return image;
}

Image CreatePlayImage(ID2D1Factory2* factory, ID2D1DeviceContext* dc, float size)
{
	Image image = {};
	image.m_Size = { size, size };
	image.m_Pivot = { size / 2.0f, size / 2.0f };

	ComPtr<ID2D1SolidColorBrush> Brush;
	HR(dc->CreateSolidColorBrush(D2D1::ColorF(0.35f, 0.35f, 0.35f, 1.0f), Brush.ReleaseAndGetAddressOf()));

	ComPtr<ID2D1PathGeometry> Geometry;
	ComPtr<ID2D1GeometrySink> Sink;
	HR(factory->CreatePathGeometry(Geometry.ReleaseAndGetAddressOf()));
	HR(Geometry->Open(Sink.ReleaseAndGetAddressOf()));
	Sink->SetFillMode(D2D1_FILL_MODE_WINDING);

	float radius = size * 0.375f;
	float angle = 0.0f;
	D2D1_POINT_2F p1 = { cos(angle) * radius, sin(angle) * radius };
	angle = ThirdPI;
	D2D1_POINT_2F p2 = { cos(angle) * radius, sin(angle) * radius };
	angle = ThirdPI * 2.0f;
	D2D1_POINT_2F p3 = { cos(angle) * radius, sin(angle) * radius };
	Sink->BeginFigure(p1, D2D1_FIGURE_BEGIN_FILLED);
	Sink->AddLine(p2);
	Sink->AddLine(p3);
	Sink->EndFigure(D2D1_FIGURE_END_CLOSED);
	HR(Sink->Close());

	ComPtr<ID2D1BitmapRenderTarget> BitmapRenderTarget;
	HR(dc->CreateCompatibleRenderTarget(image.m_Size, &BitmapRenderTarget));
	BitmapRenderTarget->BeginDraw();
	BitmapRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(image.m_Pivot.x, image.m_Pivot.y));
	BitmapRenderTarget->FillGeometry(Geometry.Get(), Brush.Get());
	HR(BitmapRenderTarget->EndDraw());
	HR(BitmapRenderTarget->GetBitmap(image.m_Bitmap.ReleaseAndGetAddressOf()));

	return image;
}

Image CreatePauseImage(ID2D1Factory2* factory, ID2D1DeviceContext* dc, float size)
{
	Image image = {};
	image.m_Size = { size, size };
	image.m_Pivot = { size / 2.0f, size / 2.0f };

	ComPtr<ID2D1SolidColorBrush> Brush;
	HR(dc->CreateSolidColorBrush(D2D1::ColorF(0.35f, 0.35f, 0.35f, 1.0f), Brush.ReleaseAndGetAddressOf()));

	ComPtr<ID2D1PathGeometry> Geometry;
	ComPtr<ID2D1GeometrySink> Sink;
	HR(factory->CreatePathGeometry(Geometry.ReleaseAndGetAddressOf()));
	HR(Geometry->Open(Sink.ReleaseAndGetAddressOf()));
	Sink->SetFillMode(D2D1_FILL_MODE_WINDING);

	float radius = size * 0.3f;
	float width = radius * 0.26f;
	float height = radius * 1.0f;
	float spacing = radius * 0.5f;
	D2D1_POINT_2F p1 = { -width - spacing, -height };
	D2D1_POINT_2F p2 = { width - spacing, -height };
	D2D1_POINT_2F p3 = { width - spacing, height };
	D2D1_POINT_2F p4 = { -width - spacing, height };
	Sink->BeginFigure(p1, D2D1_FIGURE_BEGIN_FILLED);
	Sink->AddLine(p2);
	Sink->AddLine(p3);
	Sink->AddLine(p4);
	Sink->EndFigure(D2D1_FIGURE_END_CLOSED);
	p1 = { -width + spacing, -height };
	p2 = { width + spacing, -height };
	p3 = { width + spacing, height };
	p4 = { -width + spacing, height };
	Sink->BeginFigure(p1, D2D1_FIGURE_BEGIN_FILLED);
	Sink->AddLine(p2);
	Sink->AddLine(p3);
	Sink->AddLine(p4);
	Sink->EndFigure(D2D1_FIGURE_END_CLOSED);
	HR(Sink->Close());

	ComPtr<ID2D1BitmapRenderTarget> BitmapRenderTarget;
	HR(dc->CreateCompatibleRenderTarget(image.m_Size, &BitmapRenderTarget));
	BitmapRenderTarget->BeginDraw();
	BitmapRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(image.m_Pivot.x, image.m_Pivot.y));
	BitmapRenderTarget->FillGeometry(Geometry.Get(), Brush.Get());
	HR(BitmapRenderTarget->EndDraw());
	HR(BitmapRenderTarget->GetBitmap(image.m_Bitmap.ReleaseAndGetAddressOf()));

	return image;
}