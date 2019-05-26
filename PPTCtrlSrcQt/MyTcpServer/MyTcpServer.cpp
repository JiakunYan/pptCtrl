#include "MyTcpServer.h"
#include <atlimage.h>
#define abs(x, y) x>y?x-y:y-x

/**
 * @brief ��⵱ǰ�����Ƿ�����������(VMware/VirtualBox)��ػ�����
 * @param str_card_name  ������������Ϣ
 * @return ���������������ػ�����������true, ���򷵻�false
 */
bool is_virtual_network_card_or_loopback(QString str_card_name)
{
	if (-1 != str_card_name.indexOf("VMware")
		|| -1 != str_card_name.indexOf("Loopback")
		|| -1 != str_card_name.indexOf("VirtualBox")
		)
		return true;

	return false;
}

bool isLocalIp(QHostAddress address)
{
	QList<QPair<QHostAddress, int>> localNet;
	localNet.append(QPair<QHostAddress, int>(QHostAddress("10.0.0.0"), 8));
	localNet.append(QPair<QHostAddress, int>(QHostAddress("176.16.0.0"), 12));
	localNet.append(QPair<QHostAddress, int>(QHostAddress("192.168.0.0"), 16));
	for (QPair<QHostAddress, int> subnet : localNet) {
		if (address.protocol() == QAbstractSocket::IPv4Protocol
			&& address.isInSubnet(subnet))
			return true;
	}
	return false;
}

/**
 * @brief ��ȡ����IP��ַ
 */
QString getLocalIp()
{
	// 1. ��ȡ��������ӿ�
	QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();
	QString result;
	foreach(QNetworkInterface inter, interfaces)
	{
		// ���˵�����Ҫ��������Ϣ
		if (is_virtual_network_card_or_loopback(inter.humanReadableName()))
			continue;
		if (inter.flags() & (QNetworkInterface::IsUp | QNetworkInterface::IsRunning)) {
			foreach(QHostAddress address, inter.allAddresses()) {
				if (isLocalIp(address)) {
					if (-1 != inter.name().indexOf("wireless")) {
						qDebug() << "wireless: " << address << "\n";
						if (result.size() != 0) {
							result.append("/");
						}
						result.append(address.toString());
					}
				}
			}
		}
	}
	if (result.size() == 0) {
		result.append("No IP detected");
	}
	return result;
}

MyTcpServer::MyTcpServer(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	ui.ipLine->setText(tr("%1:%2").arg(getLocalIp()).arg(HOST_PORT_NUM));   //��ȡ����IP
	// init ui
	connect(ui.startBtn, SIGNAL(released()), this, SLOT(onClickStart()));
	connect(ui.endBtn, SIGNAL(released()), this, SLOT(onClickEnd()));
}

MyTcpServer::~MyTcpServer()
{
	killTimer(videoTimerId);

	if (msgClient) {
		msgClient->close();
		msgClient = NULL;
	}
	if (videoClient) {
		videoClient->close();
		videoClient = NULL;
	}
	if (tcpServer) {
		tcpServer->close();
		tcpServer = NULL;
	}

	if (myWidget) {
		myWidget->close();
		delete myWidget;
		myWidget = NULL;
	}
}

// newConnection -> newConnectionSlot �����ӽ����Ĳۺ���
void MyTcpServer::NewConnectionSlot()
{
	static int num = 0;
	num = 1 - num;
	if (num == 1) {
		if (msgClient != NULL) {
			msgClient->close();
			delete msgClient;
		}
		msgClient = tcpServer->nextPendingConnection();
		connect(msgClient, SIGNAL(readyRead()), this, SLOT(ReadData()));
		connect(msgClient, SIGNAL(disconnected()), this, SLOT(msgDisconnectedSlot()));
		ui.statusLine->setText("Connected!");
	}
	else if (num == 0) {
		if (videoClient != NULL) {
			videoClient->close();
			delete videoClient;
		}
		videoClient = tcpServer->nextPendingConnection();
		connect(videoClient, SIGNAL(disconnected()), this, SLOT(videoDisconnectedSlot()));
		ui.statusLine->setText("Connected!");
	}
}

