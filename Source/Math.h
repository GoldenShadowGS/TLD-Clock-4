#pragma once
#include "PCH.h"

constexpr float PI = 3.141592f;
constexpr float HalfPI = PI / 2.0f;
constexpr float QuarterPI = PI / 4.0f;
constexpr float PI2 = PI * 2.0f;
constexpr float PI3_4 = PI * 1.5f;
constexpr float ThirdPI = PI2 / 3.0f;
constexpr float Deg2RadFactor = PI / 180.0f;
constexpr float Rad2DegFactor = 180.0f / PI;

#define MINUTESPERDAY 1440

inline D2D1_POINT_2F rotate(D2D1_POINT_2F point, float angle)
{
	float cosa = cos(angle);
	float sina = sin(angle);
	return { point.x * cosa - point.y * sina,point.y * cosa + point.x * sina };
}

inline D2D1_POINT_2F translate(D2D1_POINT_2F point, D2D1_POINT_2F point2)
{
	return { point.x + point2.x, point.y + point2.y };
}

inline D2D1_POINT_2F operator+(const D2D1_POINT_2F& p1, const D2D1_POINT_2F& p2)
{
	return { p1.x + p2.x, p1.y + p2.y };
}

inline D2D1_POINT_2F operator-(const D2D1_POINT_2F& p1, const D2D1_POINT_2F& p2)
{
	return { p1.x - p2.x, p1.y - p2.y };
}

inline float GetLength(const D2D1_POINT_2F& v)
{
	return sqrt(v.x * v.x + v.y * v.y);
}