#include "Captcha.h"
#include "Classes.h"

//numberOfClasses & numberOfComponents
int M, N;
vector<captcha> etalons;
//input signal
vector<int> X;
vector<double> W;
Mat experimentBitmap;
string path = "ForNeuronNet";
/*static string pathWMatrix = Application.StartupPath + "W_array.txt";*/

vector<captcha> getCaptcha(string path, string ext) {
	vector<captcha> box;

	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(path.c_str())) != NULL) {
		ent = readdir(dir);
		ent = readdir(dir);
		while ((ent = readdir(dir)) != NULL) {
			string filename = ent->d_name;
			string value = filename.substr(0, filename.find('.'));
			string extension = filename.substr(filename.find('.') + 1, filename.length() - filename.find('.') - 1);
			if (ext.find(extension) != string::npos)
			{
				Mat image = imread(path + filename, CV_LOAD_IMAGE_COLOR);
				if (image.data)
				{
					pair<Mat, string> temp(image, value);
					box.push_back(temp);
				}
			}
		}
		closedir(dir);
	}
	else
		perror("");

	return box;
}

///TODO 
//)color?
//какая-то хуйня творится, binarized весь в нулях 
vector<int> binarizeBitmap(Mat image)
{
	//задаем значение

	//TODO
	double backgroundColor = 255.0;

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

void uploadEtalons(string srcPath, string algorithmName) //algorithmName - lowcase(ex: red, blue)
{
	etalons = getCaptcha(srcPath + "\\" + algorithmName + "\\etalons\\", "jpg|jpeg|png");
}

void trainNet()
{
	//numberOfClasses
	M = etalons.size();
	vector<int> el;

	//create matrix
	for (unsigned int i = 0; i < etalons.size(); ++i)
	{
		el.swap(binarizeBitmap(etalons[i].first));
		for (unsigned int i = 0; i < el.size(); ++i)
			X.push_back(el[i]);
	}

	//numberOfComponents
	N = el.size();

	//Create matrix W
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
	cout << message << endl;
}

string testNet(Mat inputImg)
{
	vector<int> experiment = binarizeBitmap(inputImg);
	
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
		message = "Net can't determine the class of experiment object =(";
	else
		message = "Class number is " + etalons[classNum].second + " ! =)";
	
	cout << message << endl;

	return etalons[classNum].second;
}

string NeuronNetByEtalons::recognizeSegments(vector<Mat> segments, string algoritmName)
{
	string predictedValue = "";

	uploadEtalons(path, algoritmName);
	trainNet();

	for (unsigned int i = 0; i < segments.size(); ++i)
	{
		predictedValue += testNet(segments[i]);
	}

	return predictedValue;
}
