#pragma once

#include <QtWidgets/QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QNetworkInterface>
#include <QMessageBox>
#include "ui_MyTcpServer.h"
#include "MyWidget.h"

const int HOST_PORT_NUM = 5101;

class MyTcpServer : public QMainWindow
{
	Q_OBJECT

public:
	MyTcpServer(QWidget *parent = Q_NULLPTR);
	~MyTcpServer();

private:
	const int timeInterval = 500; //ms
	Ui::MyTcpServerClass ui;
	QTcpServer *tcpServer = NULL;
	QTcpSocket *msgClient = NULL;
	QTcpSocket *videoClient = NULL;
	int videoTimerId;
	MyWidget *myWidget = NULL;

	void highLight(int x, int y);
	void highLight(int x1, int y1, int x2, int y2);
	void magnify(int x, int y);
	void magnify(int x1, int y1, int x2, int y2);
	void draw(int x, int y);

	void int2byte(int length, char* l2b);
	int capture(char**);

	QRect mScreen;

private slots:
	void NewConnectionSlot();
	void msgDisconnectedSlot();
	void videoDisconnectedSlot();
	void ReadData();
	void timerEvent(QTimerEvent *event);
	void onClickStart();
	void onClickEnd();
};
