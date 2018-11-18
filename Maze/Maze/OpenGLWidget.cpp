#include "OpenGLWidget.h"
#include <iostream>
#include "MazeWidget.h"
#include "LineSeg.h"
#include <gl\gl.h>
#include <gl\GLU.h>

bool Isinview(float x[], float y[], float& reangleL, float& reangleR,float &angleL, float &angleR);
void TellDir(float angle, float &dirX, float &dirY);
float ViewMatrix[4][4] = { 
	{ 1,0,0,0 },
	{ 0,1,0,0 },
	{ 0,0,1,0 },
	{ 0,0,0,1 }
}, d;

OpenGLWidget::OpenGLWidget(QWidget *parent) : QGLWidget(parent)
{
	
	top_z = 1.5f;
	but_z = -1;

	QDir dir("Pic");
	if(dir.exists())
		pic_path = "Pic/";
	else
		pic_path = "../x64/Release/Pic/";
}
void OpenGLWidget::initializeGL()
{
	glClearColor(0,0,0,1);
	glEnable(GL_TEXTURE_2D);
	loadTexture2D(pic_path + "grass.png",grass_ID);
	loadTexture2D(pic_path + "sky.png",sky_ID);
}
void OpenGLWidget::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	if(MazeWidget::maze!=NULL)
	{
		//View 1
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glViewport(0 , 0 , MazeWidget::w/2 , MazeWidget::h);
		float maxWH = std::max(MazeWidget::maze->max_xp, MazeWidget::maze->max_yp);
		glOrtho(-0.1, maxWH + 0.1, -0.1, maxWH + 0.1, 0, 10);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		Mini_Map();

		//View 2
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glViewport(MazeWidget::w/2,0, MazeWidget::w/2, MazeWidget::h);
		/*gluPerspective 定義透視
		//視野大小, nearplane, farplane, distance
		//Note: You shouldn't use this function to get view matrix, otherwise you will get 0.
		*/
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		//gluPerspective(MazeWidget::maze->viewer_fov, 1 , 0.01 , 200);

		/* gluLookAt
		//原本相機位置
		//看的方向
		//哪邊是上面
		//Note: You shouldn't use this function to get view matrix, otherwise you will get 0.
		*/
		float viewerPosX = MazeWidget::maze->viewer_posn[Maze::X];
		float viewerPosY = MazeWidget::maze->viewer_posn[Maze::Y];
		float viewerPosZ = MazeWidget::maze->viewer_posn[Maze::Z];

		d = -1.0 / tan(degree_change(MazeWidget::maze->viewer_fov / 2.0));
		float matrix[4][4] = { 
			{ sin(degree_change(MazeWidget::maze->viewer_dir)),0,-cos(degree_change(MazeWidget::maze->viewer_dir)),0 },
			{ 0,1,0,0 },
			{ -cos(degree_change(MazeWidget::maze->viewer_dir)),0,-sin(degree_change(MazeWidget::maze->viewer_dir)),0 },
			{ 0,0,0,1 }
		};
		float trans[4][4] = {
			{ 1,0,0,-MazeWidget::maze->viewer_posn[Maze::X] },
			{ 0,1,0,-MazeWidget::maze->viewer_posn[Maze::Z] },
			{ 0,0,1,-MazeWidget::maze->viewer_posn[Maze::Y] },
			{ 0,0,0,1 } 
		};

		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				ViewMatrix[i][j] = matrix[i][j];
			}
		}

		float ans[4][4] = { 0 };
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				for (int k = 0; k < 4; k++) {
					ans[i][j] += ViewMatrix[i][k] * trans[k][j];
				}
			}
		}
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {
				ViewMatrix[i][j] = ans[i][j];
			}
		}

		//不可使用
		/*gluLookAt(viewerPosX, viewerPosZ, viewerPosY,
			viewerPosX + cos(degree_change(MazeWidget::maze->viewer_dir)), viewerPosZ, viewerPosY + sin(degree_change(MazeWidget::maze->viewer_dir)),
			0.0, -1.0, 0.0);*/

		//確認矩陣用
		/*glMatrixMode(GL_MODELVIEW);
		float model[16];
		glGetFloatv(GL_MODELVIEW_MATRIX, model);*/

		glLoadIdentity();
		Map_3D();
	}
}
void OpenGLWidget::resizeGL(int w,int h)
{
}

