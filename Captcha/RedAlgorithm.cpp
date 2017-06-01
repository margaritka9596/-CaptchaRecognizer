#include "Captcha.h"
#include "Classes.h"

const int imgW = 83;
const int imgH = 23;

const int redFactor = 2;
const double pi = 3.14159265358;

Mat RedAlgorithm::preprocessing(Mat inputImg)
{
	int rImgH = imgH * redFactor;
	int rImgW = imgW * redFactor;

	Mat mapX(Size(rImgW, rImgH), CV_32FC1);

	for (int i = 0; i < mapX.rows; ++i)
		for (int j = 0; j < mapX.cols; ++j)
			mapX.at<float>(i, j) = (float)j;

	double	t = 4.0, // top offset
		scale1 = 2.3,
		step1 = 27.0,
		h = 16.0, // character height
		stretch = rImgW * 10.0,
		ft = t * redFactor,
		fscale1 = scale1 * redFactor,
		fh = h * redFactor,
		fstep1 = pi / (step1 * redFactor);

	Mat mapY(Size(rImgW, rImgH), CV_32FC1);

	for (int i = 0; i < mapY.rows; ++i)
		for (int j = 0; j < mapY.cols; ++j)
		{
			double q = fscale1 * (1 + cos(fstep1 * j));
			mapY.at<float>(i, j) = ((i - ft) * ((fh - q) / fh) + q) * (1 + j / stretch) + ft;
		}

	Mat outputImg(Size(rImgW, rImgH), CV_8UC1);

	//масштабирование (увеличение в два раза)
	resize(inputImg, outputImg, Size(rImgW, rImgH), redFactor, redFactor, CV_INTER_CUBIC);
	//imshow("scaled input", outputImg);

	//избавляемся от волнового эффекта немного
	remap(
		outputImg, outputImg,
		mapX, mapY,
		CV_INTER_CUBIC,
		BORDER_CONSTANT,
		cvScalarAll(0));

	//бинаризация
	threshold(outputImg, outputImg, 200, 255, CV_THRESH_BINARY);
	//imshow("afterThreshold", outputImg);

	//закрашиваем мелкие связнаые области < 10px  (дырки внутри символов)
	int little_spaces = 4;
	Mat tip_spacing1 = getStructuringElement(MORPH_ELLIPSE, Size(little_spaces, little_spaces), Point(-1, -1));

	dilate(outputImg, outputImg, tip_spacing1);
	//imshow("afterDilate", outputImg); 

	Mat tip_spacing2 = getStructuringElement(MORPH_ELLIPSE, Size(little_spaces + 1, little_spaces + 1), Point(-1, -1));
	erode(outputImg, outputImg, tip_spacing1);
	//imshow("afterErode", outputImg);

	//imwrite("outputImg.jpg", outputImg);

	//Разделение символов
	//cout << "InputImg|num cols = " << outputImg.cols << endl;
	//cout << "InputImg|num rows = " << outputImg.rows << endl;

	vector<int> numWhitePixInCol(outputImg.cols);
	for (int j = 0; j < outputImg.cols; ++j)
	{
		numWhitePixInCol[j] = 0;
		for (int i = 0; i < outputImg.rows; ++i)
		{
			Vec3b colour = outputImg.at<Vec3b>(i, j);
			if (colour[0] == 255)
				++numWhitePixInCol[j];
		}
	}

	int maxHeightInPixels = *max_element(numWhitePixInCol.begin(), numWhitePixInCol.end());
	//cout << "max height of segment = " << maxHeightInPixels << endl;
	for (int j = 0; j < outputImg.cols; ++j)
		if (numWhitePixInCol[j] >= 1 && numWhitePixInCol[j] <= maxHeightInPixels / 5)
			for (int i = 0; i < outputImg.rows; ++i)
			{
				Vec3b color;
				color[0] = color[1] = color[2] = 0;
				outputImg.at<Vec3b>(i, j) = color;
			}

	//imshow("1", outputImg);
	//imwrite("outputImgSegmented.jpg", outputImg);

	return outputImg;
}

vector<Mat> RedAlgorithm::segmentate(Mat img)
{
	Mat image;
	img.copyTo(image);

	vector<Vec3b> colors;
	Mat imageWithComponents;
	image.copyTo(imageWithComponents);
	unsigned int cnt = 0;
	for (int x = 0; x < imageWithComponents.rows; ++x)
		for (int y = 0; y < imageWithComponents.cols; ++y)
		{
			Vec3b color = imageWithComponents.at<Vec3b>(x, y);
			if (color[0] == 255 && color[1] == 255 && color[2] == 255)
			{
				floodFill(imageWithComponents, Point(y, x), CV_RGB(10 * cnt + 10, 0, 0));
				color[0] = 0; color[1] = 0; color[2] = 10 * cnt++ + 10;
				colors.push_back(color);
			}
		}

	vector<Point> symbol;
	vector<Mat> segments;
	Mat imageForShowRect;

	for (unsigned int k = 0; k < colors.size(); ++k)
	{
		symbol.clear();
		for (int x = 0; x < imageWithComponents.rows; ++x)
			for (int y = 0; y < imageWithComponents.cols; ++y)
			{
				Vec3b color = imageWithComponents.at<Vec3b>(x, y);
				if (color[2] == colors[k][2])
					symbol.push_back(Point(y, x));
			}

		RotatedRect rRect = minAreaRect(symbol);
		Point2f vertices[4];
		rRect.points(vertices);

		Rect rect = boundingRect(symbol);

		///not necessary
		if (k == 0)
			image.copyTo(imageForShowRect);
		rectangle(imageForShowRect, rect, CV_RGB(255, 0, 0));
		//imshow("segmented", imageForShowRect); waitKey(); destroyWindow("segmented");

		Mat segment;
		image(rect).copyTo(segment);
		segments.push_back(segment);
	}

/*	for (int i = 0; i < segments.size(); ++i)
	{
		imshow("seg"+ i, segments[i]);
	} 
*/	//cout << "size before filter" << Segments.size() << endl;

	//фильтрация и коррекция сегментов
	vector<Mat> FinalSegments;
	int averageSegWidth = image.cols / 5;

	for (unsigned int i = 0; i < segments.size(); ++i)
	{
		Mat curSeg = segments[i];
		if ((curSeg.cols > averageSegWidth / 4) && (curSeg.rows > averageSegWidth / 4))
		{
			//segment which has 2 or three symbols
			if (curSeg.cols > averageSegWidth + averageSegWidth / 4 && curSeg.cols < averageSegWidth * 2.0)
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

		//imshow("Finalseg" + to_string(i), FinalSegments[i]); waitKey(); destroyWindow("Finalseg" + to_string(i));

		string ext = ".jpg";
		string z = "try\\SegmentNum_" + to_string(i) + ext;
		const char* z1 = z.c_str();
		//imwrite(z1, FinalSegments[i]);
	}
	//cout << "size after filter" << FinalSegments.size() << endl;

	return FinalSegments;
}

string RedAlgorithm::recognizeSegments(vector<Mat> segments)
{
	return NeuronNetByEtalons::recognizeSegments(segments, "red");
}

string RedAlgorithm::recognize(Mat inputImg)
{
	return recognizeSegments(segmentate(preprocessing(inputImg)));
}