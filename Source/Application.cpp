#include "PCH.h"
#include "Application.h"
#include "Resource.h"
#include "UIButton.h"
#include "UIText.h"
#include "UILocaltime.h"
#include "UIAlarmDisplay.h"
#include "UIEditText.h"

const WCHAR* gTitle = L"TLD Clock";
const WCHAR* gWindowClass = L"MainWindowClass";
int WindowWidth = 400;
int WindowHeight = 500;

static ATOM RegisterWindowClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex = {};
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = Application::WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wcex.lpszMenuName = 0;
	wcex.lpszClassName = gWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALLICON));
	return RegisterClassExW(&wcex);
}

int Application::Run(HINSTANCE hInstance)
{
	// Initialize Window
	RegisterWindowClass(hInstance);
	const DWORD style = WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME ^ WS_MAXIMIZEBOX;  // WS_POPUP
	const DWORD exstyle = WS_EX_NOREDIRECTIONBITMAP;
	RECT winRect = { 0, 0, WindowWidth, WindowHeight };
	AdjustWindowRectEx(&winRect, style, false, exstyle);
	HWND hWindow = CreateWindowExW(exstyle, gWindowClass, gTitle, style, CW_USEDEFAULT, 0,
		winRect.right - winRect.left, winRect.bottom - winRect.top, nullptr, nullptr, hInstance, this);

	if (!hWindow)
		return -1;

	SoundManager soundManager(8, 1);

	SaveState savestate = Loadfile();

	Timer timer(hWindow);
	timer.Reset(savestate.maintime);

	// Initialize Direct 2D
	m_Renderer.Init(hWindow);
	ID2D1Factory2* factory = m_Renderer.GetFactory();
	ID2D1DeviceContext* dc = m_Renderer.GetDeviceContext();


	// Initialize UI
	UIButton::InitResources(dc);
	UIEditText::InitResources(dc);

	HR(dc->CreateSolidColorBrush(D2D1::ColorF(0.79f, 0.75f, 0.75f, 1.0f), BKGBrush.ReleaseAndGetAddressOf()));
	HR(dc->CreateSolidColorBrush(D2D1::ColorF(0.1f, 0.1f, 0.1f, 1.0f), BorderBrush.ReleaseAndGetAddressOf()));

	auto MakeRect = [] (int centerx, int centery, int width, int height)
		{
			RECT rect = { centerx - width / 2, centery - height / 2,centerx + width / 2, centery + height / 2 };
			return rect;
		};

	const D2D1::ColorF maincolor1 = D2D1::ColorF(0.48f, 0.33f, 0.54f, 1.0f);
	const D2D1::ColorF maincolor2 = D2D1::ColorF(0.86f, 0.61f, 0.97f, 1.0f);
	const D2D1::ColorF purple1 = D2D1::ColorF(0.1f, 0.1f, 0.1f, 1.0f);
	const D2D1::ColorF purple2 = D2D1::ColorF(0.86f, 0.61f, 0.97f, 1.0f);
	const D2D1::ColorF red1 = D2D1::ColorF(0.48f, 0.13f, 0.14f, 1.0f);
	const D2D1::ColorF red2 = D2D1::ColorF(0.86f, 0.21f, 0.27f, 1.0f);
	const D2D1::ColorF green1 = D2D1::ColorF(0.13f, 0.48f, 0.33f, 1.0f);
	const D2D1::ColorF green2 = D2D1::ColorF(0.22f, 0.86f, 0.50f, 1.0f);
	const D2D1::ColorF editgreen1 = D2D1::ColorF(0.06f, 0.24f, 0.16f, 1.0f);

	std::vector<UIElementBase*> vectorUIElements;

	UIButton PlayButton(CreatePlayImage(factory, dc, 50), CreatePauseImage(factory, dc, 50), TRUE, MakeRect(300, 250, 100, 80), FALSE, [&] (BOOL LMB)
		{
			if (!timer.isStarted())
			{
				timer.Start(500);
				soundManager.Play(SOUND_START, 1.0f, 1.0f);
				return FALSE;
			}
			else
			{
				timer.Stop();
				soundManager.Play(SOUND_STOP, 1.0f, 1.0f);
				return TRUE;
			}
		});
	vectorUIElements.push_back(&PlayButton);

	UIButton ParasitesToggle(LoadBitmapImage(dc, BITMAP_PARASITE), LoadBitmapImage(dc, BITMAP_PARASITE_BW), savestate.ParasitesToggle, MakeRect(300, 450, 64, 64), TRUE, [&] (BOOL LMB)
		{
			if (savestate.ParasitesToggle)
				savestate.ParasitesToggle = FALSE;
			else
				savestate.ParasitesToggle = TRUE;
			return savestate.ParasitesToggle;
		});
	vectorUIElements.push_back(&ParasitesToggle);

	UIText MainClockText(factory, dc, &timer, MakeRect(200, 150, 100, 30), TRUE, -10.0f, maincolor1, maincolor2, [&] ()
		{
			soundManager.Play(SOUND_CLICK, 0.35f, 0.65f);
			if (timer.isStarted())
			{
				PlayButton.Press(0, 0, TRUE);
			}
			return TRUE;
		});
	MainClockText.SetTime(savestate.maintime);
	vectorUIElements.push_back(&MainClockText);


	UIBLocaltime LocalTime(MakeRect(200, 80, 120, 120), factory, dc, &timer, [&] ()
		{
			if (savestate.ParasitesToggle)
				soundManager.Play(SOUND_DRUGS, 1.0f, 1.0f, 0);
		});
	LocalTime.SetOffset(savestate.localtimeoffset);
	vectorUIElements.push_back(&LocalTime);


	UIAlarmDisplay TopAlarmDisplay(MakeRect(80, 50, 100, 50), factory, dc, &timer, red1, red2, [&] ()
		{
			soundManager.Play(SOUND_ALARM1, 1.0f, 1.0f);
		});
	TopAlarmDisplay.SetTime(savestate.alarmtime1);
	vectorUIElements.push_back(&TopAlarmDisplay);

	UIAlarmDisplay BottomAlarmDisplay(MakeRect(80, 110, 100, 50), factory, dc, &timer, green1, green2, [&] ()
		{
			soundManager.Play(SOUND_ALARM2, 1.0f, 1.0f);
		});
	MainClockText.SetAlarmPtr1(&TopAlarmDisplay);
	MainClockText.SetAlarmPtr2(&BottomAlarmDisplay);
	BottomAlarmDisplay.SetTime(savestate.alarmtime2);
	vectorUIElements.push_back(&BottomAlarmDisplay);

	//1
	UIEditText AddText1(factory, dc, MakeRect(160, 310, 100, 25), TRUE, -10.0f, purple1, purple2, [&] ()
		{
			soundManager.Play(SOUND_CLICK, 0.35f, 0.65f);
			return TRUE;
		});
	AddText1.SetTime(savestate.addtime1);
	vectorUIElements.push_back(&AddText1);

	UIButton EditSubmit1(CreateTextImage(dc, L"Add Time", 80.0f, 32.0f, 16.0f, 8.0f), MakeRect(50, 310, 80, 32), FALSE, [&] (BOOL LMB)
		{
			soundManager.Play(SOUND_CLICK, 0.35f, 0.65f);
			if (LMB)
				MainClockText.AddTime(AddText1.GetTime());
			else
				MainClockText.AddTime(-AddText1.GetTime());
			return TRUE;
		});
	vectorUIElements.push_back(&EditSubmit1);

	//2
	UIEditText AddText2(factory, dc, MakeRect(160, 360, 100, 25), TRUE, -10.0f, purple1, purple2, [&] ()
		{
			soundManager.Play(SOUND_CLICK, 0.35f, 0.65f);
			return TRUE;
		});
	AddText2.SetTime(savestate.addtime2);
	vectorUIElements.push_back(&AddText2);

	UIButton EditSubmit2(CreateTextImage(dc, L"Add Time", 80.0f, 32.0f, 16.0f, 8.0f), MakeRect(50, 360, 80, 32), FALSE, [&] (BOOL LMB)
		{
			soundManager.Play(SOUND_CLICK, 0.35f, 0.65f);
			if (LMB)
				MainClockText.AddTime(AddText2.GetTime());
			else
				MainClockText.AddTime(-AddText2.GetTime());
			return TRUE;
		});
	vectorUIElements.push_back(&EditSubmit2);

	// 3
	UIEditText AddText3(factory, dc, MakeRect(160, 410, 100, 25), TRUE, -10.0f, purple1, purple2, [&] ()
		{
			soundManager.Play(SOUND_CLICK, 0.35f, 0.65f);
			return TRUE;
		});
	AddText3.SetTime(savestate.addtime3);
	vectorUIElements.push_back(&AddText3);

	UIButton EditSubmit3(CreateTextImage(dc, L"Add Time", 80.0f, 32.0f, 16.0f, 8.0f), MakeRect(50, 410, 80, 32), FALSE, [&] (BOOL LMB)
		{
			soundManager.Play(SOUND_CLICK, 0.35f, 0.65f);
			if (LMB)
				MainClockText.AddTime(AddText3.GetTime());
			else
				MainClockText.AddTime(-AddText3.GetTime());
			return TRUE;
		});
	vectorUIElements.push_back(&EditSubmit3);

	// 4
	UIEditText AddText4(factory, dc, MakeRect(160, 460, 100, 25), TRUE, -10.0f, purple1, purple2, [&] ()
		{
			soundManager.Play(SOUND_CLICK, 0.35f, 0.65f);
			return TRUE;
		});
	AddText4.SetTime(savestate.addtime4);
	vectorUIElements.push_back(&AddText4);

	UIButton EditSubmit4(CreateTextImage(dc, L"Add Time", 80.0f, 32.0f, 16.0f, 8.0f), MakeRect(50, 460, 80, 32), FALSE, [&] (BOOL LMB)
		{
			soundManager.Play(SOUND_CLICK, 0.35f, 0.65f);
			if (LMB)
				MainClockText.AddTime(AddText4.GetTime());
			else
				MainClockText.AddTime(-AddText4.GetTime());
			return TRUE;
		});
	vectorUIElements.push_back(&EditSubmit4);

	// Top Alarm EditBox
	UIEditText TopAlarmEditBox(factory, dc, MakeRect(160, 220, 100, 25), TRUE, -10.0f, red1, red2, [&] ()
		{
			soundManager.Play(SOUND_CLICK, 0.35f, 0.65f);
			return TRUE;
		});
	TopAlarmEditBox.SetTime(savestate.alarmeditbox1);
	vectorUIElements.push_back(&TopAlarmEditBox);

	UIButton TopAlarmSubmitBox(CreateTextImage(dc, L"Set Alarm 1", 80.0f, 36.0f, 15.0f, 8.0f), MakeRect(50, 220, 84, 32), FALSE, [&] (BOOL LMB)
		{
			if (LMB)
				TopAlarmDisplay.SetTime(TopAlarmEditBox.GetTime());
			else
				TopAlarmDisplay.SetTime(0);
			return TRUE;
		});
	vectorUIElements.push_back(&TopAlarmSubmitBox);

	// Bottom Alarm EditBox
	UIEditText BottomAlarmEditBox(factory, dc, MakeRect(160, 260, 100, 25), TRUE, -10.0f, editgreen1, green2, [&] ()
		{
			soundManager.Play(SOUND_CLICK, 0.35f, 0.65f);
			return TRUE;
		});
	BottomAlarmEditBox.SetTime(savestate.alarmeditbox2);
	vectorUIElements.push_back(&BottomAlarmEditBox);

	UIButton BottomAlarmSubmitBox(CreateTextImage(dc, L"Set Alarm 2", 80.0f, 36.0f, 15.0f, 8.0f), MakeRect(50, 260, 84, 32), FALSE, [&] (BOOL LMB)
		{
			if (LMB)
				BottomAlarmDisplay.SetTime(BottomAlarmEditBox.GetTime());
			else
				BottomAlarmDisplay.SetTime(0);
			return TRUE;
		});
	vectorUIElements.push_back(&BottomAlarmSubmitBox);


	m_UIElements.Set(vectorUIElements);

	// Run Main Loop
	ShowWindow(hWindow, SW_SHOW);
	UpdateWindow(hWindow);

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	savestate.maintime = timer.GetTime();
	savestate.alarmtime1 = TopAlarmDisplay.GetRemainingtime();
	savestate.alarmtime2 = BottomAlarmDisplay.GetRemainingtime();
	savestate.alarmeditbox1 = TopAlarmEditBox.GetTime();
	savestate.alarmeditbox2 = BottomAlarmEditBox.GetTime();
	savestate.addtime1 = AddText1.GetTime();
	savestate.addtime2 = AddText2.GetTime();
	savestate.addtime3 = AddText3.GetTime();
	savestate.addtime4 = AddText4.GetTime();
	savestate.localtimeoffset = LocalTime.GetOffset();



	SaveFile(savestate);
	return (int)msg.wParam;
}

