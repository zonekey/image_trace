/**
	为了方便标记图像，将用于 caffe 训练

    按 0,1,2,3... 数字键，将会标定当前显示的图像，并且显示下一个 ..
	按 p 将返回上一个，重新标定.
	按 x 将忽略当前的，并且删除文件 ...

 */

#include <stdio.h>
#include <stdlib.h>
#include <opencv2/opencv.hpp>
#include <cc++/file.h>
#include <vector>
#include <string>
#include <algorithm>

static bool is_jpg(const char *fname)
{
	const char *ext = ost::File::getExtension(fname);
	return !strcmp(ext, ".jpg");
}

/** 从 path 中，读取所有 jpg 文件名字 */
static std::vector<std::string> load_all_jpg_fname(const char *path)
{
	std::vector<std::string> fnames;
	try {
		ost::Dir dir(path);

		while (const char *fname = dir++) {
			if (is_jpg(fname)) {
				fnames.push_back(fname);
			}
		}
	}
	catch (...) {}

	return fnames;
}

/** 从 fname 中读取已经标定的文件信息 
  		fname 的文件的格式为： 
			<filename>  <label>
 */
static std::vector<std::string> load_calibrated(const char *fname, 
		std::vector<int> &labels)
{
	std::vector<std::string> fnames;
	labels.clear();

	FILE *fp = fopen(fname, "r");
	if (fp) {
		while (!feof(fp)) {
			char line[1024];
			char *p = fgets(line, sizeof(line), fp);
			if (!p) continue;

			char name[512];
			int label;
			if (sscanf(p, "%s %d\n", name, &label) == 2) {
				fnames.push_back(name);
				labels.push_back(label);
			}
		}
		fclose(fp);
	}

	return fnames;
}

struct op_invector
{
	std::vector<std::string> vec_;

	op_invector(const std::vector<std::string> &vec):vec_(vec) {}

	bool operator()(const std::string &fname) const
	{
		for (size_t i = 0; i < vec_.size(); i++) {
			if (fname == vec_[i])
				return true;
		}
		return false;
	}
};

/** 从 jpgs 中删除已经标定的文件 */
static void remove_caled(std::vector<std::string> &jpgs, 
		const std::vector<std::string> &caled)
{
	op_invector op(caled);
	jpgs.erase(std::remove_if(jpgs.begin(), jpgs.end(), op), jpgs.end());
}

/** 显示图像，并返回标定结果 */
static int cal_img(const char *path, const char *fname, bool show_label, int last_label)
{
	std::string name = path; name += "/"; name += fname;
	cv::Mat img;

	img = cv::imread(name);
	if (img.cols == 0 || img.rows == 0) {
		return -1;
	}

	cv::putText(img, fname, cv::Point(0, 40), cv::FONT_HERSHEY_PLAIN, 2.0, cv::Scalar(0, 0, 255), 2);

	if (show_label) {
		char buf[16];
		snprintf(buf, sizeof(buf), "%d", last_label);
		cv::putText(img, buf, cv::Point(10, 100), cv::FONT_HERSHEY_PLAIN, 4.0, cv::Scalar(0, 0, 255), 2);
	}

	cv::imshow("image", img);
	int key = cv::waitKey(0);
	return key;
}

/** 保存到 train.txt */
void save_caled(const char *train_txt, const std::vector<std::string> &fnames, 
	const std::vector<int> &labels)
{
	FILE *fp = fopen(train_txt, "w");
	if (fp) {
		for (size_t i = 0; i < fnames.size(); i++) {
			fprintf(fp, "%s    %d\n", fnames[i].c_str(), labels[i]);
		}

		fclose(fp);
	}
}

int main(int argc, char **argv)
{
	const char *img_path = "./train";
	const char *train_txt = "train.txt";
	bool verify = false;

	if (argc == 2 && !strcmp(argv[1], "-v")) {
		verify = true;
	}

	fprintf(stderr, "....\n");

	std::vector<std::string> jpgs = load_all_jpg_fname(img_path);
	if (jpgs.empty()) {
		fprintf(stderr, "ERR: None jpgs loaded from %s\n", img_path);
		return -1;
	}

	fprintf(stderr, "There are %u jpg files\n", jpgs.size());

	std::vector<int> labels;
	std::vector<std::string> fnames = load_calibrated(train_txt, labels); 
	fprintf(stderr, "There are %u caled\n", fnames.size());

	remove_caled(jpgs, fnames);
	fprintf(stderr, "There are %u NEED to been calibrate\n", jpgs.size());

	cv::namedWindow("image");

	bool quit = false, show_label = false;
	int last_label = -1;

	for (size_t i = 0; !quit && i < jpgs.size(); i++) {
		int key = cal_img(img_path, jpgs[i].c_str(), show_label, last_label);
		if (key == -1) {
			continue;
		}

		show_label = false;

		switch (key) {
		case 'q':
		case 'Q':
			quit = true;
			break;

		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			fnames.push_back(jpgs[i]);
			labels.push_back(key - 0x30);	// 保存人数 ...
			break;

		case 'd':
			fprintf(stderr, "WARNING: remove file %s\n", jpgs[i].c_str());
			{
				std::stringstream ss;
				ss << img_path << '/' << jpgs[i];

				remove(ss.str().c_str());
			}
			break;	//

		case 0x250000: // left arrow
			if (i > 0) {
				i--;
				fnames.pop_back();
				last_label = labels.back();
				labels.pop_back();
				show_label = true;
			}
			i--;
			break;

		default:
			fprintf(stderr, "WARNING: unknown key=%02x(%c)\n", key, key);
			i--;
			break;
		}
	}

	save_caled(train_txt, fnames, labels);

	return 0;
}
