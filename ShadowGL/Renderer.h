#pragma once

namespace ShadowGLPrivate
{
	inline void CopyVertex(tVertex *destination, tVertex *source) { memcpy(destination, source, sizeof(tVertex)); }

	bool  CullTriangle();
	bool  ClipTriangle();	
	UByte ClipPolygonAtPlane(tVertex *inputVertex, _IN UByte count, const Float *clipPlane, tVertex *outputVertex); 

	void PrepareLighting(tVertex *vertex, UByte count);
	void PreparePolygon(tVertex *vertex, UByte count);
	void RasterizePolygon(tVertex *vertex, UByte count);

	void RasterizeTriangle(tVertex *vertex1, tVertex *vertex2, tVertex *vertex3);
	void ScanLine(tVertex *vertex1, tVertex *vertex2, tVertex *vertex3, SRendererState::SLineState &line, SRendererState::SCurPixelState &pixel);
}