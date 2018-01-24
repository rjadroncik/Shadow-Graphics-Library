#include "ShadowGL.h"

#include "Extern.h"

#include <SCFObjectExtensions.h>
#include <intrin.h>

//#define WIN32_LEAN_AND_MEAN
//#include <windows.h>

//Main Objects
namespace ShadowGLPrivate
{
	SRenderingContext RC;
	Boolean           RC_OK = false;

	STexture     Texture[MAX_TEXTURES];
	SBufferState Buffer;
}

using namespace SCFMathematics;
using namespace ShadowGLPrivate;

BOOL APIENTRY DllMain(HANDLE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(hModule);
	UNREFERENCED_PARAMETER(lpReserved);

    switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH: 
		{
			//Reset textures
			for (UByte i = 0; i < MAX_TEXTURES; i++)
			{
				Texture[i].Used = false;
				Texture[i].pData = nullptr;

				Texture[i].Components = 0;
				Texture[i].Target = 0;
				Texture[i].Format = 0;

				Texture[i].Width = 0;
				Texture[i].Height = 0;

/*				Texture[i].MinFilter = SGL_LINEAR;
				Texture[i].MagFilter = SGL_LINEAR;

				Texture[i].S_Wrap = REPEAT;
				Texture[i].T_Wrap = REPEAT;
*/
//				TexNameUsed[i] = false;

				RS.Texture.Components = 0;
			}

			break; 
		}
	case DLL_THREAD_ATTACH: { break; }
	case DLL_THREAD_DETACH:	{ break; }
	case DLL_PROCESS_DETACH: { break; }
    }

    return true;
}

SHADOWGL_API bool ShadowGL::MakeCurrent(HDC hDC, HRC hRc)
{
	UNREFERENCED_PARAMETER(hDC);

	if (!hRc) { RC_OK = false; return true; }

	RC = *((SRenderingContext*)hRc);
	RC_OK = true;

	return true;
}

