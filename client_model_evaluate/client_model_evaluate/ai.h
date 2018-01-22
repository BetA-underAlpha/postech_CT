#pragma once
#ifdef _DEBUG
#pragma comment(lib, "opencv_world340d.lib")
#else
#pragma comment(lib, "opencv_world340.lib")
#endif
#pragma comment(lib,"ws2_32.lib")

#include <Windows.h>
#include <vector>
#include <opencv2/opencv.hpp>

#define MAX_SIZE_AREA (18) // total count of area that we can get image
#define MAX_MOVE_AREA (6) // total count of area that we can move
#define MAX_SIZE_PICTURE (1000000) // maximum picture size

using namespace std;
using namespace cv;

typedef struct areaInfo
{
	/*
	*** information of area
	*** - isWall[type : int] => isWall - 1 (is wall), isWall - 0 (isn't wall)
	*** - pictureLen[type : int] => length of picture
	*** - picture_png[type : array(char)] => image that type is .png
	*** - picture_rgba[type : Mat(r,g,b,a)] => reconstruct image from picture_png
	*/

	int isWall;
	int pictureLen;
	char* picture_png;
	Mat picture_rgba;
} areaInfo;

struct img {
	int coor, pd, dis;
};

int recvPicture(SOCKET sock, int flags, int idx);

void sendResult(SOCKET sock, int playerNextPosition, int applePosition, int bombPosition);
void recvResult(SOCKET sock);

void AI(SOCKET sock);

void aiInit(int classCount, vector<pair<Mat, int> > &train_set, vector<pair<vector<double>, int> > &preprocessTrain);
void aiCode(int &playerNextPosition, int nb_class, vector<pair<vector<double>, int> > train_feature);

vector<double> featureDescript(Mat& m);
int classify(Mat example, vector<pair<vector<double>, int> > &training, int nb_class);
vector<int> predict(vector<pair<vector<double>, int> > model, int nb_class);
float model_evaluate(vector<pair<Mat, int> > training, int nb_class);
bool input(vector<pair<Mat, int> > &list, string folder, int cs);