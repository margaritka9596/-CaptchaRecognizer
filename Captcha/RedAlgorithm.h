#include "Captcha.h"

class RedAlgorithm
{
public:
	vector<Mat> preprocessing(Mat inputImg);
	string recognizeSegments(vector<Mat> segments);
	string recognize(Mat inputImg);
};
