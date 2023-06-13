#include <opencv2/opencv.hpp>
#include <iostream>
#include <string>
#include <filesystem>
#include <iterator>
#include <fstream>
#include <vector>
#include <algorithm>


using namespace cv;
using namespace std;
namespace fs = std::filesystem;

bool is_frg(cv::Mat img, int rows, int cols) // check if the pixel belong to the foreground
{
	if (img.at<uchar>(rows, cols) == 0) return false;
	else return true;
}

double f_measurment(string path1, string path2, double *pwc) // return f-measure(double) for two images
{
	unsigned int tn = 0;
	unsigned int tp = 0;
	unsigned int fp = 0;
	unsigned int fn = 0;

	cv::Mat image1 = cv::imread(path1, 0);
	cv::Mat image2 = cv::imread(path2, 0);

	for (int i = 0; i < image1.rows; i++)
	{
		for (int j = 0; j < image1.cols; j++)
		{
			if (is_frg(image1, i, j) && is_frg(image2, i, j)) tp++;
			else if (!is_frg(image1, i, j) && is_frg(image2, i, j)) fp++;
			else if (is_frg(image1, i, j) && !is_frg(image2, i, j)) fn++;
			else if (!is_frg(image1, i, j) && !is_frg(image2, i, j)) tn++;
		}
	}
	if (tp == 0 && fp == 0 && fn == 0)
	{
		*pwc = 0;
		return (double)1; // for case with two full-black images
	}
	double precision = ((double)tp / ((double)tp + (double)fp));
	double recall = ((double)tp / ((double)tp + (double)fn));
	double f_measure = ((2 * precision * recall) / (precision + recall));

	*pwc = 100 * ((double)fp + (double)fn) / ((double)tp + (double)tn + (double)fn + (double)fp);
	return f_measure;
}

double average_vector (vector<double> array, int size) // return average of double vector
{
	double average = 0;
	for (int i = 0; i < size; i++)
	{
		average += array[i];
	}
	average /= size;
	return average;
}

double median_vector(vector<double> array, int size, double *min, double *max) // return median + min and max by *
{
	sort(array.begin(), array.end());
	*min = array[0];
	*max = array[array.size()-1];
	return array[size / 2 + 1];
}

void comparison(string path, string name_algorythm, int number_start, int number_end)
{
	//create vector of paths(strings) to png's (ground truth)
	string pathtogt = path + "gt";
	fs::path path_gt{ pathtogt };
	std::vector<string> png_gt;
	fs::path path1;
	for (auto& p : fs::directory_iterator(path_gt))
	{
		path1 = p;
		png_gt.push_back(path1.generic_string());
	}
	//create vector of paths(strings) to png's (which was made by algorythm)
	string pathtoalg = path + name_algorythm;
	std::vector<string> png_alg;
	for (int i = 0; i < png_gt.size(); i++)
	{

		png_alg.push_back(pathtoalg + "/" + to_string(i) + ".png");
	}

	std::vector<double> f_vector;// vector of f-measurements
	std::vector<double> pwc; // percent of mistakes
	//create txt file to put there info about f-measure, algorythm etc
	ofstream logs(pathtogt + "/logs_" + name_algorythm + ".txt", ios_base::app);
	logs << "algrotythm name: " << name_algorythm << endl;
	logs << "path to sequence of images: " << pathtogt << endl;

	for (int i = number_start; i < number_end; i++)
	{
		double pwc_tmp = 0;
		f_vector.push_back(f_measurment(png_gt[i], png_alg[i], &pwc_tmp));
		pwc.push_back(pwc_tmp);
		logs << i << ": " << f_vector[i - number_start] << "	" << pwc[i - number_start] << endl;
		cout << "processing: " << i - number_start << " of " << number_end - number_start << endl;
	}
	double min_f = 0;
	double max_f = 0;
	double min_pwc = 0;
	double max_pwc = 0;
	logs << "______________________________" << "\n" <<
		"average f-metric = " << average_vector(f_vector, f_vector.size()) << endl <<
		"median f-metric = " << median_vector(f_vector, f_vector.size(), &min_f, &max_f) << endl <<
		"min-max = " << min_f << "	" << max_f << endl <<
		"average pwc = " << average_vector(pwc, pwc.size()) << endl <<
		"median pwc = " << median_vector(pwc, pwc.size(), &min_pwc, &max_pwc) << endl <<
		"min-max = " << min_pwc << "	" << max_pwc << endl;

	logs.close();
}


int main(int argc, char** argv)
{	
	string path = "D:/DATASET_NEW_/vid4/";
	std::vector<string> algorythms { "dpprati", "mog2", "multicue",
									"pawcs", "pbas", "subsense",
									"t2fgmm", "twopoints", "vibe",
									"vumeter", "WMM"};
	for (auto& name_alg : algorythms)
	{
		comparison(path, name_alg, 1, 100);
	}
	return 0;
}
