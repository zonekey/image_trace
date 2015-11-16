/** 读取 val.txt，将 train/ 目录下的对应的文件，移动到 val/ 目录中
 */
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <algorithm>

struct Cal {
	std::string fname;
	int label;
};

typedef std::vector<Cal> CALS;

static CALS load_cals(const char *fname)
{
	CALS cals;

	FILE *fp = fopen(fname, "r");
	if (fp) {
		while (!feof(fp)) {
			char line[1024];
			char *p = fgets(line, sizeof(line), fp);
			if (!p) continue;

			char name[512];
			int label;
			if (sscanf(p, "%s %d\n", name, &label) == 2) {
				Cal cal;
				cal.fname = name;
				cal.label = label;
				
				cals.push_back(cal);
			}
		}
		fclose(fp);
	}

	return cals;
}

int main(int argc, char **argv)
{
	const char *val_txt = "val.txt", *train_txt = "train.txt";
	const char *td = "train/", *vd = "val/";

	do {
		CALS cals = load_cals(val_txt);
		for (size_t i = 0; i < cals.size(); i++) {
			char src[1024], dst[1024];
			sprintf(src, "%s%s", vd, cals[i].fname.c_str());
			sprintf(dst, "%s%s", td, cals[i].fname.c_str());

			rename(src, dst);
		}
	} while (0);

	CALS cals = load_cals(train_txt), cals_val = load_cals(val_txt);
	cals.insert(cals.end(), cals_val.begin(), cals_val.end());

	std::random_shuffle(cals.begin(), cals.end());	// 乱序

	// 4/5 用于训练，1/5 用于测试
	size_t cnt = cals.size() * 4 / 5;

	FILE *fp = fopen(train_txt, "w");
	for (size_t i = 0; i < cnt; i++) {
		fprintf(fp, "%s %d\n", cals[i].fname.c_str(), cals[i].label);
	}
	fclose(fp);

	fp = fopen(val_txt, "w");
	for (size_t i = cnt; i < cals.size(); i++) {
		fprintf(fp, "%s %d\n", cals[i].fname.c_str(), cals[i].label);
	}
	fclose(fp);

	// 根据 val.txt 移动文件 
	cals = load_cals(val_txt);
	for (size_t i = 0; i < cals.size(); i++) {
		char src[1024], dst[1024];
		sprintf(src, "%s%s", td, cals[i].fname.c_str());
		sprintf(dst, "%s%s", vd, cals[i].fname.c_str());

		rename(src, dst);
	}

	return 0;
}
