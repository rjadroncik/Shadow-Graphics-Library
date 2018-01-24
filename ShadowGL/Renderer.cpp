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

	Float TriangleArea = 0;

	UByte *pTexture = nullptr;

	SizeI TextureWidth  = 0;
	SizeI TextureHeight = 0;

    //Triangle area multipleid by two (simply by removing the division by 2 from the regular formula)
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
	if (!RC_OK) { MessageBox(nullptr, TEXT("No Current Rendering Context!"), TEXT("Begin()"), MB_OK | MB_ICONERROR); return; } 
	if (RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

	RC.Primitive.Type = (UByte)primType;

	if (primType == SGL_TRIANGLES)
	{
		RC.Primitive.VerticesSubmitted = 0;
		RC.Primitive.Building = true;
		return;
	}
}

void SHADOWGL_API ShadowGL::End()
{
	if (!RC_OK) { MessageBox(nullptr, TEXT("No Current Rendering Context!"), TEXT("End()"), MB_OK | MB_ICONERROR); return; } 
	if (!RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }
	
	RC.Primitive.VerticesSubmitted = 0;
	RC.Primitive.Building = false;
}

void SHADOWGL_API ShadowGL::Vertex3f(Float x, Float y, Float z)
{
	if (!RC_OK) { MessageBox(nullptr, TEXT("No Current Rendering Context!"), TEXT("Vertex3f()"), MB_OK | MB_ICONERROR); return; } 
	if (!RC.Primitive.Building) { RC.ErrorCode = SGL_INVALID_OPERATION; return; }

	if ((RC.View.Size[0] <= 0) || (RC.View.Size[1] <= 0)) { return; }

    SetVector4(RC.Primitive.Vertex[RC.Primitive.VerticesSubmitted].ObjCoord, x, y, z, 1);
    CopyVector4(RC.Primitive.Vertex[RC.Primitive.VerticesSubmitted].ObjColor, RC.Vertex.Color);
    CopyVector4(RC.Primitive.Vertex[RC.Primitive.VerticesSubmitted].ObjTexCoord, RC.Vertex.TexCoord);
    CopyVector3(RC.Primitive.Vertex[RC.Primitive.VerticesSubmitted].ObjNormal, RC.Vertex.Normal);
	RC.Primitive.VerticesSubmitted++;

    switch (RC.Primitive.Type)
    {
        case SGL_POINTS:
        {
            return;
        }
        case SGL_LINES:
        {
            return;
        }
        case SGL_TRIANGLES:
        {
            if (RC.Primitive.VerticesSubmitted == ShadowGLPrivate::TRIANLGE_VERTICES)
            {
                if (CullTriangle()) { RC.Primitive.VerticesSubmitted = 0; return; }

                PrepareLighting(&RC.Primitive.Vertex[0], ShadowGLPrivate::TRIANLGE_VERTICES);

                if (ClipTriangle())
                {
                    if (RC.Primitive.ClipCount)
                    {
                        PreparePolygon(&RC.Primitive.ClipVertex[RC.Primitive.ClipArray][0], RC.Primitive.ClipCount);
                        RasterizePolygon(&RC.Primitive.ClipVertex[RC.Primitive.ClipArray][0], RC.Primitive.ClipCount);
                    }
                }
                else
                {
                    PreparePolygon(&RC.Primitive.Vertex[0], ShadowGLPrivate::TRIANLGE_VERTICES);
                    RasterizePolygon(&RC.Primitive.Vertex[0], ShadowGLPrivate::TRIANLGE_VERTICES);
                }

                RC.Primitive.VerticesSubmitted = 0;
            }
            return;
        }
    }   
}

SHADOWGL_API void ShadowGL::Color3f(Float r, Float g, Float b)
{
	if (!RC_OK) { MessageBox(nullptr, TEXT("No Current Rendering Context!"), TEXT("Color3f()"), MB_OK | MB_ICONERROR); return; } 

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
	if (!RC_OK) { MessageBox(nullptr, TEXT("No Current Rendering Context!"), TEXT("Color4f()"), MB_OK | MB_ICONERROR); return; } 

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
	if (!RC_OK) { MessageBox(nullptr, TEXT("No Current Rendering Context!"), TEXT("Color4f()"), MB_OK | MB_ICONERROR); return; } 

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
	if (!RC_OK) { MessageBox(nullptr, TEXT("No Current Rendering Context!"), TEXT("Color4f()"), MB_OK | MB_ICONERROR); return; } 

	RC.Vertex.Normal[0] = x;	
	RC.Vertex.Normal[1] = y;	
	RC.Vertex.Normal[2] = z;	
}

