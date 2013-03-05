#pragma once

#include <string>
#include <GL/glfw.h>

class Font
{
	public:
		Font(char* path, int w, int h);
		//~Font();
		void drawString(const char* text, int length);
		int getCharWidth();
		int getCharHeight();

	private:
		int w; int h;
		GLuint textureid;
		GLuint basepointer;
};
