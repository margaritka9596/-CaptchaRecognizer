#include "Captcha.h"
#include "Classes.h"

int getEllipseCount(Mat inputImg)
{
	Mat img;
	if (inputImg.channels() != 1)
		cvtColor(inputImg, img, CV_BGR2GRAY);
	else
		inputImg.copyTo(img);

	Mat image(img.rows + 2, img.cols + 2, img.type(), Scalar::all(255));
	img.copyTo(image(Rect(1, 1, img.cols, img.rows)));

	threshold(image, image, 128, 255, CV_THRESH_BINARY);
	cvtColor(image, image, CV_GRAY2BGR);

	int count = -1;
	for (int x = 0; x < image.rows; ++x)
		for (int y = 0; y < image.cols; ++y)
		{
			Vec3b color = image.at<Vec3b>(x, y);
			if (color[0] == 255 && color[1] == 255 && color[2] == 255)
			{
				floodFill(image, Point(y, x), CV_RGB(255, 0, 0));
				++count;
			}
		}

	return count;
}

Mat BlueAlgorithm::Preprocessing(Mat inputImg)
{
	Mat image, HSV, colorImage;

	inputImg.copyTo(image);
	cvtColor(image, HSV, CV_BGR2HSV);
	for (int x = 0; x < image.rows; x++) {
		for (int y = 0; y < image.cols; y++) {
			Vec3b hsv = HSV.at<Vec3b>(x, y);
			if (hsv[2] < 140)
				image.at<Vec3b>(x, y) = Vec3b(255, 255, 255);
			else
				image.at<Vec3b>(x, y) = Vec3b(0, 0, 0);
		}
	}

	bitwise_and(image, inputImg, colorImage);

	cvtColor(colorImage, HSV, CV_BGR2HSV);
	for (int x = 0; x < HSV.rows; x++) {
		for (int y = 0; y < HSV.cols; y++) {
			Vec3b color = colorImage.at<Vec3b>(x, y);
			if (color[0] != 0 || color[1] != 0 || color[2] != 0) {
				HSV.at<Vec3b>(x, y)[0] = (int)(HSV.at<Vec3b>(x, y)[0]) / 10 * 10;
				HSV.at<Vec3b>(x, y)[1] = 100;
				HSV.at<Vec3b>(x, y)[2] = 100;
			}
		}
	}
	cvtColor(HSV, colorImage, CV_HSV2BGR);

	return colorImage;
}

vector<Mat> BlueAlgorithm::Segmentate(Mat inputImg)
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
		if (symbol.size() > 20) {
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
		cvtColor(segment, segment, CV_BGR2GRAY);

		/* rotate */
		Point center = Point(segment.cols / 2, segment.rows / 2);
		double ang = rRect.angle;
		if (rRect.size.width > rRect.size.height)
			ang += -90 * ang / fabs(ang);

		Mat rot_mat = getRotationMatrix2D(center, ang, 1.0);
		warpAffine(segment, segment, rot_mat, segment.size());
		/* ------ */
		resize(segment, segment, Size(118, 227));
		threshold(segment, segment, 30, 255, CV_THRESH_BINARY_INV);

		segments.push_back(segment);
	}

	return segments;
}

string BlueAlgorithm::RecognizeSegments(vector<Mat> segments)
{
	return NeuronNetByEtalons::recognizeSegments(segments, "blue");
}

string BlueAlgorithm::Recognize(Mat inputImg)
{
	return RecognizeSegments(Segmentate(Preprocessing(inputImg)));
}
