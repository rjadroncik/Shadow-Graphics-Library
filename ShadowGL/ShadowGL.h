#pragma once

#include "Types.h"

#define	SGL_NONE							0

//Begin() argument definitions
#define	SGL_POINTS							10
#define	SGL_LINES							11
#define	SGL_TRIANGLES						12

//MatrixMode() argument definitions
#define	SGL_MODELVIEW						20
#define	SGL_PROJECTION						21
#define	SGL_TEXTURE							22

//Get() argument definitions
#define	SGL_MAX_PRIMITIVE_VERTICES			1000

#define	SGL_VERSION							1100 
#define	SGL_VERSION_NUMBER					1101 

#define	SGL_MAX_MODELVIEW_STACK_DEPTH		1010
#define	SGL_MAX_PROJECTION_STACK_DEPTH		1011
#define	SGL_MAX_TEXTURE_STACK_DEPTH			1012
#define	SGL_MAX_LIGHTS						1013
#define	SGL_MAX_TEXTURE_SIZE				1014
#define	SGL_MAX_VIEWPORT_DIMS				1015
#define SGL_MAX_ATTRIB_STACK_DEPTH          1016

#define	SGL_COLOR_CLEAR_VALUE				1020
#define	SGL_CURRENT_NORMAL					1021 
#define	SGL_DEPTH_CLEAR_VALUE				1022 

#define	SGL_MODELVIEW_MATRIX				1023
#define	SGL_PROJECTION_MATRIX				1024 
#define	SGL_TEXTURE_MATRIX					1025 

#define	SGL_TEXTURE_ENV_COLOR				1026
#define	SGL_TEXTURE_ENV_MODE				1027

#define	SGL_SHADE_MODEL						1028
#define	SGL_CULL_FACE_MODE					1029
#define	SGL_FRONT_FACE						1030

#define	SGL_VIEWPORT						1032 
#define	SGL_MATRIX_MODE						1033 

#define	SGL_MODELVIEW_STACK_DEPTH			1034 
#define	SGL_PROJECTION_STACK_DEPTH			1035 
#define	SGL_TEXTURE_STACK_DEPTH				1036 

//LightModel() argument definitions
#define	SGL_LIGHT_MODEL_AMBIENT				2100 
#define	SGL_LIGHT_MODEL_LOCAL_VIEWER		2101 
#define	SGL_LIGHT_MODEL_TWO_SIDE			2102 

//Light Identifier Definitions
#define	SGL_LIGHT_BASE						100 
#define	SGL_LIGHT0							100 
#define	SGL_LIGHT1							101 
#define	SGL_LIGHT2							102 
#define	SGL_LIGHT3							103 
#define	SGL_LIGHT4							104 
#define	SGL_LIGHT5							105 
#define	SGL_LIGHT6							106 
#define	SGL_LIGHT7							107 

//Light()/Material() argument definitions
#define	SGL_AMBIENT							2200 
#define	SGL_DIFFUSE 						2201 
#define	SGL_SPECULAR 						2202 

//Light() argument definitions
#define	SGL_POSITION 						2203 
#define	SGL_SPOT_DIRECTION					2204 
#define	SGL_SPOT_EXPONENT					2205 
#define	SGL_SPOT_CUTOFF						2206 
#define	SGL_CONSTANT_ATTENUATION			2207 
#define	SGL_LINEAR_ATTENUATION				2208 
#define	SGL_QUADRATIC_ATTENUATION			2209 

//Material() argument definitions
#define	SGL_EMISSION						2220 
#define	SGL_SHININESS 						2221 
#define	SGL_AMBIENT_AND_DIFFUSE				2222 

//Enable()/Disable() argument definitions
#define	SGL_LIGHTING						3000 
#define	SGL_NORMALIZE						3001 
#define	SGL_CULL_FACE						3002 
#define	SGL_FOG								3003
#define	SGL_DEPTH_TEST						3004
#define	SGL_TEXTURE_1D						3005 
#define	SGL_TEXTURE_2D						3006 
  
//CullFace() argument definitions
#define	SGL_FRONT							2300
#define	SGL_BACK							2301
#define	SGL_FRONT_AND_BACK					2302

//FrontFace() argument definitions
#define	SGL_CW								2303
#define	SGL_CCW								2304

