#include "Captcha.h"
#include "Classes.h"

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