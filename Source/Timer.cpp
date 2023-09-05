#include "PCH.h"
#include "Timer.h"
#include "Resource.h"

Timer::Timer(HWND hwnd) : AppWindow(hwnd)
{
	m_EventHandle = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (m_EventHandle == INVALID_HANDLE_VALUE)
		throw std::exception("Invalid Event Handle");
}

Timer::~Timer()
{
	Stop();
	if (m_EventHandle)
		CloseHandle(m_EventHandle);
}

void Timer::Start(INT64 SyncInterval)
{
	if (!m_bStarted)
	{
		m_ThreadSafeTimer.Start();
		m_bStarted = TRUE;
		if (!m_bTimerThreadRunning)
		{
			m_bTimerThreadRunning = TRUE;
			m_ClockThread = std::jthread(&Timer::ReDrawSync, this, SyncInterval);
			// Spawns a thread to synchronize drawing to tenth of a second
		}
	}
}

void Timer::Stop()
{
	if (m_bStarted)
	{
		m_bStarted = FALSE;
		SetEvent(m_EventHandle); // Trigger Event to Kill Waiting Thread
		m_ThreadSafeTimer.Stop();
	}
}

void Timer::Reset(INT64 time)
{
	Stop();
	m_ThreadSafeTimer.Reset(time);
}

void Timer::AddTime(INT64 time)
{
	m_ThreadSafeTimer.AddTime(time, isStarted());
}

INT64 Timer::GetTime()
{
	return m_ThreadSafeTimer.GetTime(m_bStarted);
}

void Timer::ReDraw()
{
	PostMessageA(AppWindow, WM_APP + 1, 0, 0);
}

void Timer::ReDrawSync(INT64 SyncInterval)
{
	while (m_bStarted)
	{
		// Syncs redraws to tenths of a second intervals
		INT64 CurrentTime = m_ThreadSafeTimer.GetTime(m_bStarted);
		INT64 difference = CurrentTime - ((CurrentTime / SyncInterval) * SyncInterval);
		INT64 sleeptime = SyncInterval - difference;
		WaitForSingleObject(m_EventHandle, (UINT)sleeptime + 1);
		ReDraw();
	}
	m_bTimerThreadRunning = FALSE;
}
