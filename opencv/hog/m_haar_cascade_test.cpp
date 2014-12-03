#include <stdio.h>
#include <stdlib.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
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

struct Ctx
{
    cv::Mat frame, orig;    // orig 为原始图像 clone
    bool pressed; // 鼠标按下
    cv::Point first_pt, last_pt;
    int count;
    std::vector<cv::Rect> faces;    // 探测到的矩形 ..
};

static void draw_recv_xor(cv::Mat &img, const cv::Rect &rc)
{
    /** 逆时针四条线 */
    cv::LineIterator 
        l1(img, rc.tl(), cv::Point(rc.x, rc.y+rc.height)),
        l2(img, cv::Point(rc.x, rc.y+rc.height), rc.br()),
        l3(img, rc.br(), cv::Point(rc.x+rc.width, rc.y)),
        l4(img, cv::Point(rc.x+rc.width, rc.y), rc.tl());

    // 使用 ...
    for (int i = 0; i < l1.count; i++, ++l1) {
        uchar *p = *l1;
        *(p) ^= 0, *(p+1) ^= 255, *(p+2) ^= 0;
    }

    for (int i = 0; i < l2.count; i++, ++l2) {
        uchar *p = *l2;
        *(p) ^= 0, *(p+1) ^= 255, *(p+2) ^= 0;
    }

    for (int i = 0; i < l3.count; i++, ++l3) {
        uchar *p = *l3;
        *(p) ^= 0, *(p+1) ^= 255, *(p+2) ^= 0;
    }

    for (int i = 0; i < l4.count; i++, ++l4) {
        uchar *p = *l4;
        *(p) ^= 0, *(p+1) ^= 255, *(p+2) ^= 0;
    }
}


static void draw_rubber_lines(struct Ctx *ctx, int x, int y)
{
    /** 如果 first_pt != last_pt，需要首先擦出 */
    if (ctx->first_pt.x != ctx->last_pt.x && ctx->first_pt.y != ctx->last_pt.y) {
        draw_recv_xor(ctx->frame, cv::Rect(ctx->first_pt, ctx->last_pt));
    }

    ctx->last_pt = cv::Point(x, y);

    if (ctx->first_pt.x != ctx->last_pt.x && ctx->first_pt.y != ctx->last_pt.y) {
        draw_recv_xor(ctx->frame, cv::Rect(ctx->first_pt, ctx->last_pt));
    }
}

static time_t _begin = time(0);

static bool in_rect(const cv::Rect &rc, int x, int y)
{
    return x > rc.x && x < rc.x + rc.width && y > rc.y && y < rc.y + rc.height;
}

static void mouse_callback(int ev, int x, int y, int flags, void *p)
{
    /** 在显示窗口中，通过鼠标，画出检测错的的区域，保存到 neg/ 目录中
     */
    struct Ctx *ctx = (struct Ctx*)p;

    switch (ev) {
        case cv::EVENT_RBUTTONDBLCLK:
            for (std::vector<cv::Rect>::const_iterator it = ctx->faces.begin(); 
                 it != ctx->faces.end(); ++it) {
                if (in_rect(*it, x, y)) {
                    // 说明这个误检！
                    std::stringstream fname;
                    fname << "neg/hard_neg_" << _begin << ctx->count++ << ".jpg";
                    cv::Mat pic = cv::Mat(ctx->orig, *it);

                    cv::imwrite(fname.str(), pic);
                    std::cout << fname.str() << " saved" << std::endl;
                }
            }
            break;

        case cv::EVENT_LBUTTONDOWN:
            ctx->pressed = true;
            ctx->first_pt = cv::Point(x, y);
            ctx->last_pt = ctx->first_pt;
            fprintf(stderr, "left button pressed!\n");
            break;

        case cv::EVENT_LBUTTONUP:
            if (ctx->pressed) {
                // TODO:
                ctx->pressed = false;

                cv::Rect roi = cv::Rect(ctx->first_pt, cv::Point(x, y));
                if (roi.area() < 100) {
                    std::cout << " toooooooooo small area" << std::endl;
                }
                else {
                    std::stringstream fname;
                    fname << "pos/hard_pos_" << _begin << ctx->count++ << ".jpg";
                    cv::Mat pic = cv::Mat(ctx->orig, roi);

                    cv::resize(pic, pic, cv::Size(64, 64));

                    cv::imwrite(fname.str(), pic);
                    std::cout << fname.str() << " saved" << std::endl;
                }
            }
            break;

        case cv::EVENT_MOUSEMOVE:
            if (ctx->pressed) {
                draw_rubber_lines(ctx, x, y);
                cv::imshow("main", ctx->frame);
            }
            break;
    }
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

    struct Ctx ctx;
    ctx.pressed = false;
    ctx.frame = img;
    ctx.orig = img.clone();
    ctx.count = 0;

	cv::CascadeClassifier cc(_cas_fname);
	std::vector<cv::Rect> rcs;

    cv::resize(img, img, cv::Size(480*1.5, 270*1.5));

    double scale_factor = 1.1;
    int min_neighbors = 3;
    cv::Size minSize(0, 0);
    cv::Size maxSize(80, 80);

	double t1 = util_now();
	cc.detectMultiScale(img, rcs, scale_factor, min_neighbors, 0, minSize, maxSize);
	double t2 = util_now();
    ctx.faces = rcs;

	fprintf(stderr, "using %.3f seconds!\n", t2 - t1);

	cv::namedWindow("main");
    cv::setMouseCallback("main", mouse_callback, &ctx);

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


