// DWriteTry.cpp : Defines the entry point for the console application.
//
#include "stdafx.h"

ID2D1Factory*           g_pD2DFactory = NULL;
IDWriteFactory*         g_pDWriteFactory = NULL;
ID2D1RenderTarget*      g_pRenderTarget = NULL;

int _tmain(int argc, _TCHAR* argv[])
{
    HRESULT hr = S_OK;

    // Create a Direct2D factory.
    CHECKHR(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &g_pD2DFactory));

    // Create a DirectWrite factory.
    CHECKHR(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
        reinterpret_cast<IUnknown**>(&g_pDWriteFactory)));

    // Create a WIC factory
    IWICImagingFactory* pWICFactory = NULL;

    CHECKHR(CoInitialize(nullptr));
    CHECKHR(CoCreateInstance(
        CLSID_WICImagingFactory1,
        nullptr,
        CLSCTX_INPROC_SERVER,
        IID_IWICImagingFactory,
        (LPVOID*)&pWICFactory));

    // Create an IWICBitmap and RT
    IWICBitmap* pWICBitmap = NULL;
    static const UINT sc_bitmapWidth = 640;
    static const UINT sc_bitmapHeight = 480;
    CHECKHR(pWICFactory->CreateBitmap(
        sc_bitmapWidth,
        sc_bitmapHeight,
        GUID_WICPixelFormat32bppBGR,
        WICBitmapCacheOnLoad,
        &pWICBitmap
        ));

    // Set the render target type to D2D1_RENDER_TARGET_TYPE_DEFAULT to use software rendering.
    CHECKHR(g_pD2DFactory->CreateWicBitmapRenderTarget(
        pWICBitmap,
        D2D1::RenderTargetProperties(),
        &g_pRenderTarget
        ));

    // Create text format and a path geometry representing an hour glass. 
    IDWriteTextFormat*  pTextFormat = NULL;
    static const WCHAR sc_fontName[] = L"Calibri";
    static const FLOAT sc_fontSize = 50;

    CHECKHR(g_pDWriteFactory->CreateTextFormat(
        sc_fontName,
        NULL,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_NORMAL,
        sc_fontSize,
        L"", //locale
        &pTextFormat
        ));

    // Create a path geometry
    ID2D1PathGeometry* pPathGeometry = NULL;
    CHECKHR(pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER));
    CHECKHR(pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER));
    CHECKHR(g_pD2DFactory->CreatePathGeometry(&pPathGeometry));

    ID2D1GeometrySink* pSink = NULL;
    CHECKHR(pPathGeometry->Open(&pSink));

    pSink->SetFillMode(D2D1_FILL_MODE_ALTERNATE);

    pSink->BeginFigure(
        D2D1::Point2F(0, 0),
        D2D1_FIGURE_BEGIN_FILLED
        );

    pSink->AddLine(D2D1::Point2F(200, 0));

    pSink->AddBezier(
        D2D1::BezierSegment(
        D2D1::Point2F(150, 50),
        D2D1::Point2F(150, 150),
        D2D1::Point2F(200, 200))
        );

    pSink->AddLine(D2D1::Point2F(0, 200));

    pSink->AddBezier(
        D2D1::BezierSegment(
        D2D1::Point2F(50, 150),
        D2D1::Point2F(50, 50),
        D2D1::Point2F(0, 0))
        );

    pSink->EndFigure(D2D1_FIGURE_END_CLOSED);

    CHECKHR(pSink->Close());

    // Create GradientStopCollection
    ID2D1GradientStopCollection *pGradientStops = NULL;
    static const D2D1_GRADIENT_STOP stops[] =
    {
        { 0.f, { 0.f, 1.f, 1.f, 1.f } },
        { 1.f, { 0.f, 0.f, 1.f, 1.f } },
    };

    CHECKHR(g_pRenderTarget->CreateGradientStopCollection(
        stops,
        ARRAYSIZE(stops),
        &pGradientStops
        ));

    ID2D1LinearGradientBrush* pLGBrush = NULL;
    ID2D1SolidColorBrush* pBlackBrush = NULL;

    CHECKHR(g_pRenderTarget->CreateLinearGradientBrush(
        D2D1::LinearGradientBrushProperties(
        D2D1::Point2F(100, 0),
        D2D1::Point2F(100, 200)),
        D2D1::BrushProperties(),
        pGradientStops,
        &pLGBrush
        ));

    CHECKHR(g_pRenderTarget->CreateSolidColorBrush(
        D2D1::ColorF(D2D1::ColorF::Black),
        &pBlackBrush
        ));


    // Render into the bitmap.
    g_pRenderTarget->BeginDraw();
    g_pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
    D2D1_SIZE_F rtSize = g_pRenderTarget->GetSize();

    // Set the world transform to a 45 degree rotation at the center of the render target
    // and write "Hello, World".
    g_pRenderTarget->SetTransform(
        D2D1::Matrix3x2F::Rotation(
        45,
        D2D1::Point2F(
        rtSize.width / 2,
        rtSize.height / 2))
        );

    static const WCHAR sc_helloWorld[] = L"Hello, World!";
    g_pRenderTarget->DrawText(
        sc_helloWorld,
        ARRAYSIZE(sc_helloWorld) - 1,
        pTextFormat,
        D2D1::RectF(0, 0, rtSize.width, rtSize.height),
        pBlackBrush);

    // Reset back to the identity transform.
    g_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(0, rtSize.height - 200));
    g_pRenderTarget->FillGeometry(pPathGeometry, pLGBrush);
    g_pRenderTarget->SetTransform(D2D1::Matrix3x2F::Translation(rtSize.width - 200, 0));
    g_pRenderTarget->FillGeometry(pPathGeometry, pLGBrush);
    CHECKHR(g_pRenderTarget->EndDraw());

    // Save the image to a file.
    IWICStream* pStream = NULL;
    CHECKHR(pWICFactory->CreateStream(&pStream));

    WICPixelFormatGUID format = GUID_WICPixelFormatDontCare;

    // Use InitializeFromFilename to write to a file. If there is need to write inside the memory, use InitializeFromMemory. 
    static const WCHAR filename[] = L"output.png";
    CHECKHR(pStream->InitializeFromFilename(filename, GENERIC_WRITE));

    IWICBitmapEncoder* pEncoder = NULL;
    CHECKHR(pWICFactory->CreateEncoder(GUID_ContainerFormatPng, NULL, &pEncoder));
    CHECKHR(pEncoder->Initialize(pStream, WICBitmapEncoderNoCache));

    IWICBitmapFrameEncode* pFrameEncode = NULL;
    CHECKHR(pEncoder->CreateNewFrame(&pFrameEncode, NULL));

    // Use IWICBitmapFrameEncode to encode the bitmap into the picture format you want.
    CHECKHR(pFrameEncode->Initialize(NULL));

    CHECKHR(pFrameEncode->SetSize(sc_bitmapWidth, sc_bitmapHeight));

    CHECKHR(pFrameEncode->SetPixelFormat(&format));
    CHECKHR(pFrameEncode->WriteSource(pWICBitmap, NULL));
    CHECKHR(pFrameEncode->Commit());
    CHECKHR(pEncoder->Commit());

    return 0;
}