SHADOWGL_API void ShadowGL::TexCoord2f(Float x, Float y)
{
	if (!RC_OK) { MessageBox(nullptr, TEXT("No Current Rendering Context!"), TEXT("Color4f()"), MB_OK | MB_ICONERROR); return; } 

	RC.Vertex.TexCoord[0] = x;	
	RC.Vertex.TexCoord[1] = 1 - y;	
	RC.Vertex.TexCoord[2] = 0;	
	RC.Vertex.TexCoord[3] = 0;	
}

namespace ShadowGLPrivate
{
	bool CullTriangle()
	{
		//Vertex Processing
		for (UByte i = 0; i < ShadowGLPrivate::TRIANLGE_VERTICES; i++)
		{
		    //Create tVertex Eye Coordinates
			MultiplyMatrix4Vector4(RC.Primitive.Vertex[i].EyeCoord, RC.Matrix.ModelView[RC.Matrix.MVCurrent], RC.Primitive.Vertex[i].ObjCoord);
		}

		//Face Culling	 
		if (RC.Enable.FaceCulling)
		{
			//Find Normal
            Float3 normal;
			if (RC.FrontFace == SGL_CCW)	{ MakeNormal(normal, (Float3&)RC.Primitive.Vertex[0].EyeCoord, (Float3&)RC.Primitive.Vertex[1].EyeCoord, (Float3&)RC.Primitive.Vertex[2].EyeCoord); }
			else							{ MakeNormal(normal, (Float3&)RC.Primitive.Vertex[2].EyeCoord, (Float3&)RC.Primitive.Vertex[1].EyeCoord, (Float3&)RC.Primitive.Vertex[0].EyeCoord); }

			//Find Triangle Center to Eye Direction Vector (Assuming Eye at (0, 0, 0)) 
            Float3 direction;
			direction[0] = (RC.Primitive.Vertex[0].EyeCoord[0] + RC.Primitive.Vertex[1].EyeCoord[0] + RC.Primitive.Vertex[2].EyeCoord[0]) / 3.0f;
			direction[1] = (RC.Primitive.Vertex[0].EyeCoord[1] + RC.Primitive.Vertex[1].EyeCoord[1] + RC.Primitive.Vertex[2].EyeCoord[1]) / 3.0f;
			direction[2] = (RC.Primitive.Vertex[0].EyeCoord[2] + RC.Primitive.Vertex[1].EyeCoord[2] + RC.Primitive.Vertex[2].EyeCoord[2]) / 3.0f;
			NormalizeVector3(direction, direction);

			float AngleCose = MultiplyVectors3(normal, direction);

			//Determine Whether to Continue or Not
			if (RC.CullStyle == SGL_BACK) { if (AngleCose > 0) { return true; } }
			else { if (AngleCose < 0) { return true; } }
		}

		return false;
	}

	bool ClipTriangle()
	{
		Boolean	needsClipping = false;

		for (UByte i = 0; i < ShadowGLPrivate::TRIANLGE_VERTICES; i++)
		{
			RC.Primitive.Vertex[i].InClipVolume	= true;
			RC.Primitive.Vertex[i].Finished		= false;

			for (UByte j = 0; j < 6; j++)
			{
				if (RC.Primitive.Vertex[i].InClipVolume &&
                    ((RC.Primitive.Vertex[i].EyeCoord[0] * RC.View.ClipPlane[j][0] + RC.Primitive.Vertex[i].EyeCoord[1] * RC.View.ClipPlane[j][1] + RC.Primitive.Vertex[i].EyeCoord[2] * RC.View.ClipPlane[j][2] + RC.View.ClipPlane[j][3]) < 0)) 
				{ 
					RC.Primitive.Vertex[i].InClipVolume = false;
					needsClipping = true;
					continue; 
				}
			}
		}

		if (!needsClipping) { return false; }

        //Clipping can result in adding vertices
		RC.Primitive.ClipCount = ClipPolygonAtPlane(&RC.Primitive.Vertex[0], ShadowGLPrivate::TRIANLGE_VERTICES, &RC.View.ClipPlane[5][0], &RC.Primitive.ClipVertex[0][0]);

		RC.Primitive.ClipCount = ClipPolygonAtPlane(&RC.Primitive.ClipVertex[0][0], RC.Primitive.ClipCount, &RC.View.ClipPlane[4][0], &RC.Primitive.ClipVertex[1][0]);
		RC.Primitive.ClipCount = ClipPolygonAtPlane(&RC.Primitive.ClipVertex[1][0], RC.Primitive.ClipCount, &RC.View.ClipPlane[3][0], &RC.Primitive.ClipVertex[0][0]);
		RC.Primitive.ClipCount = ClipPolygonAtPlane(&RC.Primitive.ClipVertex[0][0], RC.Primitive.ClipCount, &RC.View.ClipPlane[2][0], &RC.Primitive.ClipVertex[1][0]);
		RC.Primitive.ClipCount = ClipPolygonAtPlane(&RC.Primitive.ClipVertex[1][0], RC.Primitive.ClipCount, &RC.View.ClipPlane[1][0], &RC.Primitive.ClipVertex[0][0]);
		RC.Primitive.ClipCount = ClipPolygonAtPlane(&RC.Primitive.ClipVertex[0][0], RC.Primitive.ClipCount, &RC.View.ClipPlane[0][0], &RC.Primitive.ClipVertex[1][0]);

		RC.Primitive.ClipArray = 1;

        return true;
	}