SHADOWGL_API HRC ShadowGL::CreateContext(HDC hDC)
{
	UNREFERENCED_PARAMETER(hDC);

	if (RC_OK && RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return nullptr; }

	SRenderingContext *pNewRC = (SRenderingContext*)CMemory::Allocate(sizeof(SRenderingContext), true);
	
	pNewRC->Buffer.hDrawDibDC = DrawDibOpen();

	if (!pNewRC->Buffer.hDrawDibDC) 
	{
		MessageBox(nullptr, TEXT("Ned· sa vytvoriù Rendering Context"), TEXT("Chyba"), MB_OK | MB_ICONEXCLAMATION);
		return false;
	}

	//Initialize matrix state
	for (BYTE i = 0; i < MAX_MODELVIEW_MATRICES;  i++) { MakeIndentityMatrix4(pNewRC->Matrix.ModelView[i]); MakeIndentityMatrix4(pNewRC->Matrix.ModelViewInverse[i]); }
	for (BYTE i = 0; i < MAX_PROJECTION_MATRICES; i++) { MakeIndentityMatrix4(pNewRC->Matrix.Projection[i]); }
	for (BYTE i = 0; i < MAX_TEXTURE_MATRICES;    i++) { MakeIndentityMatrix4(pNewRC->Matrix.Texture[i]); }

	pNewRC->Matrix.MVCurrent = 0;
	pNewRC->Matrix.PCurrent  = 0;
	pNewRC->Matrix.TCurrent  = 0;

	pNewRC->Matrix.Mode    = SGL_MODELVIEW;
	pNewRC->Primitive.Type = SGL_NONE;

	//Initialize material state
	SetVector4(pNewRC->Material.Ambient,  0.2f, 0.2f, 0.2f, 1.0f);
	SetVector4(pNewRC->Material.Diffuse,  0.8f, 0.8f, 0.8f, 1.0f);
	SetVector4(pNewRC->Material.Specular, 0.0f, 0.0f, 0.0f, 1.0f);
	SetVector4(pNewRC->Material.Emission, 0.0f, 0.0f, 0.0f, 1.0f);
	pNewRC->Material.Shininess = 0;

	//Initialize fog state
	pNewRC->Enable.Fog	= false;
	pNewRC->Fog.Mode	= SGL_EXP;

	pNewRC->Fog.Density	= 1;
	pNewRC->Fog.Start	= 0;
	pNewRC->Fog.End		= 1;

	SetVector4(pNewRC->Fog.Color, 0.0f, 0.0f, 0.0f, 0.0f);
	
	//Rendering state
	SetVector4(pNewRC->Ambient, 0.2f, 0.2f, 0.2f, 1);

	pNewRC->Enable.FaceCulling = true;
	pNewRC->CullStyle   = SGL_BACK;
	pNewRC->FrontFace   = SGL_CCW;

	pNewRC->TexEnvMode = SGL_MODULATE;

	//Initialize primitive state
	for (BYTE i = 0; i < MAX_PRIMITIVE_VERTICES; i++)
	{	
		ZeroVector4(pNewRC->Primitive.Vertex[i].ObjCoord); 
		ZeroVector4(pNewRC->Primitive.Vertex[i].EyeCoord); 
		ZeroVector4(pNewRC->Primitive.Vertex[i].ClipCoord); 
		ZeroVector4(pNewRC->Primitive.Vertex[i].NormCoord); 
		ZeroVector4(pNewRC->Primitive.Vertex[i].ViewCoord); 

		ZeroVector4(pNewRC->Primitive.Vertex[i].ObjColor); 
		ZeroVector4(pNewRC->Primitive.Vertex[i].LitColor); 

		ZeroVector3(pNewRC->Primitive.Vertex[i].ObjNormal); 
		ZeroVector3(pNewRC->Primitive.Vertex[i].EyeNormal); 

		ZeroVector4(pNewRC->Primitive.Vertex[i].ObjTexCoord); 
		ZeroVector4(pNewRC->Primitive.Vertex[i].FinTexCoord); 
	}

	pNewRC->Primitive.Type			    = 0;
	pNewRC->Primitive.VerticesSubmitted	= 0;
	pNewRC->Primitive.Building		    = false;

	//Initialize view state
	for (BYTE i = 0; i < 4; i++) { ZeroVector4(pNewRC->View.ClipPlane[i]); }
	ZeroVector4(pNewRC->View.Position);
	ZeroVector3(pNewRC->View.Rotation);

	pNewRC->View.ClipFar	= 1;
	pNewRC->View.ClipNear	= 1;
	pNewRC->View.FOV		= 45;

	pNewRC->View.Size[1]	= 1;
	pNewRC->View.Size[0]	= 1;
	pNewRC->View.Aspect		= 1;

	pNewRC->View.Origin[0]	= 0;
	pNewRC->View.Origin[1]	= 0;

	//Clear Values
	pNewRC->Buffer.ClearColor[0] = 0;
	pNewRC->Buffer.ClearColor[1] = 0;
	pNewRC->Buffer.ClearColor[2] = 0;
	pNewRC->Buffer.ClearColor[3] = 0;

	pNewRC->Buffer.ClearDepth = 1.0;

	//Initialize lights
	for (BYTE i = 0; i < MAX_LIGHTS; i++)
	{
		SetVector4(pNewRC->Light[i].Ambient, 0, 0, 0, 1);
		SetVector4(pNewRC->Light[i].Diffuse, 0, 0, 0, 1);
		SetVector4(pNewRC->Light[i].Specular, 0, 0, 0, 1);

		pNewRC->Light[i].Enabled = false;

		//Positional Parameters
		SetVector4(pNewRC->Light[i].EyePosition, 0, 0, 1, 1);

		//Spot Parameters
		SetVector3(pNewRC->Light[i].EyeDirection, 0, 0, -1);
		pNewRC->Light[i].Exponent	= 0;
		pNewRC->Light[i].CutOff		= 180;

		//Attenuation Factors
		pNewRC->Light[i].Constant	= 1;
		pNewRC->Light[i].Linear		= 0;
		pNewRC->Light[i].Quadratic	= 0;
	}

	SetVector4(pNewRC->Light[0].Diffuse,  1, 1, 1, 1);
	SetVector4(pNewRC->Light[0].Specular, 1, 1, 1, 1);

	//Rendering state
	pNewRC->Enable.Lighting		    = false;
	pNewRC->Enable.LocalViewer		= false;
	SetVector4(pNewRC->Ambient, 0.2f, 0.2f, 0.2f, 1);
	pNewRC->Enable.Normalize		= false;

	pNewRC->Enable.FaceCulling	= true;
	pNewRC->CullStyle	= SGL_BACK;
	pNewRC->FrontFace	= SGL_CCW;
//	pNewRC->PolygonMode	= FILL;

	pNewRC->Enable.Texturing2D	= false;
	ZeroVector4(pNewRC->Ambient);
	pNewRC->TexEnvMode	= SGL_MODULATE;

/*	pNewRC->DepthTest = true;

	pNewRC->AlphaTest = false;
	pNewRC->AlphaRef = 0.0f;

	for (UByte01 = 0; UByte01 < MAX_CLIP_PLANES; UByte01++)
	{
		ZeroVector4(&pNewRC->ClipPlane[UByte01][0]);
		pNewRC->ClipPlaneEnabled[UByte01] = false;
	}
*/
	pNewRC->ErrorCode		= 0;
	pNewRC->TexCurrent2D	= 0;

	//Initialize pixel transfer state
	pNewRC->PixelTransfer.Bias_Alpha	= 0;
	pNewRC->PixelTransfer.Bias_Blue		= 0;
	pNewRC->PixelTransfer.Bias_Depth	= 0;
	pNewRC->PixelTransfer.Bias_Green	= 0;
	pNewRC->PixelTransfer.Bias_Red		= 0;

	pNewRC->PixelTransfer.Scale_Alpha	= 1;
	pNewRC->PixelTransfer.Scale_Blue	= 1;
	pNewRC->PixelTransfer.Scale_Depth	= 1;
	pNewRC->PixelTransfer.Scale_Green	= 1;
	pNewRC->PixelTransfer.Scale_Red		= 1;

	return (void*)pNewRC;
}

