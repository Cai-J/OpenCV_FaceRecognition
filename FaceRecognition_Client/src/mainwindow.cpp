#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/types_c.h"
#include "QSplitter"
#include "QTimer"
#include "QPushButton"
#include "QVBoxLayout"
#include "QHostAddress"
#include "QBuffer"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	setWindowTitle("Opencv Face Client");
	this->time_calendar = new QTimer(this);
	this->qimage = new QImage();
	this->cap.open(0);
	this->time_calendar->start(300);

	QSplitter* splitter = new QSplitter();
	QWidget* textPanel = new QWidget();
	QWidget* canvasPanel = new QWidget();

	QBoxLayout* vb1 = new QVBoxLayout();
	vb1->addWidget(this->textcanvas);
	textPanel->setLayout(vb1);

	QVBoxLayout* vb = new QVBoxLayout();
	vb->addWidget(this->canvas);
	canvasPanel->setLayout(vb);

	splitter->addWidget(textPanel);
	splitter->addWidget(canvasPanel);

	QHBoxLayout* layout = new QHBoxLayout(ui->centralWidget);
	layout->addWidget(splitter);

	connect(time_calendar, SIGNAL(timeout()), this, SLOT(timer_Update()));

	this->textcanvas->setText("System Begin...");

	// tcp
	this->mSocket = new QTcpSocket(this);
}
MainWindow::~MainWindow()
{
	this->time_calendar->stop();
	this->cap.release();
	delete ui;
}

void MainWindow::timer_Update() {
	this->mSocket->abort();
	this->mSocket->connectToHost(QHostAddress::LocalHost, 8800);
	cap >> src_image;

	cv::cvtColor(src_image, src_image, CV_BGR2RGB);
	QImage qimage((unsigned const char*)src_image.data, src_image.cols, src_image.rows, QImage::Format_RGB888);

	QByteArray byte;
	QBuffer buf(&byte);
	qimage.save(&buf, "JPEG");

	QByteArray ss = qCompress(byte, 1);
	QByteArray vv = ss.toBase64();

	QByteArray ba;
	QDataStream out(&ba, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_5_9);

	out << (quint64)0;
	out << vv;
	out.device()->seek(0);
	out << (quint64)(ba.size() - sizeof(quint64));
	this->mSocket->write(ba);
	connect(mSocket, SIGNAL(readyRead()), this, SLOT(readyRead_Slot()));

	this->canvas->setPixmap(QPixmap::fromImage(qimage));

	update();
}
void MainWindow::readyRead_Slot() {
	QString buff;
	buff = this->mSocket->readAll();
	if (buff.length() <= 0 || buff == this->unk) {
		if (this->count++ > 200) {
			this->count %= 200;
			this->textcanvas->clear();
			this->textcanvas->setText("Unknown");   // 缓存 Q
			qDebug() << this->count << endl;
		}
	}
	else {
		this->count = 0;
		this->textcanvas->setText(buff + "  PASS");
		qDebug() << buff << " " << buff.length() << endl;
		this->textcanvas->update();
	}

}
