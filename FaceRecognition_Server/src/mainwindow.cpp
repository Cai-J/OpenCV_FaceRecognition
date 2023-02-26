#pragma execution_character_set("utf-8")
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSplitter>
#include <QHBoxLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QInputDialog>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QBuffer>
#include <QImageReader>
#include "QSqlError"
#include "QSqlQuery"
#include "opencv2/imgproc/types_c.h"
#include "opencv2/opencv.hpp"

MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow)
{
	ui->setupUi(this);
	setWindowTitle("FaceRecognition_Server");
	QSplitter* splitter = new QSplitter();
	QWidget*   btnPanel = new QWidget();
	QWidget*   canvasPanel = new QWidget();

	QPushButton* selectImgBtn = new QPushButton("选择/注册");
	QPushButton* registerFaceBtn = new QPushButton("人脸注册");

	this->matchFaceOption = new QCheckBox("识别人脸");
	this->showlandmarkOption = new QCheckBox("显示landmark");
	this->showFPSOption = new QCheckBox("显示FPS");

	QVBoxLayout* vb1 = new QVBoxLayout();
	vb1->addWidget(selectImgBtn);
	vb1->addWidget(registerFaceBtn);
	vb1->addWidget(this->matchFaceOption);
	vb1->addWidget(this->showlandmarkOption);
	vb1->addWidget(this->showFPSOption);
	vb1->addStretch(1);

	QVBoxLayout* vb2 = new QVBoxLayout();
	vb2->addWidget(this->canvas);

	canvasPanel->setLayout(vb2);
	btnPanel->setLayout(vb1);

	splitter->addWidget(btnPanel);
	splitter->addWidget(canvasPanel);
	splitter->setStretchFactor(0, 1);
	splitter->setStretchFactor(1, 9);

	QHBoxLayout* layout = new QHBoxLayout(ui->centralWidget);
	layout->addWidget(splitter);

	// SQLite
	database = QSqlDatabase::addDatabase("QMYSQL"); // 设置为 MYSQL
	database.setHostName("127.0.0.1");
	database.setPort(3306);
	database.setDatabaseName("face_db");
	database.setUserName("root");
	database.setPassword("123456");
	this->psql = new QSqlQuery();

	if (!database.open()) {
		qDebug() << "Error: Failed to connect database..." << database.lastError();
	}
	else {
		qDebug() << "Succeed to connect database..." << endl;
	}

	// tcp
	this->mSercer = new QTcpServer(this);
	this->mSercer->listen(QHostAddress::LocalHost, 8800);

	connect(this->mSercer, SIGNAL(newConnection()), this, SLOT(slotNewConnection()));
	connect(selectImgBtn, SIGNAL(clicked()), this, SLOT(face_register()));
	connect(registerFaceBtn, SIGNAL(clicked()), this, SLOT(begin_register()));

	//./../../../model/
	// 人脸数据初始化
	std::string face_detetor_path = "./../../../model/yunet.onnx";
	std::string face_recog_path = "./../../../model/face_recognizer_fast.onnx";
	this->face_detector_recog.db_initFaceModels(face_detetor_path, face_recog_path);

	// 读数据库 初始化人脸模型
	this->psql->exec("select * from person");

	QPixmap p;
	while (this->psql->next()) {
		p.loadFromData(this->psql->value(3).toByteArray());
		QImage im = p.toImage();
		cv::Mat ma;
		QImag2cvMat(im, ma);
		cvtColor(ma, ma, cv::COLOR_RGB2BGR);
		this->face_detector_recog.new_registFace(this->psql->value(0).toInt(), ma, this->psql->value(1).toString().toStdString());
	}
}
MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::face_register() {
	this->register_filename = QFileDialog::getOpenFileName(
		this, tr("Open File"), "./home",
		tr("图片(*.jpg *.png)"));
	qDebug() << this->register_filename << " is select..." << endl;
	QImage* im = new QImage(this->register_filename);
	this->canvas->setPixmap(QPixmap::fromImage(*im));
}
void MainWindow::begin_register() {
	this->person_name = QInputDialog::getText(this, tr("输入名称"), tr("姓名"));
	qDebug() << this->person_name << endl;
	if (this->register_filename.isEmpty()) {
		return;
	}
	if (this->register_filename.endsWith(".jpg") || this->register_filename.endsWith(".png")) {
		cv::Mat image = cv::imread(this->register_filename.toStdString());
		image.copyTo(this->myface);
		qDebug() << "Copy Image success..." << endl;
		int index = this->register_filename.lastIndexOf("/");
		auto id = register_filename.mid(index + 1, this->register_filename.length() - index - 5);
		qDebug() << id;
		this->face_detector_recog.new_registFace(id.toInt(), this->myface, person_name.toStdString());

		QPixmap img(this->register_filename);
		QByteArray bytes;
		QBuffer buffer(&bytes);
		buffer.open(QIODevice::WriteOnly);
		img.save(&buffer, "PNG");
		QVariant imageData(bytes);

		this->psql->prepare("insert into person(s_id, s_name, s_identity, s_imgdata) value(:n, :a, :s, :p)");
		this->psql->bindValue(":n", id.toLongLong());
		this->psql->bindValue(":a", person_name);
		this->psql->bindValue(":s", 1);
		this->psql->bindValue(":p", imageData);
		qDebug() << this->psql->exec();
	}
}

