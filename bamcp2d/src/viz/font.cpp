#include "font.h"
#include "graphics2d.h"

Font::Font(char* path, int w, int h)
{
	this->w = w;
	this->h = h;
	this->textureid = G2D::LoadImage(path,false);

    this->basepointer = glGenLists(256);
    
	float cx;
    float cy;
    for(int i = 0; i < 256; i++)
    {
        cx = float(i%16)/16.0f;
        cy = 1.0f-float(i/16)/16.0f;

        glNewList(this->basepointer+i,GL_COMPILE);
        glBegin(GL_TRIANGLE_STRIP);
            glTexCoord2f(cx,cy-1/16.0f);
            glVertex2i(0,0);
            glTexCoord2f(cx+1/16.0f,cy-1/16.0f);
            glVertex2i(16,0);
            glTexCoord2f(cx,cy);
            glVertex2i(0,16);
            glTexCoord2f(cx+1/16.0f,cy);
            glVertex2i(16,16);
        glEnd();
        glTranslated(this->w,0,0);
        glEndList();
    }
    glDisable(GL_TEXTURE_2D);
}

void Font::drawString(const char* text, int length)
{
	glPushMatrix();
		glBindTexture(GL_TEXTURE_2D,this->textureid);
		glListBase(this->basepointer);
		glCallLists(length,GL_UNSIGNED_BYTE,text);
	glPopMatrix();
}

int Font::getCharWidth()
{
	return w;
}
int Font::getCharHeight()
{
	return h;
}

