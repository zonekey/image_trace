//
//  main.cpp
//  opencv_det
//
//  Created by 孙玮 on 14/11/25.
//  Copyright (c) 2014年 孙玮. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <cc++/file.h>
#include <string>
#include <opencv2/highgui/highgui.hpp>
#include <sstream>
#include <sys/stat.h>

#define WIDTH 24
#define HEIGHT 24

struct Context
{
    cv::Mat current_frame;
    
    bool mouse_pressed;
    cv::Point first_pt, second_pt;
    
	bool is_neg;	// 是否指针负样本.
    unsigned int sample_cnt;

	std::string dname; // path of raws images

	std::vector<std::string> files;	// 逐次显示的图像文件.
	cv::VideoCapture cap;	// 对应视频文件.

	std::vector<std::string>::const_iterator it_file;
};

static void rotateMe(const cv::Mat &src, cv::Mat &dst, const cv::Rect rc, int angle)
{
	/** 将 src 以 rc 中心为圆心，旋转 angle 度，然后再截取 rc 大小的图像，保存
	  	到 dst 中.

		矩形旋转后，
	 */

	float radian = angle * CV_PI / 180.0;
	float rotated_w = rc.width * fabs(cos(radian)) + rc.height * fabs(sin(radian));

	cv::Mat rotated = cv::Mat::zeros(src.cols, src.rows, src.type());
	cv::Point center(rc.x + rc.width / 2, rc.y + rc.height / 2);
//	double scale = rc.width / rotated_w;
	double scale = 1.0;

	cv::Mat rotate_mat = cv::getRotationMatrix2D(center, angle, scale);
	cv::warpAffine(src, rotated, rotate_mat, src.size());

	dst = cv::Mat(rotated, rc);
}

static void mouse_callback(int event, int x, int y, int flags, void* userdata)
{
    struct Context *ctx = (struct Context*)userdata;
    
    bool &pressed = ctx->mouse_pressed;
    
    switch (event) {
        case cv::EVENT_LBUTTONDOWN:
            pressed = true;
            ctx->first_pt = cv::Point(x, y);
            ctx->second_pt = ctx->first_pt;
            break;
            
        case cv::EVENT_LBUTTONUP:
            if (pressed) {
                pressed = false;
                
                ctx->second_pt = cv::Point(x, y);
                cv::Rect rc(ctx->first_pt, ctx->second_pt);
                if (rc.area() < 100) {
                    std::cout << "the rect area tooooo small!" << std::endl;
                }
                else if (rc.width /2 > rc.height || rc.height / 2 > rc.width) {
                    std::cout << "the rect is tooooo flat!" << std::endl;
                }
                else {
					// 不能变形:
					if (rc.width < rc.height) {
						rc.x -= (rc.height - rc.width) / 2;
						if (rc.x < 0) {
							rc.x = 0;
						}
						rc.width = rc.height;
					}
					else {
						rc.y -= (rc.width - rc.height) / 2;
						if (rc.y < 0) {
							rc.y = 0;
						}
						rc.height = rc.width;
					}

					// 但也不能越界 :)
					cv::Rect t(0, 0, ctx->current_frame.cols, ctx->current_frame.rows);
					rc &= t;

					// 拉伸为 WIDTH * HEIGHT
                    cv::Mat m(ctx->current_frame, rc), m2;
					cv::resize(m, m2, cv::Size(WIDTH, HEIGHT));

                    std::stringstream ss;
					ss << (ctx->is_neg ? "neg/neg_" : "pos/pos_") << ctx->sample_cnt++ << ".jpg";
                    cv::imwrite(ss.str(), m2);

					cv::flip(m2, m2, 1);	// 左右反转 .
                    std::stringstream ss2;
					ss2 << (ctx->is_neg ? "neg/neg_" : "pos/pos_") << ctx->sample_cnt++ << ".jpg";
                    cv::imwrite(ss2.str(), m2);

					// 顺时针旋转 30 度，每隔3度 
					for (int angle = 2; angle <= 30 && !ctx->is_neg; angle += 3) {
						cv::Mat rp;
						rotateMe(ctx->current_frame, rp, rc, angle);
						if (rp.cols != WIDTH || rp.rows != HEIGHT) {
							cv::resize(rp, rp, cv::Size(WIDTH, HEIGHT));
						}

						std::stringstream ss;
						ss << (ctx->is_neg ? "neg/neg_" : "pos/pos_") << ctx->sample_cnt++ << ".jpg";
						cv::imwrite(ss.str(), rp);

						cv::flip(rp, rp, 1);	// 左右反转 .
						std::stringstream ss2;
						ss2 << (ctx->is_neg ? "neg/neg_" : "pos/pos_") << ctx->sample_cnt++ << ".jpg";
						cv::imwrite(ss2.str(), rp);
					}

					if (ctx->is_neg) {
						char fname[128];

						resize(m2, m2, cv::Size(WIDTH*2, HEIGHT*2));

						// XXX: 
						cv::flip(m2, m2, 0); // 上下翻转.
						sprintf(fname, "neg/neg_%u.jpg", ctx->sample_cnt++);
						cv::imwrite(fname, m2);
						
						cv::flip(m2, m2, 1); // 左右.
						sprintf(fname, "neg/neg_%u.jpg", ctx->sample_cnt++);
						cv::imwrite(fname, m2);

						cv::transpose(m2, m2);
						sprintf(fname, "neg/neg_%u.jpg", ctx->sample_cnt++);
						cv::imwrite(fname, m2);

						cv::flip(m2, m2, 1);
						sprintf(fname, "neg/neg_%u.jpg", ctx->sample_cnt++);
						cv::imwrite(fname, m2);

						cv::flip(m2, m2, -1);
						sprintf(fname, "neg/neg_%u.jpg", ctx->sample_cnt++);
						cv::imwrite(fname, m2);
					}

					std::cout << ss.str() << " saved!" << std::endl;
                }
            }
            break;
            
        case cv::EVENT_MOUSEMOVE:
            if (pressed) {
				cv::Scalar color = cv::Scalar(0, 0, 255);
				if (ctx->is_neg) {
					color = cv::Scalar(0, 255, 0);
				}

                ctx->second_pt = cv::Point(x, y);
				cv::Rect rc(ctx->first_pt, ctx->second_pt);
				cv::rectangle(ctx->current_frame, rc, color);
				cv::imshow("main", ctx->current_frame);
            }
            break;
            
        default:
            break;
    }
}