void Application::Update()
{
	ID2D1DeviceContext* dc = m_Renderer.GetDeviceContext();
	dc->BeginDraw();
	dc->Clear();

	D2D_RECT_F UI_Rect = { 0.0f, 190.0f, (float)WindowWidth, (float)WindowHeight };
	D2D1_POINT_2F p1 = { 0.0f, 190.0f };
	D2D1_POINT_2F p2 = { 440.0f, 190.0f };
	dc->SetTransform(D2D1::Matrix3x2F::Identity());
	dc->FillRectangle(UI_Rect, BKGBrush.Get());
	dc->DrawLine(p1, p2, BorderBrush.Get(), 1.0f);

	m_UIElements.Draw(dc);

	HR(dc->EndDraw());
	HR(m_Renderer.GetSwapChain()->Present(1, 0));
}

void Application::SaveFile(SaveState state)
{
	std::ofstream file;
	file.open("TLDClock4.save", std::ios::binary);
	if (file.is_open())
	{
		file.write(reinterpret_cast<char*>(&state), sizeof(SaveState));
		file.close();
	}
}

SaveState Application::Loadfile()
{
	SaveState state = {};
	std::ifstream is;
	is.open("TLDClock4.save", std::ios::binary);
	if (is.is_open())
	{
		is.read(reinterpret_cast<char*>(&state), sizeof(SaveState));
		is.close();
	}
	return state;
}

LRESULT CALLBACK Application::AppWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (m_UIElements.UIProc(hWnd, message, wParam, lParam))
	{
		RECT rc;
		GetClientRect(hWnd, &rc);
		InvalidateRect(hWnd, &rc, TRUE);
	}

	switch (message)
	{
	case WM_PAINT:
	{
		// Skip Drawing if Window is Minimized
		if (!IsIconic(hWnd))
			Update();
		PAINTSTRUCT ps;
		BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

LRESULT CALLBACK Application::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static Application* app = nullptr;
	if (app)
		return app->AppWndProc(hWnd, message, wParam, lParam);
	else
	{
		if (message == WM_CREATE)
		{
			app = reinterpret_cast<Application*>(((CREATESTRUCTW*)lParam)->lpCreateParams);
			return 0;
		}
		else
		{
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
}