//Draw Left Part
void OpenGLWidget::Mini_Map()	
{					
	glBegin(GL_LINES);

		float viewerPosX = MazeWidget::maze->viewer_posn[Maze::X];
		float viewerPosY = MazeWidget::maze->viewer_posn[Maze::Y];
		float viewerPosZ = MazeWidget::maze->viewer_posn[Maze::Z];

		for(int i = 0 ; i < (int)MazeWidget::maze->num_edges; i++)
		{
			float edgeStartX = MazeWidget::maze->edges[i]->endpoints[Edge::START]->posn[Vertex::X];
			float edgeStartY = MazeWidget::maze->edges[i]->endpoints[Edge::START]->posn[Vertex::Y];
			float edgeEndX = MazeWidget::maze->edges[i]->endpoints[Edge::END]->posn[Vertex::X];
			float edgeEndY = MazeWidget::maze->edges[i]->endpoints[Edge::END]->posn[Vertex::Y];

			glColor3f(MazeWidget::maze->edges[i]->color[0] , MazeWidget::maze->edges[i]->color[1], MazeWidget::maze->edges[i]->color[2]);
			if(MazeWidget::maze->edges[i]->opaque)
			{
				glVertex2f(edgeStartX, edgeStartY);
				glVertex2f(edgeEndX, edgeEndY);
			}
		}

		//draw frustum
		float maxWH = std::max(MazeWidget::maze->max_xp, MazeWidget::maze->max_yp);
		float len = 0.1;
		glColor3f(1, 1, 1);
		glVertex2f(viewerPosX, viewerPosY);
		glVertex2f(viewerPosX + maxWH * len * cos(degree_change(MazeWidget::maze->viewer_dir - MazeWidget::maze->viewer_fov / 2)),
			viewerPosY + maxWH * len * sin(degree_change(MazeWidget::maze->viewer_dir - MazeWidget::maze->viewer_fov / 2)));

		glVertex2f(viewerPosX, viewerPosY);
		glVertex2f(viewerPosX + maxWH * len * cos(degree_change(MazeWidget::maze->viewer_dir + MazeWidget::maze->viewer_fov / 2)),
			viewerPosY + maxWH * len *  sin(degree_change(MazeWidget::maze->viewer_dir + MazeWidget::maze->viewer_fov / 2)));
	glEnd();
}


