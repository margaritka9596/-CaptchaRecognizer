#include "Classes.h"

//#define MYDIRECTORY "C:\\Users\\Margo\\Desktop\\Mine\\Study\\m1_1\\2_term\\image processing\\CaptchaRecognizer\\Captcha\\trainset\\red\\trainset\\"
#define TEXTFILE "result.txt"

typedef pair<Mat, string> captcha;

vector<captcha> getCaptcha(const char* path) {
	vector<captcha> box;
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(path)) != NULL) {
		ent = readdir(dir);
		ent = readdir(dir);
		while ((ent = readdir(dir)) != NULL) {
			string filename = ent->d_name;
			string value = filename.substr(0, filename.find('.'));
			Mat image = imread(path + filename, CV_LOAD_IMAGE_COLOR);
			pair<Mat, string> temp(image, value);
			box.push_back(temp);
		}
		closedir(dir); 
	}
	else 
		perror("");
	return box;
}

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

/*std::string getexepath()
{
	char result[MAX_PATH];
	return std::string(result, GetModuleFileName(NULL, result, MAX_PATH));
}*/

string captchaRecognize(Mat image) {
	//return "-";
	int flag = 0;
	vector<Mat> result;
	//RedAlgorithm obj = RedAlgorithm();
	RedAlgorithm redAlgoritm;
	NeuronNetByEtalons neuronNetByEtalons;
	string cptch;
	switch (flag)
	{
	case 0:
		//cout << getexepath() << endl;
		result = redAlgoritm.Preprocessing(image);
		cout << "hi" << endl;
		
		cptch = neuronNetByEtalons.recognizeSegments("red", result);
		cout << "res = " << cptch << endl;
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
}

vector<string> fitCaptchaResult(vector<captcha> box) {
	vector<string> result(box.size());
	//int endI = box.size();
	int endI = 1;

	for (unsigned int i = 0; i < endI; ++i) {
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
	fout << endl << avg << endl << accuracy << endl;
	fout.close();
	return pair<double, double>(avg, accuracy);
}

int main(int argc, char* argv[])
{
	string argv_str(argv[0]);
	string src = argv_str.substr(0, argv_str.find_last_of("\\"));
	src = src.substr(0, src.find_last_of("\\"));
	src = src.substr(0, src.find_last_of("\\"));

	cout << argv_str << endl;
	cout << src << endl;

	string redTrainsetPath = src + "\\Captcha\\trainset\\red\\trainset\\";
	const char* Path = redTrainsetPath.c_str();

	vector<captcha> box = getCaptcha(Path);
	vector<string> results = fitCaptchaResult(box);
	vector<int> distances = fitLevenshteinDistance(box, results);
	pair<double, double> temp = writeResult(box, results, distances);
	cout << temp.first << endl << temp.second << endl;
	waitKey();
	return 0;
}