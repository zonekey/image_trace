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

#define WIDTH 16
#define HEIGHT 16

class Context
{
public:
	Context()
	{
		width = WIDTH, height = HEIGHT;
		mouse_pressed = false;
		sample_cnt = 0;
		out = "pos";
	}

    cv::Mat current_frame;
    bool mouse_pressed;
    cv::Point first_pt, second_pt;
    
    unsigned int sample_cnt;

	std::string dname; // path of raws images

	std::vector<std::string> files;	// 逐次显示的图像文件.
	cv::VideoCapture cap;	// 对应视频文件.

	std::vector<std::string>::const_iterator it_file;

	int width, height;
	std::string out;	// to save output samples
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
                if (rc.area() < 144) {
                    std::cout << "the rect area tooooo small!" << std::endl;
                }
                else if (rc.width /2 > rc.height || rc.height / 2 > rc.width) {
                    std::cout << "the rect is tooooo flat!" << std::endl;
                }
                else {
					cv::Mat m2;
					if (1) {
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
            	        cv::Mat m(ctx->current_frame, rc);
						cv::resize(m, m2, cv::Size(ctx->width, ctx->height));
					}

                    std::stringstream ss;
					ss << ctx->out << "/" << ctx->sample_cnt++ << ".jpg";
                    cv::imwrite(ss.str(), m2);

					cv::flip(m2, m2, 1);	// 左右反转 .
                    std::stringstream ss2;
					ss2 << ctx->out << "/" << ctx->sample_cnt++ << ".jpg";
                    cv::imwrite(ss2.str(), m2);

					// 顺时针旋转 30 度，每隔3度 
					for (int angle = 2; angle <= 15; angle += 2) {
						cv::Mat rp;
						rotateMe(ctx->current_frame, rp, rc, angle);
						if (rp.cols != ctx->width || rp.rows != ctx->height) {
							cv::resize(rp, rp, cv::Size(ctx->width, ctx->height));
						}

						std::stringstream ss;
						ss << ctx->out << '/' << ctx->sample_cnt++ << ".jpg";
						cv::imwrite(ss.str(), rp);

						cv::flip(rp, rp, 1);	// 左右反转 .
						std::stringstream ss2;
						ss2 << ctx->out << '/' << ctx->sample_cnt++ << ".jpg";
						cv::imwrite(ss2.str(), rp);
					}

					std::cout << ss.str() << " saved!" << std::endl;
                }
            }
            break;
            
        case cv::EVENT_MOUSEMOVE:
            if (pressed) {
				cv::Scalar color = cv::Scalar(0, 0, 255);
                ctx->second_pt = cv::Point(x, y);
				cv::Rect rc(ctx->first_pt, ctx->second_pt);
				cv::rectangle(ctx->current_frame, rc, color);
				char buf[64];
				snprintf(buf, sizeof(buf), "%d-%d", rc.width, rc.height);
				cv::putText(ctx->current_frame, buf, cv::Point(0, 45), cv::FONT_HERSHEY_PLAIN, 2.0, cv::Scalar(0, 0, 255));
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
		-cnt_begin <base cnt>
		-w <width>
		-h <height>
		-out <output path>
	 */

	while (arg < argc) {
		if (argv[arg][0] == '-') {
			if (!strcmp(&argv[arg][1], "cnt_begin")) {
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
			else if (!strcmp(&argv[arg][1], "out")) {
				// -out <pos path>
				if (arg + 1 <= argc) {
					ctx->out = argv[arg+1];
					arg++;
				}
				else {
					fprintf(stderr, "ERR: -out <output path>\n");
					return -1;
				}
			}
			else if (!strcmp(&argv[arg][1], "w")) {
				// width
				if (arg + 1 <= argc) {
					ctx->width = atoi(argv[arg+1]);
					arg++;
				}
				else {
					fprintf(stderr, "ERR: -w WIDTH\n");
					return -1;
				}
			}
			else if (!strcmp(&argv[arg][1], "h")) {
				// height
				if (arg + 1 <= argc) {
					ctx->height = atoi(argv[arg+1]);
					arg++;
				}
				else {
					fprintf(stderr, "ERR: -h HEIGHT\n");
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

static cv::Mat next_frame(Context *ctx, bool &ok, size_t &cnt)
{
	static size_t _cnt = 0;

	if (!ctx->files.empty()) {
		if (ctx->it_file != ctx->files.end()) {
			std::string f = *ctx->it_file;
			++ctx->it_file;
			fprintf(stderr, "INFO: load %s\n", f.c_str());

			cv::Mat m = cv::imread(f);
			if (m.cols) {
				cv::resize(m, m, cv::Size(720, 405));
				ok = true;
				_cnt++;
				cnt = _cnt;
				return m;
			}
			else {
				ok = false;
				cnt = _cnt;
				fprintf(stderr, "ERR: load %s err\n", f.c_str());
				return m;
			}
		}
		else {
			fprintf(stderr, "INFO: no more images\n");
			ok = false;
			cnt = _cnt;
			return cv::Mat();
		}
	}
	else {
		cv::Mat frame;
		if (ctx->cap.read(frame)) {
			cv::resize(frame, frame, cv::Size(720, 405));
			_cnt++;
			cnt = _cnt;
			ok = true;
			return frame;
		}
		else {
			ok = false;
			cnt = _cnt;
			return cv::Mat();
		}
	}
}

int main(int argc, const char * argv[])
{
    Context ctx;
    ctx.sample_cnt = 0;
	ctx.mouse_pressed = false;

	if (parse_args(argc, argv, &ctx) < 0) {
		return -1;
	}

	if (ctx.dname.empty()) {
		fprintf(stderr, "ERR: MUST input image_path\n");
		fprintf(stderr, "usage: %s [-w WIDTH] [-h HEIGHT] [-cnt_begin BASE(def 0)] source [-out PATH(def 'pos')]\n", argv[0]);
		return -1;
	}

	struct stat st;
	if (stat(ctx.out.c_str(), &st) < 0) {
		fprintf(stderr, "WARNING: %s not exist, just create it\n", 
				ctx.out.c_str());
		mkdir(ctx.out.c_str(), S_ISUID|S_IRWXU);
	}
	else {
		if (!S_ISDIR(st.st_mode)) {
			fprintf(stderr, "ERR: %s is NOT a DIR\n",
					ctx.out.c_str());
			return -1;
		}
	}

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

	bool ok;
	size_t cnt;
	cv::Mat pic = next_frame(&ctx, ok, cnt);
	if (!ok) {
		for (int i = 0; i < 10 && !ok; i++) {
			pic = next_frame(&ctx, ok, cnt);
		}

		if (!ok) {
			fprintf(stderr, "ERR: load first frame err!!!!\n");
			return -1;
		}
	}

	ctx.current_frame = pic.clone();

	while (!quit) {
		if (ctx.mouse_pressed) {
			ctx.current_frame = pic.clone();
		}

		if (!ctx.mouse_pressed) {
			cv::imshow("main", ctx.current_frame);
		}
        
		int key = cv::waitKey(40);
        if (key == 27) {
			fprintf(stderr, "key: ESC\n");
            quit = true;
        }
		else if (key == 32) {
			pic = next_frame(&ctx, ok, cnt);
			if (!ok) {
				fprintf(stderr, "ERR: load frame err, just exit!\n");
				quit = true;
				continue;
			}
			char fs[16];
			snprintf(fs, sizeof(fs), "%u", cnt); 
			cv::putText(pic, fs, cv::Point(0, 30), cv::FONT_HERSHEY_PLAIN, 2.0, cv::Scalar(0, 0, 255));
			ctx.current_frame = pic.clone();
			ctx.mouse_pressed = false;
		}
		else if (key == 0xff56) {
			// page down
			for (int i = 0; i < 10; i++) {
				pic = next_frame(&ctx, ok, cnt);
				if (!ok) {
					fprintf(stderr, "ERR: load frame err, just exit!\n");
					quit = true;
					break;
				}
			}
			if (!quit) {
				char fs[16];
				snprintf(fs, sizeof(fs), "%u", cnt); 
				cv::putText(pic, fs, cv::Point(0, 30), cv::FONT_HERSHEY_PLAIN, 2.0, cv::Scalar(0, 0, 255));
				ctx.current_frame = pic.clone();
				ctx.mouse_pressed = false;
			}
		}
		else if (key != -1) {
			fprintf(stderr, "key=%x\n", key);
		}
	}

    return 0;
}
