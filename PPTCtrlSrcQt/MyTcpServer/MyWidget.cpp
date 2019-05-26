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
	int width = win_width / magnifying_rate;    // ͼƬ���
	int height = win_height / magnifying_rate;   // ͼƬ�߶�

						// ��ȡ���ڵ��豸�����ģ�Device Contexts��
	HDC hdcWindow = ::GetDC(GetDesktopWindow()); //GetDC(NULL); // Ҫ��ͼ�Ĵ��ھ����Ϊ����ȫ��
								 // ��ȡ�豸�����Ϣ�ĳߴ��С
	int nBitPerPixel = GetDeviceCaps(hdcWindow, BITSPIXEL);

	CImage image;
	// ����ͼ�����ÿ�ߣ�����
	image.Create(width, height, nBitPerPixel);
	// ��ָ����Դ�豸���������е����ؽ���λ�飨bit_block��ת��
	BitBlt(
		image.GetDC(),  // ���浽��Ŀ�� ͼƬ���� ������
		0,0,  // ��ʼ x, y ����
		width, height,  // ��ͼ���
		hdcWindow,      // ��ȡ����� �����ľ��
		(mScreen.x() + x - width / 2), (mScreen.y() + y - height / 2),//mScreen.x(), mScreen.y(),       // ָ��Դ�����������Ͻǵ� X, y �߼�����
		SRCCOPY);

	// �ͷ� DC���
	ReleaseDC(NULL, hdcWindow);
	// �ͷ�ͼƬ������
	image.ReleaseDC();
	// ��ͼƬ�� BMP �ĸ�ʽ���浽 F:\ScreenShot.bmp
	image.Save(_T("ScreenShot.bmp"));
}

