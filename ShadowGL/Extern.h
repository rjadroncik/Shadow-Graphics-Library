#pragma once

#include "StateVector.h"
#include "Renderer.h"

namespace ShadowGLPrivate
{
	extern SBufferState      Buffer;
	extern STexture          Texture[MAX_TEXTURES];
	extern SRendererState    RS;
	extern SRenderingContext RC;

	extern Boolean RC_OK;

	//Current Temporary State
	extern RECT Rect01;
	extern RECT Rect02;

	extern POINT Point01;
	extern POINT Point02;

	extern Char4 Char401;
	extern Char4 Char402;

	extern Float2 Float201;
	extern Float2 Float202;

	extern Float3 Float301;
	extern Float3 Float302;
	extern Float3 Float303;
	extern Float3 Float304;
	extern Float3 Float305;
	extern Float3 Float306;
	extern Float3 Float307;

	extern Float4 Float401;
	extern Float4 Float402;

	extern Matrix3 Matrix301;
	extern Matrix3 Matrix302;
	extern Matrix3 Matrix303;
	extern Matrix3 Matrix304;

	extern Matrix4 Matrix401;
	extern Matrix4 Matrix402;
	extern Matrix4 Matrix403;
	extern Matrix4 Matrix404;
}