#include "Captcha.h"

int getEllipseCount(Mat inputImg);
bool topEllipse(Mat inputImg);
bool bottomEllipse(Mat inputImg);

class BlueAlgorithm
{
public:
	Mat preprocessing(Mat inputImg);
	vector<Mat> segmentate(Mat inputImg);
	string recognizeSegments(vector<Mat> segments);
	string recognize(Mat inputImg);
};
