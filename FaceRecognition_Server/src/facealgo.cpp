#include "facealgo.h"

FaceAlgo::FaceAlgo()
{
	std::cout << "create instance" << std::endl;
}
FaceAlgo::~FaceAlgo()
{
}

void FaceAlgo::db_initFaceModels(std::string detect_model_path, std::string recog_model_path) {
	faceDetector = cv::FaceDetectorYN::create(detect_model_path, "", cv::Size(300, 300), 0.9f, 0.3f, 500);
	faceRecognizer = cv::FaceRecognizerSF::create(recog_model_path, "");
}
void FaceAlgo::detectFace(cv::Mat &image, std::vector<std::shared_ptr<faceInfo>> &infoList, bool showFPS) {
	cv::TickMeter tm;
	std::string msg = "FPS: ";
	tm.start();
	// Set input size before inference
	faceDetector->setInputSize(image.size());

	// Inference
	cv::Mat faces;
	faceDetector->detect(image, faces);
	tm.stop();
	// Draw results on the input image
	int thickness = 2;
	for (int i = 0; i < faces.rows; i++)
	{
		// Draw bounding box
		auto fi = std::shared_ptr<faceInfo>(new faceInfo());
		fi->name = "Unknown";
		faces.row(0).copyTo(fi->detResult);
		infoList.push_back(fi);
	}
	if (showFPS) {
		putText(image, msg + std::to_string(tm.getFPS()), cv::Point(15, 25), cv::FONT_HERSHEY_SIMPLEX, 1.0, cv::Scalar(0, 0, 255), thickness);
	}
}
void FaceAlgo::new_matchFace(int& id, cv::Mat &frame, std::vector<std::shared_ptr<faceInfo>> &infoList, bool l2) {
	double cosine_similar_thresh = 0.363;
	double l2norm_similar_thresh = 1.128;
	for (auto face : infoList) {
		cv::Mat aligned_face, feature;
		faceRecognizer->alignCrop(frame, face->detResult, aligned_face);
		faceRecognizer->feature(aligned_face, feature);
		double min_dist = 100.0;
		double max_cosine = 0.0;
		std::string matchedName = "Unknown";
		int match_id = 0;
		for (auto item : face_big_db) {
			if (l2) {
				double L2_score = faceRecognizer->match(feature, item.second.second, cv::FaceRecognizerSF::DisType::FR_NORM_L2);
				if (L2_score < min_dist) {
					min_dist = L2_score;
					matchedName = item.second.first;
					match_id = item.first;
				}
			}
			else {
				double cos_score = faceRecognizer->match(feature, item.second.second, cv::FaceRecognizerSF::DisType::FR_COSINE);
				if (cos_score > max_cosine) {
					max_cosine = cos_score;
					matchedName = item.second.first;
					match_id = item.first;
				}
			}
		}

		if (max_cosine > cosine_similar_thresh) {
			face->name.clear();
			face->name.append(matchedName);
			id = match_id;
		}
		if (l2 && min_dist < l2norm_similar_thresh) {
			face->name.clear();
			face->name.append(matchedName);
			id = match_id;
		}
	}
}

// 带id的注册
void FaceAlgo::new_registFace(int id, cv::Mat &frame, std::string name) {
	faceDetector->setInputSize(frame.size());
	cv::Mat faces;
	faceDetector->detect(frame, faces);
	cv::Mat aligned_face, feature;
	faceRecognizer->alignCrop(frame, faces.row(0), aligned_face);
	faceRecognizer->feature(aligned_face, feature);
	face_big_db.insert(std::pair<int, std::pair<std::string, cv::Mat>>(id, std::pair<std::string, cv::Mat>(name, feature.clone())));
	std::cout << "new_registFace insert:\t" << "id: " << id << "\t Name: " << name << std::endl;
}