//Fog() argument definitions
#define	SGL_FOG_MODE						2310
#define	SGL_FOG_DENSITY						2311
#define	SGL_FOG_START						2312
#define	SGL_FOG_END							2313
#define	SGL_FOG_COLOR						2314

#define	SGL_LINEAR 							2320
#define	SGL_EXP								2321
#define	SGL_EXP2							2322

//TexEnv() argument definitions 
#define	SGL_MODULATE						2400
#define	SGL_DECAL							2401
#define	SGL_BLEND							2402

//Error code definitions
#define	SGL_INVALID_OPERATION				8000
#define	SGL_INVALID_ENUM					8001
#define	SGL_INVALID_VALUE					8002										

#define	SGL_STACK_OVERFLOW					8010
#define	SGL_STACK_UNDERFLOW					8011

//SetDisplayMode() argument definitions
#define	SGL_FULLSCREEN						0x0001 
#define	SGL_WINDOW							0x0002 
#define	SGL_FULL_SIZE						0x0010 
#define	SGL_HAS_ZBUFFER						0x0100 
#define	SGL_RESTORE							0x1000 

//Clear() argument definitions
//#define	SGL_COLOR_BUFFER_BIT				0x0001
//#define	SGL_DEPTH_BUFFER_BIT				0x0010
#define	SGL_ACCUM_BUFFER_BIT				0x0200
#define	SGL_STENCIL_BUFFER_BIT				0x1000

//PushAttrib() argument definitions
#define SGL_CURRENT_BIT                    0x00000001
#define SGL_POLYGON_BIT                    0x00000008
#define SGL_PIXEL_MODE_BIT                 0x00000020
#define SGL_LIGHTING_BIT                   0x00000040
#define SGL_FOG_BIT                        0x00000080
#define SGL_DEPTH_BUFFER_BIT               0x00000100
#define SGL_VIEWPORT_BIT                   0x00000800
#define SGL_TRANSFORM_BIT                  0x00001000
#define SGL_ENABLE_BIT                     0x00002000
#define SGL_COLOR_BUFFER_BIT               0x00004000
#define SGL_HINT_BIT                       0x00008000
#define SGL_TEXTURE_BIT                    0x00040000
#define SGL_SCISSOR_BIT                    0x00080000
#define SGL_ALL_ATTRIB_BITS                0x000FFFFF

//PixelTransfer() argument definitions
#define	SGL_RED_SCALE						2410
#define	SGL_GREEN_SCALE						2411
#define	SGL_BLUE_SCALE						2412
#define	SGL_ALPHA_SCALE						2413
#define	SGL_DEPTH_SCALE						2414

#define	SGL_RED_BIAS						2415
#define	SGL_GREEN_BIAS						2416
#define	SGL_BLUE_BIAS						2417
#define	SGL_ALPHA_BIAS						2418
#define	SGL_DEPTH_BIAS						2419

//TexImage*D() argument definitions
#define	SGL_RGB								2420
#define	SGL_RGBA							2421
#define	SGL_BGR_EXT							2422
#define	SGL_BGRA_EXT						2423

#define	SGL_UNSIGNED_BYTE					2424
#define	SGL_BYTE							2425
#define	SGL_BITMAP							2426
#define	SGL_UNSIGNED_SHORT					2427
#define	SGL_SHORT							2428
#define	SGL_UNSIGNED_INT					2429
#define	SGL_INT								2430
#define	SGL_FLOAT							2431

//TexParameteri() argument definitions
#define SGL_NEAREST                         0x2600
//#define SGL_LINEAR                          0x2601
#define SGL_NEAREST_MIPMAP_NEAREST          0x2700
#define SGL_LINEAR_MIPMAP_NEAREST           0x2701
#define SGL_NEAREST_MIPMAP_LINEAR           0x2702
#define SGL_LINEAR_MIPMAP_LINEAR            0x2703

#define SGL_TEXTURE_MAG_FILTER              0x2800
#define SGL_TEXTURE_MIN_FILTER              0x2801
#define SGL_TEXTURE_WRAP_S                  0x2802
#define SGL_TEXTURE_WRAP_T                  0x2803

#define SGL_CLAMP                           0x2900
#define SGL_REPEAT                          0x2901

namespace ShadowGL
{
	//Base ShadowGL Information
	const UByte VERSION[]      = "0.9.0.0";
	const Int   VERSION_NUMBER = 900;

