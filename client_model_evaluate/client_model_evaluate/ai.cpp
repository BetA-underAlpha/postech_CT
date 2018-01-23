#include"ai.h"

#include <time.h>
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
int n = 11;
int checksum = 0;

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
	clock_t begin, end;

	

	vector<pair<Mat, int> > train_set;
	vector<pair<vector<double>, int> > train_feature;

	aiInit(4, train_set, train_feature);

	while (1)
	{	
		recvResult(sock);

		begin = clock();

		aiCode(playerNextPosition, 4, train_feature);

		end = clock();

		cout << "time : " << ((end - begin) / CLOCKS_PER_SEC) << endl;

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
	int loc[18] = { 0 };
	int position = -1;

	int more_apple = 0;

	for (auto tmp : classNumber)
	{
		loc[idx] = tmp;
	}

	if (dominance == 0) {
		vector<int> man;
		for (idx = 0; i < 18; i++) {
			if (loc[idx] == 2) {
				man.push_back(idx);
			}
		}
		for (int i = 0; i < man.size(); i++) {
			if (man[i] == 0) {
				loc[1] = 1;
				loc[5] = 1;
			}
			else if (man[i] < 6) {
				loc[man[i] + 1] = 1;
				loc[man[i] - 1] = 1;
			}
			else if (man[i] % 2 == 0) {
				loc[(man[i] / 2) - 3] = 1;
			}
			else if (man[i] != 17) {
				loc[(man[i] / 2) - 3] = 1;
				loc[(man[i] / 2) - 2] = 1;
			}
			else {
				loc[5] = 1;
				loc[0] = 1;
			}
		}



	}

	for (idx = 0; idx < 18; idx++)
	{
		if (loc[idx] == -1)
		{
			continue;
		}
		//tmp 0 사과, 1 폭탄, 2 사람, 3 꽃
		if (loc[idx] == 0 && idx < 6) //0~5사이에 사과가 있으면 먹는다 
		{   //  짝수일 때 -> idx % 2 = 0
			int num = 0;
			for (int i = -1; i < 2; i++) {
				if (loc[(idx + 3) * 2 + i] == 0) {
					num++;
				}
			}
			if (num > more_apple) {
				position = idx;
				more_apple = num;
			}

		}
		/////////////////////////////////////////////////////////////////////////////////////////////////
		if (loc[idx] == 0 && idx % 2 == 0 && idx > 5) //2칸 후 직선위치에 사과가 있을 때
		{
			int bombcheck = idx / 2 - 3; //사이의
			if (loc[bombcheck] == 3) //사이에 꽃이 있으면
			{
				position = bombcheck; //그 방향으로 이동한다
				break;
			}

		}
		if (loc[idx] == 0 && idx % 2 == 1 && idx > 5 && idx < 17) //2칸 후 꺽인 위치에 사과가 있을 때
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
		if (idx == 17 && loc[idx] == 0) //17번, 마지막 칸에 대한 조건
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

	}

	if (position < 0 || position > 5) //아직 칸을 못 정했을 경우(다 폭탄이거나 벽)
	{
		vector<int> able;
		for (int i = 0; i < 6; i++) {
			if (loc[i] != 1 && loc[i] != -1) {   //폭탄도 아니고 벽도 아닌 경우에만 벡터에 포함
				able.push_back(i);
			}
		}
		if (able.size() == 0) {
			able.clear();
			for (int i = 0; i < 6; i++) {
				if (loc[i] != -1) {   //벽이 아닌 경우에만 벡터에 포함
					able.push_back(i);
				}
			}
		}
		position = able[rand() % able.size()];

	}



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
	int max_bottom = 199;
	int max_left = 0;
	int max_right = 199;

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

	int b = 0;
	while (b == 0 && max_top<max_bottom) {
		for (int i = 0; i<m.rows; i++) {
			if (m.at<Vec4b>(max_top, i)[3] != 0) {
				b = 1;
				break;
			}
		}
		max_top += 1;
	}

	b = 0;
	while (b == 0 && max_top<max_bottom) {
		for (int i = 0; i<m.rows; i++) {
			if (m.at<Vec4b>(max_bottom, i)[3] != 0) {
				b = 1;
				break;
			}
		}
		max_bottom -= 1;
	}

	b = 0;
	while (b == 0 && max_left<max_right) {

		for (int i = 0; i<m.cols; i++) {
			if (m.at<Vec4b>(i, max_left)[3] != 0) {
				b = 1;
				break;
			}
		}
		max_left += 1;
	}

	b = 0;
	while (b == 0 && max_left<max_right ) {
		for (int i = 0; i<m.cols; i++) {
			if (m.at<Vec4b>(i, max_right)[3] != 0) {
				b = 1;
				break;
			}
		}
		max_right -= 1;
	}


	int rev_cols =  max_bottom - max_top;
	int rev_rows = max_right - max_left;
	int i, j;
	///////////////////////////////////////////////////////////
	double sum_r=0;
	double sum_g=0;
	double sum_b=0;
	Vec4b tmp;
	for (int p = 0; p < n; p++) {
		for (int l = 0; l < n; l++) {
			for (i = rev_cols*p/n; i < (rev_cols*(p + 1))/n; i++) {
				for (j = rev_rows*l/n; j < rev_rows*(l + 1)/n; j++) {
					tmp = m.at<Vec4b>(max_top+i, max_left+j);
					sum_r += tmp[2];
					sum_g += tmp[1];
					sum_b += tmp[0];
				}
			}
			sum_r = sum_r*n*n / (rev_rows*rev_cols);
			sum_g = sum_g*n*n / (rev_rows*rev_cols);
			sum_b = sum_b*n*n / (rev_rows*rev_cols);
			
			ret.push_back(sum_r);
			ret.push_back(sum_g);
			ret.push_back(sum_b);
		}
	}

	//ret.push_back((double)a);
	ret.push_back(((rev_rows*1.00) / rev_cols));

	return ret;
}

