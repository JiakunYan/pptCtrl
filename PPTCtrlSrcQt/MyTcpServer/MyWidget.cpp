#include "MyWidget.h"

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
		// draw
		p.drawPixmap(x - win_width / 2, y - win_height / 2, win_width, win_height, magnify_pixmap);
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

void MyWidget::magnify_capture() {
	int width = win_width / magnifying_rate;    // Í¼Æ¬¿í¶È
	int height = win_height / magnifying_rate;   // Í¼Æ¬¸ß¶È
	QRect target(mScreen.x() + x - width / 2, mScreen.y() + y - height / 2, width, height);
	magnify_pixmap = QPixmap();
	QScreen * screen = QGuiApplication::primaryScreen();
	magnify_pixmap = screen->grabWindow(0).copy(target);
}
