#include "PCH.h"
#include "TimeString.h"

TimeString::TimeString()
{
	FormatArray();
}

BOOL TimeString::Add(char value)
{
	if (rawIndex < maxRawSize && !(value == '0' && rawIndex == 0))
	{
		if (rawIndex == 0)
		{
			rawArray[rawIndex] = value;
			rawArray[rawIndex + 1] = '0';
			rawIndex++;
			rawIndex++;
		}
		else
		{
			rawArray[rawIndex] = rawArray[rawIndex - 1];
			rawArray[rawIndex - 1] = value;
			rawIndex++;
		}
		FormatArray();
		return TRUE;
	}
	return FALSE;
}

BOOL TimeString::Back()
{
	if (rawIndex > 1)
	{
		rawIndex--;
		rawArray[rawIndex - 1] = rawArray[rawIndex];
		rawArray[rawIndex] = 0;
		FormatArray();
		return TRUE;
	}
	else if (rawIndex == 1)
	{
		rawIndex--;
		rawArray[rawIndex] = 0;
		FormatArray();
		return TRUE;
	}
	return FALSE;
}

BOOL TimeString::Clear()
{
	if (rawIndex != 0)
	{
		for (int i = 0; i < maxRawSize; i++)
		{
			rawArray[i] = 0;
		}
		rawIndex = 0;
		FormatArray();
	}
	return TRUE;
}

void TimeString::Set(INT64 ms)
{
	INT64 tenths = ms / 500;  // 100
	INT64 Minutes = (ms / 5000) % 60;  // 1000
	INT64 Hours = (ms / 300000) % 24;  // 60000
	INT64 Days = ms / 7200000;  // 1440000

	char digits[maxRawSize] =
	{
		((Days / 10000) % 10) + 48,
		((Days / 1000) % 10) + 48,
		((Days / 100) % 10) + 48,
		((Days / 10) % 10) + 48,
		(Days % 10) + 48,
		((Hours / 10) % 10) + 48,
		(Hours % 10) + 48,
		((Minutes / 10) % 10) + 48,
		(Minutes % 10) + 48,
		(tenths % 10) + 48,
	};

	int count = CountLeadingZeroes(digits);
	int j = count;
	for (int i = 0; i < maxRawSize - count; i++)
	{
		rawArray[i] = digits[j++];
	}

	rawIndex = maxRawSize - count;
	FormatArray();
}

INT64 TimeString::GetTime()
{
	auto getDigitTime = [] (int index)
		{
			switch (index)
			{
			case 0:
				return 72000000000LL; // Days Thousands  14400000000LL
			case 1:
				return 7200000000LL; // Days Thousands  1440000000LL
			case 2:
				return 720000000LL; // Days Hundreds  144000000LL
			case 3:
				return 72000000LL; // Days Tens  14400000LL
			case 4:
				return 7200000LL; // Days Ones  1440000LL
			case 5:
				return 3000000LL; // Hours Tens  600000LL
			case 6:
				return 300000LL; // Hours Ones  60000LL
			case 7:
				return 50000LL; // Minutes Tens  10000LL
			case 8:
				return 5000LL; // Minutes Ones  1000LL
			case 9:
				return 500LL; // Tenths  100LL
			default:
				return 0LL;
			}
		};

	INT64 result = 0;
	int startindex = maxRawSize - rawIndex;

	for (int i = 0; i < rawIndex; i++)
	{
		result += (rawArray[i] - 48LL) * getDigitTime(startindex + i);
	}
	return result;
}

int TimeString::CountLeadingZeroes(char* digits)
{
	int count = 0;
	for (int i = 0; i < maxRawSize; i++)
	{
		if (digits[i] == '0')
			count++;
		else
			break;
	}
	return count;
}

void TimeString::FormatArray()
{
	int sourceindex = 0;
	formatIndex = 0;
	if (rawIndex == 0)
	{
		formatArray[0] = '0';
		formatArray[1] = '.';
		formatArray[2] = '0';
		formatIndex = 3;
	}
	while (sourceindex < rawIndex)
	{
		if (rawIndex - sourceindex == 1)
		{
			if (sourceindex == 0)
			{
				formatArray[formatIndex] = '0';
				formatIndex++;
			}
			formatArray[formatIndex] = '.';
			formatArray[formatIndex + 1] = rawArray[sourceindex];
			formatIndex += 2;
			sourceindex++;
		}
		else if ((rawIndex - sourceindex == 3) && sourceindex != 0) // insert colons between hours:minuutes and minutes:seconds
		{
			formatArray[formatIndex] = ':';
			formatArray[formatIndex + 1] = rawArray[sourceindex];
			sourceindex++;
			formatIndex += 2;
		}
		else if ((rawIndex - sourceindex == 5) && sourceindex != 0) // insert space after days
		{
			formatArray[formatIndex] = 'd';
			formatArray[formatIndex + 1] = ' ';
			formatArray[formatIndex + 2] = rawArray[sourceindex];
			sourceindex++;
			formatIndex += 3;
		}
		else
		{
			formatArray[formatIndex] = rawArray[sourceindex];
			sourceindex++;
			formatIndex++;
		}
	}
	formatArray[formatIndex] = 0;
	assert(formatIndex <= maxFormatSize);
}


