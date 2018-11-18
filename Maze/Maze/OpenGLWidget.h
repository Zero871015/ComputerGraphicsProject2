#pragma once
#include <QGLWidget>
#include <QString>
#include <QDir>
#include "Cell.h"
#include "LineSeg.h"
class OpenGLWidget :public QGLWidget
{
	Q_OBJECT
public:
	explicit OpenGLWidget(QWidget *parent = 0);

	void initializeGL();
	void paintGL();
	void resizeGL(int ,int );

	//Maze Setting
	void Mini_Map();
	void Map_3D();
	void loadTexture2D(QString, GLuint &);
	float degree_change(float );
	void Draw(float, float, float, float);
	void SomethingLikeClip(float,float);
	void SomethingLikeClip2(float,float);
	void SomethingLikeClip3(Cell*, LineSeg, LineSeg);
	void SomethingLikeClip4(Cell*, float, float);
private:
	GLuint grass_ID;
	GLuint sky_ID;
	QString pic_path;

	float top_z;
	float but_z;
};