// �ͻ������ݿɶ��źţ���Ӧ�Ķ����ݲۺ���
void MyTcpServer::ReadData()
{
	while (msgClient->canReadLine())
	{
		QString data = QString(msgClient->readLine());
		QStringList test = data.split(',');
		QString cmd = data.split(',')[0].trimmed();
		ui.cmdLine->setText(data);
		if (cmd == "l") {  //last page
			myWidget->hide();
			mouse_event(MOUSEEVENTF_WHEEL, 0, 0, 120, 0);  //ģ��������Ϲ���
		}
		else if (cmd == "r") { //next page
			myWidget->hide();
			mouse_event(MOUSEEVENTF_WHEEL, 0, 0, -120, 0);  //ģ��������¹���
		}
		else if (cmd == "h") { //high light
			int x = data.split(',')[1].toInt();
			int y = data.split(',')[2].toInt();

			highLight(x, y);
		}
		else if (cmd == "h2") { //high light with two points
			int x1 = data.split(',')[1].toInt();
			int y1 = data.split(',')[2].toInt();
			int x2 = data.split(',')[3].toInt();
			int y2 = data.split(',')[4].toInt();
			highLight(x1, y1, x2, y2);
		}
		else if (cmd == "m") { //magnify
			int x = data.split(',')[1].toInt();
			int y = data.split(',')[2].toInt();

			magnify(x, y);
		}
		else if (cmd == "m2") { //magnify with two points
			int x1 = data.split(',')[1].toInt();
			int y1 = data.split(',')[2].toInt();
			int x2 = data.split(',')[3].toInt();
			int y2 = data.split(',')[4].toInt();

			magnify(x1, y1, x2, y2);
		}
		else if (cmd == "D") { //new draw
			myWidget->hide();
			myWidget->draw_new();
		}
		else if (cmd == "d") { //continue draw
			int x = data.split(',')[1].toInt();
			int y = data.split(',')[2].toInt();

			draw(x, y);
		}
		else if (cmd == "e") { //end draw
			myWidget->draw_move();
		}
		else if (cmd == "E") { //end activity
			myWidget->win_width = myWidget->default_win_width;
			myWidget->win_height = myWidget->default_win_height;
			myWidget->hide();
			myWidget->draw_new();
		}
	}
}

void MyTcpServer::msgDisconnectedSlot() {
	if (myWidget) {
		myWidget->close();
		delete myWidget;
		myWidget = NULL;
	}
	if (msgClient) {
		msgClient->close();
		msgClient = NULL;
	}
	ui.statusLine->setText("Disconnected!");
}

void MyTcpServer::videoDisconnectedSlot() {
	if (myWidget) {
		myWidget->close();
		delete myWidget;
		myWidget = NULL;
	}
	if (videoClient) {
		videoClient->close();
		videoClient = NULL;
	}
	ui.statusLine->setText("Disconnected!");
}

void MyTcpServer::highLight(int x, int y) {
	myWidget->currentFunction = MyWidget::HightLight;
	myWidget->x = x;
	myWidget->y = y;
	myWidget->show();
	myWidget->update();
}

void MyTcpServer::highLight(int x1, int y1, int x2, int y2) {
	myWidget->currentFunction = MyWidget::HightLight;
	myWidget->x = (x1 + x2) / 2;
	myWidget->y = (y1 + y2) / 2;
	myWidget->win_width = abs(x1, x2);
	myWidget->win_height = abs(y1, y2);
	myWidget->show();
	myWidget->update();
}

void MyTcpServer::magnify(int x, int y) {
	myWidget->currentFunction = MyWidget::Magnify;
	myWidget->x = x;
	myWidget->y = y;
	myWidget->hide();
	myWidget->capture();
	myWidget->show();
	myWidget->update();
}

void MyTcpServer::magnify(int x1, int y1, int x2, int y2) {
	myWidget->currentFunction = MyWidget::Magnify;
	myWidget->x = (x1 + x2) / 2;
	myWidget->y = (y1 + y2) / 2;
	myWidget->win_width = abs(x1, x2);
	myWidget->win_height = abs(y1, y2);
	myWidget->hide();
	myWidget->capture();
	myWidget->show();
	myWidget->update();
}

void MyTcpServer::draw(int x, int y) {
	myWidget->currentFunction = MyWidget::Draw;
	myWidget->lineTo(x, y);
	myWidget->show();
	myWidget->update();
}

