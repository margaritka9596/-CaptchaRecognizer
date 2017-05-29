#include "Captcha.h"

typedef pair<Mat, string> captcha;

class NeuronNetByEtalons
{
public:
	static string recognizeSegments(vector<Mat> segments, string algoritmName);
};

vector<captcha> getCaptcha(string path, string ext);