//**********************************************************************
//
// * Draws the first-person view of the maze.
//   THIS IS THE FUINCTION YOU SHOULD MODIFY.
//
//Note: You must not use any openGL build-in function to set model matrix, view matrix and projection matrix.
//		ex: gluPerspective, gluLookAt, glTraslatef, glRotatef... etc.
//		Otherwise, You will get 0 !
//======================================================================
void OpenGLWidget::Map_3D()
{
	glLoadIdentity();
	// 畫右邊區塊的所有東西
	//MazeWidget::maze->viewer_posn[Maze::X] = 5.07232475;
	//MazeWidget::maze->viewer_posn[Maze::Y] = 0.501128852;
	//MazeWidget::maze->viewer_dir = 813.000000;
	float viewerPosX = MazeWidget::maze->viewer_posn[Maze::X];
	float viewerPosY = MazeWidget::maze->viewer_posn[Maze::Y];
	float viewerPosZ = MazeWidget::maze->viewer_posn[Maze::Z];

	glBegin(GL_QUADS);
	while (MazeWidget::maze->viewer_dir >= 360)MazeWidget::maze->viewer_dir -= 360;
	while (MazeWidget::maze->viewer_dir < 0)MazeWidget::maze->viewer_dir += 360;
	float dirL = MazeWidget::maze->viewer_dir + MazeWidget::maze->viewer_fov / 2;
	float dirR = MazeWidget::maze->viewer_dir - MazeWidget::maze->viewer_fov / 2;
	if (dirL < 0)dirL += 360;
	if (dirR >= 360)dirR -= 360;
	/*float MaxLen = sqrtf(powf(MazeWidget::maze->max_xp, 2) + powf(MazeWidget::maze->max_yp, 2));
	LineSeg LLine(viewerPosX, viewerPosY, viewerPosX + MaxLen*cos(dirL), viewerPosY + MaxLen*sin(dirL));
	LineSeg RLine(viewerPosX, viewerPosY, viewerPosX + MaxLen*cos(dirR), viewerPosY + MaxLen*sin(dirR));
	*/
	Cell* viewCell;
	for (int i = 0; i < MazeWidget::maze->num_cells; i++)
	{
		if (MazeWidget::maze->cells[i]->Point_In_Cell(viewerPosX, viewerPosY, 0, viewCell))
		{
			viewCell = MazeWidget::maze->cells[i];
			break;
		}
	}
	SomethingLikeClip4(viewCell, dirL, dirR);
	for (int i = 0; i < MazeWidget::maze->num_cells; i++)
	{
		MazeWidget::maze->cells[i]->counter = 0;
	}
	
	//SomethingLikeClip2(dirL, dirR);
	//SomethingLikeClip3(viewCell, LLine, RLine);


	/*若有興趣的話, 可以為地板或迷宮上貼圖, 此項目不影響評分*/
	glBindTexture(GL_TEXTURE_2D, sky_ID);
	
	// 畫貼圖 & 算 UV
	
	glDisable(GL_TEXTURE_2D);

	glEnd();
}
void OpenGLWidget::loadTexture2D(QString str,GLuint &textureID)
{
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
	
	QImage img(str);
	QImage opengl_grass = QGLWidget::convertToGLFormat(img);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, opengl_grass.width(), opengl_grass.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, opengl_grass.bits());
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glDisable(GL_TEXTURE_2D);
}
float OpenGLWidget::degree_change(float num)
{
	return num /180.0f * 3.14159f;
}
void mult_Matrix_view(float &x, float &y, float &z) {
	float M[4] = { x,y,z,1 };
	float ans[4] = { 0 };
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			ans[i] += ViewMatrix[i][j] * M[j];
		}
	}
	x = ans[0]; y = ans[1]; z = ans[2];
}
void newDraw(float x[2], float y[2], float z[2], float *a) {
	glBegin(GL_POLYGON);
	glColor3f(a[0], a[1], a[2]);
	float x_wall = x[0], y_wall = z[0], z_wall = y[0], w = 1;
	mult_Matrix_view(x_wall, y_wall, z_wall);
	glVertex2f(d*x_wall / z_wall, d*y_wall / z_wall);

	x_wall = x[0], y_wall = z[1], z_wall = y[0], w = 1;
	mult_Matrix_view(x_wall, y_wall, z_wall);
	glVertex2f(d*x_wall / z_wall, d*y_wall / z_wall);

	x_wall = x[1], y_wall = z[1], z_wall = y[1], w = 1;
	mult_Matrix_view(x_wall, y_wall, z_wall);
	glVertex2f(d*x_wall / z_wall, d*y_wall / z_wall);

	x_wall = x[1], y_wall = z[0], z_wall = y[1], w = 1;
	mult_Matrix_view(x_wall, y_wall, z_wall);
	glVertex2f(d*x_wall / z_wall, d*y_wall / z_wall);
	glEnd();
}
void OpenGLWidget::Draw(float x1, float y1, float x2, float y2)
{
	/*glMatrixMode(GL_PROJECTION);
	float M[16] = { sin(degree_change(MazeWidget::maze->viewer_dir)) ,0,-cos(degree_change(MazeWidget::maze->viewer_dir)),0,
	0,1,0,0,
		cos(degree_change(MazeWidget::maze->viewer_dir)) ,0,-sin(degree_change(MazeWidget::maze->viewer_dir)) ,0,
	0,0,0,1 };
	float t[16] = { 1,0,0,-MazeWidget::maze->viewer_posn[Maze::X],0,1,0,-MazeWidget::maze->viewer_posn[Maze::Z],0,0,1,-MazeWidget::maze->viewer_posn[Maze::Y],0,0,0,1 };
	glMultMatrixf(M);
	glMultMatrixf(t);*/
	
	
	float Mwtov1[4][4] = {
		{ sin(degree_change(MazeWidget::maze->viewer_dir)),0,-cos(degree_change(MazeWidget::maze->viewer_dir)),0 },
		{0,1,0,0 },
		{-cos(degree_change(MazeWidget::maze->viewer_dir)),0,-sin(degree_change(MazeWidget::maze->viewer_dir)),0 },
		{0,  0,  0,   1 }
	};
	float Mwtov2[4][4] = {
		{1,0,0,-MazeWidget::maze->viewer_posn[Maze::X] },
		{0,1,0,-MazeWidget::maze->viewer_posn[Maze::Z] },
		{0,0,1,-MazeWidget::maze->viewer_posn[Maze::Y] },
		{0,0,0,1 }
	};
	
	float R1[4][4];
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			R1[i][j] = 0;
		}
	}
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			for (int k = 0; k < 4; k++)
			{
				R1[i][j] += Mwtov1[i][k] * Mwtov2[k][j];
			}
		}
	}
	//R1確認無誤。
	float d = -1 * tan(MazeWidget::maze->viewer_fov / 2);
	/*
	float Mvtos[4][4] = {
		{1,0,0,0},
		{0,1,0,0},
		{0,0,1,0},
		{0,0,1/d,1}
	};
	float R2[4][4];
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			R2[i][j] = 0;
		}
	}
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			for (int k = 0; k < 4; k++)
			{
				R2[i][j] += Mvtos[i][k] * R1[k][j];
			}
		}
	}
	*/
	float w[4] = { x1,2,y1,1 };
	float newP[4] = { 0,0,0,0 },newP2[4]= { 0,0,0,0 };
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			newP[i] += R1[i][j] * w[j];
		}
	}
	newP[0] = newP[0] * d / newP[2];
	newP[1] = newP[1] * d / newP[2];

	float w2[4] = { x2,2,y2,1 };
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			newP2[i] += R1[i][j] * w2[j];
		}
	}
	newP2[0] = newP2[0] * d / newP2[2];
	newP2[1] = newP2[1] * d / newP2[2];


	glVertex2f(newP[0], newP[1]);
	glVertex2f(newP[0], -newP[1]);
	glVertex2f(newP2[0], -newP2[1]);
	glVertex2f(newP2[0], newP2[1]);
	
	/*
	glVertex3f(x1, 1, y1);
	glVertex3f(x1, -1, y1);
	glVertex3f(x2, -1, y2);
	glVertex3f(x2, 1, y2);*/
}

