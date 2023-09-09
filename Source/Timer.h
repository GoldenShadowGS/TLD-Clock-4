#pragma once
#include "PCH.h"

class Timer
{
	class ThreadSafeTimer
	{
	public:
		void Reset(INT64 time)
		{
			const std::lock_guard<std::mutex> lock(m_Mutex);
			m_Duration = time;
		}
		void Start()
		{
			const std::lock_guard<std::mutex> lock(m_Mutex);
			start = std::chrono::steady_clock::now();
		}
		void Stop()
		{
			const std::lock_guard<std::mutex> lock(m_Mutex);
			m_Duration += GetActiveDuration();
		}
		void AddTime(INT64 time, BOOL isStarted)
		{
			const std::lock_guard<std::mutex> lock(m_Mutex);
			m_Duration += time;
			INT64 currentTime = isStarted ? m_Duration + GetActiveDuration() : m_Duration;
			if (currentTime < 0)
				m_Duration -= currentTime;
			else if (currentTime > MAXTIME)
				m_Duration = MAXTIME;
		}
		INT64 GetTime(BOOL isStarted)
		{
			const std::lock_guard<std::mutex> lock(m_Mutex);
			if (isStarted)
				return m_Duration + GetActiveDuration();
			else
				return m_Duration;
		}
	private:
		INT64 GetActiveDuration()
		{
			std::chrono::time_point<std::chrono::steady_clock> now = std::chrono::steady_clock::now();
			return std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
		}
		std::mutex m_Mutex;
		std::chrono::time_point<std::chrono::steady_clock> start;
		INT64 m_Duration = 0;
	};
public:
	Timer(HWND hwnd);
	~Timer();
	void Start(INT64 SyncInterval);
	BOOL isStarted() { return m_bStarted; }
	void Stop();
	void Reset(INT64 time);
	void AddTime(INT64 time);
	INT64 GetTime();
private:
	void ReDraw();
	void ReDrawSync(INT64 SyncInterval); // Thread Function
	ThreadSafeTimer m_ThreadSafeTimer;
	HWND AppWindow = nullptr;
	INT64 m_AlarmTime = 0;
	HANDLE m_EventHandle = {};
	std::jthread m_ClockThread;
	std::atomic<BOOL> m_bStarted = FALSE;
	std::atomic<BOOL> m_bTimerThreadRunning = FALSE;
	static const INT64 MAXTIME = 719999995000;
};