void MainWindow::slotNewConnection() {
	while (mSercer->hasPendingConnections()) {
		basize = 0;
		this->mSocket = mSercer->nextPendingConnection();   // 获取套接字
		connect(this->mSocket, &QTcpSocket::readyRead, this, &MainWindow::slotReadyRead);
	}
}
void MainWindow::slotReadyRead() {
	QByteArray message;
	QDataStream in(this->mSocket);
	in.setVersion(QDataStream::Qt_5_9);
	if (basize == 0) {
		//判断接收的数据是否有两字节（文件大小信息）
		//若是有则保存到basize变量中，没有则返回，继续接收数据
		if (this->mSocket->bytesAvailable() < (int)sizeof(quint64)) {
			return;
		}
		in >> basize;
	}
	//若是没有获得所有数据，则返回继续接收数据
	if (this->mSocket->bytesAvailable() < basize) {
		return;
	}
	in >> message;//将接收到的数据存放到变量中
	ShowImage(message);
}

void MainWindow::ShowImage(QByteArray ba) {
	// 获取数据
	QString ss = QString::fromLatin1(ba.data(), ba.size());
	QByteArray rc;
	rc = QByteArray::fromBase64(ss.toLatin1());
	QByteArray rdc = qUncompress(rc);
	QImage img;
	img.loadFromData(rdc);

	cv::Mat mat;
	QImag2cvMat(img, mat);
	mat.copyTo(this->myface);

	std::vector<std::shared_ptr<faceInfo>> results;
	cv::cvtColor(mat, mat, CV_RGB2BGR); // 转BGR
	this->face_detector_recog.detectFace(mat, results, this->showFPSOption->isChecked());

	int id;
	if (this->matchFaceOption->isChecked())
		this->face_detector_recog.new_matchFace(id, mat, results, false);
	for (auto oneface : results) {
		cv::Rect box;
		box.x = int(oneface->detResult.at<float>(0, 0));
		box.y = int(oneface->detResult.at<float>(0, 1));
		box.width = int(oneface->detResult.at<float>(0, 2));
		box.height = int(oneface->detResult.at<float>(0, 3));
		cv::rectangle(mat, box, cv::Scalar(0, 255, 0), 2, 8, 0);

		// landmark 位置点信息绘画
		if (this->showlandmarkOption->isChecked()) {
			cv::circle(mat, cv::Point2i(int(oneface->detResult.at<float>(0, 4)), int(oneface->detResult.at<float>(0, 5))), 2, cv::Scalar(255, 0, 0));
			cv::circle(mat, cv::Point2i(int(oneface->detResult.at<float>(0, 6)), int(oneface->detResult.at<float>(0, 7))), 2, cv::Scalar(255, 255, 0));
			cv::circle(mat, cv::Point2i(int(oneface->detResult.at<float>(0, 8)), int(oneface->detResult.at<float>(0, 9))), 2, cv::Scalar(255, 0, 255));
			cv::circle(mat, cv::Point2i(int(oneface->detResult.at<float>(0, 10)), int(oneface->detResult.at<float>(0, 11))), 2, cv::Scalar(0, 255, 255));
			cv::circle(mat, cv::Point2i(int(oneface->detResult.at<float>(0, 12)), int(oneface->detResult.at<float>(0, 13))), 2, cv::Scalar(0, 0, 255));
		}
		if (this->matchFaceOption->isChecked()) {
			cv::putText(mat, oneface->name, cv::Point(box.tl()), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 255, 0));
		}

		QString qs = QString::fromStdString(oneface->name);
		QByteArray message = qs.toLocal8Bit();
		if (qs.size() <= 0) {
			if (this->send_count % 5 == 0) {
				this->mSocket->write(qs.toUtf8());
			}
			this->send_count++;
		}
		else {
			this->mSocket->write(qs.toUtf8());
			this->send_count = 0;
		}
	}
	this->canvas->setPixmap(QPixmap::fromImage(QImage(mat.data, mat.cols, mat.rows, mat.step, QImage::Format_RGB888)));

	update();
}

// QImge 转 cvMat
void MainWindow::QImag2cvMat(QImage& im, cv::Mat& ma) {
	switch (im.format()) {
	case QImage::Format_ARGB32:
	case QImage::Format_RGB32:
	case QImage::Format_ARGB32_Premultiplied:
		ma = cv::Mat(im.height(), im.width(), CV_8UC4, (void*)im.constBits(), im.bytesPerLine());
		break;
	case QImage::Format_RGB888:
		ma = cv::Mat(im.height(), im.width(), CV_8UC3, (void*)im.constBits(), im.bytesPerLine());
		break;
	case QImage::Format_Indexed8:
	case QImage::Format_Grayscale8:
		ma = cv::Mat(im.height(), im.width(), CV_8UC1, (void*)im.bits(), im.bytesPerLine());
		break;
	}
}