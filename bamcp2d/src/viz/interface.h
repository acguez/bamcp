#pragma once

#include "envs/flat/simulator2D.h"
class Font;
class Interface
{
	public:
		Interface(double sqsz, int x, int y, SIMULATOR2D* _map);
		void render();
		void getScreenPosition(int x, int y, double& sx, double& sy);
		void getScreenStartPosition2(double& sx, double&sy, double& sqsz2);
		void getScreenStartPosition3(double& sx, double&sy, double& sqsz3);
		void getScreenStartPosition4(double& sx, double&sy, double& sqsz4);
		
		void drawPredator(int x, int y);
		void drawAgent(int x, int y); 
		void StartRender();
		void StopRender();
		
		Font* font;
	private:
		int sqsz, X,Y;
		SIMULATOR2D* map;
		int agentTexId;
		int predTexId;
};
