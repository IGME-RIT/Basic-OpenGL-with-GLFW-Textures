/*
Title: Textures
File Name: main.cpp
Copyright � 2016
Author: David Erbelding
Written under the supervision of David I. Schwartz, Ph.D., and
supported by a professional development seed grant from the B. Thomas
Golisano College of Computing & Information Sciences
(https://www.rit.edu/gccis) at the Rochester Institute of Technology.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/



#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm/gtc/matrix_transform.hpp"
#include "FreeImage.h"
#include <vector>
#include "../header/shape.h"
#include "../header/transform2d.h"
#include "../header/shader.h"
#include <iostream>

Shape* square;

// The transform being used to draw our shape
Transform2D transform;

// Shaders.
Shader vertexShader;
Shader fragmentShader;

// GL index for the shader program
GLuint shaderProgram;

//Index of the world matrix within the shader program.
GLuint worldMatrixUniform;

// Textures also use uniform variables.
GLuint textureUniform;

// Texture buffer index
GLuint texture;


// Window resize callback
void resizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}




int main(int argc, char **argv)
{
	// Initializes the GLFW library
	glfwInit();

	// Initialize window
	GLFWwindow* window = glfwCreateWindow(800, 600, "Textures", nullptr, nullptr);

	glfwMakeContextCurrent(window);

	//set resize callback
	glfwSetFramebufferSizeCallback(window, resizeCallback);

	// Initializes the glew library
	glewInit();


	// Indices for square (-1, -1)[2] to (1, 1)[1]
	// [0]------[1]
	//	|		 |
	//	|		 |
	//	|		 |
	// [2]------[3]

	// Create square vertex data.
    // This vertex now contains information for position, uv (texture coordinates), and color!
	std::vector<Vertex2dUVColor> vertices;

    // You may have noticed that the colors of vertices in the previous example created a gradient.
    // This is because the gpu rasterizes our triangle data into fragments/pixels.
    // When we pass out data from the vertex shader, the rasterizer interpolates between all of the values.
    // For example, if we have red (1, 0, 0, 1) and blue(0, 0, 1, 1) vertices, a fragment exactly between them would
    // have a color (.5, 0, .5, 1). This is calculated based on distance.
    
    // We can use the same concept to read from textures.
    // In our shader, (0, 0) is the bottom left of a texture and (1, 1) is the top right.
    // If we map our texture onto our vertices this way, every fragment will have an accurate
    // position within the texture to get a color.


	vertices.push_back(
        Vertex2dUVColor(
			glm::vec2(-1, 1), // top left
			glm::vec2(0, 1), // uv coordinates top left of texture
            glm::vec4(1, 1, 1, 1) // color white
			)
		);
	vertices.push_back(
        Vertex2dUVColor(
			glm::vec2(1, 1), // top right
			glm::vec2(1, 1), // uv coordinates top right of texture
            glm::vec4(1, 1, 1, 1)
			)
		);
	vertices.push_back(
        Vertex2dUVColor(
			glm::vec2(-1, -1), // bottom left
			glm::vec2(0, 0), // uv coordinates bottom left of texture
            glm::vec4(1, 1, 1, 1) // color white
			)
		);
    vertices.push_back(
        Vertex2dUVColor(
            glm::vec2(1, -1), // bottom right
            glm::vec2(1, 0), // uv coordinates bottom right of texture
            glm::vec4(1, 1, 1, 1) // color white
			)
		);

	// Our index buffer won't have to change!
	std::vector<unsigned int> indices;
	indices.push_back(0);
	indices.push_back(1);
	indices.push_back(2);
	indices.push_back(3);
	indices.push_back(2);
	indices.push_back(1);


	// Create shape object
	square = new Shape(vertices, indices);
	

	transform.SetScale(.5f);
	transform.SetPosition(glm::vec2(.25, .25));


    // Texture Setup:

    // First we have to load the texture. We're using freeimage to do this, but you could use any library, or even write your own!

    // Get the image format (we need this to load it)
    FREE_IMAGE_FORMAT fileFormat = FreeImage_GetFileType("../assets/texture.png");
    // Load the file.
    FIBITMAP* bitmap = FreeImage_Load(fileFormat, "../assets/texture.png");
    // Convert the file to 32 bits so we can use it.
    FIBITMAP* bitmap32 = FreeImage_ConvertTo32Bits(bitmap);


    // Tell openGL to create 1 texture. Store the index of it in our texture variable.
    glGenTextures(1, &texture);

    // Bind our texture to the GL_TEXTURE_2D binding location.
    glBindTexture(GL_TEXTURE_2D, texture);

    // Fill our openGL side texture object. This function has a ton of parameters:
    // 1) Binding location (of course)
    // 2) Number of mipmaps (we won't be using this)
    // 3) Format that we want the data in. (rgba are each 8 bits)
    // 4&5) Width and Height of our texture
    // 6) Border (probably deprecated. At the time of writing this, the documentation for the function just says it must be 0)
    // 7) The format that the data we are giving it. (FreeImage stores it as a bitmap, which is bgra)
    // 8) The size of each channel per pixel we are giving it.
    // 9) A void pointer to the beginning of the pixel data.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, FreeImage_GetWidth(bitmap32), FreeImage_GetHeight(bitmap32),
        0, GL_BGRA, GL_UNSIGNED_BYTE, static_cast<void*>(FreeImage_GetBits(bitmap32)));

    // We can unload the images now that the texture data has been buffered with opengl
    FreeImage_Unload(bitmap);
    FreeImage_Unload(bitmap32);

    // glTexParameteri changes settings about the texture that affect the way the gpu reads from it.
    // You need a binding location, a setting to change, and a new value for the setting.
    
    // Here we set the texture wrapping for S to clamp.
    // This means that if we read outside the texture, we just get the value on the edge.
    // If we set it to GL_WRAP_BORDER, the texture would uuh, wrap.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);

    // Set T to clamp. S and T are the coordinates in our texture. They are more often refered to as UV coordinates.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Sometimes an image ends up being rendered much smaller on screen than it actually is.
    // The min filter changes how we decide which color to pick.
    // GL_NEAREST just uses the most accurate pixel. This can result in a weird aliasing
    // GL_LINEAR would blend between nearby pixels.
    ///Try both at different scales to see the difference.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // If the image is scaled up, we can have similar problems between pixels.
    // GL_NEAREST will give us an 8-bit effect for low res images.
    // GL_LINEAR will try to blend the colors together, but you'll still have problems.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);


    // Unbind the texture.
    glBindTexture(GL_TEXTURE_2D, 0);





    // Create shader program:

	// Initialize the shaders using the shader files.
	vertexShader.InitFromFile("../shaders/vertex.glsl", GL_VERTEX_SHADER);
	fragmentShader.InitFromFile("../shaders/fragment.glsl", GL_FRAGMENT_SHADER);

	// Create a shader program.
	shaderProgram = glCreateProgram();
	
	// Attach the vertex and fragment shaders to our program.
	vertexShader.AttachTo(shaderProgram);
	fragmentShader.AttachTo(shaderProgram);

	// Build full shader program.
	glLinkProgram(shaderProgram);

	// Get world matrix uniform location
	worldMatrixUniform = glGetUniformLocation(shaderProgram, "worldMatrix");

    // Get the texture uniform from the shader program.
    textureUniform = glGetUniformLocation(shaderProgram, "tex");
    


	// Main Loop
	while (!glfwWindowShouldClose(window))
	{
		// Clear the screen.
		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(0.0, 0.0, 0.0, 0.0);


		// rotate square
		transform.Rotate(.0002f);

		// Set the current shader program.
		glUseProgram(shaderProgram);


        // This is really dumb, but basically, openGL has a number of different texture binding locations.
        // One of these locations is "Active Texture".
        // When we use GL_TEXTURE_2D, we are actually referring to whatever the active texture is.
        // The only reason it's like this is for backwards compatibility. We can only hope this will be better in the future.
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);

        // This function call sets the texture uniform for our shader program to use the texture at GL_TEXTURE0
        glUniform1i(textureUniform, 0);
		
		// Draw the square!
		square->Draw(transform.GetMatrix(), worldMatrixUniform);


        // Unbind the texture.
        glBindTexture(GL_TEXTURE_2D, 0);


		// Stop using the shader program.
		glUseProgram(0);

		// Swap the backbuffer to the front.
		glfwSwapBuffers(window);

		// Poll input and window events.
		glfwPollEvents();

	}

	// Free memory from shader program and individual shaders
	glDeleteProgram(shaderProgram);

    // Free memory from loaded texture.
    glDeleteTextures(1, &texture);

	// Free memory from shape object
	delete square;

	// Free GLFW memory.
	glfwTerminate();


	// End of Program.
	return 0;
}
