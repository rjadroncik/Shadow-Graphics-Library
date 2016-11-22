#include "Extern.h"

#include "Renderer.h"
#include "ShadowGL.h"

using namespace SCFMathematics;
using namespace ShadowGLPrivate;

namespace ShadowGLPrivate
{
	SRendererState RS;

	Float Offset_13 = 0;
	Float Offset_12 = 0;
	Float Offset_23 = 0;

	Float Coefficient_13 = 0;
	Float Coefficient_12 = 0;
	Float Coefficient_23 = 0;

	Int X_LeftClamped  = 0;
	Int X_RightClamped = 0;

	Float3 B_LeftCoord;
	Float3 B_RightCoord;

	Float T_Area = 0;

	UByte *pTexture = NULL;

	SizeI TextureWidth  = 0;
	SizeI TextureHeight = 0;

	inline float TriangleArea2x2(_IN Float2& rVertex1, _IN Float2& rVertex2, _IN Float2& rVertex3)
	{
		return __abs((((rVertex2[0] - rVertex1[0]) * (rVertex3[1] - rVertex1[1])) - ((rVertex2[1] - rVertex1[1]) * (rVertex3[0] - rVertex1[0]))));
	}
}

#define GET_R(pixel) ((BYTE)(pixel))
#define GET_G(pixel) ((BYTE)(pixel >> 8))
#define GET_B(pixel) ((BYTE)(pixel >> 16))
#define GET_A(pixel) ((BYTE)(pixel >> 24))

#define GET_R_NOMALIZED(pixel) (GET_R(pixel) * 0.00392156862745098f)
#define GET_G_NOMALIZED(pixel) (GET_G(pixel) * 0.00392156862745098f)
#define GET_B_NOMALIZED(pixel) (GET_B(pixel) * 0.00392156862745098f)
#define GET_A_NOMALIZED(pixel) (GET_A(pixel) * 0.00392156862745098f)

