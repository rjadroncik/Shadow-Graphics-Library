#include "StateVector.h"
#include "Extern.h"
#include "ShadowGL.h"

//#include <SCFObjectExtensions.h>

using namespace SCFMathematics;
using namespace ShadowGLPrivate;

namespace ShadowGLPrivate
{
	SRenderingContext StateStack[8];
	UINT StateSaved[8];
	UINT StateStackDepth = 0;
}

SHADOWGL_API void ShadowGL::ShadeModel(Enum mode)
{
	if (!RC_OK) { MessageBox(NULL, TEXT("No Current Rendering Context!"), TEXT("ShadeModel()"), MB_OK | MB_ICONERROR); return; } 
	if (RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

	if (mode == SGL_SMOOTH) { RC.Enable.SmoothShading = TRUE;  return; }
	if (mode == SGL_FLAT)   { RC.Enable.SmoothShading = FALSE; return; }

	RC.ErrorCode = SGL_INVALID_ENUM;
}

SHADOWGL_API void ShadowGL::Enable(Enum cap)
{
	if (!RC_OK) { MessageBox(NULL, TEXT("No Current Rendering Context!"), TEXT("Enable()"), MB_OK | MB_ICONERROR); return; } 
	if (RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

	if ((cap >= SGL_LIGHT_BASE) && (cap < (SGL_LIGHT_BASE + MAX_LIGHTS)))
	{
		RC.Light[cap - SGL_LIGHT_BASE].Enabled = TRUE; return;
	}

	switch (cap)
	{
	case SGL_LIGHTING:  { RC.Enable.Lighting    = TRUE; return; }
	case SGL_CULL_FACE: { RC.Enable.FaceCulling = TRUE; return; }
	case SGL_FOG:       { RC.Enable.Fog         = TRUE; return; }

	case SGL_TEXTURE_1D: { RC.Enable.Texturing1D = TRUE; return; }
	case SGL_TEXTURE_2D: { RC.Enable.Texturing2D = TRUE; return; }
	}

	RC.ErrorCode = SGL_INVALID_ENUM;
}

SHADOWGL_API void ShadowGL::Disable(Enum cap)
{
	if (!RC_OK) { MessageBox(NULL, TEXT("No Current Rendering Context!"), TEXT("Disable()"), MB_OK | MB_ICONERROR); return; } 
	if (RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

	if ((cap >= SGL_LIGHT_BASE) && (cap < (SGL_LIGHT_BASE + MAX_LIGHTS)))
	{
		RC.Light[cap - SGL_LIGHT_BASE].Enabled = FALSE; return;
	}

	switch (cap)
	{
	case SGL_LIGHTING:  { RC.Enable.Lighting    = FALSE; return; }
	case SGL_CULL_FACE: { RC.Enable.FaceCulling = FALSE; return; }
	case SGL_FOG:       { RC.Enable.Fog         = FALSE; return; }

	case SGL_TEXTURE_1D: { RC.Enable.Texturing1D = FALSE; return; }
	case SGL_TEXTURE_2D: { RC.Enable.Texturing2D = FALSE; return; }
	}

	RC.ErrorCode = SGL_INVALID_ENUM;
}

SHADOWGL_API Boolean ShadowGL::IsEnabled(Enum cap)
{
	if (!RC_OK) { MessageBox(NULL, TEXT("No Current Rendering Context!"), TEXT("Disable()"), MB_OK | MB_ICONERROR); return FALSE; } 
	if (RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return FALSE; }

	if ((cap >= SGL_LIGHT_BASE) && (cap < (SGL_LIGHT_BASE + MAX_LIGHTS)))
	{
		return RC.Light[cap - SGL_LIGHT_BASE].Enabled;
	}

	if (cap == SGL_LIGHTING) { return RC.Enable.Lighting; }

	if (cap == SGL_FOG) { return RC.Enable.Fog; }

	if (cap == SGL_TEXTURE_1D) { return RC.Enable.Texturing1D; }
	if (cap == SGL_TEXTURE_2D) { return RC.Enable.Texturing2D; }

	if (cap == SGL_NORMALIZE) { return RC.Enable.Normalize; } 

	RC.ErrorCode = SGL_INVALID_ENUM;

	return FALSE;
}

SHADOWGL_API void ShadowGL::Lightf(Enum light, Enum pname, Float param)
{
	if (!RC_OK) { MessageBox(NULL, TEXT("No Current Rendering Context!"), TEXT("Lightf()"), MB_OK | MB_ICONERROR); return; } 
	if (RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

	if ((light < SGL_LIGHT_BASE) || (light >= (SGL_LIGHT_BASE + MAX_LIGHTS))) { RC.ErrorCode = SGL_INVALID_ENUM; return; }

	light -= SGL_LIGHT_BASE;

 	if (pname == SGL_SPOT_EXPONENT)
	{
		if ((param < 0) || (param > 128)) { RC.ErrorCode = SGL_INVALID_VALUE; return; }
		RC.Light[light].Exponent = (UByte)param; return; 
	}
 
 	if (pname == SGL_SPOT_CUTOFF)
	{
		if ((param != 180) && ((param < 0) || (param > 90))) { RC.ErrorCode = SGL_INVALID_VALUE; return; }
		RC.Light[light].CutOff = param; return; 
	}

 	if (pname == SGL_CONSTANT_ATTENUATION)
	{
		if (param < 0) { RC.ErrorCode = SGL_INVALID_VALUE; return; }
		RC.Light[light].Constant = param; return; 
	}
	
 	if (pname == SGL_LINEAR_ATTENUATION)
	{
		if (param < 0) { RC.ErrorCode = SGL_INVALID_VALUE; return; }
		RC.Light[light].Linear = param; return; 
	}

 	if (pname == SGL_QUADRATIC_ATTENUATION)
	{
		if (param < 0) { RC.ErrorCode = SGL_INVALID_VALUE; return; }
		RC.Light[light].Quadratic = param; return; 
	}

	RC.ErrorCode = SGL_INVALID_ENUM;
}

SHADOWGL_API void ShadowGL::Lightfv(Enum light, Enum pname, const Float *params)
{
	if (!RC_OK) { MessageBox(NULL, TEXT("No Current Rendering Context!"), TEXT("Lightfv()"), MB_OK | MB_ICONERROR); return; } 
	if (RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

	if ((light < SGL_LIGHT_BASE) || (light >= (SGL_LIGHT_BASE + MAX_LIGHTS))) { RC.ErrorCode = SGL_INVALID_ENUM; return; }

	light -= SGL_LIGHT_BASE;

	if (pname == SGL_AMBIENT)			{ SetVector4(RC.Light[light].Ambient, params[0], params[1], params[2], params[3]); return; }
	if (pname == SGL_DIFFUSE)			{ SetVector4(RC.Light[light].Diffuse, params[0], params[1], params[2], params[3]); return; }
	if (pname == SGL_SPECULAR)			{ SetVector4(RC.Light[light].Specular, params[0], params[1], params[2], params[3]); return; }

	if (pname == SGL_POSITION)
	{ 
        Float4 position;
		SetVector4(position, params[0], params[1], params[2], params[3]);
		MultiplyMatrix4Vector4(RC.Light[light].EyePosition, RC.Matrix.ModelView[RC.Matrix.MVCurrent], position);
		return; 
	}

	if (pname == SGL_SPOT_DIRECTION)
	{ 
        Float3 direction;
		SetVector3(direction, params[0], params[1], params[2]);
		MultiplyMatrix4Vector3(RC.Light[light].EyeDirection, RC.Matrix.ModelView[RC.Matrix.MVCurrent], direction);

		NormalizeVector3(RC.Light[light].EyeDirection, RC.Light[light].EyeDirection);
		return; 
	}
 
	if (pname == SGL_SPOT_EXPONENT)
	{
		if ((params[0] < 0) || (params[0] > 128)) { RC.ErrorCode = SGL_INVALID_VALUE; return; }
		RC.Light[light].Exponent = (UByte)params[0]; return; 
	}
 
 	if (pname == SGL_SPOT_CUTOFF)
	{
		if ((params[0] != 180) && ((params[0] < 0) || (params[0] > 90))) { RC.ErrorCode = SGL_INVALID_VALUE; return; }
		RC.Light[light].CutOff = params[0]; return; 
	}

 	if (pname == SGL_CONSTANT_ATTENUATION)
	{
		if (params[0] < 0) { RC.ErrorCode = SGL_INVALID_VALUE; return; }
		RC.Light[light].Constant = params[0]; return; 
	}
	
 	if (pname == SGL_LINEAR_ATTENUATION)
	{
		if (params[0] < 0) { RC.ErrorCode = SGL_INVALID_VALUE; return; }
		RC.Light[light].Linear = params[0]; return; 
	}

 	if (pname == SGL_QUADRATIC_ATTENUATION)
	{
		if (params[0] < 0) { RC.ErrorCode = SGL_INVALID_VALUE; return; }
		RC.Light[light].Quadratic = params[0]; return; 
	}

	RC.ErrorCode = SGL_INVALID_ENUM;
}

SHADOWGL_API void ShadowGL::LightModeli(Enum pname, Int param)
{
	if (!RC_OK) { MessageBox(NULL, TEXT("No Current Rendering Context!"), TEXT("LightModeli()"), MB_OK | MB_ICONERROR); return; } 
	if (RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

	if (pname == SGL_LIGHT_MODEL_LOCAL_VIEWER)	{ RC.Enable.LocalViewer = (Boolean)param; return; }

	RC.ErrorCode = SGL_INVALID_ENUM;
}

SHADOWGL_API void ShadowGL::LightModelfv(Enum pname, const Float *params)
{
	if (!RC_OK) { MessageBox(NULL, TEXT("No Current Rendering Context!"), TEXT("LightModelfv()"), MB_OK | MB_ICONERROR); return; } 
	if (RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

	if (pname == SGL_LIGHT_MODEL_LOCAL_VIEWER) { RC.Enable.LocalViewer = (Boolean)params[0]; return; }

	if (pname == SGL_LIGHT_MODEL_AMBIENT) { SetVector4(RC.Ambient, params[0], params[1], params[2], params[3]); return; }

	RC.ErrorCode = SGL_INVALID_ENUM;
}

SHADOWGL_API void ShadowGL::GetBooleanv(Enum pname, Boolean *params)
{
	if (RC_OK && RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

	if (pname == SGL_CULL_FACE) { *params = RC.Enable.FaceCulling; return; }

	if (pname == SGL_DEPTH_TEST) { return; } /////////////!!!!!!!!!!!!!!!!!!!!

	if (pname == SGL_FOG) { *params = RC.Enable.Fog; return; } 

	if ((pname >= SGL_LIGHT_BASE) && (pname < (SGL_LIGHT_BASE + MAX_LIGHTS))) { *params =  RC.Light[pname - SGL_LIGHT_BASE].Enabled; return; }

	if (pname == SGL_LIGHTING)   { *params = RC.Enable.Lighting;    return; } 
	if (pname == SGL_NORMALIZE)  { *params = RC.Enable.Normalize;   return; } 
	if (pname == SGL_TEXTURE_1D) { *params = RC.Enable.Texturing1D; return; } 
	if (pname == SGL_TEXTURE_2D) { *params = RC.Enable.Texturing2D; return; } 

	RC.ErrorCode = SGL_INVALID_ENUM;
}

SHADOWGL_API void ShadowGL::GetDoublev(Enum pname, Double *params)
{
	if (RC_OK && RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

	if (pname == SGL_MODELVIEW_MATRIX)  { CopyMatrix4(*(Matrix4*)params, RC.Matrix.ModelView[RC.Matrix.MVCurrent]);	return; } 
	if (pname == SGL_PROJECTION_MATRIX) { CopyMatrix4(*(Matrix4*)params, RC.Matrix.Projection[RC.Matrix.PCurrent]);	return; }
	if (pname == SGL_TEXTURE_MATRIX)    { CopyMatrix4(*(Matrix4*)params, RC.Matrix.Texture[RC.Matrix.TCurrent]);	return; } 

	RC.ErrorCode = SGL_INVALID_ENUM;
}

SHADOWGL_API void ShadowGL::GetFloatv(Enum pname, Float *params)
{
	if (RC_OK && RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

	if (pname == SGL_COLOR_CLEAR_VALUE) { CopyVector4(*(Float4*)params, RC.Buffer.ClearColor);	return; } 
	if (pname == SGL_CURRENT_NORMAL)    { CopyVector3(*(Float3*)params, RC.Vertex.Normal);		return; } 
	if (pname == SGL_DEPTH_CLEAR_VALUE)	{ *params = RC.Buffer.ClearDepth; return; } 

	if (pname == SGL_FOG_DENSITY) { *params = RC.Fog.Density; return; } 
	if (pname == SGL_FOG_COLOR)   { CopyVector4(*(Float4*)params, RC.Fog.Color); return; } 
	if (pname == SGL_FOG_START)   { *params = RC.Fog.Start; return; } 
	if (pname == SGL_FOG_END)     { *params = RC.Fog.End; return; } 

	if (pname == SGL_LIGHT_MODEL_AMBIENT) { CopyVector4(*(Float4*)params, RC.Ambient);     return; } 
	if (pname == SGL_TEXTURE_ENV_COLOR)   { CopyVector4(*(Float4*)params, RC.TexEnvColor); return; } 

	RC.ErrorCode = SGL_INVALID_ENUM;
}

SHADOWGL_API void ShadowGL::GetIntegerv(Enum pname, Int *params)
{
	if (RC_OK && RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

	if (pname == SGL_VERSION_NUMBER) { params[0] = VERSION_NUMBER; return; }

	if (pname == SGL_CULL_FACE_MODE) { *params = RC.CullStyle;   return; } 
	if (pname == SGL_FOG_MODE)       { *params = RC.Fog.Mode;    return; } 
	if (pname == SGL_FRONT_FACE)     { *params = RC.FrontFace;   return; } 
	if (pname == SGL_MATRIX_MODE)    { *params = RC.Matrix.Mode; return; } 
	if (pname == SGL_MAX_LIGHTS)     { *params = MAX_LIGHTS;     return; } 

	if (pname == SGL_MAX_MODELVIEW_STACK_DEPTH)		{ *params = MAX_MODELVIEW_MATRICES;  return; } 
	if (pname == SGL_MAX_PROJECTION_STACK_DEPTH)	{ *params = MAX_PROJECTION_MATRICES; return; }
	if (pname == SGL_MAX_TEXTURE_STACK_DEPTH)		{ *params = MAX_TEXTURE_MATRICES;    return; } 
	if (pname == SGL_MAX_TEXTURE_SIZE)				{ *params = MAX_TEXTURE_SIZE;        return; }  
	if (pname == SGL_MAX_VIEWPORT_DIMS)				{ params[0] = MAX_BUFFER_WIDTH;	params[1] = MAX_BUFFER_HEIGHT; return; }  

	if (pname == SGL_MAX_ATTRIB_STACK_DEPTH)		{ *params = 8; return; }  	

	if (pname == SGL_MODELVIEW_STACK_DEPTH)			{ *params = RC.Matrix.MVCurrent + 1; return; }
	if (pname == SGL_PROJECTION_STACK_DEPTH)		{ *params = RC.Matrix.PCurrent + 1;  return; } 
	if (pname == SGL_TEXTURE_STACK_DEPTH)			{ *params = RC.Matrix.TCurrent + 1;  return; } 

	if (pname == SGL_SHADE_MODEL)					{ if (RC.Enable.SmoothShading) { *params = SGL_SMOOTH; } else { *params = SGL_FLAT; } return; } 
	if (pname == SGL_TEXTURE_ENV_MODE)				{ *params = RC.TexEnvMode; return; }

	if (pname == SGL_VIEWPORT)
	{ 
		params[0] = (Int)RC.View.Position[0]; 
		params[1] = (Int)RC.View.Position[1]; 

		params[2] = RC.View.Size[0]; 
		params[3] = RC.View.Size[1]; 

		return; 
	}

	RC.ErrorCode = SGL_INVALID_ENUM;
}

SHADOWGL_API const UByte* ShadowGL::GetString(Enum name)
{
	if (RC_OK && RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return NULL; }

	if (name == SGL_VERSION)
	{
		return &VERSION[0];
	}

	RC.ErrorCode = SGL_INVALID_ENUM;
	return NULL;
}

/*
SHADOWGL_API void ShadowGL::GetLightfv(Enum light, Enum pname, Float *params)
{

}*/

SHADOWGL_API void ShadowGL::Materialfv(Enum face, Enum pname, const Float *params)
{
	UNREFERENCED_PARAMETER(face);

	if (!RC_OK) { MessageBox(NULL, TEXT("No Current Rendering Context!"), TEXT("Materialfv()"), MB_OK | MB_ICONERROR); return; } 
	if (RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

	if (pname == SGL_AMBIENT)
	{
		SetVector4(RC.Material.Ambient, params[0], params[1], params[2], params[3]); return;
	}
	if (pname == SGL_DIFFUSE)
	{
		SetVector4(RC.Material.Diffuse, params[0], params[1], params[2], params[3]); return;
	}
	if (pname == SGL_SPECULAR)
	{
		SetVector4(RC.Material.Specular, params[0], params[1], params[2], params[3]); return;
	}
	if (pname == SGL_EMISSION)
	{
		SetVector4(RC.Material.Emission, params[0], params[1], params[2], params[3]); return;
	}
	if (pname == SGL_SHININESS)
	{
		RC.Material.Shininess = params[0]; return;
	}

	RC.ErrorCode = SGL_INVALID_ENUM;
}

SHADOWGL_API void ShadowGL::FrontFace(Enum dir)
{
	if (!RC_OK) { MessageBox(NULL, TEXT("No Current Rendering Context!"), TEXT("FrontFace()"), MB_OK | MB_ICONERROR); return; } 
	if (RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

	if (dir == SGL_CCW) { RC.FrontFace = dir; return; }
	if (dir == SGL_CW)  { RC.FrontFace = dir; return; }

	RC.ErrorCode = SGL_INVALID_ENUM;
}

SHADOWGL_API void ShadowGL::CullFace(Enum mode)
{
	if (!RC_OK) { MessageBox(NULL, TEXT("No Current Rendering Context!"), TEXT("CullFace()"), MB_OK | MB_ICONERROR); return; } 
	if (RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

	if (mode == SGL_FRONT) { RC.CullStyle = mode; return; }
	if (mode == SGL_BACK)  { RC.CullStyle = mode; return; }

	RC.ErrorCode = SGL_INVALID_ENUM;
}

SHADOWGL_API void ShadowGL::Fogf(Enum pname, Float param)
{
	if (!RC_OK) { MessageBox(NULL, TEXT("No Current Rendering Context!"), TEXT("Fogf()"), MB_OK | MB_ICONERROR); return; } 
	if (RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

	if (pname == SGL_FOG_DENSITY) { RC.Fog.Density = param; return; }
	if (pname == SGL_FOG_START)   { RC.Fog.Start   = param; return; }
	if (pname == SGL_FOG_END)     { RC.Fog.End     = param; return; }

	if (pname == SGL_FOG_MODE)
	{ 
		if (param == SGL_LINEAR) { RC.Fog.Mode = (Enum)param; return; }
		if (param == SGL_EXP)    { RC.Fog.Mode = (Enum)param; return; }
		if (param == SGL_EXP2)   { RC.Fog.Mode = (Enum)param; return; }
		return; 
	}

	RC.ErrorCode = SGL_INVALID_ENUM;
}

SHADOWGL_API void ShadowGL::Fogi(Enum pname, Int param)
{
	if (!RC_OK) { MessageBox(NULL, TEXT("No Current Rendering Context!"), TEXT("Fogi()"), MB_OK | MB_ICONERROR); return; } 
	if (RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

	if (pname == SGL_FOG_DENSITY) { RC.Fog.Density = (Float)param; return; }
	if (pname == SGL_FOG_START)   { RC.Fog.Start   = (Float)param; return; }
	if (pname == SGL_FOG_END)     { RC.Fog.End     = (Float)param; return; }

	if (pname == SGL_FOG_MODE)
	{ 
		if (param == SGL_LINEAR) { RC.Fog.Mode = (Enum)param; return; }
		if (param == SGL_EXP)    { RC.Fog.Mode = (Enum)param; return; }
		if (param == SGL_EXP2)   { RC.Fog.Mode = (Enum)param; return; }
		return; 
	}

	RC.ErrorCode = SGL_INVALID_ENUM;
}

SHADOWGL_API void ShadowGL::Fogfv(Enum pname, const Float *params)
{
	if (!RC_OK) { MessageBox(NULL, TEXT("No Current Rendering Context!"), TEXT("Fogfv()"), MB_OK | MB_ICONERROR); return; } 
	if (RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

	if (pname == SGL_FOG_DENSITY) { RC.Fog.Density = *params; return; }
	if (pname == SGL_FOG_START)   { RC.Fog.Start   = *params; return; }
	if (pname == SGL_FOG_END)     { RC.Fog.End     = *params; return; }

	if (pname == SGL_FOG_MODE)
	{ 
		if (*params == SGL_LINEAR) { RC.Fog.Mode = (Enum)*params; return; }
		if (*params == SGL_EXP)    { RC.Fog.Mode = (Enum)*params; return; }
		if (*params == SGL_EXP2)   { RC.Fog.Mode = (Enum)*params; return; }
		return; 
	}

	if (pname == SGL_FOG_COLOR) { SetVector4(RC.Fog.Color, params[0], params[1], params[2], params[3]); return; }

	RC.ErrorCode = SGL_INVALID_ENUM;
}
/*
SHADOWGL_API void ShadowGL::TexImage1D(Enum target, Int level, Int components, SizeI width, Int border, Enum format, Enum type, const void *pixels)
{

}*/

SHADOWGL_API void ShadowGL::TexImage2D(Enum target, Int level, Int components, SizeI width, SizeI height, Int border, Enum format, Enum type, const void *pixels)
{
	if (!RC_OK) { MessageBox(NULL, TEXT("No Current Rendering Context!"), TEXT("TexImage2D()"), MB_OK | MB_ICONERROR); return; } 
	if (RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

	//Check input values
	if (target != SGL_TEXTURE_2D)                      { RC.ErrorCode = SGL_INVALID_ENUM; return; } 
	if ((format < SGL_RGB) || (format > SGL_BGRA_EXT)) { RC.ErrorCode = SGL_INVALID_ENUM; return; } 
	if ((components < 3)   || (components > 4))        { RC.ErrorCode = SGL_INVALID_ENUM; return; } 
	if ((border < 0)       || (border > 1))            { RC.ErrorCode = SGL_INVALID_ENUM; return; } 

	if ((level < 0) || ((Int)ceilf(logf(MAX_TEXTURE_SIZE) / logf(2)) < level)) { RC.ErrorCode = SGL_INVALID_ENUM; return; } 

	//Check texture dimensions	
	if ((width  < 0) || (width  > MAX_TEXTURE_SIZE)) { RC.ErrorCode = SGL_INVALID_ENUM; return; } 
	if ((height < 0) || (height > MAX_TEXTURE_SIZE)) { RC.ErrorCode = SGL_INVALID_ENUM; return; } 

	//Check whether dimensions are power of two
	bool bHeightOk = FALSE;
	bool bWidthOk  = FALSE;

	for (UINT i = 1; i < MAX_TEXTURE_SIZE; i *= 2)
	{
		if ((UINT)width  == i) { bWidthOk  = TRUE; }
		if ((UINT)height == i) { bHeightOk = TRUE; }
	}

	if (!bHeightOk || !bWidthOk) { RC.ErrorCode = SGL_INVALID_ENUM; return; } 

	//Copy information
	Texture[RC.TexCurrent2D].Width  = (UShort)width;
	Texture[RC.TexCurrent2D].Height = (UShort)height;

	Texture[RC.TexCurrent2D].Target     = target;
	Texture[RC.TexCurrent2D].Components = (UByte)components;

	Texture[RC.TexCurrent2D].Format = format;
	Texture[RC.TexCurrent2D].Used   = TRUE;

	//Allocate memory for pixels
	UInt TextureSize = height * width * components;
	Texture[RC.TexCurrent2D].pData = (UByte*)CMemory::Allocate(TextureSize);

	UINT RedOffset   = 0;
	UINT GreenOffset = 1;
	UINT BlueOffset  = 2;

	if (format == SGL_BGR_EXT)
	{
		RedOffset  = 2;
		BlueOffset = 0;
	}

	if (type == SGL_UNSIGNED_BYTE)
	{
		float RedScale   = RC.PixelTransfer.Scale_Red   / 255;
		float GreenScale = RC.PixelTransfer.Scale_Green / 255;
		float BlueScale  = RC.PixelTransfer.Scale_Blue  / 255;

		for (UInt i = 0; i < TextureSize; i += 3)
		{
			Texture[RC.TexCurrent2D].pData[i + RedOffset]   = (UByte)((RedScale   * ((UByte*)pixels)[i    ] + RC.PixelTransfer.Bias_Red)   * 255);
			Texture[RC.TexCurrent2D].pData[i + GreenOffset] = (UByte)((GreenScale * ((UByte*)pixels)[i + 1] + RC.PixelTransfer.Bias_Green) * 255);
			Texture[RC.TexCurrent2D].pData[i + BlueOffset]  = (UByte)((BlueScale  * ((UByte*)pixels)[i + 2] + RC.PixelTransfer.Bias_Blue)  * 255);
		}
		return;
	}
}

/*
SHADOWGL_API void ShadowGL::TexEnvf(Enum target, Enum pname, Float param)
{

}

SHADOWGL_API void ShadowGL::TexEnvi(Enum target, Enum pname, Float param)
{

}

SHADOWGL_API void ShadowGL::TexParameteri(Enum target, Enum pname, Int param)
{

}
*/

SHADOWGL_API void ShadowGL::GenTextures(SizeI n, Int *textures)
{
	if (!RC_OK) { MessageBox(NULL, TEXT("No Current Rendering Context!"), TEXT("GenTextures()"), MB_OK | MB_ICONERROR); return; } 
	if (RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

	if ((n <= 0) || (n > 254)) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

	UInt CurrentTexture = 0;

	for (UByte i = 1;  i < MAX_TEXTURES;  i++)
	{
		if (!RC.TexNameUsed[i])
		{
			RC.TexNameUsed[i] = TRUE;

			textures[CurrentTexture] =  i;
			
			CurrentTexture++;
		}

		if (CurrentTexture == (UInt)n) { return; }
	}
}

SHADOWGL_API void ShadowGL::DeleteTextures(SizeI n, const Int *textures)
{
	if (!RC_OK) { MessageBox(NULL, TEXT("No Current Rendering Context!"), TEXT("DeleteTextures()"), MB_OK | MB_ICONERROR); return; } 
	if (RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }
	if ((n <= 0) || (n > 254)) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

	for (Int i = 0;  i < n;  i++)
	{
		if ((textures[i] <= 0) || (textures[i] > 255)) { continue;	}

		if (RC.TexNameUsed[textures[i]])
		{
			RC.TexNameUsed[i] = FALSE;

			if (Texture[textures[i]].Used)
			{
				CMemory::Free(Texture[textures[i]].pData);

				//Clear active texture bindings
				if ((Texture[textures[i]].Target == SGL_TEXTURE_1D) && (textures[i] == RC.TexCurrent1D)) { RC.TexCurrent1D = 0; }
				if ((Texture[textures[i]].Target == SGL_TEXTURE_2D) && (textures[i] == RC.TexCurrent2D)) { RC.TexCurrent2D = 0; }

				Texture[textures[i]].Used		= FALSE;
				Texture[textures[i]].pData		= NULL;

				Texture[textures[i]].Components	= 0;
				Texture[textures[i]].Target		= 0;
				Texture[textures[i]].Format		= 0;

				Texture[textures[i]].Width		= 0;
				Texture[textures[i]].Height		= 0;

				Texture[textures[i]].pData		= NULL;
			}
		}
	}
}

SHADOWGL_API void ShadowGL::BindTexture(Enum target, Int texture)
{
	if (!RC_OK) { MessageBox(NULL, TEXT("No Current Rendering Context!"), TEXT("BindTexture()"), MB_OK | MB_ICONERROR); return; } 
	if (RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

	if ((texture <= 0) || (texture > 255)) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

	if ((target != SGL_TEXTURE_1D) && (target != SGL_TEXTURE_2D)) { RC.ErrorCode = SGL_INVALID_ENUM; return; } 

	//Check for target or assign it if texture is being created
	if (Texture[texture].Used) { if (target != Texture[texture].Target) { RC.ErrorCode = SGL_INVALID_OPERATION; return; } }
	else                       { Texture[texture].Target = target; }

	if (target == SGL_TEXTURE_1D) { RC.TexCurrent1D = (UByte)texture; return; }
	if (target == SGL_TEXTURE_2D) { RC.TexCurrent2D = (UByte)texture; return; }

	RC.ErrorCode = SGL_INVALID_ENUM;
}

SHADOWGL_API void ShadowGL::PixelTransferf(Enum pname, Float param)
{
	if (!RC_OK) { MessageBox(NULL, TEXT("No Current Rendering Context!"), TEXT("PixelTransferf()"), MB_OK | MB_ICONERROR); return; } 
	if (RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

	if (pname == SGL_DEPTH_SCALE) { RC.PixelTransfer.Scale_Depth = param; return; }
	if (pname == SGL_RED_SCALE)   { RC.PixelTransfer.Scale_Red   = param; return; }
	if (pname == SGL_GREEN_SCALE) { RC.PixelTransfer.Scale_Green = param; return; }
	if (pname == SGL_BLUE_SCALE)  { RC.PixelTransfer.Scale_Blue  = param; return; }
	if (pname == SGL_ALPHA_SCALE) { RC.PixelTransfer.Scale_Alpha = param; return; }

	if (pname == SGL_DEPTH_BIAS) { RC.PixelTransfer.Bias_Depth = param; return; }
	if (pname == SGL_RED_BIAS)   { RC.PixelTransfer.Bias_Red   = param; return; }
	if (pname == SGL_GREEN_BIAS) { RC.PixelTransfer.Bias_Green = param; return; }
	if (pname == SGL_BLUE_BIAS)  { RC.PixelTransfer.Bias_Blue  = param; return; }
	if (pname == SGL_ALPHA_BIAS) { RC.PixelTransfer.Bias_Alpha = param; return; }

	RC.ErrorCode = SGL_INVALID_ENUM;
}

SHADOWGL_API void ShadowGL::PushAttrib(Bitfield mask)
{
	StateSaved[StateStackDepth] = 0;
	StateStack[StateStackDepth] = RC;

	if (mask & SGL_ENABLE_BIT)
	{	
		StateSaved[StateStackDepth] |= SGL_ENABLE_BIT;
	}
	
	StateStackDepth++;
}

SHADOWGL_API void ShadowGL::PopAttrib()
{
	StateStackDepth--;

	if (StateSaved[StateStackDepth] & SGL_ENABLE_BIT)
	{
		RC.Enable = StateStack[StateStackDepth].Enable;
	}
}