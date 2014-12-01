#include <stdio.h>
#include <stdlib.h>
#include <opencv2/opencv.hpp>
#include <sys/time.h>

static std::string _cas_fname = "cascade/cascade.xml";
static std::string _input_fname;

static double util_now()
{
	struct timeval tv;
	gettimeofday(&tv, 0);
	return tv.tv_sec + tv.tv_usec / 1000000.0;
}

static int parse_args(int argc, char **argv)
{
	int arg = 1;
	while (arg < argc) {
		if (argv[arg][0] == '-') {
			switch (argv[arg][1]) {
				case 'c':
					if (arg+1 <= argc) {
						_cas_fname = argv[arg+1];
						arg++;
					}
					else {
						fprintf(stderr, "ERR: -c MUST follow cascade file name\n");
						return -1;
					}
					break;
			}
		}
		else {
			_input_fname = argv[arg];
		}

		arg++;
	}

	if (_input_fname.empty()) {
		fprintf(stderr, "ERR: NO input image file name\n");
		return -1;
	}

	return 0;
}

int main(int argc, char **argv)
{
	if (parse_args(argc, argv) < 0) {
		return -1;
	}

	std::cout << "using " << _cas_fname << " to detect " << _input_fname << std::endl;

	cv::Mat img = cv::imread(_input_fname);
	if (img.cols == 0 || img.rows == 0) {
		std::cout << "ERR: load " << _input_fname << std::endl;
		return -1;
	}

	cv::CascadeClassifier cc(_cas_fname);
	std::vector<cv::Rect> rcs;

	double t1 = util_now();
	cc.detectMultiScale(img, rcs);
	double t2 = util_now();

	fprintf(stderr, "using %.3f seconds!\n", t2 - t1);

	cv::namedWindow("main");
	for (std::vector<cv::Rect>::const_iterator it = rcs.begin(); it != rcs.end(); ++it) {
		cv::rectangle(img, *it, cv::Scalar(0, 0, 255));
	}
	
	cv::imshow("main", img);
	
	int key = 0;
	while (key != 27) { // ESC
		key = cv::waitKey(0);
	}

	return 0;
}