static int parse_args(int argc, const char **argv, Context *ctx)
{
	int arg = 1;

	/**
		-neg
		-cnt_begin

	 */

	while (arg < argc) {
		if (argv[arg][0] == '-') {
			if (!strcmp(&argv[arg][1], "neg")) {
				// -neg 制作负样本 .
				ctx->is_neg = true;
			}
			else if (!strcmp(&argv[arg][1], "cnt_begin")) {
				// -cnt_begin <N> 命名的 ..
				if (arg + 1 <= argc) {
					ctx->sample_cnt = atoi(argv[arg+1]);
					arg++;
				}
				else {
					fprintf(stderr, "ERR: -cnt_begin N\n");
					return -1;
				}
			}
		}
		else {
			ctx->dname = argv[arg];
		}

		arg++;
	}

	return 0;
}

static bool is_imgfile(const char *fname)
{
	const char *ext = ost::File::getExtension(fname);
	return !strcmp(ext, ".png") || !strcmp(ext, ".jpg") || !strcmp(ext, ".jpeg");
}

static std::vector<std::string> load_files(const char *dname)
{
	std::vector<std::string> fnames;
	ost::Dir dir(dname);
	const char *name = dir++;
	while (name) {
		if (is_imgfile(name)) {
			char fname[260];
			snprintf(fname, sizeof(fname), "%s/%s", dname, name);

			fnames.push_back(fname);
		}

		name = dir++;
	}

	return fnames;
}

static cv::Mat next_frame(Context *ctx)
{
	if (!ctx->files.empty()) {
		if (ctx->it_file != ctx->files.end()) {
			std::string f = *ctx->it_file;
			++ctx->it_file;
			fprintf(stderr, "INFO: load %s\n", f.c_str());

			cv::Mat m = cv::imread(f);
			if (m.cols) {
				cv::resize(m, m, cv::Size(480, 270));
				return m;
			}
			else {
				fprintf(stderr, "ERR: load %s err\n", f.c_str());
			}

			return m;
		}
		else {
			fprintf(stderr, "INFO: no more images\n");
			return cv::Mat();
		}
	}
	else {
		cv::Mat frame;
		if (ctx->cap.read(frame)) {
			cv::resize(frame, frame, cv::Size(480, 270));
			return frame;
		}
		else {
			return cv::Mat();
		}
	}
}

