#include "PCH.h"
#include "Renderer.h"
#include "Sound.h"
#include "UIElements.h"
#include "Timer.h"

struct SaveState
{
	INT64 maintime = 0;
	INT64 alarmtime1 = 0;
	INT64 alarmtime2 = 0;
	INT64 alarmeditbox1 = 0;
	INT64 alarmeditbox2 = 0;
	INT64 addtime1 = 0;
	INT64 addtime2 = 0;
	INT64 addtime3 = 0;
	INT64 addtime4 = 0;
	BOOL ParasitesToggle = FALSE;
	int localtimeoffset = 0;
};

class Application
{
public:
	int Run(HINSTANCE hInstance);
	void Update();
	void SaveFile(SaveState state);
	SaveState Loadfile();
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
private:
	LRESULT CALLBACK AppWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	Renderer m_Renderer;
	UIElements m_UIElements;

	ComPtr<ID2D1SolidColorBrush> BKGBrush;
	ComPtr<ID2D1SolidColorBrush> BorderBrush;
};