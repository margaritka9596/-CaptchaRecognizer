#include "Captcha.h"

int getEllipseCount(Mat inputImg);

class BlueAlgorithm
{
public:
	Mat Preprocessing(Mat inputImg);
	vector<Mat> Segmentate(Mat inputImg);
	string RecognizeSegments(vector<Mat> segments);
	string Recognize(Mat segments);
};
