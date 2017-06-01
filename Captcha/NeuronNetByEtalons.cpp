#include "Captcha.h"
#include "Classes.h"

//numberOfClasses & numberOfComponents
int M, N;

/*static string pathWMatrix = Application.StartupPath + "W_array.txt";*/

vector<int> toVector(Mat image)
{
	//TODO
	double backgroundColor = 128.0;

	//бинаризация
	int h = image.rows;
	int w = image.cols;

	// Mat dst = new Mat(w, h);

	vector<int> binarized(h * w + 1);
	for (int j = 0; j < h; ++j)
	{
		for (int i = 0; i < w; ++i)
		{
			//Vec2b color = image.at<Vec2b>(j,i);
			uchar color = image.at<uchar>(j, i);
			//TODO
			//cout << static_cast<int>(color) << " ";
			if (color < backgroundColor)
				binarized[j * w + i] = 1;
			else
				binarized[j * w + i] = 0;
		}
		//cout << endl;
	}
	return binarized;
}

double activationFunc(double x)
{
	double res = 0, F = 9.9, alpha = 1 / 2.0;
	if (x < 0)
		res = 0;
	else
	{
		if (x > 0)
			res = x;
		/*if (x > F)
		res = 1;
		else
		res = alpha * x;*/
	}
	return res;
}

vector<captcha> uploadEtalons(string srcPath, string algorithmName) //algorithmName - lowcase(ex: red, blue)
{
	vector<captcha> etalons = getCaptcha(srcPath + algorithmName + "\\etalons\\", "jpg|jpeg|png");
	for (unsigned int i = 0; i < etalons.size(); ++i)
		cvtColor(etalons[i].first, etalons[i].first, CV_BGR2GRAY);

	return etalons;
}

vector<double> trainNet(vector<captcha> etalons)
{
	//numberOfClasses
	M = etalons.size();
	vector<int> el;

	//create matrix
	vector<int> X;
	for (unsigned int i = 0; i < etalons.size(); ++i)
	{
		el.swap(toVector(etalons[i].first));
		for (unsigned int i = 0; i < el.size(); ++i)
			X.push_back(el[i]);
	}

	//numberOfComponents
	N = el.size();

	//Create matrix W
	vector<double> W;
	for (int j = 0; j < M; ++j)
		for (int i = 0; i < N; ++i)
			W.push_back((1 / (2.0)) * X[j * N + i]);

	//TODO
	//1)запись в файл
	/*
	using (StreamWriter writer = new StreamWriter(File.Open(pathWMatrix + "", FileMode.Create))) //"C:/Users/Margo/Desktop/WindowsFormsApplication1/res/W_array.txt"
	{
		for (int j = 0; j < M; ++j)
		{
			writer.Write("---->" + j + " = ");
			for (int i = 0; i < N; ++i)
				writer.Write(W[j * N + i]);
			writer.Write("\n");
		}
	}
	*/

	string message = "Net is ready.";
	//cout << message << endl;

	return W;
}

string testNet(Mat inputImg, vector<captcha> etalons, vector<double> W, string algorithmName)
{
	//imshow("show", inputImg); waitKey(); destroyWindow("show");

	int ellipseCount = getEllipseCount(inputImg);
	bool isTopEllipse = topEllipse(inputImg);
	bool isBottomEllipse = bottomEllipse(inputImg);

	vector<bool> possibleEtalon(etalons.size());
	for (unsigned int i = 0; i < possibleEtalon.size(); ++i)
	{
		if (algorithmName == "blue")
			possibleEtalon[i] = (
				ellipseCount == getEllipseCount(etalons[i].first) &&
				isTopEllipse == topEllipse(etalons[i].first) &&
				isBottomEllipse == bottomEllipse(etalons[i].first)
			);
		else
			possibleEtalon[i] = true;
	}
	//imshow("show", inputImg); waitKey(); destroyWindow("show");

	//bool isOne = canBeOne(inputImg);

	vector<int> experiment = toVector(inputImg);
	
	///
	int numOne = 0;
	for (unsigned int i = 0; i < experiment.size(); ++i)
	{
		if (experiment[i] != 0)
			++numOne;
	}
	//TODO
	++numOne;
	///

	//first floor
	vector<double> y1;

	for (int j = 0; j < M; ++j)
	{
		double sum = 0;
		for (int i = 0; i < N; ++i)
		{
			sum += W[j * N + i] * experiment[i];
		}
		y1.push_back(sum + N / (2.0));
	}

	//second floor
	vector<double> y2_prev(y1);
	vector<double> y2_next;

	double eps = 1 / (M * 2);// 0.05; // 0 <eps< 1/M
	int deadline = 1;
	int diffComp = deadline + 1;
	int count = 0;

	while (diffComp > deadline)
	{
		diffComp = 0;
		//покоординатно считаем новый вектор
		for (int j = 0; j < M; ++j)
		{
			double argument = y2_prev[j];
			double sum = 0;
			/*for (int i = 0; (i < M) && (i != j); ++i)*/
			for (int i = 0; i < M; ++i)
			{
				if (i != j)
					sum += y2_prev[i];
			}
			sum *= eps;
			argument -= sum;

			y2_next.push_back(activationFunc(argument));
			if (y2_prev[j] != y2_next[j])
				diffComp++;
		}

		y2_prev.swap(y2_next);
		count++;
	}

	//find class number
	double max = 0;
	int classNum = -1;
	for (int k = 0; k < M; ++k)
	{
		if (possibleEtalon[k])
			if (y2_next[k] > max)
			{
				max = y2_next[k];
				classNum = k;
			}
			else
			{
				if (y2_next[k] == max)
					classNum = -2;
			}
	}
	string message;
	if (classNum == -2)
	{
		message = "Net can't determine the class of experiment object =(";
		return "=";
	}
	else if (classNum == -1)
	{
		//imshow("show", inputImg); waitKey(); destroyWindow("show");

		message = "Net can't determine the class of experiment object =(";
		return "-";
	}
	else
		message = "Class number is " + etalons[classNum].second + " ! =)";

	//cout << message << endl;

	return etalons[classNum].second;
}

string NeuronNetByEtalons::recognizeSegments(vector<Mat> segments, string algoritmName)
{
	string predictedValue = "";

	vector<captcha> etalons = uploadEtalons(ETALONSPATH, algoritmName);
	vector<double> W = trainNet(etalons);
	
	for (unsigned int i = 0; i < segments.size(); ++i)
	{
		predictedValue += testNet(segments[i], etalons, W, algoritmName);
	}

	return predictedValue;
}
