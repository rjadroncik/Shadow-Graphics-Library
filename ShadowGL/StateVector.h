#pragma once

#include "Types.h"
#include <SCFMathematics.h>

typedef HANDLE HDRAWDIB;

using namespace SCFMathematics;

namespace ShadowGLPrivate
{
    const	UByte	MAX_PRIMITIVE_VERTICES		= 3;

    const	UByte	MAX_MODELVIEW_MATRICES		= 32;
    const	UByte	MAX_PROJECTION_MATRICES		= 2;
    const	UByte	MAX_TEXTURE_MATRICES		= 2;

    const	UByte	MAX_LIGHTS					= 8;
    const	UByte	MAX_TEXTURES				= 255;

    const	UShort	MAX_TEXTURE_SIZE			= 512;

    const	UShort	MAX_BUFFER_WIDTH			= 2048;
    const	UShort	MAX_BUFFER_HEIGHT			= 2048;

    const	UShort	MAX_THREADS			        = 8;


    typedef struct SVertex
    {
        Float4 ObjCoord;
        Float4 EyeCoord;
        Float4 ClipCoord;
        Float4 NormCoord;
        Float4 ViewCoord;

        Float3 ObjNormal;
        Float3 EyeNormal;
        
        Float4 ObjColor;
        Float4 LitColor;

        Float4 ObjTexCoord;
        Float4 FinTexCoord;

        Float FogFactor;

        Boolean InClipVolume;
        Boolean Finished;

    } tVertex;

    struct SBufferState
    {
        UInt  Back [MAX_BUFFER_WIDTH * MAX_BUFFER_HEIGHT];
        Float Depth[MAX_BUFFER_WIDTH * MAX_BUFFER_HEIGHT];
    };

    struct SRenderingContext
    {
        struct BufferState
        {
            BITMAPINFOHEADER BitmapInfoHeader;
            HDRAWDIB         hDrawDibDC;

            Float4 ClearColor;
            Float  ClearDepth;

        } Buffer;

        struct VertexState
        {
        //Current tVertex State
            Float3 Normal;
            Float4 Color;

            Float4 TexCoord;

        } Vertex;

        struct PrimState
        {
        //Current Primitive State
            UByte   VertexCount;
            tVertex Vertex[MAX_PRIMITIVE_VERTICES];

            UByte ClipArray;
            UByte ClipCount;

            tVertex ClipVertex[2][3*MAX_PRIMITIVE_VERTICES];

            UByte MaxVertices;
            UInt  Type;

            Boolean Building;	

        } Primitive;

        struct ViewState
        {
        //Current View State
            Float4 ClipPlane[6];

            Int2 Origin;
            Int2 Size;
            Int2 HalfSize;
        
            Double Aspect;
            Double FOV;

            Double ClipNear;
            Double ClipFar;
            
            Float4 Position;
            Float3 Rotation;

        } View;

        struct MatrixState
        {
        //Current Matrix State
            Enum Mode;
            
            Matrix4 ModelView       [MAX_MODELVIEW_MATRICES];
            Matrix4 ModelViewInverse[MAX_MODELVIEW_MATRICES];
            Matrix4 Projection      [MAX_PROJECTION_MATRICES];
            Matrix4 Texture         [MAX_TEXTURE_MATRICES];

            UInt MVCurrent;
            UInt PCurrent;
            UInt TCurrent;

        } Matrix;

        struct MaterialState
        {
        //Current tVertex State
            Float4 Ambient;
            Float4 Diffuse;
            Float4 Specular;
            Float4 Emission;

            Float Shininess;

        } Material;

        struct FogState
        {
        //Current Fog State
            Enum Mode;

            Float Start;
            Float End;
            Float Density;

            Float4 Color;

        } Fog;

        struct PixelTransferState
        {
        //Current Fog State
            Float Scale_Red;
            Float Scale_Green;
            Float Scale_Blue;
            Float Scale_Alpha;
            Float Scale_Depth;

            Float Bias_Red;
            Float Bias_Green;
            Float Bias_Blue;
            Float Bias_Alpha;
            Float Bias_Depth;

        } PixelTransfer;

        struct SLight
        {
            //Common parameters
            Float4 Ambient;
            Float4 Diffuse;
            Float4 Specular;
        
            Boolean Enabled;
            
            //Positional parameters
            Float4 EyePosition;
            
            //Spot parameters
            Float3 EyeDirection;
            Int Exponent;
            Float CutOff;

            //Attenuation factors
            Float Constant;
            Float Linear;
            Float Quadratic;

        } Light[MAX_LIGHTS];

        struct SEnable
        {
            Boolean SmoothShading;
            Boolean Lighting;
            Boolean Normalize;
            Boolean LocalViewer;
            Boolean Texturing1D;
            Boolean Texturing2D;
            Boolean FaceCulling;
            Boolean Fog;

        } Enable;

        Float4 Ambient;

        Float4 TexEnvColor;
        Enum TexEnvMode;

        Int TexCurrent1D;
        Int TexCurrent2D;
        Boolean TexNameUsed[MAX_TEXTURES];

        Enum CullStyle;
        Enum FrontFace;

        Enum ErrorCode;
    };

    struct STexture
    {
        Boolean		Used;

        UInt		Components;
        Enum		Target;

        UInt		Width;
        UInt		Height;

        UByte		*pData;
        Enum		Format;
    };

    struct SRendererState
    {
        struct SCurPixelState
        {
            Int Index;
            Float4 Color;

            struct SWrite
            {
                UINT Color;

            } Write;

        } Pixel[MAX_THREADS];

        struct SLineState
        {
            Int Index;
            Float Length;

            struct SPoint
            {
                Float4 Color;
                Float Fog;

                Float X;
                Float Y;
                Float Z;

                Float W;

                Float S;
                Float T;

                struct SSTCoords
                {
                    Float S;
                    Float T;

                } Numerator, Denominator;	

            } Left, Right, Current;

            struct SIterator
            {
                Float4 Color;

                Float Fog;
                Float Z;

                struct SSTCoords
                {
                    Float S;
                    Float T;

                } Numerator, Denominator;	

            } Iterator;

        } Line[MAX_THREADS];

        struct SVertex
        {
            Int Y;

            struct SClip
            {
                Float W;

            } Clip;

        } Vertex[3];

        struct SCurTriangleState
        {

        } Triangle;

        struct STextureState
        {
            UInt Components;

        } Texture;

        struct SFogState
        {
            Float4 Color;

        } Fog;

        UByte V1;
        UByte V2;
        UByte V3;
    };
}

namespace ShadowGL
{
    SHADOWGL_API	void ShadeModel(Enum mode);

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
    SHADOWGL_API	void TexEnvi(Enum target, Enum pname, Float param);

    SHADOWGL_API	void TexParameteri(Enum target, Enum pname, Int param);

    SHADOWGL_API	void GenTextures(SizeI n, Int * textures);
    SHADOWGL_API	void DeleteTextures(SizeI n, const Int * textures);

    SHADOWGL_API	void BindTexture(Enum target, Int texture);

    SHADOWGL_API	void PixelTransferf(Enum pname, Float param);

    SHADOWGL_API    void PushAttrib(Bitfield mask);
    SHADOWGL_API    void PopAttrib();
}