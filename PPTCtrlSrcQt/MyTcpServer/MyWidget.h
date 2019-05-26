#pragma once

#include <QWidget>
#include <QPainter>
#include <QPaintEvent>
#include <QDesktopWidget>
#include <QtWidgets/QApplication>


class MyWidget : public QWidget
{
	Q_OBJECT

public:
	enum Function { HightLight, Magnify, Draw } currentFunction;

	explicit MyWidget(QWidget *parent = Q_NULLPTR);
	~MyWidget();
	int x, y;
	int win_width, win_height;
	const int default_win_width = 300;
	const int default_win_height = 200;
	float magnifying_rate = 1.5;

	void draw_new();
	void lineTo(int x, int y);
	void draw_move();
	void capture();

	void set_winId(int t) { 
		winId = t; 
		QDesktopWidget *desktop = QApplication::desktop();
		mScreen = desktop->screenGeometry(winId);
		this->setGeometry(mScreen);
	}
	void set_win_width(int t) { win_width = t; }
	void set_win_height(int t) { win_height = t; }
	void set_rate(float t) { magnifying_rate = t; }
private:
	QPainterPath *path = NULL;
	QRect mScreen;
	int winId = 0;
	bool isDrawMove = 1;

protected:
	void paintEvent(QPaintEvent*);
};