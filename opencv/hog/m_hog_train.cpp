#include <stdio.h>
#include <stdlib.h>
#include <opencv2/opencv.hpp>
#include <iostream>

#if 0
#else
static size_t lines_of_file(const char *fname)
{
	size_t lines = 0;
	FILE *fp = fopen(fname, "r");
	if (fp) {
		char buf[256];
		while (!feof(fp)) {
			char *p = fgets(buf, sizeof(buf), fp);
			if (p && (strstr(p, ".jpg") || strstr(p, ".png"))) {
				lines++;
			}
		}
		fclose(fp);
	}

	return lines;
}

class MySvm : public cv::SVM
{
public:  
    int get_alpha_count()  
    {  
        return this->sv_total;  
    }  
  
    int get_sv_dim()  
    {  
        return this->var_all;  
    }  
  
    int get_sv_count()  
    {  
        return this->decision_func->sv_count;  
    }  
  
    double* get_alpha()  
    {  
        return this->decision_func->alpha;  
    }  
  
    float** get_sv()  
    {  
        return this->sv;  
    }  
  
    float get_rho()  
    {  
        return this->decision_func->rho;  
    }  
};

int main(int argc, char **argv)
{
	const char *posi_file = "posi.txt", *nega_file = "nega.txt";

	size_t pos_samples = lines_of_file(posi_file);
	size_t neg_samples = lines_of_file(nega_file);

	/**
		size_t HOGDescriptor::getDescriptorSize() const
		{
			CV_Assert(blockSize.width % cellSize.width == 0 &&
				blockSize.height % cellSize.height == 0);
			CV_Assert((winSize.width - blockSize.width) % blockStride.width == 0 &&
				(winSize.height - blockSize.height) % blockStride.height == 0 );
			return (size_t)nbins*
				(blockSize.width/cellSize.width)*
				(blockSize.height/cellSize.height)*
				((winSize.width - blockSize.width)/blockStride.width + 1)*
				((winSize.height - blockSize.height)/blockStride.height + 1);
		}
	*/

	int desc_size = 9 * (16 / 8) * (16 / 8) * ((64 - 16) / 8 + 1) * ((64 - 16 ) / 8 + 1); // 1764

	cv::Mat samples_feature(pos_samples + neg_samples, desc_size, CV_32FC1); // Ñù±¾ÌØÕ÷.

	cv::Mat samples_label(pos_samples + neg_samples, 1, CV_32FC1);		// 

	std::cout << "There are " << pos_samples << " positive samples, and " << neg_samples << " negative samples." << std::endl;

	std::cout << "begin pos:";

	int samples = 0;

	FILE *fp = fopen(posi_file, "r");
	while (!feof(fp)) {
		char buf[256];
		char *p = fgets(buf, sizeof(buf), fp);
		if (!p || (!strstr(p, ".jpg") && !strstr(p, ".png"))) {
			continue;
		}

		char *r = strchr(p, '\r');
		if (r) {
			*r = 0;
		}
		
		r = strchr(p, '\n');
		if (r) {
			*r = 0;
		}

		cv::Mat img = cv::imread(p);
		if (!img.data) {
			std::cout << 'x';
			continue;
		}

		cv::resize(img, img, cv::Size(64, 64));

		cv::HOGDescriptor hogdesc(cv::Size(64,64), cv::Size(16,16), cv::Size(8,8), cv::Size(8,8), 9);
		std::vector<float> featureVec;
		hogdesc.compute(img, featureVec, cv::Size(8, 8));
		assert(featureVec.size() == desc_size);
		for (size_t i = 0; i < featureVec.size(); i++) {
			float *row = samples_feature.ptr<float>(samples);
			row[i] = featureVec[i];
		}

		float *rd = samples_label.ptr<float>(samples);
		*rd = 1.0;

		std::cout << "+";

		samples++;
	}
	fclose(fp);

	std::cout << " end!" << std::endl;

	std::cout << "begin neg ";
	fp = fopen(nega_file, "r");
	while (!feof(fp)) {
		char buf[256];
		char *p = fgets(buf, sizeof(buf), fp);
		if (!p || (!strstr(p, ".jpg") && !strstr(p, ".png"))) {
			continue;
		}

		char *r = strchr(p, '\r');
		if (r) {
			*r = 0;
		}
		
		r = strchr(p, '\n');
		if (r) {
			*r = 0;
		}

		cv::Mat img = cv::imread(p);
		if (!img.data) {
			std::cout << "x";
			continue;
		}

		cv::resize(img, img, cv::Size(64, 64));

		cv::HOGDescriptor hogdesc(cv::Size(64,64), cv::Size(16,16), cv::Size(8,8), cv::Size(8,8), 9);
		std::vector<float> featureVec;
		hogdesc.compute(img, featureVec, cv::Size(8, 8));
		for (size_t i = 0; i < featureVec.size(); i++) {
			float *row = samples_feature.ptr<float>(samples);
			row[i] = featureVec[i];
		}

		float *rd = samples_label.ptr<float>(samples);
		*rd = -1.0;

		std::cout << "-";

		samples++;
	}
	fclose(fp);
	std::cout << " end!" << std::endl;

	std::cout << " begin SVM train ..";
	cv::SVMParams params;
	params.svm_type = CvSVM::C_SVC;
	params.kernel_type = CvSVM::LINEAR;
	params.term_crit = cvTermCriteria(CV_TERMCRIT_ITER, 100, 1e-6);

	MySvm svm;
	svm.train_auto(samples_feature, samples_label, cv::Mat(), cv::Mat(), params, 5);
	svm.save("svm.result.yaml");
	std::cout << "  end!" << std::endl;

	samples_feature.release();
	samples_label.release();

	int supportvs = svm.get_support_vector_count();
	std::cout << "support vector size of SVM: " << supportvs << std::endl;

	//
	cv::Mat sv = cv::Mat::zeros(supportvs, desc_size, CV_32FC1);
	cv::Mat alp(1, supportvs, CV_32FC1);
	cv::Mat re = cv::Mat::zeros(1, desc_size, CV_32FC1);
	cv::Mat res(1, 1, CV_32FC1);

	for (int i = 0; i < supportvs; i++) {
		const float *s = svm.get_support_vector(i);
		for (int j = 0; j < desc_size; j++) {
			float *d = sv.ptr<float>(i, j);
			*d = s[j];
		}
	}

	double *alpha = svm.get_alpha();
	int alpha_cnt = svm.get_alpha_count();

	for (int i = 0; i < supportvs; i++) {
		float *p = alp.ptr<float>(i);
		*p = alpha[i];
	}

	re = alp * sv;
	re *= -1.0;

	FILE *fp2 = fopen("svm.v.txt", "w");
	for (int i = 0; i < desc_size; i++) {
		fprintf(fp2, "%f \n", re.at<float>(i));
	}
	fclose(fp2);

	float rho = svm.get_rho();
	FILE *fp3 = fopen("svm.r.txt", "w");
	fprintf(fp3, "%f", rho);
	fclose(fp);


	return 0;
}
#endif
