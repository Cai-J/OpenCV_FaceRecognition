#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "opencv2/opencv.hpp"
#include "QLabel"
#include "QTcpSocket"

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow();

	qint64 blockSize;

private slots:
	void timer_Update();
	void readyRead_Slot();

private:
	Ui::MainWindow *ui;

	QLabel* canvas = new QLabel();
	QLabel* textcanvas = new QLabel();

	QTimer* time_calendar;
	QImage* qimage;
	cv::VideoCapture cap;
	cv::Mat src_image;

	// tcp
	QTcpSocket* mSocket;
	QString unk = QString::fromLocal8Bit("Unknown");

	int count;  // 计数，累计后处理
};

#endif // MAINWINDOW_H