void OpenGLWidget::SomethingLikeClip(float Langle, float Rangle)
{
	float viewerPosX = MazeWidget::maze->viewer_posn[Maze::X];
	float viewerPosY = MazeWidget::maze->viewer_posn[Maze::Y];
	float viewerPosZ = MazeWidget::maze->viewer_posn[Maze::Z];

	Cell *BufferL, *BufferR;
	Edge* wallL;
	Edge* wallR;
	float MaxLen = sqrtf(powf(MazeWidget::maze->max_xp, 2)+powf(MazeWidget::maze->max_yp, 2));
	LineSeg LLine(viewerPosX, viewerPosY, viewerPosX + MaxLen*cos(Langle), viewerPosY + MaxLen*sin(Langle));
	LineSeg RLine(viewerPosX, viewerPosY, viewerPosX + MaxLen*cos(Rangle), viewerPosY + MaxLen*sin(Rangle));

	//Get collision point at left.
	float endC = LLine.end[0];
	for (int i = 0; i < MazeWidget::maze->num_cells; i++)
	{
		if (MazeWidget::maze->cells[i]->Point_In_Cell(LLine.start[0], LLine.start[1], 0, BufferL))
		{
			BufferL = MazeWidget::maze->cells[i];
			break;
		}
	}
	while (endC == LLine.end[0])
	{
		/*if (BufferL == NULL)
		{
			LLine.end[0] = LLine.start[0];
			LLine.end[1] = LLine.start[1];
			break;
		}*/
		if (LLine.start[0] >= BufferL->edges[0]->endpoints[0]->posn[0])
			LLine.start[0] = BufferL->edges[0]->endpoints[0]->posn[0] - 0.01;
		if (LLine.start[0] <= BufferL->edges[2]->endpoints[0]->posn[0])
			LLine.start[0] = BufferL->edges[2]->endpoints[0]->posn[0] + 0.01;
		if (LLine.start[1] >= BufferL->edges[1]->endpoints[0]->posn[1])
			LLine.start[1] = BufferL->edges[1]->endpoints[0]->posn[1] - 0.01;
		if (LLine.start[1] <= BufferL->edges[3]->endpoints[0]->posn[1])
			LLine.start[1] = BufferL->edges[3]->endpoints[0]->posn[1] + 0.01;
		BufferL = BufferL->Clip_To_Cell(LLine.start[0], LLine.start[1], LLine.end[0], LLine.end[1], 0, wallL);
	}
	
	//Get collision point at right.
	endC = RLine.end[0];
	for (int i = 0; i < MazeWidget::maze->num_cells; i++)
	{
		if (MazeWidget::maze->cells[i]->Point_In_Cell(RLine.start[0], RLine.start[1], 0, BufferR))
		{
			BufferR = MazeWidget::maze->cells[i];
			break;
		}
	}
	while (endC == RLine.end[0])
	{
		if (RLine.start[0] >= BufferR->edges[0]->endpoints[0]->posn[0])
			RLine.start[0] = BufferR->edges[0]->endpoints[0]->posn[0] - 0.01;
		if (RLine.start[0] <= BufferR->edges[2]->endpoints[0]->posn[0])
			RLine.start[0] = BufferR->edges[2]->endpoints[0]->posn[0] + 0.01;
		if (RLine.start[1] >= BufferR->edges[1]->endpoints[0]->posn[1])
			RLine.start[1] = BufferR->edges[1]->endpoints[0]->posn[1] - 0.01;
		if (RLine.start[1] <= BufferR->edges[3]->endpoints[0]->posn[1])
			RLine.start[1] = BufferR->edges[3]->endpoints[0]->posn[1] + 0.01;
		BufferR = BufferR->Clip_To_Cell(RLine.start[0], RLine.start[1], RLine.end[0], RLine.end[1], 0, wallR);
	}
	//only one wall.
	if (wallL->index == wallR->index)
	{
		glColor3f(wallL->color[0], wallL->color[1], wallL->color[2]);
		Draw(LLine.end[0], LLine.end[1], RLine.end[0], RLine.end[1]);
	}
	else
	{
		float tempA;
		bool xp = false, yp = false;
		if (sin(Langle) > 0 && cos(Langle)>0)
		{
			xp = true;
			yp = false;
		}
		else if (sin(Langle) > 0 && cos(Langle) < 0)
		{
			xp = true;
			yp = true;
		}
		else if (sin(Langle) < 0 && cos(Langle) > 0)
		{
			xp = false;
			yp = false;
		}
		else if (sin(Langle) < 0 && cos(Langle) < 0)
		{
			xp = false;
			yp = true;
		}
		//Get wall endpoint.
		//Wall is X dir
		if (wallL->endpoints[0]->posn[0] != wallL->endpoints[1]->posn[0])
		{
			if (xp)
			{
				float tempx, tempy;
				tempx = wallL->endpoints[1]->posn[0] - viewerPosX;
				tempy = wallL->endpoints[1]->posn[1] - viewerPosY;
				if (tempx == 0)tempx += 0.00001f;
				tempA = atan2f(tempy, tempx);
				//glColor3f(wallL->color[0], wallL->color[1], wallL->color[2]);
				//Draw(LLine.end[0], LLine.end[1], wallL->endpoints[1]->posn[0], wallL->endpoints[1]->posn[1]);
			}
			else
			{
				float tempx, tempy;
				tempx = wallL->endpoints[0]->posn[0] - viewerPosX;
				tempy = wallL->endpoints[0]->posn[1] - viewerPosY;
				if (tempx == 0)tempx += 0.00001f;
				tempA = atan2f(tempy, tempx);
				//glColor3f(wallL->color[0], wallL->color[1], wallL->color[2]);
				//Draw(LLine.end[0], LLine.end[1], wallL->endpoints[0]->posn[0], wallL->endpoints[0]->posn[1]);
			}
		}
		else
		{
			if (yp)
			{
				float tempx, tempy;
				tempx = wallL->endpoints[1]->posn[0] - viewerPosX;
				tempy = wallL->endpoints[1]->posn[1] - viewerPosY;
				if (tempx == 0)tempx += 0.00001f;
				tempA = atan2f(tempy, tempx);
				//glColor3f(wallL->color[0], wallL->color[1], wallL->color[2]);
				//Draw(LLine.end[0], LLine.end[1], wallL->endpoints[1]->posn[0], wallL->endpoints[1]->posn[1]);
			}
			else
			{
				float tempx, tempy;
				tempx = wallL->endpoints[0]->posn[0] - viewerPosX;
				tempy = wallL->endpoints[0]->posn[1] - viewerPosY;
				if (tempx == 0)tempx += 0.00001f;
				tempA = atan2f(tempy, tempx);
				//glColor3f(wallL->color[0], wallL->color[1], wallL->color[2]);
				//Draw(LLine.end[0], LLine.end[1], wallL->endpoints[0]->posn[0], wallL->endpoints[0]->posn[1]);
			}
		}
		SomethingLikeClip(Langle, tempA + 0.0001);
		SomethingLikeClip(tempA - 0.0001, Rangle);
	}

}

