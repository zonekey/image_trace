//
//  main.cpp
//  opencv_det
//
//  Created by 孙玮 on 14/11/25.
//  Copyright (c) 2014年 孙玮. All rights reserved.
//

#include <iostream>
#include <opencv2/opencv.hpp>
#include <string>
#include <opencv2/nonfree/nonfree.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <sstream>
#include "util.h"
#include <sys/stat.h>

struct Context
{
    cv::Mat current_frame;
    
    bool mouse_pressed;
    cv::Point first_pt, second_pt;
    
	bool is_neg;	// 是否指针负样本.
    unsigned int sample_cnt;

	std::string input_fname; // 输入是图片列表文件.
};

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

					// 拉伸为 64x64
                    cv::Mat m(ctx->current_frame, rc), m2;
					cv::resize(m, m2, cv::Size(64, 64));

                    std::stringstream ss;
					ss << (ctx->is_neg ? "neg/neg_" : "pos/pos_") << ctx->sample_cnt++ << ".jpg";
                    cv::imwrite(ss.str(), m2);

					cv::flip(m2, m2, 1);	// 左右反转 .
                    std::stringstream ss2;
					ss2 << (ctx->is_neg ? "neg/neg_" : "pos/pos_") << ctx->sample_cnt++ << ".jpg";
                    cv::imwrite(ss2.str(), m2);

					if (ctx->is_neg) {
						char fname[128];

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
			ctx->input_fname = argv[arg];
		}

		arg++;
	}

	return 0;
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

	if (ctx.input_fname.empty()) {
		fprintf(stderr, "ERR: MUST input image_list_file_name\n");
		return -1;
	}

	std::vector<std::string> files = util_load_image_list(ctx.input_fname.c_str());
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

	if (pic.cols > 1920) {
		cv::resize(pic, pic, cv::Size(800, 600));
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

			if (pic.cols > 1920) {
				cv::resize(pic, pic, cv::Size(800, 600));
			}

			ctx.current_frame = pic.clone();
			ctx.mouse_pressed = false;
		}
    }

    return 0;
}
