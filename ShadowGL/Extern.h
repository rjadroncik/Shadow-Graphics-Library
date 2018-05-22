#pragma once

#include "StateVector.h"
#include "Renderer.h"

namespace ShadowGLPrivate
{
    extern SBufferState      Buffer;
    extern STexture          Texture[MAX_TEXTURES];
	extern SRendererState    RS[MAX_THREADS];
    extern SRenderingContext RC;

	extern Boolean RC_OK;
}