SHADOWGL_API bool ShadowGL::DeleteContext(HRC hRc)
{
	if (!hRc) { if (RC_OK) { RC.ErrorCode = SGL_INVALID_OPERATION; } return false; }

	SRenderingContext *pOldRC = (SRenderingContext*)hRc;

	DrawDibClose(pOldRC->Buffer.hDrawDibDC);
	CMemory::Free(pOldRC);

	return true;
}

SHADOWGL_API void ShadowGL::SwapBuffers(HDC hDC)
{
	if (!RC_OK) { MessageBox(nullptr, TEXT("No Current Rendering Context!"), TEXT("SwapBuffers()"), MB_OK | MB_ICONERROR); return; } 
	if (RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

	RC.Buffer.BitmapInfoHeader.biSize = sizeof(BITMAPINFOHEADER);

	RC.Buffer.BitmapInfoHeader.biWidth			= RC.View.Size[0];
	RC.Buffer.BitmapInfoHeader.biHeight			= RC.View.Size[1];
	RC.Buffer.BitmapInfoHeader.biPlanes			= 1;

	RC.Buffer.BitmapInfoHeader.biBitCount		= 32;
	RC.Buffer.BitmapInfoHeader.biCompression	= BI_RGB;
	RC.Buffer.BitmapInfoHeader.biSizeImage		= 0;
	RC.Buffer.BitmapInfoHeader.biXPelsPerMeter	= 0;
	RC.Buffer.BitmapInfoHeader.biYPelsPerMeter  = 0;

	RC.Buffer.BitmapInfoHeader.biClrUsed		= 0;
	RC.Buffer.BitmapInfoHeader.biClrImportant	= 0;

	DrawDibDraw(RC.Buffer.hDrawDibDC, hDC, 0, 0, RC.View.Size[0], RC.View.Size[1], &RC.Buffer.BitmapInfoHeader, Buffer.Back, 0, 0, -1, -1, 0);
}

SHADOWGL_API void ShadowGL::Viewport(Int posX, Int posY, SizeI width, SizeI height)
{
	if (!RC_OK) { MessageBox(nullptr, TEXT("No Current Rendering Context!"), TEXT("Viewport()"), MB_OK | MB_ICONERROR); return; } 
	if (RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

	if (posX < 0) { posX = 0; }
	if (posY < 0) { posY = 0; }

	if (width  < 1) { width  = 1; }
	if (height < 1) { height = 1; }

	if (width  > MAX_BUFFER_WIDTH)  { width  = MAX_BUFFER_WIDTH; }
	if (height > MAX_BUFFER_HEIGHT) { height = MAX_BUFFER_HEIGHT; }

	RC.View.Size[0] = width;
	RC.View.Size[1] = height;

	RC.View.Origin[0] = RC.View.Size[0] / 2 + posX;
	RC.View.Origin[1] = RC.View.Size[1] / 2 + posY;

	RC.View.Aspect = (Double)width / (Double)height;

	RC.View.HalfSize[0] = RC.View.Size[0] / 2;
	RC.View.HalfSize[1] = RC.View.Size[1] / 2;
}

SHADOWGL_API void ShadowGL::Perspective(Double fov, Double aspect, Double clipNear, Double clipFar)
{
	if (!RC_OK) { MessageBox(nullptr, TEXT("No Current Rendering Context!"), TEXT("Perspective()"), MB_OK | MB_ICONERROR); return; } 
	if (RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

    Matrix4 perspective;
	ZeroMatrix4(perspective);

	Double l_dTemp = 1.0 / tan(SCFDegToRad * fov / 2);

	perspective[0][0] = l_dTemp / aspect;

	perspective[1][1] = l_dTemp;

	perspective[2][2] = (clipFar + clipNear) / (clipNear - clipFar);
	perspective[2][3] = - 1;

	perspective[3][2] = (2 * clipFar * clipNear) / (clipNear - clipFar);

    //Near Clip Plane
	SetVector4(RC.View.ClipPlane[0], 0, 0, -1, (Float)-clipNear);

    //Far Clip Plane
	SetVector4(RC.View.ClipPlane[1], 0, 0, 1, (Float)clipFar);

    //Top Clip Plane
    Float3 tmp;
	SetVector3(tmp, 0, -1, 0);
	RotateVector3(tmp, tmp, (float)fov * 0.5f, 0, 0);
	SetVector4(RC.View.ClipPlane[2], tmp[0], tmp[1], tmp[2], 0);

    //Bottom Clip Plane
	SetVector3(tmp, 0, 1, 0);
	RotateVector3(tmp, tmp, -(float)fov * 0.5f, 0, 0);
	SetVector4(RC.View.ClipPlane[3], tmp[0], tmp[1], tmp[2], 0);

	float FovX = (Float)atan(tan(SCFDegToRad * fov / 2) * aspect) * SCFRadToDeg;

    //Right Clip Plane
	SetVector3(tmp, -1, 0, 0);
	RotateVector3(tmp, tmp, 0, -FovX, 0);
	SetVector4(RC.View.ClipPlane[4], tmp[0], tmp[1], tmp[2], 0);

    //Left Clip Plane
	SetVector3(tmp, 1, 0, 0);
	RotateVector3(tmp, tmp, 0, FovX, 0);
	SetVector4(RC.View.ClipPlane[5], tmp[0], tmp[1], tmp[2], 0);

	switch (RC.Matrix.Mode)
	{
	case SGL_MODELVIEW:
		{
            Matrix4 tmpMatrix;
			CopyMatrix4(tmpMatrix, RC.Matrix.ModelView[RC.Matrix.MVCurrent]);
			MultiplyMatrices4(RC.Matrix.ModelView[RC.Matrix.MVCurrent], tmpMatrix, perspective);
			return;
		}

	case SGL_PROJECTION:
		{
            Matrix4 tmpMatrix;
			CopyMatrix4(tmpMatrix, RC.Matrix.Projection[RC.Matrix.PCurrent]);
			MultiplyMatrices4(RC.Matrix.Projection[RC.Matrix.PCurrent], tmpMatrix, perspective);
			return;
		}

	case SGL_TEXTURE:
		{
            Matrix4 tmpMatrix;
			CopyMatrix4(tmpMatrix, RC.Matrix.Texture[RC.Matrix.TCurrent]);
			MultiplyMatrices4(RC.Matrix.Texture[RC.Matrix.TCurrent], tmpMatrix, perspective);
			return;
		}
	}
}

SHADOWGL_API void ShadowGL::Frustum(Double left, Double right, Double bottom, Double top, Double clipNear, Double clipFar)
{
	if (!RC_OK) { MessageBox(nullptr, TEXT("No Current Rendering Context!"), TEXT("Frustum()"), MB_OK | MB_ICONERROR); return; } 
	if (RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

    Matrix4 frustum;
	ZeroMatrix4(frustum);

	frustum[0][0] = 2 * clipNear / (right - left);

	frustum[1][1] = 2 * clipNear / (top - bottom);

	frustum[2][0] = (right + left) / (right - left);
	frustum[2][1] = (top + bottom) / (top - bottom);
	frustum[2][2] = - (clipFar + clipNear) / (clipFar - clipNear);
	frustum[2][3] = - 1;

	frustum[3][2] = - (2 * clipFar * clipNear) / (clipFar - clipNear);

	switch (RC.Matrix.Mode)
	{
	case SGL_MODELVIEW:
		{
            Matrix4 tmp;
			CopyMatrix4(tmp, RC.Matrix.ModelView[RC.Matrix.MVCurrent]);
			MultiplyMatrices4(RC.Matrix.ModelView[RC.Matrix.MVCurrent], tmp, frustum);
			return;
		}

	case SGL_PROJECTION:
		{
            Matrix4 tmp;
			CopyMatrix4(tmp, RC.Matrix.Projection[RC.Matrix.PCurrent]);
			MultiplyMatrices4(RC.Matrix.Projection[RC.Matrix.PCurrent], tmp, frustum);
			return;
		}

	case SGL_TEXTURE:
		{
            Matrix4 tmp;
			CopyMatrix4(tmp, RC.Matrix.Texture[RC.Matrix.TCurrent]);
			MultiplyMatrices4(RC.Matrix.Texture[RC.Matrix.TCurrent], tmp, frustum);
			return;
		}
	}
}

SHADOWGL_API void ShadowGL::LoadIdentity()
{
	if (!RC_OK) { MessageBox(nullptr, TEXT("No Current Rendering Context!"), TEXT("LoadIdentity()"), MB_OK | MB_ICONERROR); return; } 
	if (RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

	switch (RC.Matrix.Mode)
	{
	case SGL_MODELVIEW:  { MakeIndentityMatrix4(RC.Matrix.ModelView [RC.Matrix.MVCurrent]); MakeIndentityMatrix4(RC.Matrix.ModelViewInverse[RC.Matrix.MVCurrent]); return; }
	case SGL_PROJECTION: { MakeIndentityMatrix4(RC.Matrix.Projection[RC.Matrix.PCurrent]);  return; }
	case SGL_TEXTURE:    { MakeIndentityMatrix4(RC.Matrix.Texture   [RC.Matrix.TCurrent]);  return; }
	}
}

SHADOWGL_API void ShadowGL::PushMatrix()
{
	if (!RC_OK) { MessageBox(nullptr, TEXT("No Current Rendering Context!"), TEXT("PushMatrix()"), MB_OK | MB_ICONERROR); return; } 
	if (RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

	switch (RC.Matrix.Mode)
	{
	case SGL_MODELVIEW:
		{
			if (RC.Matrix.MVCurrent >= (MAX_MODELVIEW_MATRICES - 1)) { RC.ErrorCode = SGL_STACK_OVERFLOW; return; }

			CopyMatrix4(RC.Matrix.ModelView       [RC.Matrix.MVCurrent + 1], RC.Matrix.ModelView       [RC.Matrix.MVCurrent]);
			CopyMatrix4(RC.Matrix.ModelViewInverse[RC.Matrix.MVCurrent + 1], RC.Matrix.ModelViewInverse[RC.Matrix.MVCurrent]);
			RC.Matrix.MVCurrent++;
			return;
		}
	case SGL_PROJECTION:
		{
			if (RC.Matrix.PCurrent >= (MAX_PROJECTION_MATRICES - 1)) { RC.ErrorCode = SGL_STACK_OVERFLOW; return; }

			CopyMatrix4(RC.Matrix.Projection[RC.Matrix.PCurrent + 1], RC.Matrix.Projection[RC.Matrix.PCurrent]);
			RC.Matrix.PCurrent++;
			return;
		}

	case SGL_TEXTURE:
		{
			if (RC.Matrix.TCurrent >= (MAX_TEXTURE_MATRICES - 1)) { RC.ErrorCode = SGL_STACK_OVERFLOW; return; }

			CopyMatrix4(RC.Matrix.Texture[RC.Matrix.TCurrent + 1], RC.Matrix.Texture[RC.Matrix.TCurrent]);
			RC.Matrix.TCurrent++;
			return;
		}
	}
}

SHADOWGL_API void ShadowGL::PopMatrix()
{
	if (!RC_OK) { MessageBox(nullptr, TEXT("No Current Rendering Context!"), TEXT("PopMatrix()"), MB_OK | MB_ICONERROR); return; } 
	if (RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

	if (RC.Matrix.Mode == SGL_MODELVIEW)
	{
		if (RC.Matrix.MVCurrent <= 0) { RC.ErrorCode = SGL_STACK_UNDERFLOW; return; }
		RC.Matrix.MVCurrent--;
	}

	if (RC.Matrix.Mode == SGL_PROJECTION)
	{
		if (RC.Matrix.PCurrent <= 0) { RC.ErrorCode = SGL_STACK_UNDERFLOW; return; }
		RC.Matrix.PCurrent--;
	}

	if (RC.Matrix.Mode == SGL_TEXTURE)
	{
		if (RC.Matrix.TCurrent <= 0) { RC.ErrorCode = SGL_STACK_UNDERFLOW; return; }
		RC.Matrix.TCurrent--;
	}
}

SHADOWGL_API void ShadowGL::MatrixMode(Enum mode)
{ 
	if (!RC_OK) { MessageBox(nullptr, TEXT("No Current Rendering Context!"), TEXT("MatrixMode()"), MB_OK | MB_ICONERROR); return; } 
	if (RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

	if ((mode == SGL_MODELVIEW) || (mode == SGL_PROJECTION) || (mode == SGL_TEXTURE)) { RC.Matrix.Mode = mode; }
}

SHADOWGL_API void ShadowGL::Rotatef(Float angle, Float x, Float y, Float z)
{
	if (!RC_OK) { MessageBox(nullptr, TEXT("No Current Rendering Context!"), TEXT("Rotatef()"), MB_OK | MB_ICONERROR); return; } 
	if (RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

    Float3 axis;
	SetVector3(axis, x, y, z);
	NormalizeVector3(axis, axis);

    Matrix3 rotation;
	AxisRotationMatrixN(rotation, axis, angle);

	//Post Multiply Current Matrix
	switch (RC.Matrix.Mode)
	{
	case SGL_MODELVIEW: 
		{
            Matrix4 tmp;

            CopyMatrix4(tmp, RC.Matrix.ModelView[RC.Matrix.MVCurrent]);
			MultiplyMatrix4Matrix3(RC.Matrix.ModelView[RC.Matrix.MVCurrent], tmp, rotation);
			
            Matrix3 inverseRotation;
			TransposeMatrix3(inverseRotation, rotation);

			CopyMatrix4(tmp, RC.Matrix.ModelViewInverse[RC.Matrix.MVCurrent]);
			MultiplyMatrix3Matrix4(RC.Matrix.ModelViewInverse[RC.Matrix.MVCurrent], inverseRotation, tmp);
			return;
		}
	case SGL_PROJECTION:
		{
            Matrix4 tmp;
			CopyMatrix4(tmp, RC.Matrix.Projection[RC.Matrix.PCurrent]);
			MultiplyMatrix4Matrix3(RC.Matrix.Projection[RC.Matrix.PCurrent], tmp, rotation);
			return;
		}

	case SGL_TEXTURE:
		{
            Matrix4 tmp;
			CopyMatrix4(tmp, RC.Matrix.Texture[RC.Matrix.TCurrent]);
			MultiplyMatrix4Matrix3(RC.Matrix.Texture[RC.Matrix.TCurrent], tmp, rotation);
			return;
		}	
	}
}

SHADOWGL_API void ShadowGL::Translatef(Float x, Float y, Float z)
{
	if (!RC_OK) { MessageBox(nullptr, TEXT("No Current Rendering Context!"), TEXT("Translatef()"), MB_OK | MB_ICONERROR); return; } 
	if (RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

    Matrix4 translate;
	MakeIndentityMatrix4(translate);

	translate[3][0] = x;
	translate[3][1] = y;
	translate[3][2] = z; 

	//Post Multiply Current Matrix
	switch (RC.Matrix.Mode)
	{
	case SGL_MODELVIEW:
		{
            Matrix4 tmp;
			CopyMatrix4(tmp, RC.Matrix.ModelView[RC.Matrix.MVCurrent]);
			MultiplyMatrices4(RC.Matrix.ModelView[RC.Matrix.MVCurrent], tmp, translate);
		
            Matrix4 inverseTranslate;
			CopyMatrix4(inverseTranslate, translate);
			inverseTranslate[3][0] = - inverseTranslate[3][0];
			inverseTranslate[3][1] = - inverseTranslate[3][1];
			inverseTranslate[3][2] = - inverseTranslate[3][2];

			CopyMatrix4(tmp, RC.Matrix.ModelViewInverse[RC.Matrix.MVCurrent]);
			MultiplyMatrices4(RC.Matrix.ModelViewInverse[RC.Matrix.MVCurrent], inverseTranslate, tmp);
			return;
		}

	case SGL_PROJECTION:
		{
            Matrix4 tmp;
			CopyMatrix4(tmp, RC.Matrix.Projection[RC.Matrix.PCurrent]);
			MultiplyMatrices4(RC.Matrix.Projection[RC.Matrix.PCurrent], tmp, translate);
			return;
		}

	case SGL_TEXTURE:
		{
            Matrix4 tmp;
			CopyMatrix4(tmp, RC.Matrix.Texture[RC.Matrix.TCurrent]);
			MultiplyMatrices4(RC.Matrix.Texture[RC.Matrix.TCurrent], tmp, translate);
			return;
		}
	}
}

SHADOWGL_API void ShadowGL::Scalef(Float x, Float y, Float z)
{
	if (!RC_OK) { MessageBox(nullptr, TEXT("No Current Rendering Context!"), TEXT("Scalef()"), MB_OK | MB_ICONERROR); return; } 
	if (RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

    Matrix3 scale;
	MakeIndentityMatrix3(scale);

	scale[0][0] = x;
	scale[1][1] = y;
	scale[2][2] = z; 

	//Post Multiply Current Matrix
	switch (RC.Matrix.Mode)
	{
	case SGL_MODELVIEW:
		{
            Matrix4 tmp;
			CopyMatrix4(tmp, RC.Matrix.ModelView[RC.Matrix.MVCurrent]);
			MultiplyMatrix4Matrix3(RC.Matrix.ModelView[RC.Matrix.MVCurrent], tmp, scale);
			
            Matrix3 inverseScale;
			CopyMatrix3(inverseScale, scale);
			inverseScale[0][0] = 1 / inverseScale[0][0];
			inverseScale[1][1] = 1 / inverseScale[1][1];
			inverseScale[2][2] = 1 / inverseScale[2][2];

			CopyMatrix4(tmp, RC.Matrix.ModelViewInverse[RC.Matrix.MVCurrent]);
			MultiplyMatrix3Matrix4(RC.Matrix.ModelViewInverse[RC.Matrix.MVCurrent], inverseScale, tmp);
			return;
		}

	case SGL_PROJECTION:
		{
            Matrix4 tmp;
			CopyMatrix4(tmp, RC.Matrix.Projection[RC.Matrix.PCurrent]);
			MultiplyMatrix4Matrix3(RC.Matrix.Projection[RC.Matrix.PCurrent], tmp, scale);
			return;
		}

	case SGL_TEXTURE:
		{
            Matrix4 tmp;
			CopyMatrix4(tmp, RC.Matrix.Texture[RC.Matrix.TCurrent]);
			MultiplyMatrix4Matrix3(RC.Matrix.Texture[RC.Matrix.TCurrent], tmp, scale);
			return;
		}
	}
}

SHADOWGL_API void ShadowGL::ClearColor(ClampF red, ClampF green, ClampF blue, ClampF alpha)
{
	if (!RC_OK) { MessageBox(nullptr, TEXT("No Current Rendering Context!"), TEXT("ClearColor()"), MB_OK | MB_ICONERROR); return; } 
	if (RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

	RC.Buffer.ClearColor[0]= alpha;
	RC.Buffer.ClearColor[1]= red;
	RC.Buffer.ClearColor[2]= green;
	RC.Buffer.ClearColor[3]= blue;
}

SHADOWGL_API void ShadowGL::ClearDepth(ClampD depth)
{
	if (!RC_OK) { MessageBox(nullptr, TEXT("No Current Rendering Context!"), TEXT("ClearDepth()"), MB_OK | MB_ICONERROR); return; } 
	if (RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

	RC.Buffer.ClearDepth = (ClampF)depth;
}

SHADOWGL_API void ShadowGL::Clear(Bitfield mask)
{
	if (!RC_OK) { MessageBox(nullptr, TEXT("No Current Rendering Context!"), TEXT("Clear()"), MB_OK | MB_ICONERROR); return; } 
	if (RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

	if (mask & SGL_COLOR_BUFFER_BIT)
	{
        UINT ClearColor =	(UByte)(255.0 * RC.Buffer.ClearColor[0]); ClearColor <<= 8;
		ClearColor +=		(UByte)(255.0 * RC.Buffer.ClearColor[1]); ClearColor <<= 8;
		ClearColor +=		(UByte)(255.0 * RC.Buffer.ClearColor[2]); ClearColor <<= 8;
		ClearColor +=		(UByte)(255.0 * RC.Buffer.ClearColor[3]); 

        memset(Buffer.Back, ClearColor, RC.View.Size[0] * RC.View.Size[1] * 4);

        /*		
        UInt BufferSize = RC.View.Size[0] * RC.View.Size[1] / 16;

        __int64 ClearColor =	(UByte)(255.0 * RC.Buffer.ClearColor[0]); ClearColor <<= 8;
		ClearColor +=			(UByte)(255.0 * RC.Buffer.ClearColor[1]); ClearColor <<= 8;
		ClearColor +=			(UByte)(255.0 * RC.Buffer.ClearColor[2]); ClearColor <<= 8;
		ClearColor +=			(UByte)(255.0 * RC.Buffer.ClearColor[3]); 

		ClearColor <<= 8;

		ClearColor +=			(UByte)(255.0 * RC.Buffer.ClearColor[0]); ClearColor <<= 8;
		ClearColor +=			(UByte)(255.0 * RC.Buffer.ClearColor[1]); ClearColor <<= 8;
		ClearColor +=			(UByte)(255.0 * RC.Buffer.ClearColor[2]); ClearColor <<= 8;
		ClearColor +=			(UByte)(255.0 * RC.Buffer.ClearColor[3]); 

		_asm
		{
			mov ebx, offset Buffer.Back
			mov ecx, [BufferSize]
					
			movq mm0, [ClearColor]
			movq mm1, [ClearColor]
			movq mm2, [ClearColor]
			movq mm3, [ClearColor]
			movq mm4, [ClearColor]
			movq mm5, [ClearColor]
			movq mm6, [ClearColor]
			movq mm7, [ClearColor]

			EraseBack:

				movq	qword ptr [ebx], mm0
				movq	qword ptr [ebx + 8], mm1
				movq	qword ptr [ebx + 16], mm2
				movq	qword ptr [ebx + 24], mm3
				movq	qword ptr [ebx + 32], mm4
				movq	qword ptr [ebx + 40], mm5
				movq	qword ptr [ebx + 48], mm6
				movq	qword ptr [ebx + 56], mm7
				add		ebx, 64

				loop EraseBack

			emms
		}
        */
	}

	if (mask & SGL_DEPTH_BUFFER_BIT)
	{
        //memset(Buffer.Depth, *(UInt*)((void*)&RC.Buffer.ClearDepth), RC.View.Size[0] * RC.View.Size[1] * 4);

		/*
		for (Int i = 0; i < (Int)(RC.View.Size[0] * RC.View.Size[1]); i++)
		{
			Buffer.Depth[i] = RC.Buffer.ClearDepth;
		}
		*/

        UInt BufferSize = RC.View.Size[0] * RC.View.Size[1] / 16;
		UInt ClearDepth = *(UInt*)((void*)&RC.Buffer.ClearDepth);

		_asm
		{
			mov ebx, offset Buffer.Depth
			mov ecx, [BufferSize]

			movd	mm0, [ClearDepth]
			psllq	mm0, 32
			movd	mm1, [ClearDepth]
			POR		mm0, mm1

			movq mm1, mm0
			movq mm2, mm0
			movq mm3, mm0
			movq mm4, mm0
			movq mm5, mm1
			movq mm6, mm1
			movq mm7, mm1

			EraseDepth:

				movq	qword ptr [ebx], mm0
				movq	qword ptr [ebx + 8], mm1
				movq	qword ptr [ebx + 16], mm2
				movq	qword ptr [ebx + 24], mm3
				movq	qword ptr [ebx + 32], mm4
				movq	qword ptr [ebx + 40], mm5
				movq	qword ptr [ebx + 48], mm6
				movq	qword ptr [ebx + 56], mm7
				add		ebx, 64

				loop EraseDepth
	
			emms
		}
	}
}
