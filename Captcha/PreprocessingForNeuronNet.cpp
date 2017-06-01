#include "Captcha.h"
#include "PreprocessingForNeuronNet.h"

vector<Mat> PreprocessingForNeuronNet::ResizeAndChangeBackground(vector<Mat> inputSegments)
{
	vector<Mat> result;

	for (unsigned int i = 0; i < inputSegments.size(); ++i)
	{
		Mat inputImg, outputImg;
		inputSegments[i].copyTo(inputImg);

		cvtColor(inputImg, inputImg, CV_BGR2GRAY);

		//TODO
		threshold(inputImg, inputImg, 0.0, 255.0, THRESH_BINARY_INV);

		int rImgW = 118;
		int rImgH = 227;
		resize(inputImg, outputImg, Size(rImgW, rImgH));

		result.push_back(outputImg);
	}

	return result;
}