void OpenGLWidget::SomethingLikeClip2(float Langle,float Rangle)
{
	float viewerPosX = MazeWidget::maze->viewer_posn[Maze::X];
	float viewerPosY = MazeWidget::maze->viewer_posn[Maze::Y];
	float viewerPosZ = MazeWidget::maze->viewer_posn[Maze::Z];

	Cell *BufferL, *Buffer2, *BufferR;
	Edge* wallL;
	Edge* wallR;
	float MaxLen = sqrtf(powf(MazeWidget::maze->max_xp, 2)*powf(MazeWidget::maze->max_yp, 2));
	LineSeg LLine(viewerPosX, viewerPosY, viewerPosX+MaxLen*cos(Langle), viewerPosY+MaxLen*sin(Langle));
	LineSeg RLine(viewerPosX, viewerPosY, viewerPosX+MaxLen*cos(Rangle), viewerPosY+MaxLen*sin(Rangle));

	//Get collision point at left.
	for (int i = 0; i < MazeWidget::maze->num_cells; i++)
	{
		if (MazeWidget::maze->cells[i]->Point_In_Cell(viewerPosX, viewerPosY, viewerPosZ, BufferL))
		{
			BufferL = MazeWidget::maze->cells[i];
			break;
		}
	}
	Buffer2 = BufferL->Clip_To_Cell(LLine.start[0], LLine.start[1], LLine.end[0], LLine.end[1], 0, wallL);
	while (Buffer2 != NULL)
	{
		BufferL = Buffer2;
		Buffer2 = BufferL->Clip_To_Cell(LLine.start[0], LLine.start[1], LLine.end[0], LLine.end[1], 0, wallL);
	}

	//Get collision point at right.
	for (int i = 0; i < MazeWidget::maze->num_cells; i++)
	{
		if (MazeWidget::maze->cells[i]->Point_In_Cell(viewerPosX, viewerPosY, viewerPosZ, BufferR))
		{
			BufferR = MazeWidget::maze->cells[i];
			break;
		}
	}
	Buffer2 = BufferR->Clip_To_Cell(RLine.start[0], RLine.start[1], RLine.end[0], RLine.end[1], 0, wallR);
	while (Buffer2 != NULL)
	{
		BufferR = Buffer2;
		Buffer2 = BufferR->Clip_To_Cell(RLine.start[0], RLine.start[1], RLine.end[0], RLine.end[1], 0, wallR);
	}

	for (float now = Rangle;; now += 0.0008)
	{
		LineSeg nowLine(viewerPosX, viewerPosY, MaxLen*cos(now), MaxLen*sin(now));
		Cell *bufNow,*Buffer3;
		Edge *wallNow;
		for (int i = 0; i < MazeWidget::maze->num_cells; i++)
		{
			if (MazeWidget::maze->cells[i]->Point_In_Cell(viewerPosX, viewerPosY, viewerPosZ, bufNow))
			{
				bufNow = MazeWidget::maze->cells[i];
				break;
			}
		}
		Buffer3 = bufNow->Clip_To_Cell(nowLine.start[0], nowLine.start[1], nowLine.end[0], nowLine.end[1], 0, wallNow);
		while (Buffer3 != NULL)
		{
			bufNow = Buffer3;
			Buffer3 = bufNow->Clip_To_Cell(nowLine.start[0], nowLine.start[1], nowLine.end[0], nowLine.end[1], 0, wallNow);
		}

		if (wallNow->index == wallL->index)
		{
			glColor3f(wallL->color[0], wallL->color[1], wallL->color[2]);
			Draw(nowLine.end[0], nowLine.end[1], LLine.end[0], LLine.end[1]);
			break;
		}
		glColor3f(wallR->color[0], wallR->color[1], wallR->color[2]);
		Draw(RLine.end[0], RLine.end[1], nowLine.end[0], nowLine.end[1]);
		if (wallNow->index != wallR->index)
		{
			wallR = wallNow;
			RLine = nowLine;
		}
	}
}

