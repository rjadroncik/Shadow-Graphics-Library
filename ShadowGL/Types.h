#pragma once

#include <SCFGraphics.h>
//#include <SCFStandardUndef.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#ifdef SHADOWGL_EXPORTS
#define SHADOWGL_API __declspec(dllexport)
#else
#define SHADOWGL_API __declspec(dllimport)
#endif

typedef unsigned int	Bitfield;
typedef unsigned int	Enum;
typedef int				SizeI;

typedef unsigned int	Boolean;
typedef unsigned int	Bitfield;

typedef signed char		Byte;
typedef unsigned char	UByte;

typedef signed short	Short;
typedef unsigned short	UShort;

typedef signed int		Int;
typedef unsigned int	UInt;

typedef float			Float;
typedef float			ClampF;

typedef double			Double;
typedef double			ClampD;

typedef	void *			HRC;