void SHADOWGL_API ShadowGL::Begin(Enum primType)
{
	if (!RC_OK) { MessageBox(NULL, TEXT("No Current Rendering Context!"), TEXT("Begin()"), MB_OK | MB_ICONERROR); return; } 
	if (RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

	RC.Primitive.Type = (UByte)primType;

	if (primType == SGL_TRIANGLES)
	{
		RC.Primitive.VertexCount = 0;
		RC.Primitive.MaxVertices = 3;

		RC.Primitive.Building = TRUE;
		return;
	}
}

void SHADOWGL_API ShadowGL::End()
{
	if (!RC_OK) { MessageBox(NULL, TEXT("No Current Rendering Context!"), TEXT("End()"), MB_OK | MB_ICONERROR); return; } 
	if (!RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }
	
	RC.Primitive.VertexCount = 0;
	RC.Primitive.MaxVertices = 0;

	RC.Primitive.Building = FALSE;
}

void SHADOWGL_API ShadowGL::Vertex3f(Float x, Float y, Float z)
{
	if (!RC_OK) { MessageBox(NULL, TEXT("No Current Rendering Context!"), TEXT("Vertex3f()"), MB_OK | MB_ICONERROR); return; } 
	if (!RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

	if ((RC.View.Size[0] <= 0) || (RC.View.Size[1] <= 0)) { return; }

	SetVector4 (RC.Primitive.Vertex[RC.Primitive.VertexCount].ObjCoord, x, y, z, 1);
	CopyVector4(RC.Primitive.Vertex[RC.Primitive.VertexCount].ObjColor,    RC.Vertex.Color);
	CopyVector4(RC.Primitive.Vertex[RC.Primitive.VertexCount].ObjTexCoord, RC.Vertex.TexCoord);
	CopyVector3(RC.Primitive.Vertex[RC.Primitive.VertexCount].ObjNormal,   RC.Vertex.Normal);

	if (RC.Primitive.VertexCount == (RC.Primitive.MaxVertices - 1))
	{
		if ((RC.Primitive.Type != SGL_POINTS) && (RC.Primitive.Type != SGL_LINES))
		{
			if (CullPrimitive()) { RC.Primitive.VertexCount = 0; return; }
		}

		PrepareLighting(&RC.Primitive.Vertex[0], RC.Primitive.MaxVertices);
			
		if (ClipPrimitive())
		{
			if (RC.Primitive.ClipCount)
			{
				PreparePrimitive(&RC.Primitive.ClipVertex[RC.Primitive.ClipArray][0], RC.Primitive.ClipCount);
				RasterizePrimitive(&RC.Primitive.ClipVertex[RC.Primitive.ClipArray][0], RC.Primitive.ClipCount);
			}
		}
		else
		{
			PreparePrimitive(&RC.Primitive.Vertex[0], RC.Primitive.MaxVertices);
			RasterizePrimitive(&RC.Primitive.Vertex[0], RC.Primitive.MaxVertices);
		}

		RC.Primitive.VertexCount = 0;
		return;
	}

	RC.Primitive.VertexCount++;
}

SHADOWGL_API void ShadowGL::Color3f(Float r, Float g, Float b)
{
	if (!RC_OK) { MessageBox(NULL, TEXT("No Current Rendering Context!"), TEXT("Color3f()"), MB_OK | MB_ICONERROR); return; } 

	if (r < 0) { r = 0; } if (r > 1) { r = 1; }
	if (g < 0) { g = 0; } if (g > 1) { g = 1; }
	if (b < 0) { b = 0; } if (b > 1) { b = 1; }

	RC.Vertex.Color[0] = r;	
	RC.Vertex.Color[1] = g;	
	RC.Vertex.Color[2] = b;	
	RC.Vertex.Color[3] = 1;	
}
 
SHADOWGL_API void ShadowGL::Color4f(Float r, Float g, Float b, Float a)
{
	if (!RC_OK) { MessageBox(NULL, TEXT("No Current Rendering Context!"), TEXT("Color4f()"), MB_OK | MB_ICONERROR); return; } 

	if (r < 0) { r = 0; } if (r > 1) { r = 1; }
	if (g < 0) { g = 0; } if (g > 1) { g = 1; }
	if (b < 0) { b = 0; } if (b > 1) { b = 1; }
	if (a < 0) { a = 0; } if (a > 1) { a = 1; }

	RC.Vertex.Color[0] = r;	
	RC.Vertex.Color[1] = g;	
	RC.Vertex.Color[2] = b;	
	RC.Vertex.Color[3] = a;	
} 

SHADOWGL_API void ShadowGL::Color4fv(const Float *v)
{
	if (!RC_OK) { MessageBox(NULL, TEXT("No Current Rendering Context!"), TEXT("Color4f()"), MB_OK | MB_ICONERROR); return; } 

	RC.Vertex.Color[0] = v[0];	
	RC.Vertex.Color[1] = v[1];	
	RC.Vertex.Color[2] = v[2];	
	RC.Vertex.Color[3] = v[3];	

	if (RC.Vertex.Color[0] < 0) { RC.Vertex.Color[0] = 0; } if (RC.Vertex.Color[0] > 1) { RC.Vertex.Color[0] = 1; }
	if (RC.Vertex.Color[1] < 0) { RC.Vertex.Color[1] = 0; } if (RC.Vertex.Color[1] > 1) { RC.Vertex.Color[1] = 1; }
	if (RC.Vertex.Color[2] < 0) { RC.Vertex.Color[2] = 0; } if (RC.Vertex.Color[2] > 1) { RC.Vertex.Color[2] = 1; }
	if (RC.Vertex.Color[3] < 0) { RC.Vertex.Color[3] = 0; } if (RC.Vertex.Color[3] > 1) { RC.Vertex.Color[3] = 1; }
} 

SHADOWGL_API void ShadowGL::Normal3f(Float x, Float y, Float z)
{
	if (!RC_OK) { MessageBox(NULL, TEXT("No Current Rendering Context!"), TEXT("Color4f()"), MB_OK | MB_ICONERROR); return; } 

	RC.Vertex.Normal[0] = x;	
	RC.Vertex.Normal[1] = y;	
	RC.Vertex.Normal[2] = z;	
}

SHADOWGL_API void ShadowGL::TexCoord2f(Float x, Float y)
{
	if (!RC_OK) { MessageBox(NULL, TEXT("No Current Rendering Context!"), TEXT("Color4f()"), MB_OK | MB_ICONERROR); return; } 

	RC.Vertex.TexCoord[0] = x;	
	RC.Vertex.TexCoord[1] = 1 - y;	
	RC.Vertex.TexCoord[2] = 0;	
	RC.Vertex.TexCoord[3] = 0;	
}

namespace ShadowGLPrivate
{
	bool CullPrimitive()
	{
	//Prepare Normal Transformation Matrix
		TruncateTopLeftToMatrix3(Matrix301, RC.Matrix.ModelView[RC.Matrix.MVCurrent]);

	//Vertex Processing
		for (UByte i = 0; i < RC.Primitive.MaxVertices; i++)
		{
		//Create tVertex Eye Coordinates
			MultiplyMatrix4Vector4(RC.Primitive.Vertex[i].EyeCoord, RC.Matrix.ModelView[RC.Matrix.MVCurrent], RC.Primitive.Vertex[i].ObjCoord);
		}

	//Face Culling	 
		if (RC.Enable.FaceCulling)
		{
		//Find Normal
			if (RC.FrontFace == SGL_CCW)	{ MakeNormal(Float301, (Float3&)RC.Primitive.Vertex[0].EyeCoord, (Float3&)RC.Primitive.Vertex[1].EyeCoord, (Float3&)RC.Primitive.Vertex[2].EyeCoord); }
			else							{ MakeNormal(Float301, (Float3&)RC.Primitive.Vertex[2].EyeCoord, (Float3&)RC.Primitive.Vertex[1].EyeCoord, (Float3&)RC.Primitive.Vertex[0].EyeCoord); }

		//Find Triangle Center to Eye Direction Vector (Assuming Eye at (0, 0, 0)) 
			Float302[0] = (RC.Primitive.Vertex[0].EyeCoord[0] + RC.Primitive.Vertex[1].EyeCoord[0] + RC.Primitive.Vertex[2].EyeCoord[0]) / 3.0f;
			Float302[1] = (RC.Primitive.Vertex[0].EyeCoord[1] + RC.Primitive.Vertex[1].EyeCoord[1] + RC.Primitive.Vertex[2].EyeCoord[1]) / 3.0f;
			Float302[2] = (RC.Primitive.Vertex[0].EyeCoord[2] + RC.Primitive.Vertex[1].EyeCoord[2] + RC.Primitive.Vertex[2].EyeCoord[2]) / 3.0f;
			NormalizeVector3(Float302, Float302);

			float AngleCose = MultiplyVectors3(Float301, Float302);

		//Determine Whether to Continue or Not
			if (RC.CullStyle == SGL_BACK) { if (AngleCose > 0) { return TRUE; } }
			else { if (AngleCose < 0) { return TRUE; } }
		}

		return FALSE;
	}

	bool ClipPrimitive()
	{
		Boolean	Continue = FALSE;

		for (UByte i = 0; i < RC.Primitive.MaxVertices; i++)
		{
			RC.Primitive.Vertex[i].InClipVolume	= TRUE;
			RC.Primitive.Vertex[i].Finished		= FALSE;

			for (UByte j = 0; j < 6; j++)
			{
				if (RC.Primitive.Vertex[i].InClipVolume && ((RC.Primitive.Vertex[i].EyeCoord[0] * RC.View.ClipPlane[j][0] + RC.Primitive.Vertex[i].EyeCoord[1] * RC.View.ClipPlane[j][1] + RC.Primitive.Vertex[i].EyeCoord[2] * RC.View.ClipPlane[j][2] + RC.View.ClipPlane[j][3]) < 0)) 
				{ 
					RC.Primitive.Vertex[i].InClipVolume = FALSE;
					Continue = TRUE;
					continue; 
				}
			}
		}

		if (!Continue) { return FALSE; }

		RC.Primitive.ClipCount = ClipPrimitiveAtPlane(&RC.Primitive.Vertex[0], RC.Primitive.MaxVertices, &RC.View.ClipPlane[5][0], &RC.Primitive.ClipVertex[0][0]);

		RC.Primitive.ClipCount = ClipPrimitiveAtPlane(&RC.Primitive.ClipVertex[0][0], RC.Primitive.ClipCount, &RC.View.ClipPlane[4][0], &RC.Primitive.ClipVertex[1][0]);
		RC.Primitive.ClipCount = ClipPrimitiveAtPlane(&RC.Primitive.ClipVertex[1][0], RC.Primitive.ClipCount, &RC.View.ClipPlane[3][0], &RC.Primitive.ClipVertex[0][0]);
		RC.Primitive.ClipCount = ClipPrimitiveAtPlane(&RC.Primitive.ClipVertex[0][0], RC.Primitive.ClipCount, &RC.View.ClipPlane[2][0], &RC.Primitive.ClipVertex[1][0]);
		RC.Primitive.ClipCount = ClipPrimitiveAtPlane(&RC.Primitive.ClipVertex[1][0], RC.Primitive.ClipCount, &RC.View.ClipPlane[1][0], &RC.Primitive.ClipVertex[0][0]);
		RC.Primitive.ClipCount = ClipPrimitiveAtPlane(&RC.Primitive.ClipVertex[0][0], RC.Primitive.ClipCount, &RC.View.ClipPlane[0][0], &RC.Primitive.ClipVertex[1][0]);

		RC.Primitive.ClipArray = 1;

		if (!RC.Enable.SmoothShading)
		{
			//RC.Primitive.ClipVertex[RC.Primitive.ClipArray][0].ObjColor[0] = RC.Primitive.Vertex[0].ObjColor[0];
			//RC.Primitive.ClipVertex[RC.Primitive.ClipArray][0].ObjColor[1] = RC.Primitive.Vertex[0].ObjColor[1];
			//RC.Primitive.ClipVertex[RC.Primitive.ClipArray][0].ObjColor[2] = RC.Primitive.Vertex[0].ObjColor[2];
			//RC.Primitive.ClipVertex[RC.Primitive.ClipArray][0].ObjColor[3] = RC.Primitive.Vertex[0].ObjColor[3];

			//RC.Primitive.ClipVertex[RC.Primitive.ClipArray][0].ObjNormal[0] = RC.Primitive.Vertex[0].ObjNormal[0];
			//RC.Primitive.ClipVertex[RC.Primitive.ClipArray][0].ObjNormal[1] = RC.Primitive.Vertex[0].ObjNormal[1];
			//RC.Primitive.ClipVertex[RC.Primitive.ClipArray][0].ObjNormal[2] = RC.Primitive.Vertex[0].ObjNormal[2];
		
			RC.Primitive.ClipVertex[RC.Primitive.ClipArray][0].LitColor[0] = RC.Primitive.Vertex[0].LitColor[0];
			RC.Primitive.ClipVertex[RC.Primitive.ClipArray][0].LitColor[1] = RC.Primitive.Vertex[0].LitColor[1];
			RC.Primitive.ClipVertex[RC.Primitive.ClipArray][0].LitColor[2] = RC.Primitive.Vertex[0].LitColor[2];
			RC.Primitive.ClipVertex[RC.Primitive.ClipArray][0].LitColor[3] = RC.Primitive.Vertex[0].LitColor[3];
		}	

		return TRUE;
	}

	UByte ClipPrimitiveAtPlane(tVertex* inputVertex, _IN UByte count, const Float* clipPlane, tVertex* outputVertex) 
	{
		UByte	v1, v2;
		UByte	VertexIndex = 0;
		Float	t = 0;

		for (UByte i = 0; i < count; i++)
		{
			inputVertex[i].InClipVolume = FALSE;
			inputVertex[i].Finished     = FALSE;

			if ((inputVertex[i].EyeCoord[0] * clipPlane[0] + inputVertex[i].EyeCoord[1] * clipPlane[1] + inputVertex[i].EyeCoord[2] * clipPlane[2] + clipPlane[3]) >= 0)
			{
				inputVertex[i].InClipVolume = TRUE;
			}
		}

		for (UByte i = 0; i < count; i++)
		{
			v1 = i;
			if (v1 < (count - 1)) { v2 = v1 + 1; } else { v2 = 0; } 

			//If Vertices Lie on Different Sides of The Plane 
			if (inputVertex[v1].InClipVolume != inputVertex[v2].InClipVolume)
			{
				//Store First Vertex if It is In Clip Volume	
				if (inputVertex[v1].InClipVolume && !inputVertex[v1].Finished) { CopyVertex(&outputVertex[VertexIndex], &inputVertex[v1]); VertexIndex++; }

				//Find Vector Plane Crosspoint
				if (CrossPointVectorPlane((Float3&)inputVertex[v1].EyeCoord, (Float3&)inputVertex[v2].EyeCoord, *(Float4*)clipPlane, (Float3&)outputVertex[VertexIndex].EyeCoord)) { outputVertex[VertexIndex].EyeCoord[3] = inputVertex[v1].EyeCoord[3]; }

				//Measure v1 to New Vertex Distance
				float Distance01 = MeasureDistance3((Float3&)inputVertex[v1].EyeCoord, (Float3&)outputVertex[VertexIndex].EyeCoord);

				//Measure v1 to v2 Distance
				float Distance02 = MeasureDistance3((Float3&)inputVertex[v1].EyeCoord, (Float3&)inputVertex[v2].EyeCoord); 

				//Compute Coefficients
				t = Distance01 / Distance02; Distance01 = 1 - t;

				//Compute Values for New Vertex
				outputVertex[VertexIndex].ObjCoord[0] = inputVertex[v1].ObjCoord[0] * Distance01 + inputVertex[v2].ObjCoord[0] * t;
				outputVertex[VertexIndex].ObjCoord[1] = inputVertex[v1].ObjCoord[1] * Distance01 + inputVertex[v2].ObjCoord[1] * t;
				outputVertex[VertexIndex].ObjCoord[2] = inputVertex[v1].ObjCoord[2] * Distance01 + inputVertex[v2].ObjCoord[2] * t;
				outputVertex[VertexIndex].ObjCoord[3] = inputVertex[v1].ObjCoord[3] * Distance01 + inputVertex[v2].ObjCoord[3] * t;

				//CopyVector4(outputVertex[VertexIndex].,	inputVertex[v1].LitColor);
				//CopyVector4(outputVertex[VertexIndex].EyeNormal, inputVertex[v1].EyeNormal);

				if (RC.Enable.SmoothShading)
				{
					outputVertex[VertexIndex].LitColor[0] = inputVertex[v1].LitColor[0] * Distance01 + inputVertex[v2].LitColor[0] * t;
					outputVertex[VertexIndex].LitColor[1] = inputVertex[v1].LitColor[1] * Distance01 + inputVertex[v2].LitColor[1] * t;
					outputVertex[VertexIndex].LitColor[2] = inputVertex[v1].LitColor[2] * Distance01 + inputVertex[v2].LitColor[2] * t;
					outputVertex[VertexIndex].LitColor[3] = inputVertex[v1].LitColor[3] * Distance01 + inputVertex[v2].LitColor[3] * t;

					//outputVertex[VertexIndex].ObjColor[0] = inputVertex[v1].ObjColor[0] * Distance01 + inputVertex[v2].ObjColor[0] * t;
					//outputVertex[VertexIndex].ObjColor[1] = inputVertex[v1].ObjColor[1] * Distance01 + inputVertex[v2].ObjColor[1] * t;
					//outputVertex[VertexIndex].ObjColor[2] = inputVertex[v1].ObjColor[2] * Distance01 + inputVertex[v2].ObjColor[2] * t;
					//outputVertex[VertexIndex].ObjColor[3] = inputVertex[v1].ObjColor[3] * Distance01 + inputVertex[v2].ObjColor[3] * t;

					//outputVertex[VertexIndex].ObjNormal[0] = inputVertex[v1].ObjNormal[0] * Distance01 + inputVertex[v2].ObjNormal[0] * t;
					//outputVertex[VertexIndex].ObjNormal[1] = inputVertex[v1].ObjNormal[1] * Distance01 + inputVertex[v2].ObjNormal[1] * t;
					//outputVertex[VertexIndex].ObjNormal[2] = inputVertex[v1].ObjNormal[2] * Distance01 + inputVertex[v2].ObjNormal[2] * t;
				}

				if (RC.Enable.Texturing2D)
				{
					outputVertex[VertexIndex].ObjTexCoord[0] = inputVertex[v1].ObjTexCoord[0] * Distance01 + inputVertex[v2].ObjTexCoord[0] * t;
					outputVertex[VertexIndex].ObjTexCoord[1] = inputVertex[v1].ObjTexCoord[1] * Distance01 + inputVertex[v2].ObjTexCoord[1] * t;
					outputVertex[VertexIndex].ObjTexCoord[2] = inputVertex[v1].ObjTexCoord[2] * Distance01 + inputVertex[v2].ObjTexCoord[2] * t;
					outputVertex[VertexIndex].ObjTexCoord[3] = inputVertex[v1].ObjTexCoord[3] * Distance01 + inputVertex[v2].ObjTexCoord[3] * t;
				}
		
				VertexIndex++;

				//Store First Vertex if It is In Clip Volume	
				if (inputVertex[v2].InClipVolume && !inputVertex[v2].Finished) { CopyVertex(&outputVertex[VertexIndex], &inputVertex[v2]); VertexIndex++; }

				//Mark Both Vertices As Finished
				inputVertex[v1].Finished = TRUE; inputVertex[v2].Finished = TRUE;
			}

			//If Both Vertices Lie on The Same Side of The Plane & Are In Clip Volume
			if (inputVertex[v1].InClipVolume && inputVertex[v2].InClipVolume)
			{
				//Store The Edge as It is Completely In the Current Clipping Volume
				if (inputVertex[v1].InClipVolume && !inputVertex[v1].Finished) { CopyVertex(&outputVertex[VertexIndex], &inputVertex[v1]); inputVertex[v1].Finished = TRUE; VertexIndex++; }
				if (inputVertex[v2].InClipVolume && !inputVertex[v2].Finished) { CopyVertex(&outputVertex[VertexIndex], &inputVertex[v2]); inputVertex[v2].Finished = TRUE; VertexIndex++; }
			}
		}

		return VertexIndex;
	}

	void PrepareLighting(tVertex *vertex, UByte count)
	{
		TruncateTopLeftToMatrix3(Matrix301, RC.Matrix.ModelViewInverse[RC.Matrix.MVCurrent]); 

		MultiplyMatrices4(Matrix401, RC.Matrix.ModelView[RC.Matrix.MVCurrent], RC.Matrix.ModelViewInverse[RC.Matrix.MVCurrent]);

		//Vertex processing continued...
		for (UByte i = 0; i < count; i++)
		{
			//Process vertex normals (to eye coordinates) 
			MultiplyVectorT3Matrix3(vertex[i].EyeNormal, vertex[i].ObjNormal, Matrix301);

			//Defaulting to SGL_NORMALIZE enabled
			NormalizeVector3(vertex[i].EyeNormal, vertex[i].EyeNormal);
		}

		//Enable/disable smooth shading 

		UINT  VerticesToProcess = (RC.Enable.SmoothShading) ? (count) : (1);
		Float Attenuation;
		Float SpotFactor;

		//Lighting calculations
		for (UInt i = 0; i < VerticesToProcess; i++)
		{
			if (RC.Enable.Lighting)
			{
				//Global contributions (emmision + ambient)
				CopyVector3(  (Float3&)vertex[i].LitColor, (Float3&)RC.Material.Emission);
				MADVectors3CW((Float3&)vertex[i].LitColor, (Float3&)RC.Material.Ambient, (Float3&)RC.Ambient);

				Float3 CurLight; 

				//Light contributions
				for (UByte j = 0; j < MAX_LIGHTS; j++)
				{
					if (!RC.Light[j].Enabled) { continue; }

					ZeroVector3(CurLight);

					//Light to vertex distance
					MakeVector3(Float301, (Float3&)RC.Light[j].EyePosition, (Float3&)vertex[i].EyeCoord);
					float LightToVertexDistance = MeasureVector3(Float301);

					//Attenuation factor
					if (vertex[i].EyeCoord[3] != 0)
					{
						Attenuation = RC.Light[j].Constant + RC.Light[j].Linear * LightToVertexDistance + RC.Light[j].Quadratic * LightToVertexDistance * LightToVertexDistance;

						if (Attenuation) { Attenuation = 1 / Attenuation; }
						else             { Attenuation = 1; }
					}
					else { Attenuation = 1; }

					//Spotlight factor
					if (RC.Light[j].CutOff == 180) { SpotFactor = 1; }
					else
					{
						//Max of dot product of light to vertex vector & light direction
						NormalizeVector3(Float301, Float301);
						float SpotAngleCose = MultiplyVectors3(Float301, RC.Light[j].EyeDirection);

						if (SpotAngleCose < 0) { SpotAngleCose = 0; }
						
						//Factor raised to power of "exponent" (if necessary)
						if (SpotAngleCose < cos(RC.Light[j].CutOff * SCFDegToRad)) { SpotFactor = 0; }
						else
						{
							if (RC.Light[j].Exponent == 0) { SpotFactor = 1; } 
							else                           { SpotFactor = (Float)powf(SpotAngleCose, (float)RC.Light[j].Exponent); }
						}
					}

					//Ambient contribution
					MADVectors3CW(CurLight, (Float3&)RC.Material.Ambient, (Float3&)RC.Light[j].Ambient);
					
					//Normalized vertex to light vector	
					MakeVector3(Float301, (Float3&)vertex[i].EyeCoord, (Float3&)RC.Light[j].EyePosition);
					NormalizeVector3(Float301, Float301);

					//Max of dot product of: Vertex to light vector & normal 
					float DiffuseFactor = __max(0, MultiplyVectors3(vertex[i].EyeNormal, Float301));
			
					//Compute & add diffuse contribution
					ScaleVector3(Float302, (Float3&)RC.Material.Diffuse, DiffuseFactor);
					MADVectors3CW(CurLight, Float302, (Float3&)RC.Light[j].Diffuse);
					
					//Specular contribution
					if (DiffuseFactor != 0)
					{
						if (RC.Enable.LocalViewer)
						{
							NormalizeVector3(Float302, (Float3&)vertex[i].EyeCoord);
							SubtractVectors3(Float301, Float301, Float302);
						}
						else { Float301[2] += 1; }

						NormalizeVector3(Float301, Float301);

						//Max of dot product of: normal & specular half vector
						float SpecularFactor = __max(0, MultiplyVectors3(vertex[i].EyeNormal, Float301));

						//Raise specular factor to the power of material shininess
						if (RC.Material.Shininess == 0) { SpecularFactor = 1; }
						else                            { SpecularFactor = (Float)pow(SpecularFactor, RC.Material.Shininess); }
						
						//Add Specular Contribution
						ScaleVector3(Float302, (Float3&)RC.Material.Specular, SpecularFactor);
						MADVectors3CW(CurLight, Float302, (Float3&)RC.Light[j].Specular);
					}

					ScaleVector3(CurLight, CurLight, Attenuation);
					ScaleVector3(CurLight, CurLight, SpotFactor);

					AddVectors3((Float3&)vertex[i].LitColor, (Float3&)vertex[i].LitColor, CurLight);
				}

				vertex[i].LitColor[3] = RC.Material.Diffuse[3];
			}
			else { CopyVector4(vertex[i].LitColor, vertex[i].ObjColor); }

			if (vertex[i].LitColor[0] > 1) { vertex[i].LitColor[0] = 1; } 
			if (vertex[i].LitColor[1] > 1) { vertex[i].LitColor[1] = 1; } 
			if (vertex[i].LitColor[2] > 1) { vertex[i].LitColor[2] = 1; } 
			if (vertex[i].LitColor[3] > 1) { vertex[i].LitColor[3] = 1; } 
		
			if (vertex[i].LitColor[0] < 0) { vertex[i].LitColor[0] = 0; } 
			if (vertex[i].LitColor[1] < 0) { vertex[i].LitColor[1] = 0; } 
			if (vertex[i].LitColor[2] < 0) { vertex[i].LitColor[2] = 0; } 
			if (vertex[i].LitColor[3] < 0) { vertex[i].LitColor[3] = 0; } 
		}
	}

	void PreparePrimitive(tVertex *vertex, UByte count)
	{
		//Vertex processing continued...
		for (UByte i = 0; i < count; i++)
		{
			//Create vertex clip coordinates
			MultiplyMatrix4Vector4(vertex[i].ClipCoord, RC.Matrix.Projection[RC.Matrix.PCurrent], vertex[i].EyeCoord);
		
			//Create normalized vertex coordinates
			DivideVector4(vertex[i].NormCoord, vertex[i].ClipCoord, vertex[i].ClipCoord[3]);

			//Create vertex window coordinates
			vertex[i].ViewCoord[0] = RC.View.Origin[0] + RC.View.HalfSize[0] * vertex[i].NormCoord[0];  		
			vertex[i].ViewCoord[1] = RC.View.Origin[1] + RC.View.HalfSize[1] * vertex[i].NormCoord[1];  		
			vertex[i].ViewCoord[2] = vertex[i].NormCoord[2];// / 2 + 0.5f;  		

			//Process texture coordinates
			if (RC.Enable.Texturing2D)
			{
				MultiplyMatrix4Vector4(vertex[i].FinTexCoord, RC.Matrix.Texture[RC.Matrix.TCurrent], vertex[i].ObjTexCoord);
			}
		}

		//Enable/disable smooth shading 
		UINT VerticesToProcess = (RC.Enable.SmoothShading) ? (count) : (1);

		//Lighting calculations
		for (UInt i = 0; i < VerticesToProcess; i++)
		{
			if (RC.Enable.Fog)
			{
				float FogContribution = 0;

				if ((RC.Fog.Mode == SGL_LINEAR) && ((RC.Fog.End - RC.Fog.Start) != 0))
				{
					FogContribution = (RC.Fog.End + vertex[i].EyeCoord[2]) / (RC.Fog.End - RC.Fog.Start);
				}
				
				if (RC.Fog.Mode == SGL_EXP)
				{
					FogContribution = (Float)exp(-vertex[i].ViewCoord[2] * RC.Fog.Density);
				}

				if (RC.Fog.Mode == SGL_EXP2)
				{
					FogContribution = (Float)exp(-(vertex[i].ViewCoord[2] * RC.Fog.Density * vertex[i].ViewCoord[2] * RC.Fog.Density));
				}

				if (FogContribution < 0) { FogContribution = 0; } 
				if (FogContribution > 1) { FogContribution = 1; } 

				float InverseFogContribution = 1 - FogContribution; 

				vertex[i].FogFactor = FogContribution;

				if (!RC.Enable.Texturing1D && !RC.Enable.Texturing2D)
				{
					vertex[i].LitColor[0] = FogContribution * vertex[i].LitColor[0] + InverseFogContribution * RC.Fog.Color[0];
					vertex[i].LitColor[1] = FogContribution * vertex[i].LitColor[1] + InverseFogContribution * RC.Fog.Color[1];
					vertex[i].LitColor[2] = FogContribution * vertex[i].LitColor[2] + InverseFogContribution * RC.Fog.Color[2];
				}
			}

			if (vertex[i].LitColor[0] > 1) { vertex[i].LitColor[0] = 1; } 
			if (vertex[i].LitColor[1] > 1) { vertex[i].LitColor[1] = 1; } 
			if (vertex[i].LitColor[2] > 1) { vertex[i].LitColor[2] = 1; } 
			if (vertex[i].LitColor[3] > 1) { vertex[i].LitColor[3] = 1; } 
		
			if (vertex[i].LitColor[0] < 0) { vertex[i].LitColor[0] = 0; } 
			if (vertex[i].LitColor[1] < 0) { vertex[i].LitColor[1] = 0; } 
			if (vertex[i].LitColor[2] < 0) { vertex[i].LitColor[2] = 0; } 
			if (vertex[i].LitColor[3] < 0) { vertex[i].LitColor[3] = 0; } 
		}
	}

	void RasterizePrimitive(tVertex *vertex, UByte count)
	{
		if (RC.Primitive.Type == SGL_TRIANGLES)
		{
			for (UByte i = 0; i < (count - 2); i++)
			{
				//Select Vertices
				RS.V1 = 0;
				RS.V2 = i + 1;
				RS.V3 = i + 2;
			
				//Init Values For Flat Shading
				if (!RC.Enable.SmoothShading)
				{
					CopyVector4(RS.Line.Current.Color, vertex[0].LitColor); 

					RS.Pixel.Write.Color = (UByte)(255 * RS.Line.Current.Color[3]);
					RS.Pixel.Write.Color <<= 8;
					RS.Pixel.Write.Color += (UByte)(255 * RS.Line.Current.Color[2]);
					RS.Pixel.Write.Color <<= 8;
					RS.Pixel.Write.Color += (UByte)(255 * RS.Line.Current.Color[1]);
					RS.Pixel.Write.Color <<= 8;
					RS.Pixel.Write.Color += (UByte)(255 * RS.Line.Current.Color[0]);
				}
				
				//Sort Vertices
				if (vertex[RS.V1].ViewCoord[1] > vertex[RS.V2].ViewCoord[1]) { UByte l_TempByte = RS.V1; RS.V1 = RS.V2; RS.V2 = l_TempByte; }
				if (vertex[RS.V2].ViewCoord[1] > vertex[RS.V3].ViewCoord[1]) { UByte l_TempByte = RS.V2; RS.V2 = RS.V3; RS.V3 = l_TempByte; }
				if (vertex[RS.V1].ViewCoord[1] > vertex[RS.V2].ViewCoord[1]) { UByte l_TempByte = RS.V1; RS.V1 = RS.V2; RS.V2 = l_TempByte; }
				if (vertex[RS.V2].ViewCoord[1] > vertex[RS.V3].ViewCoord[1]) { UByte l_TempByte = RS.V2; RS.V2 = RS.V3; RS.V3 = l_TempByte; }

				if (vertex[RS.V1].ViewCoord[1] == vertex[RS.V2].ViewCoord[1]) { if (vertex[RS.V1].ViewCoord[0] > vertex[RS.V2].ViewCoord[0]) { UByte l_TempByte = RS.V1; RS.V1 = RS.V2; RS.V2 = l_TempByte; } }

				//Draw Triangle
				RasterizeTriangle(&vertex[RS.V1], &vertex[RS.V2], &vertex[RS.V3]); 
			}
		}
	}

	void RasterizeTriangle(tVertex *vertex1, tVertex *vertex2, tVertex *vertex3)
	{
		//Texturing variables
		RS.Vertex[0].Clip.W = vertex1->ClipCoord[3]; 
		RS.Vertex[1].Clip.W	= vertex2->ClipCoord[3];
		RS.Vertex[2].Clip.W = vertex3->ClipCoord[3];

		//Prepare iterator[1,3] 
		MakeVector3(Float304, (Float3&)vertex1->ViewCoord, (Float3&)vertex3->ViewCoord);
		DivideVector3(Float304, Float304, Float304[1]);

		//Prepare iterator[1,2] 
		MakeVector3(Float305, (Float3&)vertex1->ViewCoord, (Float3&)vertex2->ViewCoord);
		DivideVector3(Float305, Float305, Float305[1]);

		//Prepare iterator[2,3] 
		MakeVector3(Float306, (Float3&)vertex2->ViewCoord, (Float3&)vertex3->ViewCoord);
		DivideVector3(Float306, Float306, Float306[1]);

		//Prepare other values
		CopyVector4(RS.Fog.Color, RC.Fog.Color);

		if (RC.Enable.Texturing2D)
		{
			pTexture              = Texture[RC.TexCurrent2D].pData;
			TextureWidth          = Texture[RC.TexCurrent2D].Width;
			TextureHeight         = Texture[RC.TexCurrent2D].Height;
			RS.Texture.Components = Texture[RC.TexCurrent2D].Components;
		}

		T_Area = TriangleArea2x2((Float2&)vertex1->ViewCoord, (Float2&)vertex2->ViewCoord, (Float2&)vertex3->ViewCoord);

		//Compute coefficients
		if ((vertex3->ViewCoord[1] - vertex1->ViewCoord[1]) != 0) { Coefficient_13 = (vertex1->ViewCoord[0] - vertex3->ViewCoord[0]) / (vertex3->ViewCoord[1] - vertex1->ViewCoord[1]); }
		else { Coefficient_13 = 0; }

		if ((vertex2->ViewCoord[1] - vertex1->ViewCoord[1]) != 0) { Coefficient_12 = (vertex1->ViewCoord[0] - vertex2->ViewCoord[0]) / (vertex2->ViewCoord[1] - vertex1->ViewCoord[1]); }
		else { Coefficient_12 = 0; }

		if ((vertex3->ViewCoord[1] - vertex2->ViewCoord[1]) != 0) { Coefficient_23 = (vertex2->ViewCoord[0] - vertex3->ViewCoord[0]) / (vertex3->ViewCoord[1] - vertex2->ViewCoord[1]); }
		else { Coefficient_23 = 0; }

		//Compute offsets
		Offset_13 = - Coefficient_13 * vertex1->ViewCoord[1] - vertex1->ViewCoord[0];
		Offset_12 = - Coefficient_12 * vertex1->ViewCoord[1] - vertex1->ViewCoord[0];
		Offset_23 = - Coefficient_23 * vertex2->ViewCoord[1] - vertex2->ViewCoord[0];

		//Apply left-top rasterization rules to vertex values
		if (vertex1->ViewCoord[1] > ((Int)vertex1->ViewCoord[1] + 0.5f)) { RS.Vertex[0].Y = (Int)vertex1->ViewCoord[1] + 1; }
		else { RS.Vertex[0].Y = (Int)vertex1->ViewCoord[1]; }

		if (vertex2->ViewCoord[1] < ((Int)vertex2->ViewCoord[1] + 0.5f)) { RS.Vertex[1].Y = (Int)vertex2->ViewCoord[1] - 1; }
		else { RS.Vertex[1].Y = (Int)vertex2->ViewCoord[1]; }

		if (vertex3->ViewCoord[1] <= ((Int)vertex3->ViewCoord[1] + 0.5f)) { RS.Vertex[2].Y = (Int)vertex3->ViewCoord[1] - 1; }
		else { RS.Vertex[2].Y = (Int)vertex3->ViewCoord[1]; }

		//Scan one line at a time
		if (vertex2->ViewCoord[0] >= (- Coefficient_13 * vertex2->ViewCoord[1] - Offset_13)) 
		{
			//Vertex2 is on the right
			for (RS.Line.Index = RS.Vertex[0].Y; RS.Line.Index <= RS.Vertex[1].Y; RS.Line.Index++)
			{
				if (vertex1->ViewCoord[1] != vertex2->ViewCoord[1])
				{
					RS.Line.Left.X  = - Coefficient_13 * ((Float)RS.Line.Index + 0.5f) - Offset_13;
					RS.Line.Right.X = - Coefficient_12 * ((Float)RS.Line.Index + 0.5f) - Offset_12;
				}
				else { RS.Line.Left.X = vertex1->ViewCoord[0]; RS.Line.Right.X = vertex2->ViewCoord[0]; }

				ScanLine(vertex1, vertex2, vertex3);
			}

			for (RS.Line.Index = RS.Vertex[1].Y + 1; RS.Line.Index <= RS.Vertex[2].Y; RS.Line.Index++)
			{
				//Compute left & right coordinate values
				if (vertex2->ViewCoord[1] != vertex3->ViewCoord[1])
				{
					RS.Line.Left.X  = - Coefficient_13 * ((Float)RS.Line.Index + 0.5f) - Offset_13;
					RS.Line.Right.X = - Coefficient_23 * ((Float)RS.Line.Index + 0.5f) - Offset_23;
				}
				else {	RS.Line.Left.X = vertex2->ViewCoord[0]; RS.Line.Right.X = vertex3->ViewCoord[0]; }

				ScanLine(vertex1, vertex2, vertex3);
			}
		}
		else
		{
			//Vertex2 is on the left
			for (RS.Line.Index = RS.Vertex[0].Y; RS.Line.Index <= RS.Vertex[1].Y; RS.Line.Index++)
			{
				if (vertex1->ViewCoord[1] != vertex2->ViewCoord[1])
				{
					RS.Line.Right.X = - Coefficient_13 * ((Float)RS.Line.Index + 0.5f) - Offset_13;
					RS.Line.Left.X  = - Coefficient_12 * ((Float)RS.Line.Index + 0.5f) - Offset_12;
				}
				else { RS.Line.Right.X = vertex1->ViewCoord[0]; RS.Line.Left.X = vertex2->ViewCoord[0]; }

				ScanLine(vertex1, vertex2, vertex3);
			}

			for (RS.Line.Index = RS.Vertex[1].Y + 1; RS.Line.Index <= RS.Vertex[2].Y; RS.Line.Index++)
			{
				if (vertex2->ViewCoord[1] != vertex3->ViewCoord[1])
				{
					RS.Line.Right.X = - Coefficient_13 * ((Float)RS.Line.Index + 0.5f) - Offset_13;
					RS.Line.Left.X  = - Coefficient_23 * ((Float)RS.Line.Index + 0.5f) - Offset_23;
				}
				else { RS.Line.Right.X = vertex2->ViewCoord[0]; RS.Line.Left.X = vertex3->ViewCoord[0]; }

				ScanLine(vertex1, vertex2, vertex3);
			}
		}
	}

	void ScanLine(tVertex *vertex1, tVertex *vertex2, tVertex *vertex3)
	{
		//Apply left-top rasterization rules to vertex values
		X_LeftClamped  = (RS.Line.Left.X  > ((Int)RS.Line.Left.X  + 0.5f)) ? (Int)RS.Line.Left.X  + 1 : X_LeftClamped  = (Int)RS.Line.Left.X; 
		X_RightClamped = (RS.Line.Right.X < ((Int)RS.Line.Right.X + 0.5f)) ? (Int)RS.Line.Right.X - 1 : X_RightClamped = (Int)RS.Line.Right.X;

		//Compute barycentric coords
		SetVector2(Float201, (Float)X_LeftClamped  + 0.5f, (Float)RS.Line.Index + 0.5f);
		SetVector2(Float202, (Float)X_RightClamped + 0.5f, (Float)RS.Line.Index + 0.5f);

		B_LeftCoord[0] = TriangleArea2x2(Float201, (Float2&)vertex2->ViewCoord, (Float2&)vertex3->ViewCoord) / T_Area;
		B_LeftCoord[1] = TriangleArea2x2(Float201, (Float2&)vertex1->ViewCoord, (Float2&)vertex3->ViewCoord) / T_Area;
		B_LeftCoord[2] = TriangleArea2x2(Float201, (Float2&)vertex1->ViewCoord, (Float2&)vertex2->ViewCoord) / T_Area;

		B_RightCoord[0] = TriangleArea2x2(Float202, (Float2&)vertex2->ViewCoord, (Float2&)vertex3->ViewCoord) / T_Area;
		B_RightCoord[1] = TriangleArea2x2(Float202, (Float2&)vertex1->ViewCoord, (Float2&)vertex3->ViewCoord) / T_Area;
		B_RightCoord[2] = TriangleArea2x2(Float202, (Float2&)vertex1->ViewCoord, (Float2&)vertex2->ViewCoord) / T_Area;

		//Prepare z-buffer values
		RS.Line.Left.Z    = B_LeftCoord[0]  * vertex1->ViewCoord[2] + B_LeftCoord[1]  * vertex2->ViewCoord[2] + B_LeftCoord[2]  * vertex3->ViewCoord[2];
		RS.Line.Right.Z   = B_RightCoord[0] * vertex1->ViewCoord[2] + B_RightCoord[1] * vertex2->ViewCoord[2] + B_RightCoord[2] * vertex3->ViewCoord[2];
		RS.Line.Current.Z = RS.Line.Left.Z;

		//Prepare line length & buffer index
		RS.Pixel.Index = RS.Line.Index * RC.View.Size[0] + X_LeftClamped;
		RS.Line.Length = (Float)(RS.Line.Right.X - RS.Line.Left.X);

		//Prepare color
		if (RC.Enable.SmoothShading)
		{
			RS.Line.Left.Color[0] = B_LeftCoord[0] * vertex1->LitColor[0] + B_LeftCoord[1] * vertex2->LitColor[0] + B_LeftCoord[2] * vertex3->LitColor[0];
			RS.Line.Left.Color[1] = B_LeftCoord[0] * vertex1->LitColor[1] + B_LeftCoord[1] * vertex2->LitColor[1] + B_LeftCoord[2] * vertex3->LitColor[1];
			RS.Line.Left.Color[2] = B_LeftCoord[0] * vertex1->LitColor[2] + B_LeftCoord[1] * vertex2->LitColor[2] + B_LeftCoord[2] * vertex3->LitColor[2];
			RS.Line.Left.Color[3] = B_LeftCoord[0] * vertex1->LitColor[3] + B_LeftCoord[1] * vertex2->LitColor[3] + B_LeftCoord[2] * vertex3->LitColor[3];

			RS.Line.Right.Color[0] = B_RightCoord[0] * vertex1->LitColor[0] + B_RightCoord[1] * vertex2->LitColor[0] + B_RightCoord[2] * vertex3->LitColor[0];
			RS.Line.Right.Color[1] = B_RightCoord[0] * vertex1->LitColor[1] + B_RightCoord[1] * vertex2->LitColor[1] + B_RightCoord[2] * vertex3->LitColor[1];
			RS.Line.Right.Color[2] = B_RightCoord[0] * vertex1->LitColor[2] + B_RightCoord[1] * vertex2->LitColor[2] + B_RightCoord[2] * vertex3->LitColor[2];
			RS.Line.Right.Color[3] = B_RightCoord[0] * vertex1->LitColor[3] + B_RightCoord[1] * vertex2->LitColor[3] + B_RightCoord[2] * vertex3->LitColor[3];
		
			CopyVector4(RS.Line.Current.Color, RS.Line.Left.Color);
		}
	//	else { CopyVector4(&RS.Line.Current.Color[0], &vertex1->LitColor[0]); }

		//Prepare texture coords
		if (RC.Enable.Texturing2D)
		{
			RS.Line.Left.Numerator.S  = B_LeftCoord[0]  * vertex1->FinTexCoord[0] / RS.Vertex[0].Clip.W + B_LeftCoord[1]  * vertex2->FinTexCoord[0] / RS.Vertex[1].Clip.W + B_LeftCoord[2]  * vertex3->FinTexCoord[0] / RS.Vertex[2].Clip.W;
			RS.Line.Right.Numerator.S = B_RightCoord[0] * vertex1->FinTexCoord[0] / RS.Vertex[0].Clip.W + B_RightCoord[1] * vertex2->FinTexCoord[0] / RS.Vertex[1].Clip.W + B_RightCoord[2] * vertex3->FinTexCoord[0] / RS.Vertex[2].Clip.W;

			RS.Line.Left.Numerator.T  = B_LeftCoord[0]  * vertex1->FinTexCoord[1] / RS.Vertex[0].Clip.W + B_LeftCoord[1]  * vertex2->FinTexCoord[1] / RS.Vertex[1].Clip.W + B_LeftCoord[2]  * vertex3->FinTexCoord[1] / RS.Vertex[2].Clip.W;
			RS.Line.Right.Numerator.T = B_RightCoord[0] * vertex1->FinTexCoord[1] / RS.Vertex[0].Clip.W + B_RightCoord[1] * vertex2->FinTexCoord[1] / RS.Vertex[1].Clip.W + B_RightCoord[2] * vertex3->FinTexCoord[1] / RS.Vertex[2].Clip.W;

			RS.Line.Left.Denominator.S  = B_LeftCoord[0]  / RS.Vertex[0].Clip.W + B_LeftCoord[1]  / RS.Vertex[1].Clip.W + B_LeftCoord[2]  / RS.Vertex[2].Clip.W;
			RS.Line.Right.Denominator.S = B_RightCoord[0] / RS.Vertex[0].Clip.W + B_RightCoord[1] / RS.Vertex[1].Clip.W + B_RightCoord[2] / RS.Vertex[2].Clip.W;

			RS.Line.Left.Denominator.T  = B_LeftCoord[0]  / RS.Vertex[0].Clip.W + B_LeftCoord[1]  / RS.Vertex[1].Clip.W + B_LeftCoord[2]  / RS.Vertex[2].Clip.W;
			RS.Line.Right.Denominator.T = B_RightCoord[0] / RS.Vertex[0].Clip.W + B_RightCoord[1] / RS.Vertex[1].Clip.W + B_RightCoord[2] / RS.Vertex[2].Clip.W;

			RS.Line.Current.Numerator.S = RS.Line.Left.Numerator.S;
			RS.Line.Current.Numerator.T = RS.Line.Left.Numerator.T;

			RS.Line.Current.Denominator.S = RS.Line.Left.Denominator.S;
			RS.Line.Current.Denominator.T = RS.Line.Left.Denominator.T;

			if (RC.Enable.Fog)
			{
				if (RC.Enable.SmoothShading)
				{
					RS.Line.Left.Fog    = B_LeftCoord[0]  * vertex1->FogFactor + B_LeftCoord[1]  * vertex2->FogFactor + B_LeftCoord[2]  * vertex3->FogFactor;
					RS.Line.Right.Fog   = B_RightCoord[0] * vertex1->FogFactor + B_RightCoord[1] * vertex2->FogFactor + B_RightCoord[2] * vertex3->FogFactor;

					RS.Line.Current.Fog = RS.Line.Left.Fog;
				}
			}
		}

		//Prepare iterators
		if (RS.Line.Length != 0)
		{ 
			RS.Line.Iterator.Z = (RS.Line.Right.Z - RS.Line.Left.Z) / RS.Line.Length; 

			if (RC.Enable.SmoothShading)
			{
				SubtractVectors4(RS.Line.Iterator.Color, RS.Line.Right.Color,    RS.Line.Left.Color);
				DivideVector4   (RS.Line.Iterator.Color, RS.Line.Iterator.Color, RS.Line.Length);
			}

			if (RC.Enable.Texturing2D)
			{
				RS.Line.Iterator.Numerator.S   = (RS.Line.Right.Numerator.S   - RS.Line.Left.Numerator.S)   / RS.Line.Length;
				RS.Line.Iterator.Denominator.S = (RS.Line.Right.Denominator.S - RS.Line.Left.Denominator.S) / RS.Line.Length;
				
				RS.Line.Iterator.Numerator.T   = (RS.Line.Right.Numerator.T   - RS.Line.Left.Numerator.T)   / RS.Line.Length;
				RS.Line.Iterator.Denominator.T = (RS.Line.Right.Denominator.T - RS.Line.Left.Denominator.T) / RS.Line.Length;

				if (RC.Enable.Fog) { RS.Line.Iterator.Fog = (RS.Line.Right.Fog - RS.Line.Left.Fog) / RS.Line.Length; }
			}
		}
		else 
		{
			RS.Line.Iterator.Z = 0; 

			if (RC.Enable.SmoothShading) { ZeroVector4(RS.Line.Iterator.Color); }

			if (RC.Enable.Texturing2D)
			{
				RS.Line.Iterator.Numerator.S   = 0;
				RS.Line.Iterator.Denominator.S = 0;
				
				RS.Line.Iterator.Numerator.T   = 0;
				RS.Line.Iterator.Denominator.T = 0;

				if (RC.Enable.Fog) { RS.Line.Iterator.Fog = 0; }
			}
		}

		//Rasterize line
		if (RC.Enable.Texturing2D)
		{
			UINT Texel = 0;

			for (X_LeftClamped; X_LeftClamped <= X_RightClamped; X_LeftClamped++)
			{
				//Calculate current texture coordinates
				RS.Line.Current.S = RS.Line.Current.Numerator.S / RS.Line.Current.Denominator.S;
				RS.Line.Current.T = RS.Line.Current.Numerator.T / RS.Line.Current.Denominator.T;

				//Implicitly set texture wrap mode to repeat
				if (RS.Line.Current.S > 1) { RS.Line.Current.S -= (Int)RS.Line.Current.S; }
				if (RS.Line.Current.S < 0) { RS.Line.Current.S -= (Int)RS.Line.Current.S - 1; }
				if (RS.Line.Current.T > 1) { RS.Line.Current.T -= (Int)RS.Line.Current.T; }
				if (RS.Line.Current.T < 0) { RS.Line.Current.T -= (Int)RS.Line.Current.T - 1; }

				//Use linear filtering 
				if (true)
				{
					float uMinusHalf = TextureWidth * RS.Line.Current.S - 0.5f;
					float vMinusHalf = TextureHeight * RS.Line.Current.T - 0.5f;

					if (uMinusHalf < 0) { uMinusHalf = 0; }
					if (vMinusHalf < 0) { vMinusHalf = 0; }

					Int i0 = (Int)uMinusHalf;
					Int j0 = (Int)vMinusHalf;
					Int i1 = i0 + 1;
					Int j1 = j0 + 1;

					if (i0 == TextureWidth)  { i0--; }
					if (j0 == TextureHeight) { j0--; }
					if (i1 == TextureWidth)  { i1--; }
					if (j1 == TextureHeight) { j1--; }

					//Fetch 4 texels
					UINT Texel00 = *(UINT*)(&pTexture[(j0 * TextureWidth + i0) * RS.Texture.Components]);
					UINT Texel01 = *(UINT*)(&pTexture[(j0 * TextureWidth + i1) * RS.Texture.Components]);
					UINT Texel10 = *(UINT*)(&pTexture[(j1 * TextureWidth + i0) * RS.Texture.Components]);
					UINT Texel11 = *(UINT*)(&pTexture[(j1 * TextureWidth + i1) * RS.Texture.Components]);

					float alpha = uMinusHalf - (Int)uMinusHalf;
					float beta  = vMinusHalf - (Int)vMinusHalf;

					RS.Pixel.Color[0] = RS.Line.Current.Color[0] * ((1 - alpha) * (1 - beta) * GET_R_NOMALIZED(Texel00) + alpha * (1 - beta) * GET_R_NOMALIZED(Texel01) + (1 - alpha) * beta * GET_R_NOMALIZED(Texel10) + alpha * beta * GET_R_NOMALIZED(Texel11));
					RS.Pixel.Color[1] = RS.Line.Current.Color[1] * ((1 - alpha) * (1 - beta) * GET_G_NOMALIZED(Texel00) + alpha * (1 - beta) * GET_G_NOMALIZED(Texel01) + (1 - alpha) * beta * GET_G_NOMALIZED(Texel10) + alpha * beta * GET_G_NOMALIZED(Texel11));
					RS.Pixel.Color[2] = RS.Line.Current.Color[2] * ((1 - alpha) * (1 - beta) * GET_B_NOMALIZED(Texel00) + alpha * (1 - beta) * GET_B_NOMALIZED(Texel01) + (1 - alpha) * beta * GET_B_NOMALIZED(Texel10) + alpha * beta * GET_B_NOMALIZED(Texel11));
					RS.Pixel.Color[3] = RS.Line.Current.Color[3];
				}
				else
				{
					//Fetch texel
					Texel = *(UINT*)(&pTexture[((Int)(TextureHeight * RS.Line.Current.T) * TextureWidth + (Int)(TextureWidth * RS.Line.Current.S)) * RS.Texture.Components]);

					RS.Pixel.Color[0] = RS.Line.Current.Color[0] * ((BYTE)(Texel)       * 0.00392156862745098f);
					RS.Pixel.Color[1] = RS.Line.Current.Color[1] * ((BYTE)(Texel >> 8)  * 0.00392156862745098f);
					RS.Pixel.Color[2] = RS.Line.Current.Color[2] * ((BYTE)(Texel >> 16) * 0.00392156862745098f);
					RS.Pixel.Color[3] = RS.Line.Current.Color[3];
				}

				//Implicitly use SGL_MODULATE as texture environment mode
	/*			RS.Pixel.Color[0] = RS.Line.Current.Color[0] * (pTexture[TexelIndex]		* 0.00392156862745098f);
				RS.Pixel.Color[1] = RS.Line.Current.Color[1] * (pTexture[TexelIndex + 1]	* 0.00392156862745098f);
				RS.Pixel.Color[2] = RS.Line.Current.Color[2] * (pTexture[TexelIndex + 2]	* 0.00392156862745098f);
				RS.Pixel.Color[3] = RS.Line.Current.Color[3];
	*/
				//Apply fog
				if (RC.Enable.Fog)
				{
					RS.Pixel.Color[0] = RS.Line.Current.Fog * RS.Pixel.Color[0] + (1 - RS.Line.Current.Fog) * RS.Fog.Color[0];
					RS.Pixel.Color[1] = RS.Line.Current.Fog * RS.Pixel.Color[1] + (1 - RS.Line.Current.Fog) * RS.Fog.Color[1];
					RS.Pixel.Color[2] = RS.Line.Current.Fog * RS.Pixel.Color[2] + (1 - RS.Line.Current.Fog) * RS.Fog.Color[2];
				}

				RS.Pixel.Write.Color  = (UByte)(255 * RS.Pixel.Color[3]); RS.Pixel.Write.Color <<= 8;
				RS.Pixel.Write.Color += (UByte)(255 * RS.Pixel.Color[0]); RS.Pixel.Write.Color <<= 8;
				RS.Pixel.Write.Color += (UByte)(255 * RS.Pixel.Color[1]); RS.Pixel.Write.Color <<= 8;
				RS.Pixel.Write.Color += (UByte)(255 * RS.Pixel.Color[2]);

				//Perform depth test & write to buffers
				if (RS.Line.Current.Z < Buffer.Depth[RS.Pixel.Index])
				{
					Buffer.Back [RS.Pixel.Index] = RS.Pixel.Write.Color; 
					Buffer.Depth[RS.Pixel.Index] = RS.Line.Current.Z; 
				}
				RS.Pixel.Index++;

				//Update vertex color 
				if (RC.Enable.SmoothShading)
				{ 
					AddVectors4(RS.Line.Current.Color, RS.Line.Current.Color, RS.Line.Iterator.Color);	

					//Update fog state
					if (RC.Enable.Fog) { RS.Line.Current.Fog += RS.Line.Iterator.Fog; }
				}

				//Update z buffer state
				RS.Line.Current.Z += RS.Line.Iterator.Z;

				//Update texture state
				RS.Line.Current.Numerator.S   += RS.Line.Iterator.Numerator.S;
				RS.Line.Current.Denominator.S += RS.Line.Iterator.Denominator.S;

				RS.Line.Current.Numerator.T   += RS.Line.Iterator.Numerator.T;
				RS.Line.Current.Denominator.T += RS.Line.Iterator.Denominator.T;
			}
		}
		else
		{
			if (RC.Enable.SmoothShading)
			{
				for (X_LeftClamped; X_LeftClamped <= (Int)X_RightClamped; X_LeftClamped++)
				{
					RS.Pixel.Write.Color  = (UByte)(255 * RS.Line.Current.Color[3]); RS.Pixel.Write.Color <<= 8;
					RS.Pixel.Write.Color += (UByte)(255 * RS.Line.Current.Color[0]); RS.Pixel.Write.Color <<= 8;
					RS.Pixel.Write.Color += (UByte)(255 * RS.Line.Current.Color[1]); RS.Pixel.Write.Color <<= 8;
					RS.Pixel.Write.Color += (UByte)(255 * RS.Line.Current.Color[2]);

					//Scan one line
					if (RS.Line.Current.Z < Buffer.Depth[RS.Pixel.Index]) { Buffer.Back[RS.Pixel.Index] = RS.Pixel.Write.Color; Buffer.Depth[RS.Pixel.Index] = RS.Line.Current.Z; }
					RS.Pixel.Index++;

					//Update state
					AddVectors4(RS.Line.Current.Color, RS.Line.Current.Color, RS.Line.Iterator.Color);	
					RS.Line.Current.Z += RS.Line.Iterator.Z;
				}
			}
			else
			{
				for (X_LeftClamped; X_LeftClamped <= (Int)X_RightClamped; X_LeftClamped++)
				{
					//Scan one line & update state
					if (RS.Line.Current.Z < Buffer.Depth[RS.Pixel.Index]) { Buffer.Back[RS.Pixel.Index] = RS.Pixel.Write.Color; Buffer.Depth[RS.Pixel.Index] = RS.Line.Current.Z; }
					RS.Pixel.Index++; RS.Line.Current.Z += RS.Line.Iterator.Z;
				}
			}
		}
	}
}