	UByte ClipPolygonAtPlane(tVertex* inputVertex, _IN UByte count, const Float* clipPlane, tVertex* outputVertex) 
	{
		UByte	v1, v2;
		UByte	VertexIndex = 0;
		Float	t = 0;

		for (UByte i = 0; i < count; i++)
		{
			inputVertex[i].InClipVolume = false;
			inputVertex[i].Finished     = false;

			if ((inputVertex[i].EyeCoord[0] * clipPlane[0] + inputVertex[i].EyeCoord[1] * clipPlane[1] + inputVertex[i].EyeCoord[2] * clipPlane[2] + clipPlane[3]) >= 0)
			{
				inputVertex[i].InClipVolume = true;
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

                //Since we already applied lighting we only use the lit color onwards
                //We could uncomment and calculate this if we wanted to use in on the per-pixel level later on
                //outputVertex[VertexIndex].ObjColor[0] = inputVertex[v1].ObjColor[0] * Distance01 + inputVertex[v2].ObjColor[0] * t;
                //outputVertex[VertexIndex].ObjColor[1] = inputVertex[v1].ObjColor[1] * Distance01 + inputVertex[v2].ObjColor[1] * t;
                //outputVertex[VertexIndex].ObjColor[2] = inputVertex[v1].ObjColor[2] * Distance01 + inputVertex[v2].ObjColor[2] * t;
                //outputVertex[VertexIndex].ObjColor[3] = inputVertex[v1].ObjColor[3] * Distance01 + inputVertex[v2].ObjColor[3] * t;

                //outputVertex[VertexIndex].ObjNormal[0] = inputVertex[v1].ObjNormal[0] * Distance01 + inputVertex[v2].ObjNormal[0] * t;
                //outputVertex[VertexIndex].ObjNormal[1] = inputVertex[v1].ObjNormal[1] * Distance01 + inputVertex[v2].ObjNormal[1] * t;
                //outputVertex[VertexIndex].ObjNormal[2] = inputVertex[v1].ObjNormal[2] * Distance01 + inputVertex[v2].ObjNormal[2] * t;

                outputVertex[VertexIndex].LitColor[0] = inputVertex[v1].LitColor[0] * Distance01 + inputVertex[v2].LitColor[0] * t;
                outputVertex[VertexIndex].LitColor[1] = inputVertex[v1].LitColor[1] * Distance01 + inputVertex[v2].LitColor[1] * t;
                outputVertex[VertexIndex].LitColor[2] = inputVertex[v1].LitColor[2] * Distance01 + inputVertex[v2].LitColor[2] * t;

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
				inputVertex[v1].Finished = true; inputVertex[v2].Finished = true;
			}

			//If Both Vertices Lie on The Same Side of The Plane & Are In Clip Volume
			if (inputVertex[v1].InClipVolume && inputVertex[v2].InClipVolume)
			{
				//Store The Edge as It is Completely In the Current Clipping Volume
				if (inputVertex[v1].InClipVolume && !inputVertex[v1].Finished) { CopyVertex(&outputVertex[VertexIndex], &inputVertex[v1]); inputVertex[v1].Finished = true; VertexIndex++; }
				if (inputVertex[v2].InClipVolume && !inputVertex[v2].Finished) { CopyVertex(&outputVertex[VertexIndex], &inputVertex[v2]); inputVertex[v2].Finished = true; VertexIndex++; }
			}
		}

		return VertexIndex;
	}