void MyTcpServer::timerEvent(QTimerEvent *event)
{
	if (event->timerId() == videoTimerId && videoClient != NULL && videoClient->state() == QAbstractSocket::ConnectedState)
	{
		int length;
		char* sendMessage;
		length = capture(&sendMessage);
		char length2byte[4];
		int2byte(length, length2byte);
		videoClient->write(length2byte, 4);
		videoClient->write(sendMessage, length);
		videoClient->flush();
		delete[] sendMessage;
	}
}
void MyTcpServer::onClickStart() {
	ui.ipLine->setText(tr("%1:%2").arg(getLocalIp()).arg(HOST_PORT_NUM));   //��ȡ����IP
	tcpServer = new QTcpServer(this);
	tcpServer->listen(QHostAddress::Any, HOST_PORT_NUM);
	connect(tcpServer, SIGNAL(newConnection()), this, SLOT(NewConnectionSlot()));

	videoTimerId = startTimer(timeInterval);

	QDesktopWidget *desktop = QApplication::desktop();
	int screenId = desktop->screenCount() - 1;

	mScreen = desktop->screenGeometry(screenId);
	myWidget = new MyWidget();
	myWidget->set_winId(screenId);
	
	ui.statusLine->setText("Wait for connection");
}

void MyTcpServer::onClickEnd() {
	killTimer(videoTimerId);

	if (msgClient) {
		msgClient->close();
		msgClient = NULL;
	}
	if (videoClient) {
		videoClient->close();
		videoClient = NULL;
	}
	if (tcpServer) {
		tcpServer->close();
		tcpServer = NULL;
	}

	if (myWidget) {
		myWidget->close();
		delete myWidget;
		myWidget = NULL;
	}
	ui.statusLine->setText("End");
}

void MyTcpServer::int2byte(int length, char* l2b) {
	for (int i = 3; i >= 0; --i) {
		l2b[i] = (length & 0xFF);
		length >>= 8;
	}
}

int MyTcpServer::capture(char** pBuffer)
{
	// ��ȡ���ڵ��豸�����ģ�Device Contexts��
	HDC hdcWindow = ::GetDC(GetDesktopWindow()); //GetDC(NULL); // Ҫ��ͼ�Ĵ��ھ����Ϊ����ȫ��
								 // ��ȡ�豸�����Ϣ�ĳߴ��С
	int nBitPerPixel = GetDeviceCaps(hdcWindow, BITSPIXEL);

	CImage image;
	// ����ͼ�����ÿ�ߣ�����
	image.Create(mScreen.width(), mScreen.height(), nBitPerPixel);
	// ��ָ����Դ�豸���������е����ؽ���λ�飨bit_block��ת��
	BitBlt(
		image.GetDC(),  // ���浽��Ŀ�� ͼƬ���� ������
		0, 0,     // ��ʼ x, y ����
		mScreen.width(), mScreen.height(),  // ��ͼ���
		hdcWindow,      // ��ȡ����� �����ľ��
		mScreen.x(), mScreen.y(),           // ָ��Դ�����������Ͻǵ� X, y �߼�����
		SRCCOPY);

	// �ͷ� DC���
	ReleaseDC(NULL, hdcWindow);
	// �ͷ�ͼƬ������
	image.ReleaseDC();
	// ��ͼƬ�� BMP �ĸ�ʽ���浽 F:\ScreenShot.bmp

	unsigned long ulBufferLen = 0;
	IStream* pOutStream = NULL;
	if (CreateStreamOnHGlobal(NULL, TRUE, &pOutStream) == S_OK)
	{
		image.Save(pOutStream, Gdiplus::ImageFormatJPEG);
		HGLOBAL hOutGlobal = NULL;
		GetHGlobalFromStream(pOutStream, &hOutGlobal);
		LPBYTE pBits = (LPBYTE)GlobalLock(hOutGlobal);
		ulBufferLen = (DWORD)GlobalSize(pBits);
		*pBuffer = new char[ulBufferLen];
		memcpy(*pBuffer, pBits, ulBufferLen);
		GlobalUnlock(hOutGlobal);
		pOutStream->Release();
	}
	return ulBufferLen;
}