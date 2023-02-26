#ifndef FACEALGO_H
#define FACEALGO_H

#include <iostream>
#include "opencv2/opencv.hpp"
#include "opencv2/dnn.hpp"

struct faceInfo {
	std::string name;
	cv::Mat detResult;
};

class FaceAlgo
{
public:
	FaceAlgo();
	~FaceAlgo();

	void detectFace(cv::Mat &frame, std::vector<std::shared_ptr<faceInfo>> &results, bool showFPS);
	void db_initFaceModels(std::string detect_model_path, std::string recog_model_path);
	void new_registFace(int id, cv::Mat& faceRoi, std::string name);
	void new_matchFace(int& id, cv::Mat &frame, std::vector<std::shared_ptr<faceInfo>> &results, bool l2 = false);

private:
	std::map<std::string, cv::Mat> face_models;
	cv::Ptr<cv::FaceDetectorYN> faceDetector;
	cv::Ptr<cv::FaceRecognizerSF> faceRecognizer;

	std::map<int, std::pair<std::string, cv::Mat>> face_big_db;
};

#endif // FACEALGO_H