void OpenGLWidget::SomethingLikeClip3(Cell *cell, LineSeg LLine, LineSeg RLine)
{

	float viewerPosX = MazeWidget::maze->viewer_posn[Maze::X];
	float viewerPosY = MazeWidget::maze->viewer_posn[Maze::Y];
	float viewerPosZ = MazeWidget::maze->viewer_posn[Maze::Z];
	float Langle = atan2f(LLine.end[1] - LLine.start[1], LLine.end[0] - LLine.start[0]);
	LineSeg oriR = RLine;
	Cell *BufferL, *BufferR, *Buf;
	Edge *WallL, *WallR;
	BufferL = cell->Clip_To_Cell(LLine.start[0], LLine.start[1], LLine.end[0], LLine.end[1], 0, WallL);
	BufferR = cell->Clip_To_Cell(RLine.start[0], RLine.start[1], RLine.end[0], RLine.end[1], 0, WallR);
	if(BufferL!=NULL)
		if (BufferL->Point_In_Cell(LLine.start[0], LLine.start[1], 0, Buf) == false)
		{
			if (LLine.start[0] > MazeWidget::maze->max_xp)
				LLine.start[0] = MazeWidget::maze->max_xp - 0.01;
			else if (LLine.start[0] < 0)
				LLine.start[0] = 0.01;
			else if (LLine.start[1] > MazeWidget::maze->max_yp)
				LLine.start[1] = MazeWidget::maze->max_yp - 0.01;
			else if (LLine.start[1] < 0)
				LLine.start[1] = 0.01;
			else if (Buf->edges[3] == BufferL->edges[1])
				LLine.start[1] = Buf->edges[3]->endpoints[0]->posn[1] - 0.01;
			else if (Buf->edges[1] == BufferL->edges[3])
				LLine.start[1] = Buf->edges[1]->endpoints[0]->posn[1] + 0.01;
			else if (Buf->edges[0] == BufferL->edges[2])
				LLine.start[0] = Buf->edges[0]->endpoints[0]->posn[0] + 0.01;
			else if (Buf->edges[2] == BufferL->edges[0])
				LLine.start[0] = Buf->edges[2]->endpoints[0]->posn[0] - 0.01;
		}
	//only one wall
	if (WallL == WallR)
	{
		//wall is opaque
		if(BufferL==NULL)
		{
			glColor3f(WallL->color[0], WallL->color[1], WallL->color[2]);
			Draw(LLine.end[0], LLine.end[1], RLine.end[0], RLine.end[1]);
		}
		else
		{
			SomethingLikeClip3(BufferL, LLine, RLine);
		}
	}
	else
	{
		//Find wall endpoint.
		
		float wallpointx, wallpointy;
		bool xp = false, yp = false;
		if (sin(Langle) > 0 && cos(Langle)>0)
		{
			xp = true;
			yp = false;
		}
		else if (sin(Langle) > 0 && cos(Langle) < 0)
		{
			xp = true;
			yp = true;
		}
		else if (sin(Langle) < 0 && cos(Langle) > 0)
		{
			xp = false;
			yp = false;
		}
		else if (sin(Langle) < 0 && cos(Langle) < 0)
		{
			xp = false;
			yp = true;
		}
		if (WallL->endpoints[0]->posn[0] != WallL->endpoints[1]->posn[0])
		{
			if (xp)
			{
				wallpointx = WallL->endpoints[1]->posn[0];
				wallpointy = WallL->endpoints[1]->posn[1];
			}
			else
			{
				wallpointx = WallL->endpoints[0]->posn[0];
				wallpointy = WallL->endpoints[0]->posn[1];
			}
		}
		else
		{
			if (yp)
			{
				wallpointx = WallL->endpoints[1]->posn[0];
				wallpointy = WallL->endpoints[1]->posn[1];
			}
			else
			{
				wallpointx = WallL->endpoints[0]->posn[0];
				wallpointy = WallL->endpoints[0]->posn[1];
			}
		}
		int dir;
		for (int i = 0; i < 4; i++)
		{
			if (cell->edges[i]->index == WallL->index)
			{
				dir = i;
				break;
			}
		}
		float newangle = atan2f(wallpointy - viewerPosY, wallpointx - viewerPosX);
		float MaxLen = sqrtf(powf(MazeWidget::maze->max_xp, 2) + powf(MazeWidget::maze->max_yp, 2));
		LineSeg *temp;
		temp = new LineSeg(wallpointx-0.001*cos(newangle-0.01), wallpointy-0.001*sin(newangle - 0.01), viewerPosX + MaxLen*cos(newangle - 0.01), viewerPosY + MaxLen*sin(newangle - 0.01));
		SomethingLikeClip3(cell, *temp, oriR);
		
		if (BufferL != NULL)
		{
			if (dir == 0)
				temp = new LineSeg(wallpointx - 0.001*cos(newangle) + 0.001, wallpointy - 0.001*sin(newangle), viewerPosX + MaxLen*cos(newangle - 0.001), viewerPosY + MaxLen*sin(newangle - 0.001));
			else if (dir == 1)
				temp = new LineSeg(wallpointx - 0.001*cos(newangle), wallpointy - 0.001*sin(newangle) + 0.001, viewerPosX + MaxLen*cos(newangle - 0.001), viewerPosY + MaxLen*sin(newangle - 0.001));
			else if (dir == 2)
				temp = new LineSeg(wallpointx - 0.001*cos(newangle) - 0.001, wallpointy - 0.001*sin(newangle), viewerPosX + MaxLen*cos(newangle - 0.001), viewerPosY + MaxLen*sin(newangle - 0.001));
			else if (dir == 3)
				temp = new LineSeg(wallpointx - 0.001*cos(newangle), wallpointy - 0.001*sin(newangle) - 0.001, viewerPosX + MaxLen*cos(newangle - 0.001), viewerPosY + MaxLen*sin(newangle - 0.001));
			SomethingLikeClip3(BufferL, LLine, *temp);
		}
		else
		{
			glColor3f(WallL->color[0], WallL->color[1], WallL->color[2]);
			Draw(LLine.end[0], LLine.end[1], wallpointx, wallpointy);
		}
	}
	
}

