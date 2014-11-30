#include "util.h"
#include <stdio.h>

static bool is_image_filename(const char *s)
{
	const char *exts [] = {
		".jpg", ".png",
		0,
	};

	const char *ext = exts[0];
	while (ext) {
		if (strstr(s, ext)) {
			return true;
		}
		ext++;
	}

	return false;
}

std::vector<std::string> util_load_image_list(const char *fname)
{
	std::vector<std::string> fl;

	FILE *fp = fopen(fname, "r");
	if (!fp) {
		fprintf(stderr, "ERR: %s: can't open %s\n", __FUNCTION__, fname);
		return fl;
	}

	while (!feof(fp)) {
		char buf[256];
		char *p = fgets(buf, sizeof(buf), fp);
		if (!p) continue;
		char *q = strchr(p, '\r');
		if (q) *q = 0;
		q = strchr(p, '\n');
		if (q) *q = 0;

		if (is_image_filename(p)) {
			fl.push_back(p);
		}
	}
	fclose(fp);
	return fl;
}

