//
#pragma once

#include "Recluse/Types.hpp"

#include <math.h>

#define R_CONST_PI                3.141592653589793238462643383279502884197169399375
#define R_CONST_PI_HALF           1.57079632679489661923   // pi/2
#define R_CONST_PI_QUARTER        0.785398163397448309616 // pi/4
#define R_CONST_2_PI              6.283185307 // 2 * pi
#define R_CONST_TOLERANCE         0.0001     // 
#define R_EPSILON                 0.0000001 // 
#define R_E                       2.71828182845904523536   // e

#define R_CLAMP(v, min, max)    ((v) > (max) ? (max) : ((v) < (min) ? (min) : (v)))  
#define R_MAX(a, b)             ((a) < (b) ? (b) : (a))
#define R_MIN(a, b)             ((a) > (b) ? (b) : (a)) 
#define R_LERP(a, b, t)         ((a) * (1.0f - (t)) + (b) * (t))
#define R_SMOOTHSTEP(e0, e, t)  R_CLAMP(((t) - (e0)) / ((e1) - (e0))), 0.0f, 1.0f)
#define R_RADIANS(deg)          ((deg) * (static_cast<R32>(CONST_PI) / 180.0f))
#define R_DEGREES(rad)          ((rad) * (180.0f / static_cast<R32>(CONST_PI)))
#define R_ABS(a)                ((a) >= 0.0f ? (a) : -(a))

#define R_FLOOR(a)              floorf((a))
#define R_CEIL(a)               ceilf((a))
#define R_LOG2(a)               log2f((a))

// Check if the value (a) is odd.
#define R_IS_ODD(a)				((a) & 1)

// Check if the value (a) is even.
#define R_IS_EVEN(a)			(!R_IS_ODD(a))

// Check if the value (a) is a power of 2.
#define R_IS_POWER_OF_2(a)		(((a) & ((a) - 1)) == 0)


namespace Recluse {
namespace Math {
template<typename T>
static Bool isOdd(T d)
{
	return R_IS_ODD(d);
}


template<typename T>
static Bool isEven(T d)
{
	return R_IS_EVEN(d);
}


template<typename T>
static Bool isPowerOf2(T d)
{
	return R_IS_POWER_OF_2(d);
}


// Get the minimum of the two values.
template<typename T>
static T minimum(T a, T b)
{
	return R_MIN(a, b);
}


// Get the maximum of the two values.
template<typename T>
static T maximum(T a, T b)
{
	return R_MAX(a, b);
}


// Degrees to radians.
template<typename T>
static T deg2Rad(T deg)
{
	return R_RADIANS(deg);
}


// Radians to degrees.
template<typename T>
static T rad2Deg(T rads)
{
	return R_DEGREES(rads);
}


template<typename Class, typename Type>
static Class clamp(const Class& c, Type mmin, Type mmax)
{
	return R_CLAMP(c, mmin, mmax);
}


template<typename Class, typename Type>
static Class lerp(const Class& a, const Class& b, Type t)
{
	return R_LERP(a, b, t);
}


template<typename Class>
static Class ceil(Class v)
{
	return (Class)::ceil((F64)v);
}


template<typename Class>
static Class floor(Class v)
{
	return (Class)::floor((F64)v);
}


template<typename Class>
static Class divUp(Class a, Class b)
{
	return (a + (b - static_cast<Class>(1))) / b;
}
} // Math
} // Recluse