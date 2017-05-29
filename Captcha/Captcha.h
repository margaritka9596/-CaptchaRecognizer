#include <iostream>
#include <fstream>
#include <vector>
#include <dirent.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <cmath>
#include <math.h>

#define TEXTFILE "result.txt"

using namespace std;
using namespace cv;

typedef pair<Mat, string> captcha;