	void PrepareLighting(tVertex *vertex, UByte count)
	{
        Matrix3 inverseModelView;
		TruncateTopLeftToMatrix3(inverseModelView, RC.Matrix.ModelViewInverse[RC.Matrix.MVCurrent]); 

		//Vertex processing continued...
		for (UByte i = 0; i < count; i++)
		{
			//Process vertex normals (to eye coordinates) 
			MultiplyVectorT3Matrix3(vertex[i].EyeNormal, vertex[i].ObjNormal, inverseModelView);

			//Defaulting to SGL_NORMALIZE enabled
			NormalizeVector3(vertex[i].EyeNormal, vertex[i].EyeNormal);
		}

		//Enable/disable smooth shading (when not using smooth shading we need only one lit color
        //Ideally it would be calculated at the middle of the shape, but for performance we just use the first vertex
		Float Attenuation;
		Float SpotFactor;

		//Lighting calculations
		for (UInt i = 0; i < count; i++)
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
                    Float3 lightToVertex;
					MakeVector3(lightToVertex, (Float3&)RC.Light[j].EyePosition, (Float3&)vertex[i].EyeCoord);
					float LightToVertexDistance = MeasureVector3(lightToVertex);

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
						NormalizeVector3(lightToVertex, lightToVertex);
						float SpotAngleCose = MultiplyVectors3(lightToVertex, RC.Light[j].EyeDirection);

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
                    Float3 vertexToLightDirection;
					MakeVector3(vertexToLightDirection, (Float3&)vertex[i].EyeCoord, (Float3&)RC.Light[j].EyePosition);
					NormalizeVector3(vertexToLightDirection, vertexToLightDirection);

					//Max of dot product of: Vertex to light vector & normal 
					float DiffuseFactor = __max(0, MultiplyVectors3(vertex[i].EyeNormal, vertexToLightDirection));
			
					//Specular contribution
					if (DiffuseFactor != 0)
                    {
                        //Compute & add diffuse contribution
                        Float3 diffuse;
					    ScaleVector3(diffuse, (Float3&)RC.Material.Diffuse, DiffuseFactor);
					    MADVectors3CW(CurLight, diffuse, (Float3&)RC.Light[j].Diffuse);

						if (RC.Enable.LocalViewer)
						{
							NormalizeVector3(diffuse, (Float3&)vertex[i].EyeCoord);
							SubtractVectors3(vertexToLightDirection, vertexToLightDirection, diffuse);
						}
						else { vertexToLightDirection[2] += 1; }

						NormalizeVector3(vertexToLightDirection, vertexToLightDirection);

						//Max of dot product of: normal & specular half vector
						float SpecularFactor = __max(0, MultiplyVectors3(vertex[i].EyeNormal, vertexToLightDirection));

						//Raise specular factor to the power of material shininess
						if (RC.Material.Shininess == 0) { SpecularFactor = 1; }
						else                            { SpecularFactor = (Float)pow(SpecularFactor, RC.Material.Shininess); }
						
						//Add Specular Contribution
                        Float3 scalar;
						ScaleVector3(scalar, (Float3&)RC.Material.Specular, SpecularFactor);
						MADVectors3CW(CurLight, scalar, (Float3&)RC.Light[j].Specular);
					}

					ScaleVector3(CurLight, CurLight, Attenuation);
					ScaleVector3(CurLight, CurLight, SpotFactor);

					AddVectors3((Float3&)vertex[i].LitColor, (Float3&)vertex[i].LitColor, CurLight);
				}

