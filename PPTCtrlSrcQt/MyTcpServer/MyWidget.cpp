#include "MyWidget.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <atlimage.h>

extern bool open;

MyWidget::MyWidget(QWidget *parent)
	: QWidget(parent), win_width(default_win_width), win_height(default_win_height)
{
	this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowStaysOnTopHint);
	this->setAttribute(Qt::WA_TranslucentBackground);
	this->setWindowTitle(" ");
	// multiple screen
	QDesktopWidget *desktop = QApplication::desktop();
	mScreen = desktop->screenGeometry(winId);
	this->setGeometry(mScreen);
}

MyWidget::~MyWidget()
{
	delete path;
}

void MyWidget::paintEvent(QPaintEvent*)
{
	if (currentFunction == HightLight) {
		QPainter h(this);
		int cx = mScreen.width();
		int cy = mScreen.height();
		h.fillRect(0, 0, x - win_width / 2, cy, QColor(0, 0, 0, 100));
		h.fillRect(x - win_width / 2, 0, cx - x + win_width / 2, y - win_height / 2, QColor(0, 0, 0, 100));
		h.fillRect(x - win_width / 2, 0 + y + win_height / 2, cx - x + win_width / 2, cy - y - win_height / 2, QColor(0, 0, 0, 100));
		h.fillRect(x + win_width / 2, y - win_height / 2, cx - x - win_width / 2, win_height, QColor(0, 0, 0, 100));
	}
	else if (currentFunction == Magnify) {
		QPainter p(this);
		QPen pen(Qt::black, 5, Qt::DotLine, Qt::RoundCap, Qt::RoundJoin);
		p.setPen(pen);
		QPixmap pixmap;
		pixmap.load("ScreenShot.bmp");
		p.drawPixmap(x - win_width / 2, y - win_height / 2, win_width, win_height, pixmap);
		p.drawRect(x - win_width / 2, y - win_height / 2, win_width, win_height);
	}
	else if (currentFunction == Draw) {
		QPainter p(this);
		QPen pen;

		pen.setColor(Qt::red);
		pen.setWidth(10);
		p.setPen(pen);

		p.drawPath(*path);
	}
}

void MyWidget::draw_new() {
	if (path) {
		delete path;
	}
	path = new QPainterPath();
	isDrawMove = 1;
}
void MyWidget::lineTo(int x, int y) {
	if (!path) {
		path = new QPainterPath();
	}
	if (isDrawMove) {
		path->moveTo(x, y);
		isDrawMove = 0;
	}
	else {
		path->lineTo(x, y);
	}
}
void MyWidget::draw_move() {
	isDrawMove = 1;
}

void MyWidget::capture()
{
	int width = win_width / magnifying_rate;    // 图片宽度
	int height = win_height / magnifying_rate;   // 图片高度

						// 获取窗口的设备上下文（Device Contexts）
	HDC hdcWindow = ::GetDC(GetDesktopWindow()); //GetDC(NULL); // 要截图的窗口句柄，为空则全屏
								 // 获取设备相关信息的尺寸大小
	int nBitPerPixel = GetDeviceCaps(hdcWindow, BITSPIXEL);

	CImage image;
	// 创建图像，设置宽高，像素
	image.Create(width, height, nBitPerPixel);
	// 对指定的源设备环境区域中的像素进行位块（bit_block）转换
	BitBlt(
		image.GetDC(),  // 保存到的目标 图片对象 上下文
		0,0,  // 起始 x, y 坐标
		width, height,  // 截图宽高
		hdcWindow,      // 截取对象的 上下文句柄
		(mScreen.x() + x - width / 2), (mScreen.y() + y - height / 2),//mScreen.x(), mScreen.y(),       // 指定源矩形区域左上角的 X, y 逻辑坐标
		SRCCOPY);

	// 释放 DC句柄
	ReleaseDC(NULL, hdcWindow);
	// 释放图片上下文
	image.ReleaseDC();
	// 将图片以 BMP 的格式保存到 F:\ScreenShot.bmp
	image.Save(_T("ScreenShot.bmp"));
}

