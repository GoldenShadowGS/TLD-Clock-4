#include "PCH.h"
#include "Renderer.h"
#include "Sound.h"
#include "UIElements.h"
#include "Timer.h"

struct SaveState
{
	BOOL ParasitesToggle = FALSE;
	INT64 maintime = 0;
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