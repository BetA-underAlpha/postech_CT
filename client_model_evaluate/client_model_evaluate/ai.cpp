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
	int loc[18];
	int position = 3;

	for (auto tmp : classNumber)
	{
		loc[idx] = tmp;

		//tmp 0 사과, 1 폭탄, 2 사람, 3 꽃
		if (tmp == 0 && idx < 6) //0~5사이에 사과가 있으면 먹는다 
		{	//  짝수일 때 -> idx % 2 = 0
			position = idx;
			break;
		}

		if (tmp == 0 && idx % 2 == 0 && idx > 5) //2칸 후 직선위치에 사과가 있을 때
		{
			int bombcheck = idx / 2 - 3; //사이의
			if (loc[bombcheck] == 3) //사이에 꽃이 있으면
			{
				position = bombcheck; //그 방향으로 이동한다
				break;
			}

		}

		if (tmp == 0 && idx % 2 == 1 && idx > 5 && idx < 17) //2칸 후 꺽인 위치에 사과가 있을 때
		{
			int bombcheck2 = (idx - 1) / 2 - 3; //시계방향 중간 위치
			int bombcheck3 = (idx - 1) / 2 - 2; //반시계방향 중간 위치

			if (loc[bombcheck2] == 3) //시계방향 중간 위치에 꽃이 있으면
			{
				position = bombcheck2; //그 방향으로 이동한다
				break;
			}

			if (loc[bombcheck3] == 3) //반시계방향 중간 위치에 꽃이 있으면
			{
				position = bombcheck3; //그 방향으로 이동한다
				break;
			}

		}

		if (idx == 17 && tmp == 0) //17번, 마지막 칸에 대한 조건
		{

			if (loc[5] == 3) //시계방향 중간 위치에 꽃이 있으면
			{
				position = 5; //그 방향으로 이동한다
				break;
			}

			if (loc[0] == 3) //반시계방향 중간 위치에 꽃이 있으면
			{
				position = 0; //그 방향으로 이동한다
				break;
			}

		}
		if (idx == 17 && tmp != 0) //17번, 마지막 칸에 대한 조건
		{
			if (loc[0] != 1)
			{
				position = 0;
				break;
			}
			if (loc[1] != 1)
			{
				position = 1;
				break;
			}
			if (loc[2] != 1)
			{
				position = 2;
				break;
			}
			if (loc[3] != 1)
			{
				position = 3;
				break;
			}
			if (loc[4] != 1)
			{
				position = 4;
				break;
			}
			if (loc[5] != 1)
			{
				position = 5;
				break;
			}
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

	int DENSITY = 10;

	int a = 0;
	int max_top = 0;
	int max_bottom = 200;
	int max_left = 0;
	int max_right = 200;

	/*////////////////////////////////////////////////////////   number of a==0
	Vec4b tmp = m.at<Vec4b>(0, 0);
	if (tmp[3] == 0)
		a++;
	tmp = m.at<Vec4b>(0, m.cols - 1);
	if (tmp[3] == 0)
		a++;
	tmp = m.at<Vec4b>(m.rows - 1, 0);
	if (tmp[3] == 0)
		a++;
	tmp = m.at<Vec4b>(m.rows - 1, m.cols - 1);
	if (tmp[3] == 0)
		a++;

	////////////////////////////////////////////////////////*/

	if (a>0) {
		int b = 0;
		while (b == 0) {
			max_top += 10;
			for (int i = 0; i<m.rows; i++) {
				if (m.at<Vec4b>(max_top, i)[3] != 0) {
					b = 1;
					break;
				}
			}
		}

		b = 0;
		while (b == 0) {
			max_bottom -= 10;
			for (int i = 0; i<m.rows; i++) {
				if (m.at<Vec4b>(max_bottom, i)[3] != 0) {
					b = 1;
					break;
				}
			}
		}

		b = 0;
		while (b == 0) {
			max_left += 10;
			for (int i = 0; i<m.cols; i++) {
				if (m.at<Vec4b>(i, max_left)[3] != 0) {
					b = 1;
					break;
				}
			}
		}

		b = 0;
		while (b == 0) {
			max_right -= 10;
			for (int i = 0; i<m.cols; i++) {
				if (m.at<Vec4b>(i, max_right)[3] != 0) {
					b = 1;
					break;
				}
			}
		}
	}

	int rev_rows = max_top - max_botom;
	int rev_cols = max_right - max_left;

	///////////////////////////////////////////////////////////
	int sum_r;
	int sum_g;
	int sum_b;
	for (int l = 0; l < n; l++) {
		for (int m = 0; m < n; m++) {
			for (int j = (rev_cols / n)*m; j < (rev_cols / n)*m; j++) {
				for (i = (rev_rows / n)*l; i <(rev_rows / n)*(l + 1); i++) {
					Vec4b tmp = m.at<Vec4b>(i + rev_rows, j + rev_cols);
					sum_r += tmp[2];
					sum_g += tmp[1];
					sum_b += tmp[0];
				}
			}
			sum_r = sum_r / (rev_rows*rev_cols)*n*n;
			sum_g = sum_g / (rev_rows*rev_cols)*n*n;
			sum_b = sum_b / (rev_rows*rev_cols)*n*n;

			ret.push_back((double)sum_r);
			ret.push_back((double)sum_g);
			ret.push_back((double)sum_b);
		}
	}

	ret.push_back((double)a);
	ret.push_back((double)((rev_rows*1.00) / rev_cols);

	return ret;
}

int classify(Mat example, vector<pair<vector<double>, int> > &training, int nb_class) {
	const int k = 10;
	int predict = -1;

	/*-----don't touch-----*/

	auto exam_des = featureDescript(example);

	int num0 = 0; //사과
	int num1 = 0; //폭탄
	int num2 = 0; //얼굴
	int num3 = 0; //꽃
	
	vector<pair<int, double> v;

	for (int i = 0; i < training.size(); i++) {
		tmp_distance = dist(training[i].first, exam_des);
		
		v.push_back(pair<int, double>(training[i].second, tmp_distance));
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
			num3++;
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
	//double edge = 0;
	ratio = 0;
	//n은 알아서 전역변수로 선언 해놓고
	for (int i = 0; i<n*n * 3; i++) {
		//위치 보정값 w 결정(Center is important), 0.5<w<1.5
		int a = i%n;
		int b = i / n;
		int w = 0;
		if (a > 2 / n) {
			w += -2 * a / n;
			w += 2.5;
		}
		else {
			w += 2 * a / n;
			w += 0.5;
		}
		if (b > 2 / n) {
			w += -2 * b / n;
			w += 2.5;
		}
		else {
			w += 2 * b / n;
			w += 0.5;
		}
		w /= 2;
		d += w*(vec1[i] - vec2[i])*(vec1[i] - vec2[i]);
	}
	//double edge = abs(vec1[n*n*3]-vec2[n*n*3]);
	//edge = 8*edge*edge*edge;
	//d += 3*n*n*edge*edge;
	ratio = vec1[3 * n*n + 1] / vec2[3 * n*n + 1];
	if (ratio > 1) {
		ratio = 1 / ratio;
	}
	ratio *= 500;
	ratio = 500 - ratio;
	d += 3 * n*n*ratio*ratio;
	return d;
}