#include "Captcha.h"
#include "PreprocessingForNeuronNet.h"

Mat changeBackground(Mat inputImg)
{
	for (int i = 0; i < inputImg.cols; ++i)
		for (int j = 0; j < inputImg.rows; ++j)
		{

			uchar &color = inputImg.at<uchar>(j, i);
			if (color == 255)
				color = 0;
			else
				color = 255;
		}

	return inputImg;
}

vector<Mat> PreprocessingForNeuronNet::ResizeAndChangeBackground(vector<Mat> inputSegments)
{
	vector<Mat> result;

	for (int i = 0; i < inputSegments.size(); ++i)
	{
		Mat inputImg;
		inputSegments[i].copyTo(inputImg);

		cvtColor(inputImg, inputImg, CV_BGR2GRAY);
		//inputImg = changeBackground(inputImg);
		threshold(inputImg, inputImg, 0, 255, THRESH_BINARY_INV);

		int rImgW = 118;
		int rImgH = 227;
		Size dsize(rImgW, rImgH);

		Mat outputImg;
		resize(inputImg, outputImg, dsize);

		result.push_back(outputImg);
	}

	return result;
}