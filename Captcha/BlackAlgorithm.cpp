#include "Captcha.h"
#include "Classes.h"

const int imgW = 83;
const int imgH = 23;

const int blackFactor = 2;

Mat BlackAlgorithm::preprocessing(Mat inputImg)
{

/*	imshow("input", inputImg);
	waitKey();
	*/

	int rImgH = imgH * blackFactor;
	int rImgW = imgW * blackFactor;

	Mat mapX(Size(rImgW, rImgH), CV_32FC1);

	for (int i = 0; i < mapX.rows; ++i)
		for (int j = 0; j < mapX.cols; ++j)
			mapX.at<float>(i, j) = (float)j;

	double	t = 4.0, // top offset
		scale1 = 1.0,//2.3
		step1 = 25.0,//27.0
		h = 16.0, // character height
		stretch = rImgW * 10.0, //10.0
		ft = t * blackFactor,
		fscale1 = scale1 * blackFactor,
		fh = h * blackFactor,
		fstep1 = pi / (step1 * blackFactor);

	Mat mapY(Size(rImgW, rImgH), CV_32FC1);

	for (int i = 0; i < mapY.rows; ++i)
		for (int j = 0; j < mapY.cols; ++j)
		{
			double q = fscale1 * (1 + cos(fstep1 * j));
			mapY.at<float>(i, j) = ((i - ft) * ((fh - q) / fh) + q) * (1 + j / stretch) + ft;
		}

	Mat outputImg(Size(rImgW, rImgH), CV_8UC1);
	
	//масштабирование (увеличение в два раза)
	resize(inputImg, outputImg, Size(rImgW, rImgH), blackFactor, blackFactor, CV_INTER_CUBIC);

	threshold(outputImg, outputImg, 200, 255, CV_THRESH_BINARY_INV);

	//избавляемся от волнового эффекта
	remap(
		outputImg, outputImg,
		mapX, mapY,
		CV_INTER_CUBIC,
		BORDER_CONSTANT,
		cvScalarAll(0));

	//imshow("remaped input", outputImg);
	//waitKey();

	threshold(outputImg, outputImg, 120, 255, CV_THRESH_BINARY);

	//imshow("threshold input", outputImg);
	//waitKey();

	Mat element = getStructuringElement(MORPH_ELLIPSE, Size(2, 2), Point(-1, -1));
	dilate(outputImg, outputImg, element);
	erode(outputImg, outputImg, element);
	
	//__________________________________

	//рисуем черные линии для разделения символов
	vector<int> numBlackPixInColUp(outputImg.cols);
	vector<int> numBlackPixInColDown(outputImg.cols);
	for (int j = 0; j < outputImg.cols; ++j)
	{
		numBlackPixInColUp[j] = 0;
		for (int i = 0; i < outputImg.rows; ++i)
		{
			Vec3b colour = outputImg.at<Vec3b>(i, j);
			if (colour[0] == 0)
				++numBlackPixInColUp[j];
			else
				break;
		}
		numBlackPixInColDown[j] = 0;
		for (int i = outputImg.rows - 1; i >= 0; --i)
		{
			Vec3b colour = outputImg.at<Vec3b>(i, j);
			if (colour[0] == 0)
				++numBlackPixInColDown[j];
			else
				break;
		}
	}

	int thresholdHeight = outputImg.rows;
	int thr = thresholdHeight / 2 - 1;
	for (int j = 0; j < outputImg.cols; ++j)
	{
		if (numBlackPixInColUp[j] >= thr || numBlackPixInColDown[j] >= thr)
		{
			line(outputImg, Point(j, 0), Point(j, thresholdHeight - 1), CV_RGB(0, 0, 0), 1);
		}
	}

	//imshow("lines", outputImg);
	//waitKey();

	//_____________________________

	vector<Mat> segments = segmentate(outputImg);

	/*for (int i = 0; i < segments.size(); ++i)
	{
		imshow("seg", segments[i]);
		waitKey();
		destroyWindow("seg");
	}*/

	return outputImg;
}

