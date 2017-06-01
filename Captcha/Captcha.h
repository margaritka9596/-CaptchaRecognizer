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

//class Recognizer 
//{
//public:
//	virtual Mat preprocess(Mat) = 0;
//	virtual vector<Mat> segmentate(Mat) = 0;
//	virtual string recognizeSegment(Mat) = 0;
//	virtual string recognize(Mat) = 0;
//};
