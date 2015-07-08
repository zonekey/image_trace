#include <stdio.h>
#include <stdlib.h>
#include <opencv2/opencv.hpp>

class Arguments
{
	std::string source_, meta_;
	int width_, height_;

public:
	Arguments(int argc, char **argv)
	{
		source_ = "233.mp4";
		width_ = 480;
		height_ = 270;
		//meta_ = "cascade_12_12/cascade.xml";
		meta_ = "cascade_body_20_30/cascade.xml";
		
		parse(argc, argv);
	}

	const char *source() const { return source_.c_str(); }
	const char *meta() const { return meta_.c_str(); }
	int width() const { return width_; }
	int height() const { return height_; }
	
private:
	int parse(int argc, char **argv)
	{
		int arg = 1;
		while (arg < argc) {
			if (argv[arg][0] == '-') {
				if (!strcmp(&argv[arg][1], "w")) {
					if (arg + 1 < argc) {
						width_ = atoi(argv[arg+1]);
						arg++;
					}
					else {
						fprintf(stderr, "ERR: -w WIDTH\n");
					}
				}
				else if (!strcmp(&argv[arg][1], "h")) {
					if (arg + 1 < argc) {
						height_ = atoi(argv[arg+1]);
						arg++;
					}
					else {
						fprintf(stderr, "ERR: -h HEIGHT\n");
					}
				}
				else if (!strcmp(&argv[arg][1], "meta")) {
					if (arg + 1 < argc) {
						meta_ = argv[arg+1];
						arg++;
					}
					else {
						fprintf(stderr, "ERR: -meta META_FILENAME\n");
					}
				}
			}
			else {
				source_ = argv[arg];
			}

			arg++;
		}

		return 0;
	}
};


int main(int argc, char **argv)
{
	Arguments arg(argc, argv);

	const char *meta = arg.meta();
	const char *src = arg.source();

	fprintf(stderr, "try open %s\n", src);
	cv::VideoCapture cap(src);
	cv::CascadeClassifier cascade;
	if (!cascade.load(meta)) {
		fprintf(stderr, "ERR: can't load cascade classifier file %s\n", meta);
		return -1;
	}

	cv::namedWindow("origin");

	cv::Mat frame;
	for (;;) {
		if (!cap.read(frame)) {
			break;
		}

		cv::resize(frame, frame, cv::Size(arg.width(), arg.height()));

		cv::Mat gray;
		cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

		std::vector<cv::Rect> targets;
		cascade.detectMultiScale(gray, targets);

		for (int i = 0; i < targets.size(); i++) {
			cv::rectangle(frame, targets[i], cv::Scalar(0, 0, 255));
		}

		cv::imshow("origin", frame);

		int key = cv::waitKey(30);
		if (key == 27) {
			break;
		}
	}

	return 0;
}