void OpenGLWidget::SomethingLikeClip4(Cell *cell, float Langle, float Rangle)
{
	cell->counter = 1;	//The cell is already checked.

	if (cell == NULL)return;	//Can not find the cell.

	//Foreach edge.
	for (int i = 0; i < 4; i++)
	{
		float x[2] = { cell->edges[i]->endpoints[Edge::START]->posn[Vertex::X],cell->edges[i]->endpoints[Edge::END]->posn[Vertex::X] };
		float y[2] = { cell->edges[i]->endpoints[Edge::START]->posn[Vertex::Y],cell->edges[i]->endpoints[Edge::END]->posn[Vertex::Y] };
		float *a = cell->edges[i]->color, angL, angR;
		if (cell->edges[i]->opaque)
		{
			if (Isinview(x, y, angL, angR, Langle, Rangle))
			{
				float viewX = MazeWidget::maze->viewer_posn[Maze::X], viewY = MazeWidget::maze->viewer_posn[Maze::Y];
				float z[2] = { -2,2 };
				if (x[0] == x[1]) 
				{
					float m1 = tan(angL / 180.0f * 3.14159f), m2 = tan(angR / 180.0f * 3.14159f);
					float y1 = m1 * (x[0] - viewX) + viewY, y2 = m2 * (x[1] - viewX) + viewY;
					if (y1 > y[1]) y1 = y[1];
					if (y1 < y[0]) y1 = y[0];
					if (y2 > y[1]) y2 = y[1];
					if (y2 < y[0]) y2 = y[0];
					y[0] = y1; y[1] = y2;

					newDraw(x, y, z, a);
				}
				else if (y[0] == y[1]) 
				{
					float m1 = tan(angL / 180.0f * 3.14159f), m2 = tan(angR / 180.0f * 3.14159f);
					float x1 = (y[0] - viewY) / m1 + viewX, x2 = (y[1] - viewY) / m2 + viewX;
					if (x1 > x[1]) x1 = x[1];
					if (x1 < x[0]) x1 = x[0];
					if (x2 > x[1]) x2 = x[1];
					if (x2 < x[0]) x2 = x[0];
					x[0] = x1; x[1] = x2;

					newDraw(x, y, z, a);
				}
			}
		}
		else {
			if (cell->edges[i]->Neighbor(cell)->counter != 1) {
				if (Isinview(x, y, angL, angR, Langle, Rangle))
					SomethingLikeClip4(cell->edges[i]->Neighbor(cell), angL, angR);
			}
		}
	}

}

