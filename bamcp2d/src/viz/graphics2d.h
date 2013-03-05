#pragma once

//EXTENSIONS
#define GL_CLAMP_TO_BORDER_ARB            0x812D

namespace G2D
{
	void Initialize(int,int); //Initialize the window with given resolution

	int LoadImage(char*, bool filtering); //Load image with given name and return id

	void DrawImage(int id, int x, int y, int z, int w, int h); //draw an image at position x y with size w h
	

};

