#pragma once
#include <QtWidgets/QApplication>
#include <QtCore>
#include <QPixmap>
#include <QScreen>
#include <QBuffer>

QPixmap capture(QRect target) {
	QScreen *screen = QGuiApplication::primaryScreen();
	return screen->grabWindow(0).copy(target);
}

QByteArray compress(QPixmap &pixmap, int quality) {
	Q_ASSERT(quality > 100 || quality < 0);
	QByteArray array;
	QBuffer buffer(&array);
	pixmap.save(&buffer, "JPEG", quality);
	return array;
}