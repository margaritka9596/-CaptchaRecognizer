#include "Classes.h"

#define MYDIRECTORY "trainset\\blue\\trainset\\"
#define Path "trainset\\blue\\testset\\"

int min(int a, int b, int c) {
	if (a < b)
		if (a < c) return a;
		else return c;
	else
		if (b < c) return b;
		else return c;
}

int calcLevenshteinDistance(string s1, string s2) {
	int M = s1.length(), N = s2.length();
	if (M == 0 && N == 0)
		return 0;
	else if (M == 0)
		return N;
	else if (N == 0)
		return M;
	else {
		string tmp1 = s1.substr(0, M - 1), tmp2 = s2.substr(0, N - 1);
		return min(
			calcLevenshteinDistance(tmp1, s2) + 1,
			calcLevenshteinDistance(s1, tmp2) + 1,
			calcLevenshteinDistance(tmp1, tmp2) + (int)(s1[M-1] != s2[N-1]));
	}
}

string getDiff(string value, string testvalue) {
	string result = "";
	int n = value.length();
	int N = testvalue.length();
	int min = n;
	if (N < n)
		min = N;
	for (int i = 0; i < min; ++i)
		if (value[i] == testvalue[i])
			result += value[i];
		else
			result += '*';
	for (int i = min; i < n; ++i)
		result += '-';
	for (int i = n; i < N; ++i)
		result += '^';

	return result;
}

/*
std::string getexepath()
{
	char result[MAX_PATH];
	return std::string(result, GetModuleFileName(NULL, result, MAX_PATH));
}
*/

string captchaRecognize(Mat image) {
	/*
	int flag = 0;
	vector<Mat> result;
	vector<Mat> result1;

	//RedAlgorithm obj = RedAlgorithm();
	RedAlgorithm redAlgoritm;
	NeuronNetByEtalons neuronNetByEtalons;
	string cptch;
	
	//
	string pathToEtalons = "C:\\Users\\Margo\\Desktop\\try\\";
	const char* pathToEtalonsChar = pathToEtalons.c_str();
	//
	switch (flag)
	{
	case 0:
		//cout << getexepath() << endl;
		result = redAlgoritm.Preprocessing(image);
		std::cout << "hi" << endl;
		
		///
		
		DIR *dir;
		struct dirent *ent;
		if ((dir = opendir(pathToEtalonsChar)) != NULL) {
			ent = readdir(dir);
			ent = readdir(dir);
			while ((ent = readdir(dir)) != NULL) {
				string filename = ent->d_name;
				string label = filename.substr(0, filename.find('.'));
				Mat image = imread(pathToEtalonsChar + filename, CV_LOAD_IMAGE_GRAYSCALE);

				string extension = filename.substr(filename.find('.'), filename.length());
				if (extension == ".jpg")
					result1.push_back(image);
			}
			closedir(dir);
		}
		else
			perror("");
		///

		cptch = neuronNetByEtalons.recognizeSegments(result1, "red");

		std::cout << "res = " << cptch << endl;
		//imshow("image1", resullt);
		return "-";
		break;
	case 1:
		return "-";
		break;
	default:
		return "-";
		break;
	} 
	*/
	BlueAlgorithm blue;
	return blue.Recognize(image);
}

vector<string> fitCaptchaResult(vector<captcha> box) {
	vector<string> result(box.size());
	int endI = box.size();
	//int endI = 1;

	for (int i = 0; i < endI; ++i) {
		result[i] = captchaRecognize(box[i].first);
	}
	return result;
}

vector<int> fitLevenshteinDistance(vector<captcha> box, vector<string> results) {
	vector<int> result(box.size());
	for (unsigned int i = 0; i < result.size(); ++i)
		result[i] = calcLevenshteinDistance(box[i].second, results[i]);
	return result;
}

pair<double, double> writeResult(vector<captcha> box, vector<string> results, vector<int> distances) {
	ofstream fout(TEXTFILE);
	double avg = 0, accuracy = 0;
	for (unsigned int i = 0; i < box.size(); ++i) {
		fout << box[i].second << " " << results[i] << " " << getDiff(box[i].second, results[i]) << " " << distances[i] << endl;
		avg += distances[i];
		accuracy += (int)(box[i].second == results[i]);
	}
	avg /= box.size();
	accuracy /= box.size();
	fout << endl << avg << endl << accuracy * 100 << "%" << endl;
	fout.close();
	return pair<double, double>(avg, accuracy);
}

int main(int argc, char* argv[])
{
	//string argv_str(argv[0]);
	//string src = argv_str.substr(0, argv_str.find_last_of("\\"));
	//src = src.substr(0, src.find_last_of("\\"));
	//src = src.substr(0, src.find_last_of("\\"));

	//std::cout << argv_str << endl;
	//std::cout << src << endl;

	//string redTrainsetPath = src + "\\Captcha\\trainset\\red\\trainset\\";
	//const char* Path = redTrainsetPath.c_str();

	vector<captcha> box = getCaptcha(Path, "jpg|jpeg|png");
	vector<string> results = fitCaptchaResult(box);
	vector<int> distances = fitLevenshteinDistance(box, results);
	pair<double, double> temp = writeResult(box, results, distances);
	std::cout << temp.first << endl << temp.second << endl;

	//vector<captcha> box = getCaptcha(MYDIRECTORY, "jpeg|jpg|png");
	//for (unsigned int i = 0; i < box.size(); ++i)
	//{
	//	BlueAlgorithm rec;
	//	imshow("show", box[i].first);
	//	cout << rec.Recognize(box[i].first) << endl;
	//	waitKey();
	//	destroyWindow("show");
	//}

	return 0;
}