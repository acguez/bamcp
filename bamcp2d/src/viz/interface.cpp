#include "interface.h"
#include <stdlib.h>
#include <limits>
#include <iostream>

#if defined(GX)
	#include <GL/glfw.h>
	#include "viz/graphics2d.h"
	#include "viz/font.h"
#endif

Interface::Interface(double _sqsz, int _x, int _y, SIMULATOR2D* _map) : sqsz(_sqsz), X(_x), Y(_y), map(_map)
{

#if defined(GX)
	agentTexId = G2D::LoadImage((char*)"../agent.tga",true);
	predTexId = G2D::LoadImage((char*)"../predator.tga",true);
	font = new Font("../font.tga",8,16);
#endif
}

void Interface::getScreenPosition(int x, int y, double& sx, double& sy){
	//sx = (x+1)*sqsz+sqsz/2;
	//sy = (y+1)*sqsz+sqsz/2;
	sx = (x+1)*sqsz+sqsz/2 + 30;
	sy = (y+1)*sqsz+sqsz/2 + 50;
}
void Interface::getScreenStartPosition2(double& sx, double&sy, double& sqsz2){
	sqsz2 = sqsz/2;
//	sx = sqsz*X+2.5+sqsz2+sqsz2/2;
//	sy = sqsz2+sqsz2/2;
	sx = sqsz*X+2.5+sqsz2+sqsz2/2 + 30;
	sy = sqsz2+sqsz2/2 + 50;

}
void Interface::getScreenStartPosition3(double& sx, double&sy, double& sqsz3){
	sqsz3 = sqsz/2;
//	sx = sqsz*X+sqsz3*X+2.5+sqsz3;
//	sy = 0;
	sx = sqsz*X+sqsz3*X+2.5+sqsz3 + 30;
	sy = 0 + 50;

}
void Interface::getScreenStartPosition4(double& sx, double&sy, double& sqsz4){
	sqsz4 = sqsz/2;
//	sx = sqsz*X+2.5;
//	sy = sqsz4*Y;
	sx = sqsz*X+2.5 + 30;
	sy = sqsz4*Y + 50;

}

void Interface::drawPredator(int x, int y){

#if defined(GX)
	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0,1.0,1.0);
	double sx,sy;
	getScreenPosition(x,y,sx,sy);
	G2D::DrawImage(predTexId,sx-sqsz/2,sy-sqsz/2,0.0f,sqsz*1.5,sqsz*1.5);		
	glDisable(GL_TEXTURE_2D);
#endif
}
void Interface::drawAgent(int x,int y){

#if defined(GX)
	glEnable(GL_TEXTURE_2D);
	glColor3f(1.0,1.0,1.0);
	double sx,sy;
	getScreenPosition(x,y,sx,sy);
	G2D::DrawImage(agentTexId,sx-sqsz/2,sy-sqsz/2,0.0f,sqsz*1.5,sqsz*1.5);		
	glDisable(GL_TEXTURE_2D);
	//Draw grid around agent
	/*
	glColor3f(246.0/255,48.0/255,82.0/255);
	glPushMatrix();
	glTranslated(sx-3*sqsz/2,sy-3*sqsz/2,0.0f);
	glBegin(GL_LINES);
		double ll = 3*sqsz;
		glVertex3f(0.0,0.0,0.0);
		glVertex3f(0.0,ll,0.0);
		glVertex3f(0.0,ll,0.0);
		glVertex3f(ll,ll,0.0);
		glVertex3f(ll,ll,0.0);
		glVertex3f(ll,0.0,0.0);
		glVertex3f(ll,0.0,0.0);
		glVertex3f(0.0,0.0,0.0);
	glEnd();
	glPopMatrix();
	*/
#endif
}



void Interface::StartRender(){

#if defined(GX)
	//get window size
	int windowx;
	int windowy;
	glfwGetWindowSize(&windowx, &windowy);
	
	glLineWidth(1.0);
	
	map->render(30.0,50.0,sqsz);
#endif

	//glfwSwapBuffers();		//Swap buffers and update input	
	/*
	if(glfwGetKey('N') == GLFW_PRESS)
	{
		NonRelease = true; 
	}
	else if(NonRelease)
	{
		NonRelease = false;
		this->map->changeDisplayedResource();
	}
*/
}

void Interface::StopRender(){

#if defined(GX)
	glfwSwapBuffers();		//Swap buffers and update input	
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
#endif	
	
}
