## What is it?
A software reimplementation of a subset of the OpenGL 1.2 API. The early OpenGLs were based on a Fixed Function Pipeline which is replicated in this library. The library performs depth buffer based rendering and supports a some basic functionality:
* **2D textures** - Perspective correct texturing using barycentric coordinate interpolation. 
* **Bilinear filtering** - Perfomed both for minification and magnification. No mip-mapping yet.
* **Smooth shading** - Currently forced on as it keeps the code a bit simpler.
* **Vertex lighting** - Calculates ambient diffuse and specular contribution from point light sources at individual vertices and interpolates between them on pixel level.
* **Vertex fog** - Calculates fog contribution at individual vertices and interpolates between them on pixel level.
* **Frustum clipping** - Triangles are clipped at view frustum edges and split into new triangles to avoid per-pixel calculations outside view rectangle.

## Why does it exist?
To prove to myself I can read the OpenGL spec fully understand it and if need be, implement it. Project started in 2004 and has only occasionally been tinkered with since.

## Dependencies 
For convenience I have started using some of my other libraries:
* [SCFObjectExtensions](https://github.com/rjadroncik/Shadow-Common-Files/tree/master/Source/ObjectExtensions) - Used to work with texture memory, though the library itself contains a lot more functionality.
* [SCFMathematics](https://github.com/rjadroncik/Shadow-Common-Files/tree/master/Source/Mathematics) - Used for vector and 3D math.
* [SCFImaging](https://github.com/rjadroncik/Shadow-Common-Files/tree/master/Source/Imaging) - Used to load images in the test app. 
* [SCFTimer](https://github.com/rjadroncik/Shadow-Common-Files/tree/master/Source/Timer) - Used to work with high precision timers to simulate in-game passage of time. 

## Test app
I have made a simple test app allowing me to test out the various features.

![](https://raw.githubusercontent.com/rjadroncik/Shadow-Graphics-Library/master/ShadowGraphicsLibrary.png)