vector<Mat> BlackAlgorithm::segmentate(Mat inputImg)
{
	Mat image, imageWithComponents, HSV;
	inputImg.copyTo(image);
	inputImg.copyTo(imageWithComponents);

	vector<Vec3b> colors;
	unsigned int cnt = 255;
	cvtColor(imageWithComponents, HSV, CV_BGR2HSV);
	for (int y = 0; y < imageWithComponents.cols; y++) {
		for (int x = 0; x < imageWithComponents.rows; x++) {
			Vec3b color = imageWithComponents.at<Vec3b>(x, y);
			if (color[0] != 0 || color[1] != 0 || (color[2] < cnt && color[2] != 0)) {
				floodFill(imageWithComponents, Point(y, x), CV_RGB(cnt, 0, 0));
				color[0] = 0; color[1] = 0; color[2] = cnt--;
				colors.push_back(color);
			}
		}
	}

	vector<Point> symbol;
	vector<RotatedRect> rotatedRectangles;
	vector<Rect> rectangles;
	vector<Mat> segments;
	for (unsigned int k = 0; k < colors.size(); ++k) {
		symbol.clear();
		for (int x = 0; x < imageWithComponents.rows; x++) {
			for (int y = 0; y < imageWithComponents.cols; y++)
				if (imageWithComponents.at<Vec3b>(x, y)[2] == colors[k][2])
					symbol.push_back(Point(y, x));
		}
		if (symbol.size() > 60) {
			RotatedRect rRect = minAreaRect(symbol);
			rotatedRectangles.push_back(rRect);

			Rect rect = boundingRect(symbol);
			rectangles.push_back(rect);
		}
	}
	for (unsigned int i = 0; i < rectangles.size(); ++i) {
		RotatedRect rRect = rotatedRectangles[i];
		Rect rect = rectangles[i];

		Mat segment;
		image(rect).copyTo(segment);
		
		//Mat element = getStructuringElement(MORPH_ELLIPSE, Size(2, 2), Point(-1, -1));
		//erode(segment, segment, element);
		//dilate(segment, segment, element);
		
		segments.push_back(segment);
	}

	//фильтрация сегментов и разрезания
	//если большой сегмент
	vector<Mat> FinalSegments;
	int averageSegWidth = (inputImg.cols - 20) / 6;

	for (unsigned int i = 0; i < segments.size(); ++i)
	{
		Mat curSeg = segments[i];
		if ((curSeg.cols > averageSegWidth / 4) && (curSeg.rows > averageSegWidth / 4))
		{
			//segment which has 2 or three symbols
			//if (curSeg.cols > averageSegWidth + averageSegWidth / 4 && curSeg.cols < averageSegWidth * 2.0)
			if (curSeg.cols > averageSegWidth * 2.0)
			{
				int halfW = curSeg.cols / 3;
				int halfH = curSeg.rows;

				FinalSegments.push_back(curSeg(Rect(0, 0, halfW, halfH))); //left part
				FinalSegments.push_back(curSeg(Rect(halfW, 0, halfW - 1, halfH))); //right part
				FinalSegments.push_back(curSeg(Rect(2 * halfW, 0, halfW - 1, halfH))); //right part
			}
			else
			{
				if (curSeg.cols > averageSegWidth + averageSegWidth / 4)
				{
					int halfW = curSeg.cols / 2;
					int halfH = curSeg.rows;

					FinalSegments.push_back(curSeg(Rect(0, 0, halfW, halfH))); //left part
					FinalSegments.push_back(curSeg(Rect(halfW, 0, halfW - 1, halfH))); //right part
				}
				else
					FinalSegments.push_back(curSeg);
			}
			
		}
	}

	for (unsigned int i = 0; i < FinalSegments.size(); ++i)
	{
		Mat temp;
		FinalSegments[i].copyTo(temp);
		cvtColor(temp, temp, CV_BGR2GRAY);
		int rImgW = 118;
		int rImgH = 227;
		resize(temp, temp, Size(rImgW, rImgH));
		threshold(temp, temp, 40, 255.0, THRESH_BINARY_INV);
		temp.copyTo(FinalSegments[i]);
	}

	return FinalSegments;
}

string BlackAlgorithm::recognizeSegments(vector<Mat> segments)
{
	return NeuronNetByEtalons::recognizeSegments(segments, "black");
}

string BlackAlgorithm::recognize(Mat inputImg)
{
	return recognizeSegments(segmentate(preprocessing(inputImg)));
}