	SHADOWGL_API void ClearColor(ClampF red, ClampF green, ClampF blue, ClampF alpha);
	SHADOWGL_API void ClearDepth(ClampD depth);

	SHADOWGL_API void Clear(Bitfield mask);

	SHADOWGL_API void SwapBuffers(HDC hDC);

	SHADOWGL_API HRC CreateContext(HDC hDC);
	SHADOWGL_API bool DeleteContext(HRC hRc);

	SHADOWGL_API bool MakeCurrent(HDC hDC, HRC hRc);

	SHADOWGL_API void Viewport(Int posX, Int posY, SizeI width, SizeI height);

	SHADOWGL_API void Perspective(Double fov, Double aspect, Double clipNear, Double clipFar);
	SHADOWGL_API void Frustum(Double left, Double right, Double bottom, Double top, Double clipNear, Double clipFar);

	SHADOWGL_API void MatrixMode(Enum mode);

	SHADOWGL_API void PushMatrix();
	SHADOWGL_API void PopMatrix();

	SHADOWGL_API void LoadIdentity();

	SHADOWGL_API void Rotatef(Float angle, Float x, Float y, Float z);
	SHADOWGL_API void Translatef(Float x, Float y, Float z);
	SHADOWGL_API void Scalef(Float x, Float y, Float z);

    SHADOWGL_API	void Enable(Enum cap);
    SHADOWGL_API	void Disable(Enum cap);

    SHADOWGL_API	Boolean IsEnabled(Enum cap);

    SHADOWGL_API	void Lightf(Enum light, Enum pname, Float param);
    SHADOWGL_API	void Lightfv(Enum light, Enum pname, const Float *params);

    SHADOWGL_API	void LightModeli(Enum pname, Int param);
    SHADOWGL_API	void LightModelfv(Enum pname, const Float *params);

    SHADOWGL_API	void GetBooleanv(Enum pname, Boolean * params);
    SHADOWGL_API	void GetDoublev(Enum pname, Double * params);
    SHADOWGL_API	void GetFloatv(Enum pname, Float * params);
    SHADOWGL_API	void GetIntegerv(Enum pname, Int * params);

    SHADOWGL_API	void GetLightfv(Enum light, Enum pname, Float * params);

    SHADOWGL_API	const UByte* GetString(Enum name);

    SHADOWGL_API	void Materialfv(Enum face, Enum pname, const Float *params);

    SHADOWGL_API	void FrontFace(Enum dir);
    SHADOWGL_API	void CullFace(Enum mode);

    SHADOWGL_API	void Fogf(Enum pname, Float param);
    SHADOWGL_API	void Fogi(Enum pname, Int param);
    SHADOWGL_API	void Fogfv(Enum pname, const Float *params);

    SHADOWGL_API	void TexImage1D(Enum target, Int level, Int components, SizeI width, Int border, Enum format, Enum type, const void *pixels);
    SHADOWGL_API	void TexImage2D(Enum target, Int level, Int components, SizeI width, SizeI height, Int border, Enum format, Enum type, const void *pixels);

    SHADOWGL_API	void TexEnvf(Enum target, Enum pname, Float param);
    SHADOWGL_API	void TexEnvi(Enum target, Enum pname, Int param);

    SHADOWGL_API	void TexParameteri(Enum target, Enum pname, Int param);

    SHADOWGL_API    void GetTexParameteriv(Enum target, Enum pname, Int *params);

    SHADOWGL_API	void GenTextures(SizeI n, Int * textures);
    SHADOWGL_API	void DeleteTextures(SizeI n, const Int * textures);

    SHADOWGL_API	void BindTexture(Enum target, Int texture);

    SHADOWGL_API	void PixelTransferf(Enum pname, Float param);

    SHADOWGL_API    void PushAttrib(Bitfield mask);
    SHADOWGL_API    void PopAttrib();

    SHADOWGL_API void Begin(Enum primType);
    SHADOWGL_API void End();

    SHADOWGL_API void Vertex3f(Float x, Float y, Float z);

    SHADOWGL_API void Color3f(Float r, Float g, Float b);
    SHADOWGL_API void Color4f(Float r, Float g, Float b, Float a);
    SHADOWGL_API void Color4fv(const Float *v);

    SHADOWGL_API void Normal3f(Float x, Float y, Float z);
    SHADOWGL_API void TexCoord2f(Float x, Float y);
}
