#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCheckBox>
#include <QLabel>
#include "facealgo.h"
#include "QTcpServer"
#include "QTcpSocket"
#include "QSqlDatabase"

namespace Ui {
	class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow();

	void ShowImage(QByteArray ba);
	void QImag2cvMat(QImage& im, cv::Mat& ma);

	int send_count = 0;

private slots:
	void slotNewConnection();
	void slotReadyRead();
	void face_register();
	void begin_register();

private:
	QCheckBox* matchFaceOption;
	QCheckBox* showlandmarkOption;
	QCheckBox* showFPSOption;

	QLabel* canvas = new QLabel();
	FaceAlgo face_detector_recog;
	cv::Mat myface;

	// tcp
	QTcpServer* mSercer;
	QTcpSocket* mSocket;
	quint64 basize;

	// database
	QSqlDatabase database;
	QSqlQuery*	psql;

	// temp register
	QString register_filename;  // 注册人照片位置
	QString person_name;		// 注册人姓名

private:
	Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
