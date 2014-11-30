/** 输入图片，随机从中选取指定大小的负样本 

  		./m_neg_maker input.jpg -w 20 -h 20 -n 300 -p neg10_
		从 input.jpg 中，随机生成 20x20 的 300 张图片，并使用 neg10_N.jpg 进行命名
		
		TODO：在生成过程中，考虑对图片进行左右旋转 ？？
 */

#include <stdio.h>
#include <stdio.h>
#include <opencv2/opencv.hpp>

struct Params
{
	std::string input_pic_fname;
	int width, height;
	int count;
	std::string prefix;
};

static Params parse_args(int argc, char **argv)
{
	Params params;
	params.width = 20;
	params.height = 20;
	params.count = 300;
	params.prefix = "neg10_";	// 缺省前缀.

	int arg = 1;

	while (arg < argc) {
		if (argv[arg][0] == '-') {
			switch (argv[arg][1]) {
				case 'w':
					if (arg+1 < argc) {
						params.width = atoi(argv[arg+1]);
						arg++;
					}
					else {
						fprintf(stderr, "ERR: %s: -w N\n", __FUNCTION__);
						exit(-1);
					}
					break;

				case 'h':
					if (arg+1 < argc) {
						params.height = atoi(argv[arg+1]);
						arg++;
					}
					else {
						fprintf(stderr, "ERR: %s: -h N\n", __FUNCTION__);
						exit(-1);
					}
					break;

				case 'n':
					if (arg+1 < argc) {
						params.count = atoi(argv[arg+1]);
						arg++;
					}
					else {
						fprintf(stderr, "ERR: %s: -n Count\n", __FUNCTION__);
						exit(-1);
					}
					break;

				case 'p':
					if (arg+1 < argc) {
						params.prefix = argv[arg+1];
						arg++;
					}
					else {
						fprintf(stderr, "ERR: %s: -p PREFIX\n", __FUNCTION__);
						exit(-1);
					}
					break;
			}
		}
		else {
			params.input_pic_fname = argv[arg];
		}

		arg++;
	}

	return params;
}

int main(int argc, char **argv)
{
	Params params = parse_args(argc, argv);
	if (params.input_pic_fname.empty()) {
		return -1;
	}

	return 0;
}