int classify(Mat example, vector<pair<vector<double>, int> > &training, int nb_class) {
	const int k = 10;
	int predict = -1;

	/*-----don't touch-----*/

	auto exam_des = featureDescript(example);
	double tmp_distance;

	int num[4] = { 0 };

	

	vector<pair<double, int> > v;

	for (int i = 0; i < training.size(); i++) {
		//printf("%f", training[i].first);
		tmp_distance = dist(training[i].first, exam_des);
		//printf("%lf", tmp_distance);
		v.push_back(pair<double, int>(tmp_distance, training[i].second));
	}

	sort(v.begin(), v.end());

	for (int i = 0; i < k; i++) {

		int key_value = v[i].second;

		if (key_value == 0) {
			num[0]++;
		}
		else if (key_value == 1) {
			num[1]++;
		}
		else if (key_value == 2) {
			num[2]++;
		}
		else if (key_value == 3) {
			num[3]++;
		}
	}

	int max_num = 0;

	for (int i = 0; i < 4; i++) {
		if (num[i] >= num[max_num]) {
			max_num = i;
		}
	}

	/*-----don't touch-----*/
	//if (max_num == 0) printf("0");
	return max_num;
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
	const int k = 13;
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
		if (file.channels() == 3)
		{
			vector<cv::Mat> channels;
			split(file, channels);
			Mat alpha;
			channels[0].copyTo(alpha);

			for (int i = 0; i < file.rows; i++)
			{
				for (int j = 0; j < file.cols; j++)
				{
					char& a = alpha.at<char>(i, j);
					a = 255;
				}
			}
			channels.push_back(alpha);
			merge(channels, file);
		}



		list.push_back({ file, cs });

	} while (FindNextFile(hFind, &FindData));

	FindClose(hFind);

	return true;
}

double dist(vector<double> vec1, vector<double>vec2) {

	double d = 0;
	//double edge = 0;
	double ratio = 0;
	//n은 알아서 전역변수로 선언 해놓고
	for (int i = 0; i<n*n * 3; i++) {
		//위치 보정값 w 결정(Center is important), 0.5<w<1.5
		int a = (i / 3) % n;
		int b = (i / 3) / n;
		double w = 0;
		if (a > n / 2) {
			w += -2 * a / n;
			w += 2.5;
		}
		else {
			w += 2 * a / n;
			w += 0.5;
		}
		if (b > n / 2) {
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
	ratio = vec1[3*n*n] / vec2[3*n*n];
	if (ratio > 1) {
		ratio = 1 / ratio;
	}
	ratio *= 255;
	ratio = 255 - ratio;
	d += n*n*ratio*ratio/4;
	return d;
}