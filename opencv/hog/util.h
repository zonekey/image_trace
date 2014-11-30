#pragma once

#include <opencv2/opencv.hpp>

// 从 fname 指定的列表文件中, 读取每行的 .jpg, .png 文件名字, 保存到列表
std::vector<std::string> util_load_image_list(const char *fname);
