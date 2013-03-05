#include "graphics2d.h"
#include <GL/glfw.h>
#include <iostream>
#include <cmath>


void G2D::Initialize(int pwindowx, int pwindowy)
{
	glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, GL_TRUE); //open a window with resizing disabled
	glfwOpenWindowHint(GLFW_FSAA_SAMPLES, 0); //no anti-aliasing in a 2d game!!

	glfwOpenWindow( pwindowx, pwindowy, 8, 8, 8, 0, 0, 0, GLFW_WINDOW); //open the window now
	glfwSetWindowTitle("Serious work");

#ifdef DEBUG
    std::cout << glGetString( GL_VENDOR ) << std::endl;
    std::cout << glGetString( GL_RENDERER ) << std::endl;
    std::cout << glGetString( GL_VERSION ) << std::endl;
#endif

	glfwSwapInterval(1); //ENABLE vsync
	//glfwSwapInterval(0); //DISABLE vsync

	//get window size in case the one we asked for was unsupported
	int windowx;
	int windowy;
	glfwGetWindowSize(&windowx, &windowy);

	//Orthographic 2D projection
	glMatrixMode (GL_PROJECTION); glLoadIdentity (); 
	glOrtho (0, windowx, 0, windowy, -1, 1);
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f); //I want it painted black

	glEnable(GL_ALPHA_TEST); //use alpha testing
	glAlphaFunc(GL_GREATER, 0.1f); //do not draw pixels below this alpha value 
	glEnable(GL_BLEND); //we want alpha blending
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); //use occlusion blending 

//	glEnable(GL_TEXTURE_2D); //use textures

}

int G2D::LoadImage(char* path, bool filtering)
{

	GLuint id; //get a name for the texture
	glGenTextures(1,&id); //generate a texture name
	glBindTexture(GL_TEXTURE_2D,id); //bind the texture to the id

	//modulate the color of the polygon with the texture
	glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	
	float transparent[] = {0.0f, 0.0f, 0.0f, 0.0f};
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, transparent);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER_ARB);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER_ARB);

	
	if(filtering)
	{
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR); 
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		if (!glfwLoadTexture2D(path,GLFW_BUILD_MIPMAPS_BIT )) //Load our image
		{
			std::cout << "Image file failed to load correctly." << std::endl;
		}

	}
	else
	{
		// we do not use interpolation or mipmaps in a 2d game
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		if (!glfwLoadTexture2D(path, 0)) //Load our image
		{
			std::cout << "Image file failed to load correctly." << std::endl;
		}
	}
	return id; //return the id of the texture
}

void G2D::DrawImage(int id, int x, int y, int z, int w, int h)
{
	if (id >= 0){
		glBindTexture(GL_TEXTURE_2D,id); //bind the texture to the polygon

		glBegin( GL_TRIANGLE_STRIP ); //draw a textured rectangle at the given position with given size
			glTexCoord2i(0,0); glVertex3i(x,y,z);
			glTexCoord2i(1,0); glVertex3i(x+w,y,z);
			glTexCoord2i(0,1); glVertex3i(x,y+h,z);
			glTexCoord2i(1,1); glVertex3i(x+w,y+h,z);
		glEnd();
	}
}
  