int main(int argc, const char * argv[])
{
    Context ctx;
    ctx.sample_cnt = 0;
	ctx.mouse_pressed = false;
	ctx.is_neg = false;

	if (parse_args(argc, argv, &ctx) < 0) {
		return -1;
	}

	if (ctx.dname.empty()) {
		fprintf(stderr, "ERR: MUST input image_path\n");
		return -1;
	}

	struct stat st;
	if (stat(ctx.dname.c_str(), &st) < 0) {
		fprintf(stderr, "ERR: can't stat %s\n", ctx.dname.c_str());
		return -2;
	}

	if (st.st_mode & S_IFDIR) {
		ctx.files = load_files(ctx.dname.c_str());
		ctx.it_file = ctx.files.begin();
		fprintf(stderr, "INFO: load %u samples\n", ctx.files.size());
	}
	else {
		if (!ctx.cap.open(ctx.dname)) {
			fprintf(stderr, "ERR: can't open VideoCapture using %s\n", ctx.dname.c_str());
			return -1;
		}
	}

	cv::namedWindow("main");
	bool quit = false;

    cv::setMouseCallback("main", mouse_callback, &ctx);

	cv::Mat pic = next_frame(&ctx);
	if (pic.cols == 0) {
		return -1;
	}

	ctx.current_frame = pic.clone();

	while (!quit) {
		if (ctx.mouse_pressed) {
			ctx.current_frame = pic.clone();
		}

		if (!ctx.mouse_pressed) {
			cv::imshow("main", ctx.current_frame);
		}
        
		int key = cv::waitKey(30);
        if (key == 27) {
			fprintf(stderr, "key: ESC\n");
            quit = true;
        }
		else if (key == 32) {
			pic = next_frame(&ctx);
			ctx.current_frame = pic.clone();
			ctx.mouse_pressed = false;
		}
	}


#if 0
	std::vector<std::string> files = load_files(ctx.dname.c_str());
	if (files.empty()) {
		fprintf(stderr, "ERR: NO image files\n");
		return -1;
	}

	if (ctx.is_neg) {
		mkdir("neg", 0775);
	}
	else {
		mkdir("pos", 0775);
	}

    cv::namedWindow("main");
	bool quit = false;
    
    cv::setMouseCallback("main", mouse_callback, &ctx);
	cv::Mat pic;

	while (pic.cols == 0 && !files.empty()) {
		pic = cv::imread(files.back());
		files.pop_back();

		if (pic.cols > 0) break;
		else {
			fprintf(stderr, "ERR: read img %s err\n", files.back().c_str());
		}

	}

	if (pic.cols >= 1800) {
		cv::resize(pic, pic, cv::Size(960, 540));
	}

	if (pic.cols > 0) {
		ctx.current_frame = pic.clone();
	}

	while (pic.cols > 0 && !quit) {
		if (ctx.mouse_pressed) {
			ctx.current_frame = pic.clone();
		}

		if (!ctx.mouse_pressed) {
			// FIXME: 如果鼠标按下时，可能导致覆盖 ..
			cv::imshow("main", ctx.current_frame);
		}
        
		int key = cv::waitKey(30);
        if (key == 27) {
			fprintf(stderr, "key: ESC\n");
            quit = true;
        }
		else if (key == 32) {
			fprintf(stderr, "key: SPACE\n");
			pic.release();
			pic.cols = 0;
			while (pic.cols == 0 && !files.empty()) {
				pic = cv::imread(files.back());
				files.pop_back();

				if (pic.cols > 0) break;
				else {
					quit = true;
					fprintf(stderr, "ERR: read img %s err\n", files.back().c_str());
				}
			}

			if (pic.cols >= 1800) {
				cv::resize(pic, pic, cv::Size(960, 540));
			}

			ctx.current_frame = pic.clone();
			ctx.mouse_pressed = false;
		}
    }
#endif

    return 0;
}
