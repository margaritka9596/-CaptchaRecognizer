#include "Captcha.h"

class BlackAlgorithm //: Recognizer
{
public:
	Mat preprocessing(Mat inputImg);
	vector<Mat> segmentate(Mat inputImg);
	string recognizeSegments(vector<Mat> segments);
	string recognize(Mat inputImg);
};