				vertex[i].LitColor[3] = RC.Material.Diffuse[3];
			}
			else { CopyVector4(vertex[i].LitColor, vertex[i].ObjColor); }

            //Clamping lit colors
            ClampVector4(vertex[i].LitColor, 0, 1);
		}
	}

	void PreparePolygon(tVertex *vertex, UByte count)
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

        if (RC.Enable.Fog)
        {
		    //Lighting calculations
		    for (UInt i = 0; i < count; i++)
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
                
                //Clamping lit colors
                ClampVector4(vertex[i].LitColor, 0, 1);
			}
		}
	}

	void RasterizePolygon(tVertex *vertex, UByte count)
	{
		for (UByte i = 0; i < (count - 2); i++)
		{
			//Select Vertices
			RS.V1 = 0;
			RS.V2 = i + 1;
			RS.V3 = i + 2;
			
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

	void RasterizeTopPartOfTriangleWithMiddleVertexRight(tVertex *vertex1, tVertex *vertex2, tVertex *vertex3)
	{
		for (RS.Line[0].Index = RS.Vertex[0].Y; RS.Line[0].Index <= RS.Vertex[1].Y; RS.Line[0].Index++)
		{
			if (vertex1->ViewCoord[1] != vertex2->ViewCoord[1])
			{
				RS.Line[0].Left.X  = - Coefficient_13 * ((Float)RS.Line[0].Index + 0.5f) - Offset_13;
				RS.Line[0].Right.X = - Coefficient_12 * ((Float)RS.Line[0].Index + 0.5f) - Offset_12;
			}
			else { RS.Line[0].Left.X = vertex1->ViewCoord[0]; RS.Line[0].Right.X = vertex2->ViewCoord[0]; }

			ScanLine(vertex1, vertex2, vertex3, RS.Line[0], RS.Pixel[0]);
		}
	}

	void RasterizeBottomPartOfTriangleWithMiddleVertexRight(tVertex *vertex1, tVertex *vertex2, tVertex *vertex3)
	{    
		for (RS.Line[1].Index = RS.Vertex[1].Y + 1; RS.Line[1].Index <= RS.Vertex[2].Y; RS.Line[1].Index++)
		{
			//Compute left & right coordinate values
			if (vertex2->ViewCoord[1] != vertex3->ViewCoord[1])
			{
				RS.Line[1].Left.X  = - Coefficient_13 * ((Float)RS.Line[1].Index + 0.5f) - Offset_13;
				RS.Line[1].Right.X = - Coefficient_23 * ((Float)RS.Line[1].Index + 0.5f) - Offset_23;
			}
			else {	RS.Line[1].Left.X = vertex2->ViewCoord[0]; RS.Line[1].Right.X = vertex3->ViewCoord[0]; }

			ScanLine(vertex1, vertex2, vertex3, RS.Line[1], RS.Pixel[1]);
		}
	}

    void RasterizeTopPartOfTriangleWithMiddleVertexLeft(tVertex *vertex1, tVertex *vertex2, tVertex *vertex3)
	{
		for (RS.Line[0].Index = RS.Vertex[0].Y; RS.Line[0].Index <= RS.Vertex[1].Y; RS.Line[0].Index++)
		{
			if (vertex1->ViewCoord[1] != vertex2->ViewCoord[1])
			{
				RS.Line[0].Right.X = - Coefficient_13 * ((Float)RS.Line[0].Index + 0.5f) - Offset_13;
				RS.Line[0].Left.X  = - Coefficient_12 * ((Float)RS.Line[0].Index + 0.5f) - Offset_12;
			}
			else { RS.Line[0].Right.X = vertex1->ViewCoord[0]; RS.Line[0].Left.X = vertex2->ViewCoord[0]; }

			ScanLine(vertex1, vertex2, vertex3, RS.Line[0], RS.Pixel[0]);
		}
    }

    void RasterizeBottomPartOfTriangleWithMiddleVertexLeft(tVertex *vertex1, tVertex *vertex2, tVertex *vertex3)
    {
        for (RS.Line[1].Index = RS.Vertex[1].Y + 1; RS.Line[1].Index <= RS.Vertex[2].Y; RS.Line[1].Index++)
		{
			if (vertex2->ViewCoord[1] != vertex3->ViewCoord[1])
			{
				RS.Line[1].Right.X = - Coefficient_13 * ((Float)RS.Line[1].Index + 0.5f) - Offset_13;
				RS.Line[1].Left.X  = - Coefficient_23 * ((Float)RS.Line[1].Index + 0.5f) - Offset_23;
			}
			else { RS.Line[1].Right.X = vertex2->ViewCoord[0]; RS.Line[1].Left.X = vertex3->ViewCoord[0]; }

			ScanLine(vertex1, vertex2, vertex3, RS.Line[1], RS.Pixel[1]);
		}
    }

	void RasterizeTriangle(tVertex *vertex1, tVertex *vertex2, tVertex *vertex3)
	{
		//Texturing variables
		RS.Vertex[0].Clip.W = vertex1->ClipCoord[3]; 
		RS.Vertex[1].Clip.W	= vertex2->ClipCoord[3];
		RS.Vertex[2].Clip.W = vertex3->ClipCoord[3];

		//Prepare other values
		CopyVector4(RS.Fog.Color, RC.Fog.Color);

		if (RC.Enable.Texturing2D)
		{
			pTexture              = Texture[RC.TexCurrent2D].pData;
			TextureWidth          = Texture[RC.TexCurrent2D].Width;
			TextureHeight         = Texture[RC.TexCurrent2D].Height;
			RS.Texture.Components = Texture[RC.TexCurrent2D].Components;
		}

		TriangleArea = TriangleArea2x2((Float2&)vertex1->ViewCoord, (Float2&)vertex2->ViewCoord, (Float2&)vertex3->ViewCoord);

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

		if (vertex2->ViewCoord[0] >= (- Coefficient_13 * vertex2->ViewCoord[1] - Offset_13)) 
		{
            RasterizeTopPartOfTriangleWithMiddleVertexRight(vertex1, vertex2, vertex3);
            RasterizeBottomPartOfTriangleWithMiddleVertexRight(vertex1, vertex2, vertex3);
		}
		else
		{
            RasterizeTopPartOfTriangleWithMiddleVertexLeft(vertex1, vertex2, vertex3);
            RasterizeBottomPartOfTriangleWithMiddleVertexLeft(vertex1, vertex2, vertex3);
		}
	}

	void ScanLine(tVertex *vertex1, tVertex *vertex2, tVertex *vertex3, SRendererState::SLineState &line, SRendererState::SCurPixelState &pixel)
	{
        //Prepare line length
        line.Length = (Float)(line.Right.X - line.Left.X);
        if (line.Length == 0) { return; }

        //Apply left-top rasterization rules to vertex values
		Int X_LeftClamped  = (line.Left.X  > ((Int)line.Left.X  + 0.5f)) ? (Int)line.Left.X  + 1 : (Int)line.Left.X; 
		Int X_RightClamped = (line.Right.X < ((Int)line.Right.X + 0.5f)) ? (Int)line.Right.X - 1 : (Int)line.Right.X;

		Float2 positionLeft;
		Float2 positionRight;

		//Compute barycentric coords
		SetVector2(positionLeft, (Float)X_LeftClamped  + 0.5f, (Float)line.Index + 0.5f);
		SetVector2(positionRight, (Float)X_RightClamped + 0.5f, (Float)line.Index + 0.5f);

        Float3 B_LeftCoord;
        Float3 B_RightCoord;

        B_LeftCoord[0] = TriangleArea2x2(positionLeft, (Float2&)vertex2->ViewCoord, (Float2&)vertex3->ViewCoord) / TriangleArea;
		B_LeftCoord[1] = TriangleArea2x2(positionLeft, (Float2&)vertex1->ViewCoord, (Float2&)vertex3->ViewCoord) / TriangleArea;
		B_LeftCoord[2] = TriangleArea2x2(positionLeft, (Float2&)vertex1->ViewCoord, (Float2&)vertex2->ViewCoord) / TriangleArea;

		B_RightCoord[0] = TriangleArea2x2(positionRight, (Float2&)vertex2->ViewCoord, (Float2&)vertex3->ViewCoord) / TriangleArea;
		B_RightCoord[1] = TriangleArea2x2(positionRight, (Float2&)vertex1->ViewCoord, (Float2&)vertex3->ViewCoord) / TriangleArea;
		B_RightCoord[2] = TriangleArea2x2(positionRight, (Float2&)vertex1->ViewCoord, (Float2&)vertex2->ViewCoord) / TriangleArea;

		//Prepare z-buffer values
		line.Left.Z    = B_LeftCoord[0]  * vertex1->ViewCoord[2] + B_LeftCoord[1]  * vertex2->ViewCoord[2] + B_LeftCoord[2]  * vertex3->ViewCoord[2];
		line.Right.Z   = B_RightCoord[0] * vertex1->ViewCoord[2] + B_RightCoord[1] * vertex2->ViewCoord[2] + B_RightCoord[2] * vertex3->ViewCoord[2];
		line.Current.Z = line.Left.Z;
        line.Iterator.Z = (line.Right.Z - line.Left.Z) / line.Length;

		//Prepare buffer index
		pixel.Index = line.Index * RC.View.Size[0] + X_LeftClamped;

		//Prepare color
		line.Left.Color[0] = B_LeftCoord[0] * vertex1->LitColor[0] + B_LeftCoord[1] * vertex2->LitColor[0] + B_LeftCoord[2] * vertex3->LitColor[0];
		line.Left.Color[1] = B_LeftCoord[0] * vertex1->LitColor[1] + B_LeftCoord[1] * vertex2->LitColor[1] + B_LeftCoord[2] * vertex3->LitColor[1];
		line.Left.Color[2] = B_LeftCoord[0] * vertex1->LitColor[2] + B_LeftCoord[1] * vertex2->LitColor[2] + B_LeftCoord[2] * vertex3->LitColor[2];
		line.Left.Color[3] = B_LeftCoord[0] * vertex1->LitColor[3] + B_LeftCoord[1] * vertex2->LitColor[3] + B_LeftCoord[2] * vertex3->LitColor[3];

		line.Right.Color[0] = B_RightCoord[0] * vertex1->LitColor[0] + B_RightCoord[1] * vertex2->LitColor[0] + B_RightCoord[2] * vertex3->LitColor[0];
		line.Right.Color[1] = B_RightCoord[0] * vertex1->LitColor[1] + B_RightCoord[1] * vertex2->LitColor[1] + B_RightCoord[2] * vertex3->LitColor[1];
		line.Right.Color[2] = B_RightCoord[0] * vertex1->LitColor[2] + B_RightCoord[1] * vertex2->LitColor[2] + B_RightCoord[2] * vertex3->LitColor[2];
		line.Right.Color[3] = B_RightCoord[0] * vertex1->LitColor[3] + B_RightCoord[1] * vertex2->LitColor[3] + B_RightCoord[2] * vertex3->LitColor[3];
		
		CopyVector4(line.Current.Color, line.Left.Color);
        SubtractVectors4(line.Iterator.Color, line.Right.Color, line.Left.Color);
        DivideVector4(line.Iterator.Color, line.Iterator.Color, line.Length);

		//Prepare texture coords
		if (RC.Enable.Texturing2D)
		{
			line.Left.Numerator.S  = B_LeftCoord[0]  * vertex1->FinTexCoord[0] / RS.Vertex[0].Clip.W + B_LeftCoord[1]  * vertex2->FinTexCoord[0] / RS.Vertex[1].Clip.W + B_LeftCoord[2]  * vertex3->FinTexCoord[0] / RS.Vertex[2].Clip.W;
			line.Right.Numerator.S = B_RightCoord[0] * vertex1->FinTexCoord[0] / RS.Vertex[0].Clip.W + B_RightCoord[1] * vertex2->FinTexCoord[0] / RS.Vertex[1].Clip.W + B_RightCoord[2] * vertex3->FinTexCoord[0] / RS.Vertex[2].Clip.W;

			line.Left.Numerator.T  = B_LeftCoord[0]  * vertex1->FinTexCoord[1] / RS.Vertex[0].Clip.W + B_LeftCoord[1]  * vertex2->FinTexCoord[1] / RS.Vertex[1].Clip.W + B_LeftCoord[2]  * vertex3->FinTexCoord[1] / RS.Vertex[2].Clip.W;
			line.Right.Numerator.T = B_RightCoord[0] * vertex1->FinTexCoord[1] / RS.Vertex[0].Clip.W + B_RightCoord[1] * vertex2->FinTexCoord[1] / RS.Vertex[1].Clip.W + B_RightCoord[2] * vertex3->FinTexCoord[1] / RS.Vertex[2].Clip.W;

			line.Left.Denominator.S  = B_LeftCoord[0]  / RS.Vertex[0].Clip.W + B_LeftCoord[1]  / RS.Vertex[1].Clip.W + B_LeftCoord[2]  / RS.Vertex[2].Clip.W;
			line.Right.Denominator.S = B_RightCoord[0] / RS.Vertex[0].Clip.W + B_RightCoord[1] / RS.Vertex[1].Clip.W + B_RightCoord[2] / RS.Vertex[2].Clip.W;

			line.Left.Denominator.T  = B_LeftCoord[0]  / RS.Vertex[0].Clip.W + B_LeftCoord[1]  / RS.Vertex[1].Clip.W + B_LeftCoord[2]  / RS.Vertex[2].Clip.W;
			line.Right.Denominator.T = B_RightCoord[0] / RS.Vertex[0].Clip.W + B_RightCoord[1] / RS.Vertex[1].Clip.W + B_RightCoord[2] / RS.Vertex[2].Clip.W;

			line.Current.Numerator.S = line.Left.Numerator.S;
			line.Current.Numerator.T = line.Left.Numerator.T;

			line.Current.Denominator.S = line.Left.Denominator.S;
			line.Current.Denominator.T = line.Left.Denominator.T;

			if (RC.Enable.Fog)
			{
				line.Left.Fog    = B_LeftCoord[0]  * vertex1->FogFactor + B_LeftCoord[1]  * vertex2->FogFactor + B_LeftCoord[2]  * vertex3->FogFactor;
				line.Right.Fog   = B_RightCoord[0] * vertex1->FogFactor + B_RightCoord[1] * vertex2->FogFactor + B_RightCoord[2] * vertex3->FogFactor;
				line.Current.Fog = line.Left.Fog;
                line.Iterator.Fog = (line.Right.Fog - line.Left.Fog) / line.Length;
            }
		}

		if (RC.Enable.Texturing2D)
		{
			line.Iterator.Numerator.S   = (line.Right.Numerator.S   - line.Left.Numerator.S)   / line.Length;
			line.Iterator.Denominator.S = (line.Right.Denominator.S - line.Left.Denominator.S) / line.Length;
				
			line.Iterator.Numerator.T   = (line.Right.Numerator.T   - line.Left.Numerator.T)   / line.Length;
			line.Iterator.Denominator.T = (line.Right.Denominator.T - line.Left.Denominator.T) / line.Length;

			UINT Texel = 0;

			for (X_LeftClamped; X_LeftClamped <= X_RightClamped; X_LeftClamped++)
			{
				//Calculate current texture coordinates
				line.Current.S = line.Current.Numerator.S / line.Current.Denominator.S;
				line.Current.T = line.Current.Numerator.T / line.Current.Denominator.T;

                //Implicitly set texture wrap mode to repeat
                if (line.Current.S > 1) { line.Current.S -= (Int)line.Current.S; }
                if (line.Current.S < 0) { line.Current.S -= (Int)line.Current.S - 1; }
                if (line.Current.T > 1) { line.Current.T -= (Int)line.Current.T; }
                if (line.Current.T < 0) { line.Current.T -= (Int)line.Current.T - 1; }

                //Perform depth test
                if (line.Current.Z < Buffer.Depth[pixel.Index])
                {
				    //Use linear filtering 
				    if (true)
				    {
					    float uMinusHalf = TextureWidth * line.Current.S - 0.5f;
					    float vMinusHalf = TextureHeight * line.Current.T - 0.5f;

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

                        //Implicitly use SGL_MODULATE as texture environment mode
					    pixel.Color[0] = line.Current.Color[0] * ((1 - alpha) * (1 - beta) * GET_R_NOMALIZED(Texel00) + alpha * (1 - beta) * GET_R_NOMALIZED(Texel01) + (1 - alpha) * beta * GET_R_NOMALIZED(Texel10) + alpha * beta * GET_R_NOMALIZED(Texel11));
					    pixel.Color[1] = line.Current.Color[1] * ((1 - alpha) * (1 - beta) * GET_G_NOMALIZED(Texel00) + alpha * (1 - beta) * GET_G_NOMALIZED(Texel01) + (1 - alpha) * beta * GET_G_NOMALIZED(Texel10) + alpha * beta * GET_G_NOMALIZED(Texel11));
					    pixel.Color[2] = line.Current.Color[2] * ((1 - alpha) * (1 - beta) * GET_B_NOMALIZED(Texel00) + alpha * (1 - beta) * GET_B_NOMALIZED(Texel01) + (1 - alpha) * beta * GET_B_NOMALIZED(Texel10) + alpha * beta * GET_B_NOMALIZED(Texel11));
					    pixel.Color[3] = line.Current.Color[3];
				    }
				    else
				    {
					    //Fetch texel
					    Texel = *(UINT*)(&pTexture[((Int)(TextureHeight * line.Current.T) * TextureWidth + (Int)(TextureWidth * line.Current.S)) * RS.Texture.Components]);

                        //Implicitly use SGL_MODULATE as texture environment mode
                        pixel.Color[0] = line.Current.Color[0] * ((BYTE)(Texel)       * 0.00392156862745098f);
					    pixel.Color[1] = line.Current.Color[1] * ((BYTE)(Texel >> 8)  * 0.00392156862745098f);
					    pixel.Color[2] = line.Current.Color[2] * ((BYTE)(Texel >> 16) * 0.00392156862745098f);
					    pixel.Color[3] = line.Current.Color[3];
                    }

				    //Apply fog
				    if (RC.Enable.Fog)
				    {
					    pixel.Color[0] = line.Current.Fog * pixel.Color[0] + (1 - line.Current.Fog) * RS.Fog.Color[0];
					    pixel.Color[1] = line.Current.Fog * pixel.Color[1] + (1 - line.Current.Fog) * RS.Fog.Color[1];
					    pixel.Color[2] = line.Current.Fog * pixel.Color[2] + (1 - line.Current.Fog) * RS.Fog.Color[2];
				    }

				    pixel.Write.Color  = (UByte)(255 * pixel.Color[3]); pixel.Write.Color <<= 8;
				    pixel.Write.Color += (UByte)(255 * pixel.Color[0]); pixel.Write.Color <<= 8;
				    pixel.Write.Color += (UByte)(255 * pixel.Color[1]); pixel.Write.Color <<= 8;
				    pixel.Write.Color += (UByte)(255 * pixel.Color[2]);

				    //Write to buffers
                    Buffer.Back[pixel.Index] = pixel.Write.Color;
                    Buffer.Depth[pixel.Index] = line.Current.Z;
                }

                pixel.Index++;
                line.Current.Z += line.Iterator.Z;

				//Update vertex color 
				AddVectors4(line.Current.Color, line.Current.Color, line.Iterator.Color);

                //Update fog state
                if (RC.Enable.Fog) { line.Current.Fog += line.Iterator.Fog; }

				//Update texture state
				line.Current.Numerator.S   += line.Iterator.Numerator.S;
				line.Current.Denominator.S += line.Iterator.Denominator.S;

				line.Current.Numerator.T   += line.Iterator.Numerator.T;
				line.Current.Denominator.T += line.Iterator.Denominator.T;
            }
		}
		else
		{
			for (X_LeftClamped; X_LeftClamped <= (Int)X_RightClamped; X_LeftClamped++)
			{
				//Scan one line
				if (line.Current.Z < Buffer.Depth[pixel.Index]) 
                {
                    pixel.Write.Color = (UByte)(255 * line.Current.Color[3]); pixel.Write.Color <<= 8;
                    pixel.Write.Color += (UByte)(255 * line.Current.Color[0]); pixel.Write.Color <<= 8;
                    pixel.Write.Color += (UByte)(255 * line.Current.Color[1]); pixel.Write.Color <<= 8;
                    pixel.Write.Color += (UByte)(255 * line.Current.Color[2]);

                    Buffer.Back[pixel.Index] = pixel.Write.Color; 
                    Buffer.Depth[pixel.Index] = line.Current.Z; 
                }

				//Update state
                pixel.Index++;
                line.Current.Z += line.Iterator.Z;

                AddVectors4(line.Current.Color, line.Current.Color, line.Iterator.Color);
            }
		}
	}
}