// --------------

TimeStringSmall::TimeStringSmall()
{
	FormatArray();
}

BOOL TimeStringSmall::Add(char value)
{
	if (rawIndex < maxRawSize && !(value == '0' && rawIndex == 0))
	{
		if (rawIndex == 0)
		{
			rawArray[rawIndex] = value;
			rawArray[rawIndex + 1] = '0';
			rawIndex++;
			rawIndex++;
		}
		else
		{
			rawArray[rawIndex] = rawArray[rawIndex - 1];
			rawArray[rawIndex - 1] = value;
			rawIndex++;
		}
		FormatArray();
		return TRUE;
	}
	return FALSE;
}

BOOL TimeStringSmall::Back()
{
	if (rawIndex > 1)
	{
		rawIndex--;
		rawArray[rawIndex - 1] = rawArray[rawIndex];
		rawArray[rawIndex] = 0;
		FormatArray();
		return TRUE;
	}
	else if (rawIndex == 1)
	{
		rawIndex--;
		rawArray[rawIndex] = 0;
		FormatArray();
		return TRUE;
	}
	return FALSE;
}

BOOL TimeStringSmall::Clear()
{
	if (rawIndex != 0)
	{
		for (int i = 0; i < maxRawSize; i++)
		{
			rawArray[i] = 0;
		}
		rawIndex = 0;
		FormatArray();
		return TRUE;
	}
	return FALSE;
}

void TimeStringSmall::Set(INT64 ms)
{
	INT64 tenths = ms / 500;  // 100
	INT64 Minutes = (ms / 5000) % 60;  // 1000
	INT64 Hours = (ms / 300000) % 24;  // 60000
	INT64 Days = ms / 7200000;  // 1440000

	char digits[maxRawSize] =
	{
		(Days % 10) + 48,
		((Hours / 10) % 10) + 48,
		(Hours % 10) + 48,
		((Minutes / 10) % 10) + 48,
		(Minutes % 10) + 48,
		(tenths % 10) + 48
	};

	int count = CountLeadingZeroes(digits);
	int j = count;
	for (int i = 0; i < maxRawSize - count; i++)
	{
		rawArray[i] = digits[j++];
	}

	rawIndex = maxRawSize - count;
	FormatArray();
}

INT64 TimeStringSmall::GetTime()
{
	auto getDigitTime = [] (int index)
		{
			switch (index)
			{
			case 0:
				return 7200000LL; // Days Ones  1440000LL
			case 1:
				return 3000000LL; // Hours Tens  600000LL
			case 2:
				return 300000LL; // Hours Ones  60000LL
			case 3:
				return 50000LL; // Minutes Tens  10000LL
			case 4:
				return 5000LL; // Minutes Ones  1000LL
			case 5:
				return 500LL; // Tenths  100LL
			default:
				return 0LL;
			}
		};

	INT64 result = 0;
	int startindex = maxRawSize - rawIndex;

	for (int i = 0; i < rawIndex; i++)
	{
		result += (rawArray[i] - 48LL) * getDigitTime(startindex + i);
	}
	return result;
}

int TimeStringSmall::CountLeadingZeroes(char* digits)
{
	int count = 0;
	for (int i = 0; i < maxRawSize; i++)
	{
		if (digits[i] == '0')
			count++;
		else
			break;
	}
	return count;
}

void TimeStringSmall::FormatArray()
{
	int sourceindex = 0;
	formatIndex = 0;
	if (rawIndex == 0)
	{
		formatArray[0] = '0';
		formatArray[1] = '.';
		formatArray[2] = '0';
		formatIndex = 3;
	}
	while (sourceindex < rawIndex)
	{
		if (rawIndex - sourceindex == 1)
		{
			if (sourceindex == 0)
			{
				formatArray[formatIndex] = '0';
				formatIndex++;
			}
			formatArray[formatIndex] = '.';
			formatArray[formatIndex + 1] = rawArray[sourceindex];
			formatIndex += 2;
			sourceindex++;
		}
		else if ((rawIndex - sourceindex == 3) && sourceindex != 0) // insert colons between hours:minuutes and minutes:seconds
		{
			formatArray[formatIndex] = ':';
			formatArray[formatIndex + 1] = rawArray[sourceindex];
			sourceindex++;
			formatIndex += 2;
		}
		else if ((rawIndex - sourceindex == 5) && sourceindex != 0) // insert space after days
		{
			formatArray[formatIndex] = 'd';
			formatArray[formatIndex + 1] = ' ';
			formatArray[formatIndex + 2] = rawArray[sourceindex];
			sourceindex++;
			formatIndex += 3;
		}
		else
		{
			formatArray[formatIndex] = rawArray[sourceindex];
			sourceindex++;
			formatIndex++;
		}
	}
	formatArray[formatIndex] = 0;
	assert(formatIndex <= maxFormatSize);
}
