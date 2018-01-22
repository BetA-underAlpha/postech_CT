#include"ai.h"

#include <iostream>
#include <io.h>
#include <string>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <stdio.h>
#include <conio.h>

using namespace std;
using namespace cv;

areaInfo area[MAX_SIZE_AREA];
int dominance = 0;
int n = 0;

void aiInit(int classCount, vector<pair<Mat, int> > &train_set, vector<pair<vector<double>, int> > &preprocessTrain)
{
	preprocessTrain.clear();
	train_set.clear();

	for (int i = 0; i < MAX_SIZE_AREA; i++)
	{
		area[i].isWall = FALSE;
		area[i].pictureLen = 0;
		area[i].picture_png = (char*)calloc(MAX_SIZE_PICTURE, sizeof(char));
	}

	for (int i = 0; i < classCount; i++)
	{
		string path = to_string(i);
		if (!input(train_set, path, i))
			cout << "error!\n";
	}

	for (int i = 0; i < train_set.size(); i++)
	{
		auto tmp = train_set[i];
		preprocessTrain.push_back({ featureDescript(tmp.first), tmp.second });
	}
}

void sendResult(SOCKET sock, int playerNextPosition)
{
	char sendString[4];

	*((int*)sendString) = playerNextPosition;

	send(sock, sendString, 4, 0);
}

int recvPicture(SOCKET sock, int flags, int idx)
{
	int recvLen, tmpLen, isFine;
	int iter = 0;
	char recvTmpChar[4];

	recv(sock, recvTmpChar, 4, flags);

	isFine = *((int*)recvTmpChar);

	recv(sock, recvTmpChar, 4, flags);

	recvLen = *((int*)recvTmpChar);

	printf("state : %d, %d\n", isFine, recvLen);

	if (isFine == 1)
		area[idx].isWall = FALSE;
	else
		area[idx].isWall = TRUE;

	if (area[idx].isWall)
		printf("%d : wall detcted\n", idx);

	area[idx].pictureLen = recvLen;

	if (area[idx].isWall)
		return -1;

	while (1)
	{
		if (iter >= recvLen)
			break;

		tmpLen = recv(sock, (area[idx].picture_png) + iter, recvLen - iter, flags);

		if (tmpLen == -1)
			continue;

		iter = iter + tmpLen;
	}

	(area[idx].picture_png)[recvLen] = 0;

	vector<uchar> trans;

	trans.assign(area[idx].picture_png, area[idx].picture_png + area[idx].pictureLen);
	area[idx].picture_rgba = imdecode(InputArray(trans), CV_8SC4);

	trans.clear();

	return recvLen;
}

void recvResult(SOCKET sock)
{
	char recvTmpChar[4];

	int tmpDominance;

	recv(sock, recvTmpChar, 4, 0);

	tmpDominance = *((int*)recvTmpChar);

	if (tmpDominance == 2)
		dominance = 1;
	else
		dominance = 0;

	printf("dominance : %d\n", dominance);

	for (int i = 0; i < MAX_SIZE_AREA; i++)
	{
		printf("%d / %d - ", i, MAX_SIZE_AREA);

		recvPicture(sock, 0, i);
	}
}

void AI(SOCKET sock)
{
	int playerNextPosition;
	int bombPosition;
	int applePosition;

	vector<pair<Mat, int> > train_set;
	vector<pair<vector<double>, int> > train_feature;

	aiInit(4, train_set, train_feature);

	while (1)
	{
		recvResult(sock);

		aiCode(playerNextPosition, 4, train_feature);

		sendResult(sock, playerNextPosition);
	}
}

int towardPoint(int idx)
{
	switch (idx)
	{
	case 0:
	case 6:
	case 7:
		return 0;
	case 1:
	case 8:
	case 9:
		return 1;
	case 2:
	case 11:
	case 10:
		return 2;
	case 3:
	case 12:
	case 13:
		return 3;
	case 4:
	case 14:
	case 15:
		return 4;
	case 5:
	case 16:
	case 17:
		return 5;

	}
}

int moveCharacter(vector<int> classNumber)
{
	int idx = 0;
	/*-----don't touch-----*/

	// sample code using idx and classNumber on idx

	int position = 3;

	for (auto tmp : classNumber)
	{
		if (tmp == 1)
		{
			position = towardPoint(idx);
			break;
		}

		idx++;
	}

	if (dominance)
		position = -1;

	/*-----don't touch-----*/

	return position;
}

void aiCode(int &playerNextPosition, int nb_class, vector<pair<vector<double>, int> > train_feature)
{
	vector<int> result = predict(train_feature, nb_class);

	playerNextPosition = moveCharacter(result);

	printf("Next : %d\n", playerNextPosition);
}

