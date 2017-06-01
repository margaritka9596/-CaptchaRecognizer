#include <iostream>
#include <fstream>
#include <vector>
#include <dirent.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <cmath>
#include <math.h>

#define TRAINSETPATH "trainset\\"
#define ETALONSPATH "ForNeuronNet\\"
#define TEXTFILENAME "result_"

using namespace std;
using namespace cv;

typedef pair<Mat, string> captcha;

vector<captcha> getCaptcha(string path, string ext);
/*
class Recognizer 
{
	Recognizer();
	
	virtual Mat preprocess(Mat);
	virtual vector<Mat> segmentate(Mat);
	virtual string recognizeSegment(Mat);
	virtual string recognize(Mat);
};
*/
