#pragma once

namespace ShadowGL
{
	SHADOWGL_API void Begin(Enum primType);

	SHADOWGL_API void End();

	SHADOWGL_API void Vertex3f(Float x, Float y, Float z);

	SHADOWGL_API void Color3f(Float r, Float g, Float b); 
	SHADOWGL_API void Color4f(Float r, Float g, Float b, Float a); 
	SHADOWGL_API void Color4fv(const Float *v);

	SHADOWGL_API void Normal3f(Float x, Float y, Float z);
	SHADOWGL_API void TexCoord2f(Float x, Float y);
}

namespace ShadowGLPrivate
{
	inline void CopyVertex(tVertex *destination, tVertex *source) { memcpy(destination, source, sizeof(tVertex)); }

	bool  CullPrimitive();
	bool  ClipPrimitive();	
	UByte ClipPrimitiveAtPlane(tVertex *inputVertex, _IN UByte count, const Float *clipPlane, tVertex *outputVertex); 

	void PrepareLighting(tVertex *vertex, UByte count);
	void PreparePrimitive(tVertex *vertex, UByte count);
	void RasterizePrimitive(tVertex *vertex, UByte count);

	void RasterizeTriangle(tVertex *vertex1, tVertex *vertex2, tVertex *vertex3);
	void ScanLine(tVertex *vertex1, tVertex *vertex2, tVertex *vertex3);
}