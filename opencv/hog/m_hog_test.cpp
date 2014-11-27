#include <iostream>
#include <opencv2/opencv.hpp>
#include <fstream>

#define SVM_V "svm.v.txt"

static std::vector<float> load_svm_v(const char *fname)
{
	std::vector<float> v;
	FILE *fp = fopen(fname, "r");
	if (!fp) {
		fprintf(stderr, "ERR: can't load SVM data from %s\n", fname);
		exit(-1);
	}
	while (!feof(fp)) {
		char line[128];
		if (fgets(line, sizeof(line), fp)) {
			v.push_back(atof(line));
		}
	}
	fclose(fp);

	return v;
}

static std::vector<cv::Rect> det(cv::Mat &img)
{
	std::vector<cv::Rect> faces;
	std::vector<float> v = load_svm_v(SVM_V);
	cv::HOGDescriptor hog(cv::Size(64, 64), cv::Size(16, 16), cv::Size(8, 8), cv::Size(8, 8), 9);
	hog.setSVMDetector(v);
	hog.detectMultiScale(img, faces);
	return faces;
}

int main(int argc, char **argv)
{
	if (argc < 2) {
		fprintf(stderr, "usage: %s <pic fname>\n", argv[0]);
		return -1;
	}

	cv::Mat img = cv::imread(argv[1]);
	if (!img.data) {
		fprintf(stderr, "ERR: can't open pic %s\n", argv[1]);
		return -1;
	}

	cv::resize(img, img, cv::Size(480, 270));

	cv::namedWindow("main");
	std::vector<cv::Rect> faces = det(img);
	std::cout << faces.size() << " faces detected!" << std::endl;

	for (int i = 0; i < faces.size(); ++i) {
		cv::rectangle(img, faces[i], cv::Scalar(0, 0, 255), 2);
	}

	cv::imshow("main", img);

	cv::waitKey(0);

	return 0;
}