bool Isinview(float x[], float y[],float &reangleL,float &reangleR,float &angleL, float &angleR)
{
	float viewerX = MazeWidget::maze->viewer_posn[Maze::X], viewerY = MazeWidget::maze->viewer_posn[Maze::Y];
	float disY = 0, disX = 0;

	float disY1 = 0, disX1 = 0, disY2 = 0, disX2 = 0;
	//Get the direction of 2 endpoints of the wall.
	float dirX1 = x[0] - viewerX, dirX2 = x[1] - viewerX;
	float dirY1 = y[0] - viewerY, dirY2 = y[1] - viewerY;
	dirX1 = (dirX1 > 0 ? 1 : dirX1 < 0 ? -1 : 0);
	dirX2 = (dirX2 > 0 ? 1 : dirX2 < 0 ? -1 : 0);
	dirY1 = (dirY1 > 0 ? 1 : dirY1 < 0 ? -1 : 0);
	dirY2 = (dirY2 > 0 ? 1 : dirY2 < 0 ? -1 : 0);

	//Get the direction of Ldir, dir and Rdir. 
	TellDir(MazeWidget::maze->viewer_dir, disX, disY);
	TellDir(angleR, disX1, disY1);
	TellDir(angleL, disX2, disY2);

	//Check dir of wall and dir of view is all same.
	if ((dirX1 == disX || dirX1 == disX1 || dirX1 == disX2 || dirX2 == disX || dirX2 == disX1 || dirX2 == disX2))
	{
		if ((dirY1 == disY || dirY1 == disY1 || dirY1 == disY2 || dirY2 == disY || dirY2 == disY1 || dirY2 == disY2))
		{
			dirX1 = x[0] - viewerX;
			dirX2 = x[1] - viewerX;
			dirY1 = y[0] - viewerY;
			dirY2 = y[1] - viewerY;
			//Get slope of R/L view.
			float m1 = (dirY1 / dirX1), m2 = (dirY2 / dirX2);

			//Get angle of wall point.
			//Wall is horizontal.
			if (x[0] == x[1]) 
			{
				disX = x[0] - viewerX;
				if (disX > 0) 
				{
					reangleR = atan2(dirY1, dirX1);
					reangleL = atan2(dirY2, dirX2);
				}
				else 
				{
					reangleL = atan2(dirY1, dirX1);
					reangleR = atan2(dirY2, dirX2);
				}
			}
			//Vertical.
			else if (y[0] == y[1])
			{
				disY = y[0] - viewerY;
				if (disY > 0)
				{
					reangleL = atan2(dirY1, dirX1);
					reangleR = atan2(dirY2, dirX2);
				}
				else 
				{
					reangleR = atan2(dirY1, dirX1);
					reangleL = atan2(dirY2, dirX2);
				}
			}
			//Change to degree.
			reangleL = reangleL * 180 / acos(-1);
			reangleR = reangleR * 180 / acos(-1);
			if (reangleL < 0) reangleL += 360;
			if (reangleR < 0) reangleR += 360;

			float difLL = angleL - reangleL;
			float difRR = angleR - reangleR;
			float difLR = angleL - reangleR;
			float difRL = angleR - reangleL;

			if (difLL < -180)  difLL += 360;
			if (difLL > 180)  difLL -= 360;
			if (difRR < -180)  difRR += 360;
			if (difRR > 180)  difRR -= 360;
			if (difLR < -180)  difLR += 360;
			if (difLR > 180)  difLR -= 360;
			if (difRL < -180)  difRL += 360;
			if (difRL > 180)  difRL -= 360;

			if ((difLL < 0 && difLR < 0) || (difRR > 0 && difRL > 0)) return false;
			if (difRR > 0) reangleR = angleR;
			if (difLL < 0) reangleL = angleL;
			return true;
		}
	}
	return false;
}
void TellDir(float angle, float &dirX, float &dirY) {
	while (angle > 360)angle -= 360;
	while (angle < 0)angle += 360;

	//If angle==0 or 180, dirY=0.
	if (angle > 0 && angle < 180) dirY = 1;
	else if (angle > 180 && angle < 360) dirY = -1;
	//If angle==90 or 270, dirX=0.
	if ((angle < 90 && angle >= 0) || (angle > 270 && angle <= 360)) dirX = 1;
	else if ((angle > 90 && angle < 270)) dirX = -1;
}