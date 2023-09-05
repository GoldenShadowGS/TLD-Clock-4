#pragma once
#include "PCH.h"

class SevenSegmentText;

class TimeString
{
public:
	TimeString();
	BOOL Add(char value);
	BOOL Back();
	BOOL Clear();
	void Set(INT64 ms);
	INT64 GetTime();
private:
	inline int GetSize() const { return formatIndex; }
	inline char GetChar(int index) const { return formatArray[index]; }
	int CountLeadingZeroes(char* digits);
	void FormatArray();
	static const int maxRawSize = 10;
	static const int maxFormatSize = maxRawSize + 6;
	int rawIndex = 0;
	int formatIndex = 0;
	char rawArray[maxRawSize] = {};
	char formatArray[maxFormatSize] = {};
	friend class SevenSegment;
};

class TimeStringSmall
{
public:
	TimeStringSmall();
	BOOL Add(char value);
	BOOL Back();
	BOOL Clear();
	void Set(INT64 ms);
	INT64 GetTime();
private:
	inline int GetSize() const { return formatIndex; }
	inline char GetChar(int index) const { return formatArray[index]; }
	int CountLeadingZeroes(char* digits);
	void FormatArray();
	static const int maxRawSize = 6;
	static const int maxFormatSize = maxRawSize + 6;
	int rawIndex = 0;
	int formatIndex = 0;
	char rawArray[maxRawSize] = {};
	char formatArray[maxFormatSize] = {};
	friend class SevenSegment;
};