Documentation Author: Niko Procopi 2019

This tutorial was designed for Visual Studio 2017 / 2019
If the solution does not compile, retarget the solution
to a different version of the Windows SDK. If you do not
have any version of the Windows SDK, it can be installed
from the Visual Studio Installer Tool

Welcome to the Textures Tutorial!
Prerequesites: Shaders, OOP

In this tutorial, we will be introducing a new
library called FreeImage. The headers and linkings are 
already setup in this tutorial, we will use this 
library to load texture files before passing them
to OpenGL.

We will be using PNG images as the standard texture
format for all tutorials to come. PNG has lossless
compression, making it smaller than BMP, with better
quality than JPEG.

Prior to now, in each vertex, we had a position and a color.
From now on, each vertex will have a texture coordinate. A
texture coordinate is an (X, Y) coordinate on the texture image,
that can be put on each vertex, to tell the GPU which pixels of
the texture go on each polygon. These (X, Y) texture coordinates
go by a few different names: most people call them UV coordinates,
some poeple say XY coordinates, some say ST coordinates, but they 
are all literally the same thing. On a simple square, the UV
coordinates are (0,0) for the bottom left corner, (0,1) for the 
top left corner, (1,0) for the bottom right corner, and (1,1)
for the top right corner. You can see this in main.cpp lines 108-138

When the rasterizer interpolates these values, (from
vertex shader to fragment shader), the fragment shader
will then know exactly which texture coordinate (pixel) 
to put on each pixel of the polygon, that's how this works!

With FreeImage, we need to get the format of the image
we are working with. We (the programmers), know that
it is PNG, but the program does not know that by default,
so we need to tell it:
	FREE_IMAGE_FORMAT fileFormat = FreeImage_GetFileType("../assets/texture.png");

With the type, we load the image into a buffer of pixels.
This buffer of pixels is inside the FIBITMAP structure
	FIBITMAP* bitmap = FreeImage_Load(fileFormat, "../assets/texture.png");
	
By default, it loads a texture as 24-bit, or 3-bytes per pixel,
one for red, one for green, one for blue. However, most APIs dont support
24-bit textures, only 32-bit textures, so we need to give each pixel an alpha
channel. There is a simple way to do this, FreeImage does it for us with this:
	FIBITMAP* bitmap32 = FreeImage_ConvertTo32Bits(bitmap);
	
Now that the image is all set in RAM, we need to pass it to the GPU 
with OpenGL. First thing we have to do is make a texture buffer.
This creates an empty NULL texture
	glGenTextures(1, &texture);
	
Next, we have to "bind" the texture, the same way how we bind
VAOs and VBOs to make them globally accessible by OpenGL functions:
	glBindTexture(GL_TEXTURE_2D, texture);
	
Next, this is the tricky part, transferring the pixel data from
FIBITMAP to the OpenGL texture buffer. In main.cpp lines 173 to 183,
we explain each parameter of the glTexImage2D function, and how
the image is transferred to OpenGL.

After the image is transferred to OpenGL, we no longer need the pixel
data in FIBITMAP, because it has been copied to OpenGL, so we can
delete the copy in FreeImage:
    FreeImage_Unload(bitmap);
    FreeImage_Unload(bitmap32);
	
Lines 190 - 210 adjust texture parameters, which tells OpenGL
how to handle the texture. The comments explain how they work very nicely.
We will go more in-depth with texture parameters in the "more graphics" 
section, in the mipmapping and anisotropic filtering tutorials

Next, we need to give the texture to the fragment shader as a uniform,
the same way the world matrix was given to the vertex shasder as a uniform
	textureUniform = glGetUniformLocation(shaderProgram, "tex");
	
The value of this texture is zero, because it was the first texture in the
array of OpenGL textures to be made. The first index of any array is zero
(GL_TEXTURE0 = 0, GL_TEXTURE1 = 1), etc
	glUniform1i(textureUniform, 0);
	
The C++ code is done, the rest is GLSL

In the Vertex shader, we take the UV coordinate in, and we pass it to
the rasterizer, we do not need to change the UV in any way:
	uv = in_uv;
	
The rasterizer interpolates the uv coordinate for each pixel,
as explained earlier in the documentation

We can get a pixel from our texture with the "texture()" function:
	texture(tex, uv)
	
We can multiply this pixel by the vertex color to give it a tint.
Our vertex color is white, so there wont be a tint, but you can change it
and experiment with it.

Congratulations, you have a texture on a polygon