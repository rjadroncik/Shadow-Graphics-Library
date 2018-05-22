#pragma once

namespace ShadowGLPrivate
{
	inline void CopyVertex(tVertex *destination, tVertex *source) { memcpy(destination, source, sizeof(tVertex)); }

	bool  CullTriangle(SEnable& enable);
    bool  ClipTriangle(SEnable& enable);
	UByte ClipPolygonAtPlane(SEnable& enable, tVertex *inputVertex, _IN UByte count, const Float *clipPlane, tVertex *outputVertex);

	void PrepareLighting(SEnable& enable, tVertex *vertex, UInt count);

	void PreparePolygon(SRendererState& state, tVertex *vertex, UInt count);
	void RasterizePolygon(SRendererState& state, tVertex *vertex, UInt count);

	void RasterizeTriangle(SRendererState& state, tVertex *vertex1, tVertex *vertex2, tVertex *vertex3);
	void ScanLine(SRendererState& state, tVertex *vertex1, tVertex *vertex2, tVertex *vertex3, SRendererState::SLineState &line, SRendererState::SCurPixelState &pixel);
}