vector<double> featureDescript(Mat& m) {
	vector<double> ret;

	/*-----don't touch-----*/

	int a = 0;
	int max_top = 0;
	int max_bottom = 200;
	int max_left = 0;
	int max_right = 200;

	Vec4b tmp = m.at<Vec4b>(0, 0);
	if (tmp[3] == 0)
		a++;
	Vec4b tmp = m.at<Vec4b>(0, m.cols);
	if (tmp[3] == 0)
		a++;
	Vec4b tmp = m.at<Vec4b>(m.rows, 0);
	if (tmp[3] == 0)
		a++;
	Vec4b tmp = m.at<Vec4b>(m.rows, m.cols);
	if (tmp[3] == 0)
		a++;

	ret.push_back((double)a);

	if (a>0) {
		int b = 0;
		while (1) {
			for (int i = 0; i<m.rows; i++) {
				if (m.at<Vec4b>(max_top, i)[3] != 0) {
					b = 1;
					break;
				}
			}
			if (b == 1) {
				break;
			}
			max_top += 10;
		}
		b = 0;
		while (1) {
			for (int i = 0; i<m.rows; i++) {
				if (m.at<Vec4b>(max_bottom, i)[3] != 0) {
					b = 1;
					break;
				}
			}
			if (b == 1) {
				break;
			}
			max_bottom -= 10;
		}
		b = 0;
		while (1) {
			for (int i = 0; i<m.cols; i++) {
				if (m.at<Vec4b>(i, max_left)[3] != 0) {
					b = 1;
					break;
				}
			}
			if (b == 1) {
				break;
			}
			max_left += 10;
		}
		b = 0;
		while (1) {
			for (int i = 0; i<m.cols; i++) {
				if (m.at<Vec4b>(i, max_right)[3] != 0) {
					b = 1;
					break;
				}
			}
			if (b == 1) {
				break;
			}
			max_right -= 10;
		}


		return ret;
	}

int classify(Mat example, vector<pair<vector<double>, int> > &training, int nb_class) {
	const int k = 10;
	int predict = -1;

	/*-----don't touch-----*/

	auto exam_des = featureDescript(example);

	int num0 = 0; //»ç°ú
	int num1 = 0; //ÆøÅº
	int num2 = 0; //¾ó±¼
	int num3 = 0; //²É
	
	vector<pair<int, double> v;

	for (int i = 0; i < training.size(); i++) {
		tmp_distance = dist(training[i].first, exam_des);
		
		v.push_back(pair<int, double>(training[i].second, tmp_distance))
	}

	sort(v.begin(), v.end());

	for (int i = 0; i < k; i++) {

		int key_value = v[i].first;
		if (key_value == 0) {
			num0++;
		}
		else if (key_value == 1) {
			num1++;
		}
		else if (key_value == 2) {
			num2++;
		}
		else if (key_value == 3) {
			num3++
		}
	}

	predict = max(num0, max(num1, max(num2, num3)));

	/*-----don't touch-----*/

	return predict;
}

vector<int> predict(vector<pair<vector<double>, int> > model, int nb_class) {
	vector<int> ret;

	for (int i = 0; i < MAX_SIZE_AREA; i++) {
		if (area[i].isWall)
			ret.push_back(-1);
		else
		{
			int pd_res = classify(area[i].picture_rgba, model, nb_class);
			ret.push_back(pd_res);
		}
	}

	return ret;
}

float model_evaluate(vector<pair<Mat, int> > training, int nb_class) {
	float error = 0.0;
	const int k = 11;
	vector<vector<int> > k_fold(k + 1);
	vector<bool> check(training.size(), false);
	int sz = training.size() / k;

	for (int i = 0; i < k; i++) {
		for (int j = 0; j < sz; j++) {
			int next = rand() % training.size();

			if (check[next]) {
				j--;
				continue;
			}
			k_fold[i].push_back(next);
			check[next] = true;
		}
	}

	for (int i = 0; i < k; i++) {
		vector<pair<Mat, int> > test;
		vector<pair<vector<double>, int> > train;
		for (int j = 0; j < training.size(); j++) {
			bool check = false;
			for (auto next : k_fold[i])
				if (next == j) {
					check = true;
					break;
				}

			if (check)
				test.push_back(training[j]);
			else
				train.push_back({ featureDescript(training[j].first), training[j].second });
		}

		int result = 0;
		for (auto here : test) {
			int res = classify(here.first, train, nb_class);
			if (here.second != res)
				result++;
		}

		error += (float)result / sz;
	}

	return (float)error / k;
}

bool input(vector<pair<Mat, int> > &list, string folder, int cs) {
	HANDLE hFind;
	WIN32_FIND_DATA FindData;
	char path[255];

	string make_path = ".\\" + folder + "\\*.*";
	string realPath = ".\\" + folder + "\\";

	strcpy(path, make_path.c_str());

	hFind = FindFirstFile((LPCSTR)path, &FindData);

	if (hFind == INVALID_HANDLE_VALUE)
		return 0;

	do
	{
		if (!strcmp(FindData.cFileName, ".") || !strcmp(FindData.cFileName, ".."))
			continue;

		string name(FindData.cFileName);
		Mat file = imread(realPath + name, -1);

		list.push_back({ file, cs });

	} while (FindNextFile(hFind, &FindData));

	FindClose(hFind);

	return true;
}

double dist(Vector<double> vec1, Vector<double>vec2) {

	double d = 0;
	double edge = 0;
	ratio = 0;
	//nÀº ¾Ë¾Æ¼­ Àü¿ªº¯¼ö·Î ¼±¾ð ÇØ³õ°í
	for (int i = 0; i<n*n * 3; i++) {
		d += (vec1[i] - vec2[i])*(vec1[i] - vec2[i]);
	}
	double edge = abs(vec1[n*n * 3] - vec2[n*n * 3]);
	edge *= 125;
	d += 3 * n*n*edge*edge;
	ratio = vec1[3 * n*n + 1] / vec2[3 * n*n + 1];
	if (ratio > 1) {
		ratio = 1 / ratio;
	}
	ratio *= 500;
	ratio = 500 - ratio;
	d += 3 * n*n*ratio*ratio;
	return d;
}