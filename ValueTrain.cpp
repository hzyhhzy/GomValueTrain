
#include "pch.h"
#include "data.h"
#include <map>
#include <chrono>
#include <random>
#include <cmath>
#include <bitset>
#include <thread>
//#include <Windows.h>
using namespace std;
const int FEATURE_VERSION = 3;
typedef  unsigned long featureIdType;
const string EXE7ZName = "C:\\Progra~1\\7-Zip\\7z.exe";
const string compressCommand = " a -r -mx=0 -m0=LZMA2 -mf=on -mhc=on -mmt=6 -t7z ";
const string tmpDir = "C:\\ValueTrain";
inline long combine(char a, char b, char c, char d, int bias = 5)
{
	int i = a;
	i = i << bias;
	i |= b;
	i = i << bias;
	i |= c;
	i = i << bias;
	i |= d;
	return i;
}
struct FeatureFileHeader
{
	long version;
	long rule;//freestyle=1,standard=2,renju=3
	long featureIDSize;//2
	long featureCountsSize;//2
	unsigned long size;//data rows
};
struct FeaturesAndResult
{
	vector<featureIdType> featureIDs;
	vector<unsigned short> featureCounts;
	long result;
	float value;
};
void writeFAR(const FeaturesAndResult& far, FILE* file)
{
	
	long size = far.featureCounts.size();
	long result = far.result;
	float value = far.value;
	fwrite(&size, 4, 1, file);
	fwrite(&result, 4, 1, file);
	fwrite(&value, 4, 1, file);
	fwrite(far.featureIDs.begin()._Ptr, sizeof(featureIdType), size, file);
	fwrite(far.featureCounts.begin()._Ptr, 2, size, file);
}
void readFAR(FeaturesAndResult& far, FILE* file)
{
	long size;
	long result;
	float value;
	fread(&size, 4, 1, file);
	fread(&result, 4, 1, file);
	fread(&value, 4, 1, file);
	far.result = result;
	far.value = value;
	far.featureCounts.resize(size);
	far.featureIDs.resize(size);
	fread(far.featureIDs.begin()._Ptr, sizeof(featureIdType), size, file);
	fread(far.featureCounts.begin()._Ptr, 2, size, file);
}

bool readFeatureFileOld(string fileName, vector<FeaturesAndResult> &featureData)
{
	FILE *fp;

	errno_t err;
	err = fopen_s(&fp, fileName.data(), "rb");
	if (err)
	{
		cout << "No such file" << endl;
		return false;
	}
	FeatureFileHeader ffh;
	fread(&ffh, sizeof(FeatureFileHeader), 1, fp);
	if (ffh.version != FEATURE_VERSION || ffh.rule != 1 || ffh.featureIDSize != sizeof(featureIdType) || ffh.featureCountsSize != 2)
	{
		cout << "This file is not supported" << endl;
		return false;
	}
	int size = ffh.size;
	featureData.resize(size);
	for (int i = 0; i < size; i++)

	{
		readFAR(featureData[i], fp);
	}
	fclose(fp);
	return true;
}


bool readFeatureFile(string fileName/*没扩展名*/, vector<FeaturesAndResult> &featureData)
{
	string commandLine = EXE7ZName + " x " + fileName + ".7z -aoa -o\"" + tmpDir + "\" >>7zlog.txt";
	//cout << commandLine << endl;
	system(commandLine.c_str());
	string featureFileName =   tmpDir + "\\" + fileName + ".feature";


	FILE *fp;

	errno_t err;
	err = fopen_s(&fp, featureFileName.c_str(), "rb");
	if (err)
	{
		cout << "No such file" << endl;
		return false;
	}
	FeatureFileHeader ffh;
	fread(&ffh, sizeof(FeatureFileHeader), 1, fp);
	if (ffh.version != FEATURE_VERSION || ffh.rule != 1 || ffh.featureIDSize != sizeof(featureIdType) || ffh.featureCountsSize != 2)
	{
		cout << "This file is not supported" << endl;
		system(("del " + featureFileName).c_str());
		return false;
	}
	int size = ffh.size;
	featureData.resize(size);
	for (int i = 0; i < size; i++)

	{
		readFAR(featureData[i], fp);
	}
	fclose(fp);
	system(("del " + featureFileName).c_str());
	return true;
}
bool writeFeatureFile(string fileName, const vector<FeaturesAndResult> &featureData)
{

	string featureFileName =  tmpDir + "\\" + fileName + ".feature";
	FILE *fp;
	cout << featureFileName << endl;

	errno_t err;
	err = fopen_s(&fp, featureFileName.data(), "wb");
	if (err)
	{
		cout << "Failed to write this file" << endl;
		return false;
	}
	FeatureFileHeader ffh;
	ffh.version = FEATURE_VERSION;
	ffh.rule = 1;
	ffh.featureIDSize = sizeof(featureIdType);
	ffh.featureCountsSize = 2;
	ffh.size = featureData.size();
	fwrite(&ffh, sizeof(FeatureFileHeader), 1, fp);
	for (int i = 0; i < featureData.size(); i++)

	{
		writeFAR(featureData[i], fp);
	}
	fclose(fp);

	string commandLine = EXE7ZName + compressCommand+fileName+".7z "+featureFileName;
	system(commandLine.c_str());

	system(("del " + featureFileName).c_str());
	return true;
}
struct FeaturesParams
{
	//static const int LENGTH = 5;
	static const int SIZE = 3 * 2 *2* 32 * 32 * 32 * 32;
	//double straightParams[2187];
	//double diagonalParams[2187];
	vector<double> params;// [3 * 2 * 32 * 32 * 32 * 32]/*[2][2][2][32][32]*/;// /*对胜还是负的贡献(0胜,1负）*/，direction(直直0，直斜1，斜斜2），color（0我，1对手），这个点的color，left（0b01011代表+X+XX,0b11011代表OO+XX,最高位的连着的1是对手子或者墙）,right（同left）   left>right, 否则互换
	FeaturesParams() { params.resize(3*2 * 2 * 32 * 32 * 32 * 32); }
	/*void initSimpleVC4()
	{
		//自己差一手五连
		for (int leftL = 0; leftL <= 4; leftL++)
			for (int rightL = 0; rightL <= 4; rightL++)
			{
				if (leftL + rightL <= 4)continue;
			}
	}*/

};

FeaturesAndResult getFeaturesOfBoard(const Board& board)
{
	FeaturesAndResult far;
	far.value = board.result == 'W' ? 1 : board.result == 'D' ? 0.5 : 0;
	far.result = board.result == 'W' ? 1 : board.result == 'D' ? 0 : -1;
	static const int L = 4;
	map<unsigned long, unsigned short> featureList;

	for (int color = 1; color <= 2; color++)
	{
		for (int y0 = 0; y0 < BS; y0++)
		{
			for (int x0 = 0; x0 < BS; x0++)
			{
				if (board.colors[x0 + BS * y0] == 3 - color)continue;
				bool thisPointColor = (board.colors[x0 + BS * y0]!=0);
				int con[8];//8个方向
				for (int i = 0; i < 8; i++)con[i] = 0;
				//	static const int adjs[8] = { -1,1,-BS,BS,-BS - 1,BS + 1,-BS + 1,BS - 1 };
				static const int adjx[8] = { -1,1,0,0,-1,1,1,-1 };
				static const int adjy[8] = { 0,0,-1,1,-1,1,-1,1 };
				for (int direction = 0; direction < 8; direction++)
				{
					for (int i = 1; i <= L; i++)//L
					{
						int x = x0 + adjx[direction] * i;
						int y = y0 + adjy[direction] * i;
						if (x < 0 || x >= BS || y < 0 || y >= BS || board.colors[x + BS * y] == 3 - color)
						{
							con[direction] *= 2;
							int remain = 4 - i + 1;
							for (int j = 0; j < remain; j++)//懒得化简了
							{
								con[direction] *= 2;
								con[direction] += 1;
							}
							break;
						}
						else if (board.colors[x + BS * y] == 0)
						{
							con[direction] *= 2;
						}
						else if (board.colors[x + BS * y] == color)
						{
							con[direction] *= 2;
							con[direction]++;
						}

					}
				}
				{//两直
					long ids[8];
					ids[0] = combine(con[0], con[1], con[2], con[3]);
					ids[1] = combine(con[0], con[1], con[3], con[2]);
					ids[2] = combine(con[1], con[0], con[2], con[3]);
					ids[3] = combine(con[1], con[0], con[3], con[2]);
					ids[4] = combine(con[2], con[3], con[1], con[0]);
					ids[5] = combine(con[2], con[3], con[0], con[1]);
					ids[6] = combine(con[3], con[2], con[1], con[0]);
					ids[7] = combine(con[3], con[2], con[1], con[0]);
					long id = *std::min_element(ids, ids + 8);
					if (color == 2)id = id | (1 << 20);
					id = id | (0 << 22);
					if(thisPointColor)id = id | (1 << 21);
					featureList[id]++;
				}
				{//两斜
					long ids[8];
					ids[0] = combine(con[4], con[5], con[6], con[7]);
					ids[1] = combine(con[4], con[5], con[7], con[6]);
					ids[2] = combine(con[5], con[4], con[6], con[7]);
					ids[3] = combine(con[5], con[4], con[7], con[6]);
					ids[4] = combine(con[6], con[7], con[5], con[4]);
					ids[5] = combine(con[6], con[7], con[4], con[5]);
					ids[6] = combine(con[7], con[6], con[5], con[4]);
					ids[7] = combine(con[7], con[6], con[5], con[4]);
					long id = *std::min_element(ids, ids + 8);

					if (color == 2)id = id | (1 << 20);
					id = id | (1 << 22);
					if (thisPointColor)id = id | (1 << 21);
					featureList[id]++;
				}
				{//一直一斜_1
					long id1, id2;
					id1 = combine(con[3], con[2], con[5], con[4]);
					id2 = combine(con[2], con[3], con[4], con[5]);
					long id = min(id1, id2);

					if (color == 2)id = id | (1 << 20);
					id = id | (2 << 22);
					if (thisPointColor)id = id | (1 << 21);
					featureList[id]++;
				}
				{//一直一斜_2
					long id1, id2;
					id1 = combine(con[3], con[2], con[7], con[6]);
					id2 = combine(con[2], con[3], con[6], con[7]);
					long id = min(id1, id2);

					if (color == 2)id = id | (1 << 20);
					id = id | (2 << 22);
					if (thisPointColor)id = id | (1 << 21);
					featureList[id]++;
				}
				{//一直一斜_3
					long id1, id2;
					id1 = combine(con[0], con[1], con[7], con[6]);
					id2 = combine(con[1], con[0], con[6], con[7]);
					long id = min(id1, id2);

					if (color == 2)id = id | (1 << 20);
					id = id | (2 << 22);
					if (thisPointColor)id = id | (1 << 21);
					featureList[id]++;
				}
				{//一直一斜_4
					long id1, id2;
					id1 = combine(con[0], con[1], con[4], con[5]);
					id2 = combine(con[1], con[0], con[5], con[4]);
					long id = min(id1, id2);

					if (color == 2)id = id | (1 << 20);
					id = id | (2 << 22);
					if (thisPointColor)id = id | (1 << 21);
					featureList[id]++;
				}
			}
		}

	}
	for (auto i = featureList.begin(); i != featureList.end(); i++)
	{
		far.featureIDs.push_back(i->first);
		far.featureCounts.push_back(i->second);
	}
	return far;

}
/*
FeaturesAndResult getFeaturesOfBoard(const Board& board)
{

	FeaturesAndResult far;
	far.value = board.result == 'W' ? 1 : board.result == 'D' ? 0.5 : 0;
	far.featureCounts.push_back(1);
	far.featureIDs.push_back(1);
	return far;
}
*/
double evaluate(const FeaturesAndResult& far, const FeaturesParams& params)
{
	double res = 0;
	for (int i = 0; i < far.featureCounts.size(); i++)
		res += params.params[far.featureIDs[i]] * far.featureCounts[i];
	return res;

}

int simpleWinlossMove(const Board& board, int& certainResult);

double evaluateWithDepth(const Board& board, const FeaturesParams& params, int depth, int& bestMove)//暴力法
{

	int certainResult;
	bestMove = simpleWinlossMove(board, certainResult);
	if (certainResult == 1)return 1e10;
	if (certainResult == -1)return -1e10;
	if (depth < 0)return 0;
	if (bestMove != -1)
	{
		Board newboard = inverseBoard(board);
		newboard.colors[bestMove] = 2;
		int loc;
		double eval = -evaluateWithDepth(newboard, params, depth, loc);
		return eval;
	}
	if (depth == 0)
	{
		return evaluate(getFeaturesOfBoard(board), params);
	}
	double maxEval = -1e30;

	bool isEmptyBoard = true;
	for (int i = 0; i < BS*BS; i++)
	{
		if (board.colors[i] != 0)
		{
			isEmptyBoard = false;
			break;
		}
	}

	for (int x = 0; x < BS; x++)
	{
		for (int y = 0; y < BS; y++)
		{
			if (board.colors[x + y * BS] != 0)continue;
			if (!isEmptyBoard)
			{
				bool foundNearbyStone = false;
				for (int xt = max(0, x - 3); xt <= min(BS - 1, x + 3); xt++)
				{
					for (int yt = max(0, y - 3); yt <= min(BS - 1, y + 3); yt++)
					{
						if (board.colors[xt + yt * BS] != 0)
						{
							foundNearbyStone = true;
							break;
						}
					}
					if (foundNearbyStone)break;
				}
				if (!foundNearbyStone)continue;
			}
			Board newboard = inverseBoard(board);
			newboard.colors[x + y * BS] = 2;
			int loc;
			double eval = -evaluateWithDepth(newboard, params, depth - 1, loc);
			if (eval > maxEval)
			{
				maxEval = eval;
				bestMove = x + y * BS;
			}
		}
	}
	return maxEval;

}
int simpleWinlossMove(const Board& board, int& certainResult)//考虑简单活三活四
{
	int move = -1;
	certainResult = 0;

	static const int adjx[8] = { -1,1,0,0,-1,1,1,-1 };
	static const int adjy[8] = { 0,0,-1,1,-1,1,-1,1 };

	for (int dir = 0; dir < 4; dir++)//找连五
	{
		int adja = dir * 2;
		int adjb = dir * 2 + 1;
		for (int x0 = 0; x0 < BS; x0++)
		{
			for (int y0 = 0; y0 < BS; y0++)
			{
				if (board.colors[x0 + y0 * BS] != 0)continue;
				int connectNum = 1;
				for (int i = 1; i < BS; i++)
				{
					int x = x0 + i * adjx[adja];
					int y = y0 + i * adjy[adja];
					if (x < 0 || x >= BS || y < 0 || y >= BS)break;
					if (board.colors[x + y * BS] != 1)break;
					connectNum++;
				}
				for (int i = 1; i < BS; i++)
				{
					int x = x0 + i * adjx[adjb];
					int y = y0 + i * adjy[adjb];
					if (x < 0 || x >= BS || y < 0 || y >= BS)break;
					if (board.colors[x + y * BS] != 1)break;
					connectNum++;
				}
				if (connectNum >= 5)
				{
					certainResult = 1;
					return x0 + BS * y0;
				}
			}
		}
	}

	int oppFiveMove = -1;
	for (int dir = 0; dir < 4; dir++)//找对手冲四活四
	{
		int adja = dir * 2;
		int adjb = dir * 2 + 1;
		for (int x0 = 0; x0 < BS; x0++)
		{
			for (int y0 = 0; y0 < BS; y0++)
			{
				if (board.colors[x0 + y0 * BS] != 0)continue;
				int connectNum = 1;
				for (int i = 1; i < BS; i++)
				{
					int x = x0 + i * adjx[adja];
					int y = y0 + i * adjy[adja];
					if (x < 0 || x >= BS || y < 0 || y >= BS)break;
					if (board.colors[x + y * BS] != 2)break;
					connectNum++;
				}
				for (int i = 1; i < BS; i++)
				{
					int x = x0 + i * adjx[adjb];
					int y = y0 + i * adjy[adjb];
					if (x < 0 || x >= BS || y < 0 || y >= BS)break;
					if (board.colors[x + y * BS] != 2)break;
					connectNum++;
				}
				if (connectNum >= 5)
				{
					if (oppFiveMove == -1)oppFiveMove = x0 + BS * y0;
					else//对手两个连五点，活四或者双四
					{
						certainResult = -1;
						return x0 + BS * y0;
					}
				}
			}
		}
	}
	if (oppFiveMove != -1)return oppFiveMove;//冲四必堵

	//找活四
	for (int dir = 0; dir < 4; dir++)
	{
		int adja = dir * 2;
		int adjb = dir * 2 + 1;
		for (int x0 = 0; x0 < BS; x0++)
		{
			for (int y0 = 0; y0 < BS; y0++)
			{
				if (board.colors[x0 + y0 * BS] != 0)continue;
				int connectNum = 1;
				bool life1 = false, life2 = false;//两端是不是活的
				for (int i = 1; i < BS; i++)
				{
					int x = x0 + i * adjx[adja];
					int y = y0 + i * adjy[adja];
					if (x < 0 || x >= BS || y < 0 || y >= BS)break;

					if (board.colors[x + y * BS] == 2)break;
					if (board.colors[x + y * BS] == 0)
					{
						life1 = true;
						break;
					}
					connectNum++;
				}
				for (int i = 1; i < BS; i++)
				{
					int x = x0 + i * adjx[adjb];
					int y = y0 + i * adjy[adjb];
					if (x < 0 || x >= BS || y < 0 || y >= BS)break;
					if (board.colors[x + y * BS] == 2)break;
					if (board.colors[x + y * BS] == 0)
					{
						life2 = true;
						break;
					}
					connectNum++;
				}
				if (connectNum >= 5)
				{
					cout << "Maybe some bugs here" << endl;
					return -1;
				}
				if (connectNum == 4 && life1&&life2)//活四
				{
					//	cout << "Maybe some bugs here" << endl;
					certainResult = 1;
					return x0 + BS * y0;
				}
			}
		}
	}

	return -1;

}


double applyGrad(const FeaturesAndResult& far, FeaturesParams& params, double lr)//return loss
{
	double res = evaluate(far, params);
	if (res > 20)res = 20;
	if (res < -20)res = -20;
	double ev = exp(res);
	double p = 1 - 1 / (ev + 1);
	double loss = -(far.value*log(p) + (1 - far.value)*log(1 - p));
	double baseGrad = (ev*(-1 + far.value) + far.value) / (1 + ev);
	//cout << baseGrad << endl;
	for (int i = 0; i < far.featureCounts.size(); i++)
		params.params[far.featureIDs[i]] += far.featureCounts[i] * baseGrad*lr;
	return loss;

}

double getLoss(const FeaturesAndResult& far, const FeaturesParams& params)//return loss
{
	double res = evaluate(far, params);
	if (res > 20)res = 20;
	if (res < -20)res = -20;
	double ev = exp(res);
	double p = 1 - 1 / (ev + 1);
	double loss = -(far.value*log(p) + (1 - far.value)*log(1 - p));
	return loss;
}
double applyGrad2(const FeaturesAndResult& far, FeaturesParams& params, double lr)//return loss
{
	double res = evaluate(far, params);
	double p = tanh(res);
	p = (p + 1) / 2;
	double loss = (far.value - p)*(far.value - p);
	double baseGrad = (p - far.value) / (1 + cosh(2 * res));
	//cout << baseGrad << endl;
	for (int i = 0; i < far.featureCounts.size(); i++)
		params.params[far.featureIDs[i]] -= far.featureCounts[i] * baseGrad*lr;
	return loss;

}

double getLoss2(const FeaturesAndResult& far, const FeaturesParams& params)//return loss
{
	double res = evaluate(far, params);
	double p = tanh(res);
	p = (p + 1) / 2;
	double loss = (far.value - p)*(far.value - p);
	return loss;
}
int getBestMove(const Board& board, const FeaturesParams& params)
{
	//
	int bestMove = -1;
	double bestValue = -100000;
	for (int x = 0; x < BS; x++)
		for (int y = 0; y < BS; y++)
		{
			if (board.colors[x + BS * y] != 0)continue;
			Board tmpboard = board;
			tmpboard.colors[x + BS * y] = 1;
			double value = evaluate(getFeaturesOfBoard(tmpboard), params);
			if (value > bestValue)
			{
				bestMove = x + BS * y;
				bestValue = value;
			}

		}
	return bestMove;
}
/*
int getBestMove(const Board& board, const FeaturesParams& params,int depth,double& eval)
{
	//
	int certainResult;
	int bestMove = simpleWinlossMove(board, certainResult);
	if (bestMove != -1)
	{
		if (certainResult == 1)eval = 1;
		if (certainResult == 1)eval = 1;

	}
	if (depth <= 0)
	{
		eval = 0;
	}
	double bestValue = -100000;
	for (int x = 0; x < BS; x++)
		for (int y = 0; y < BS; y++)
		{
			if (board.colors[x + BS * y] != 0)continue;
			Board tmpboard = board;
			tmpboard.colors[x + BS * y] = 1;
			double value = evaluate(getFeaturesOfBoard(tmpboard), params);
			if (value > bestValue)
			{
				bestMove = x + BS * y;
				bestValue = value;
			}

		}
	return bestMove;
}
*/
void dataToFeaturesSingleThread(const vector<Board>* data, int start, int end, vector<FeaturesAndResult>* featureData, int threadId)
{
	//cout << "thread" << threadId;
//	cout << " start" << start << " end " << end << endl;
	for (int i = start; i < end; i++)
	{
		featureData->push_back(getFeaturesOfBoard((*data)[i]));
		if ((i - end) % 10000 == 0)cout << "Thread " << threadId << " Remain " << (end - i) << " rows" << endl;

	}



}
void dataToFeatures(const vector<Board>& data, vector<FeaturesAndResult>& featureData)
{
	static const int threadNum = 6;
	vector<FeaturesAndResult> featureDataPart[threadNum];
	int eachSize = data.size() / threadNum;
	std::thread threads[threadNum];
	for (int id = 0; id < threadNum; id++)
	{
		int start = id * eachSize;
		int end = (id + 1) * eachSize;
		if (id == threadNum - 1)end = data.size();
		threads[id] = std::thread(dataToFeaturesSingleThread, &data, start, end, &featureDataPart[id], id);

	}
	for (int id = 0; id < threadNum; id++)
	{
		threads[id].join();
	}
	for (int id = 0; id < threadNum; id++)
	{
		featureData.insert(featureData.end(), featureDataPart[id].begin(), featureDataPart[id].end());
	}





}
void dataToFeatures(const vector<Board>& data, int beginPlace, int endPlace, vector<FeaturesAndResult>& featureData)
{
	static const int threadNum = 6;
	vector<FeaturesAndResult> featureDataPart[threadNum];
	int eachSize = (endPlace - beginPlace) / threadNum;
	std::thread threads[threadNum];
	for (int id = 0; id < threadNum; id++)
	{
		int start = beginPlace + id * eachSize;
		int end = beginPlace + (id + 1) * eachSize;
		if (id == threadNum - 1)end = endPlace;
		threads[id] = std::thread(dataToFeaturesSingleThread, &data, start, end, &featureDataPart[id], id);

	}
	for (int id = 0; id < threadNum; id++)
	{
		threads[id].join();
	}
	for (int id = 0; id < threadNum; id++)
	{
		featureData.insert(featureData.end(), featureDataPart[id].begin(), featureDataPart[id].end());
	}





}
int readParams(string fileName, FeaturesParams& params)
{
	ifstream fs(fileName);
	for (int i = 0; i < params.SIZE; i++)params.params[i] = 0;
	if (fs.good())
	{
		double tmp, epoch;
		fs >> epoch;
		fs >> tmp;
		fs >> tmp;
		fs >> tmp;
		fs >> tmp;
		fs >> tmp;
		if (tmp != FeaturesParams::SIZE)
		{
			cout << "Bad param file" << endl;
		}
		else
		{
			for (int i = 0; i < params.SIZE; i++)fs >> params.params[i];
			fs.close();
			return epoch;
		}
	}
	else
	{
		cout << "No param file" << endl;
	}
	return 0;
}
int main_train()
{
	//	vector<Board> data;
		//vector<Board> testdata;
	double lr;
	cout << "LR : ";
	cin >> lr;
	//readDataFile("train.data", data);
	//readDataFile("test.data", testdata);
	vector<FeaturesAndResult> featureData, testFeatureData;
	readFeatureFile("train.feature", featureData);
	readFeatureFile("test.feature", testFeatureData);
	//dataToFeatures(data, featureData);
	//dataToFeatures(testdata, testFeatureData);
	//data.resize(0);
	//testdata.resize(0);
	/*
	while (!testdata.empty())
	{
		testFeatureData.push_back(getFeaturesOfBoard(testdata[testdata.size() - 1]));
		if (testdata.size() % 100000 == 0)cout << "testData Remain " << testdata.size() << "rows" << endl;
		testdata.pop_back();
	}
	*/


	unsigned randomseed = std::chrono::system_clock::now().time_since_epoch().count();
	mt19937 rand_num(randomseed);
	uniform_int_distribution<long long> dist(0, featureData.size());  // 给定范围


	FeaturesParams params;
	int startEpoch = readParams("params.txt", params);
	//for (int i = 0; i < params.SIZE; i++)params.params[i] = 0;

	for (int epoch = startEpoch + 1; epoch < 1000000; epoch++)
	{
		double trainloss = 0;
		double testloss = 0;
		double testloss2 = 0;
		int trainNum = 0;
		int testNum = 0;
		for (int sample = 0; sample < 10000000; sample++)
		{
			int sampleID = dist(rand_num);
			trainNum++;
			trainloss += applyGrad(featureData[sampleID], params, lr);
		}
		for (int i = 0; i < testFeatureData.size(); i++)
		{

			testNum++;
			testloss += getLoss(testFeatureData[i], params);
			testloss2 += getLoss2(testFeatureData[i], params);
		}
		if (epoch % 10 == 0)
		{
			cout << "Writeing params..." << endl;
			ofstream paramsOutFile(to_string(epoch) + "_params.txt");
			paramsOutFile << epoch << "  " << trainloss / trainNum << " " << testloss / testNum << " " << testloss2 / testNum << " " << lr << endl;
			paramsOutFile << FeaturesParams::SIZE << endl;
			for (int i = 0; i < FeaturesParams::SIZE; i++)paramsOutFile << params.params[i] << " ";
			paramsOutFile.close();

			cout << "Writeing finished" << endl;
		}

		cout << "epoch " << epoch << " : train loss=" << trainloss / trainNum << "  test loss=" << testloss / testNum << "  test loss2=" << testloss2 / testNum << endl;;
	}
	//Board board;
//	board.result = 'W';
	//board.colors[0] = 1;
	//FeaturesAndResult far = getFeaturesOfBoard(board);
//	for (int i = 0; i < far.featureCounts.size(); i++)
	//	cout << "Feature " << far.featureIDs[i] << " :  Counts=" << far.featureCounts[i] << endl;

	Board board;
	while (1)
	{
		//	for (int i = 0; i < 2187 * 2; i++)cout << params.params[i] << " ";
		board.print();
		cout << "EV: " << evaluate(getFeaturesOfBoard(board), params) << endl;
		int bestMove = getBestMove(board, params);
		cout << "Best next Black move: x=" << bestMove % BS << " y=" << bestMove / BS << endl;
		int x, y, c;
		cin >> x >> y >> c;
		if (x >= 0 && x < BS&&y>00 && y < BS&&c >= 0 && c < 3)
			board.colors[x + BS * y] = c;

	}
	return 0;
}
int main_trainMultiFile()
{
	//	vector<Board> data;
		//vector<Board> testdata;

	const int infoNum = 1000000;
	int filenum;
	cout << "FILENUM : ";
	cin >> filenum;
	double lr,l2;
	cout << "LR : ";
	cin >> lr;
	cout << "L2 : ";
	cin >> l2;
	const int applyL2EachNSamples = 3000;
	double L2decreaseRatio = 1 - applyL2EachNSamples * lr*l2;
	//readDataFile("train.data", data);
	//readDataFile("test.data", testdata);
	vector<FeaturesAndResult> featureData;
	//dataToFeatures(data, featureData);
	//dataToFeatures(testdata, testFeatureData);
	//data.resize(0);
	//testdata.resize(0);
	/*
	while (!testdata.empty())
	{
		testFeatureData.push_back(getFeaturesOfBoard(testdata[testdata.size() - 1]));
		if (testdata.size() % 100000 == 0)cout << "testData Remain " << testdata.size() << "rows" << endl;
		testdata.pop_back();
	}
	*/


	unsigned randomseed = std::chrono::system_clock::now().time_since_epoch().count();
	mt19937 rand_num(randomseed); // 给定范围


	FeaturesParams params;
	int startEpoch = readParams("params.txt", params);
	//for (int i = 0; i < params.SIZE; i++)params.params[i] = 0;

	for (int epoch = startEpoch + 1; epoch < 1000000; epoch++)
	{
		double trainloss = 0;
		double testloss = 0;
		double testloss2 = 0;
		int trainNum = 0;
		int testNum = 0;
		for (int fileID = 0; fileID < filenum; fileID++)
		{
			cout << "Reading file " << fileID << "......";
			readFeatureFile("train_" + to_string(fileID) , featureData);
			cout << "finished" << endl;
			for (int sample = 0; sample < featureData.size() * 3; sample++)//过3遍是因为读文件太慢
			{
				uniform_int_distribution<long long> dist(0, featureData.size());

				int sampleID = dist(rand_num);
				/*
				while(featureData[sampleID].featureCounts.size()==0)
				{
					sampleID = dist(rand_num);
				}*/
				trainNum++;
				if (trainNum%applyL2EachNSamples == 0)for (int i = 0; i < FeaturesParams::SIZE; i++)params.params[i] *= L2decreaseRatio;
				trainloss += applyGrad(featureData[sampleID], params, lr);

			}
			cout << "epoch " << epoch << " file " << fileID << " trainNum " << trainNum << " avg loss=" << trainloss / trainNum << endl;
		}

		readFeatureFile("test", featureData);//test
		for (int i = 0; i < featureData.size(); i++)
		{

			testNum++;
			testloss += getLoss(featureData[i], params);
			testloss2 += getLoss2(featureData[i], params);
		}

		cout << "epoch " << epoch << " : train loss=" << trainloss / trainNum << "  test loss=" << testloss / testNum << "  test loss2=" << testloss2 / testNum << endl;;

		cout << "Writeing params..." << endl;
		ofstream paramsOutFile(to_string(epoch) + "_params.txt");
		paramsOutFile << epoch << "  " << trainloss / trainNum << " " << testloss / testNum << " " << testloss2 / testNum << " " << lr << endl;
		paramsOutFile << FeaturesParams::SIZE << endl;
		for (int i = 0; i < FeaturesParams::SIZE; i++)paramsOutFile << params.params[i] << " ";
		paramsOutFile.close();

		cout << "Writeing finished" << endl;

	}
	//Board board;
//	board.result = 'W';
	//board.colors[0] = 1;
	//FeaturesAndResult far = getFeaturesOfBoard(board);
//	for (int i = 0; i < far.featureCounts.size(); i++)
	//	cout << "Feature " << far.featureIDs[i] << " :  Counts=" << far.featureCounts[i] << endl;

	Board board;
	while (1)
	{
		//	for (int i = 0; i < 2187 * 2; i++)cout << params.params[i] << " ";
		board.print();
		cout << "EV: " << evaluate(getFeaturesOfBoard(board), params) << endl;
		int bestMove = getBestMove(board, params);
		cout << "Best next Black move: x=" << bestMove % BS << " y=" << bestMove / BS << endl;
		int x, y, c;
		cin >> x >> y >> c;
		if (x >= 0 && x < BS&&y>00 && y < BS&&c >= 0 && c < 3)
			board.colors[x + BS * y] = c;

	}
	return 0;
}
int main_trainWithoutFeatureFile()
{
	vector<Board> data;
	vector<Board> testdata;
	double lr;
	cout << "LR : ";
	cin >> lr;
	readDataFile("train.data", data);
	readDataFile("test.data", testdata);
	//dataToFeatures(data, featureData);
	//dataToFeatures(testdata, testFeatureData);
	//data.resize(0);
	//testdata.resize(0);
	/*
	while (!testdata.empty())
	{
		testFeatureData.push_back(getFeaturesOfBoard(testdata[testdata.size() - 1]));
		if (testdata.size() % 100000 == 0)cout << "testData Remain " << testdata.size() << "rows" << endl;
		testdata.pop_back();
	}
	*/


	unsigned randomseed = std::chrono::system_clock::now().time_since_epoch().count();
	mt19937 rand_num(randomseed);
	uniform_int_distribution<long long> dist(0, data.size());  // 给定范围


	FeaturesParams params;
	int startEpoch = readParams("params.txt", params);
	//for (int i = 0; i < params.SIZE; i++)params.params[i] = 0;

	for (int epoch = startEpoch + 1; epoch < 1000000; epoch++)
	{
		double trainloss = 0;
		double testloss = 0;
		double testloss2 = 0;
		int trainNum = 0;
		int testNum = 0;
		for (int sample = 0; sample < 10000000; sample++)
		{
			int sampleID = dist(rand_num);
			trainNum++;
			trainloss += applyGrad(getFeaturesOfBoard(data[sampleID]), params, lr);
		}
		for (int i = 0; i < testdata.size(); i++)
		{

			testNum++;
			testloss += getLoss(getFeaturesOfBoard(testdata[i]), params);
			testloss2 += getLoss2(getFeaturesOfBoard(testdata[i]), params);
		}
		if (epoch % 10 == 0)
		{
			cout << "Writeing params..." << endl;
			ofstream paramsOutFile(to_string(epoch) + "_params.txt");
			paramsOutFile << epoch << "  " << trainloss / trainNum << " " << testloss / testNum << " " << testloss2 / testNum << " " << lr << endl;
			paramsOutFile << FeaturesParams::SIZE << endl;
			for (int i = 0; i < FeaturesParams::SIZE; i++)paramsOutFile << params.params[i] << " ";
			paramsOutFile.close();

			cout << "Writeing finished" << endl;
		}

		cout << "epoch " << epoch << " : train loss=" << trainloss / trainNum << "  test loss=" << testloss / testNum << "  test loss2=" << testloss2 / testNum << endl;;
	}
	//Board board;
//	board.result = 'W';
	//board.colors[0] = 1;
	//FeaturesAndResult far = getFeaturesOfBoard(board);
//	for (int i = 0; i < far.featureCounts.size(); i++)
	//	cout << "Feature " << far.featureIDs[i] << " :  Counts=" << far.featureCounts[i] << endl;

	Board board;
	while (1)
	{
		//	for (int i = 0; i < 2187 * 2; i++)cout << params.params[i] << " ";
		board.print();
		cout << "EV: " << evaluate(getFeaturesOfBoard(board), params) << endl;
		int bestMove = getBestMove(board, params);
		cout << "Best next Black move: x=" << bestMove % BS << " y=" << bestMove / BS << endl;
		int x, y, c;
		cin >> x >> y >> c;
		if (x >= 0 && x < BS&&y>00 && y < BS&&c >= 0 && c < 3)
			board.colors[x + BS * y] = c;

	}
	return 0;
}
int main_convertSgf()
{

	cout << "Convert sgf to data file" << endl << "Filename: ";
	string filename;
	cin >> filename;
	convertSgfToData(filename);
	return 0;
}
int main_writeFeatures()
{
	cout << "Write Feature to file" << endl << "Filename: ";
	string filename;
	cin >> filename;
	vector<Board> data;
	readDataFile(filename + ".data", data);
	vector<FeaturesAndResult> featureData;
	dataToFeatures(data, featureData);
	writeFeatureFile(filename + ".feature", featureData);
}
int main_writeFeaturesToMultiFile()
{
	cout << "Write Feature to file" << endl << "Filename: ";
	string filename;
	cin >> filename;
	cout << "Out File Num: ";
	int filenum;
	cin >> filenum;
	vector<Board> data;
	readDataFile(filename + ".data", data);
	for (int fileID = 0; fileID < filenum; fileID++)
	{
		cout << "Now File " << fileID << endl << endl;
		int size = data.size() / filenum;
		int begin = size * fileID;
		int end = size * (fileID + 1);
		if (fileID == filenum + 1)end = data.size();

		vector<FeaturesAndResult> featureData;
		dataToFeatures(data, begin, end, featureData);
		writeFeatureFile(filename + "_" + to_string(fileID) , featureData);
	}
}
int main_test()
{

	Board board;
	board.result = 'W';
	//board.colors[0] = 1;
	FeaturesAndResult far = getFeaturesOfBoard(board);
	for (int i = 0; i < far.featureCounts.size(); i++)
		cout << "Feature " << bitset<23>(far.featureIDs[i]) << " :  Counts=" << far.featureCounts[i] << endl;
	return 0;
}
Board getRandomOpening(mt19937& rand_num)
{
	int b1 = rand_num() % (BS*BS);
	int w1, b2;
	int b1x = b1 % BS;
	int b1y = b1 / BS;
	while (1)
	{
		w1 = rand_num() % (BS*BS);
		int w1x = w1 % BS;
		int w1y = w1 / BS;
		if (w1 == b1)continue;
		if (((w1x - b1x)*(w1x - b1x) + (w1y - b1y)*(w1y - b1y)) < 6)break;
	}
	while (1)
	{
		b2 = rand_num() % (BS*BS);
		int b2x = b2 % BS;
		int b2y = b2 / BS;
		if (b2 == b1 || b2 == w1)continue;
		int w1x = w1 % BS;
		int w1y = w1 / BS;
		if (((w1x - b2x)*(w1x - b2x) + (w1y - b2y)*(w1y - b2y)) < 25)break;
	}
	Board board;
	board.colors[b1] = 1;
	board.colors[b2] = 1;
	board.colors[w1] = 2;
	return board;
}
int main_match()
{
	const int depth = 1;
	string paraname1, paraname2;
	cout << "para 1:";
	cin >> paraname1;
	cout << "para 2:";
	cin >> paraname2;
	int gamesNum;
	cout << "gamesNum:";
	cin >> gamesNum;

	unsigned randomseed = std::chrono::system_clock::now().time_since_epoch().count();
	mt19937 rand_num(randomseed);
	FeaturesParams params1, params2;
	readParams(paraname1, params1);
	readParams(paraname2, params2);
	int win = 0, lose = 0, draw = 0,doublewin=0,doublelose=0;
	for (int gameCount = 0; gameCount < gamesNum / 2; gameCount++)
	{
		Board initialboard = getRandomOpening(rand_num);
	//	initialboard.print();
		Board board = initialboard;
		int result1 = 0;
		int result2 = 0;
		for (int turnNum = 0; turnNum < 0.4*BS*BS; turnNum++)
		{

			int bestMove;
			double eval = evaluateWithDepth(board, params1, depth, bestMove);
			if (eval > 1e7)
			{
				win++;
				result1 = 1;
				break;
			}
			if (bestMove < 0)cout << "Nowhere to play" << endl;
			else
			{
				board.colors[bestMove] = 1;
			}

			eval = evaluateWithDepth(inverseBoard(board), params2, depth, bestMove);
			if (eval > 1e7)
			{
				lose++;

				result1 = -1;
				break;
			}
			if (bestMove < 0)cout << "Nowhere to play" << endl;
			else
			{
				board.colors[bestMove] = 2;
			}

		}
		board = initialboard;
		for (int turnNum = 0; turnNum < 0.4*BS*BS; turnNum++)
		{

			int bestMove;
			double eval = evaluateWithDepth(board, params2, depth, bestMove);
			if (eval > 1e7)
			{
				lose++;
				result2 = -1;
				break;
			}
			if (bestMove < 0)cout << "Nowhere to play" << endl;
			else
			{
				board.colors[bestMove] = 1;
			}

			eval = evaluateWithDepth(inverseBoard(board), params1, depth, bestMove);
			if (eval > 1e7)
			{
				win++;
				result2 = 1;
				break;
			}
			if (bestMove < 0)cout << "Nowhere to play" << endl;
			else
			{
				board.colors[bestMove] = 2;
			}

		}
		if (result1 == 1 && result2 == 1)doublewin++;
		if (result1 == -1 && result2 == -1)doublelose++;
		cout << "Win=" << win << " Lose=" << lose << " DoubleWin=" << doublewin <<  " DoubleLose=" << doublelose << " Draw=" << gameCount * 2 - win - lose+2 << endl;
	}
}
int main_play()
{

	FeaturesParams params;
	int startEpoch = readParams("params.txt", params);
	Board board;
	while (1)
	{
		//	for (int i = 0; i < 2187 * 2; i++)cout << params.params[i] << " ";
		board.print();
		cout << "Black EV: " << evaluate(getFeaturesOfBoard(board), params) << endl;
		cout << "White EV: " << evaluate(getFeaturesOfBoard(inverseBoard(board)), params) << endl;
		//int bestMoveB = getBestMove(board, params);
		//cout << "Best next Black move: " << char(bestMoveB % BS+'A') << char(bestMoveB / BS + 'A') << endl;
		//int bestMoveW = getBestMove(inverseBoard(board), params);
		//cout << "Best next White move: " << char(bestMoveW % BS + 'A') << char(bestMoveW / BS + 'A') << endl;
		string command;
		cin >> command;
		if (command == "play")
		{
			string colorstr;
			cin >> colorstr;
			int color;
			if (colorstr == "b" || colorstr == "B")color = 1;
			else if (colorstr == "w" || colorstr == "W")color = 2;
			else if (colorstr == "E" || colorstr == "e")color = 0;
			else
			{
				cout << "Error color" << endl;
				continue;
			}
			string place;
			cin >> place;
			if (place.size() != 2)
			{
				cout << "Error Loc" << endl;
				continue;
			}
			int x = place[0] - 'a';
			int y = place[1] - 'a';
			if (!(x >= 0 && x < BS&&y >= 0 && y < BS))
			{
				cout << "Out of board" << endl;
				continue;
			}
			board.colors[x + BS * y] = color;
		}

		else if (command == "genmove")
		{
			string colorstr;
			cin >> colorstr;
			if (colorstr == "b" || colorstr == "B")
			{
				int bestMove;
				double eval = evaluateWithDepth(board, params, 2, bestMove);
				cout << "Eval=" << eval << ", BestMove=" << char(bestMove % BS + 'A') << char(bestMove / BS + 'A') << endl;
				if (bestMove < 0)cout << "Nowhere to play" << endl;
				else
				{
					board.colors[bestMove] = 1;
				}
			}
			else if (colorstr == "w" || colorstr == "W")
			{

				int bestMove;
				double eval = evaluateWithDepth(inverseBoard(board), params, 2, bestMove);
				cout << "Eval=" << eval << ", BestMove=" << char(bestMove % BS + 'A') << char(bestMove / BS + 'A') << endl;
				if (bestMove < 0)cout << "Nowhere to play" << endl;
				else
				{
					board.colors[bestMove] = 2;
				}
			}
			else
			{
				cout << "Error color" << endl;
				continue;
			}

		}
		else if (command == "clear")
		{
			board = Board();
		}
		else
		{
			cout << "Error command" << endl;
		}


	}
	return 0;
}
int main_pv()
{
	const int maxDepth = 1;
	FeaturesParams params;
	ofstream logfile("D:\\piskvorks\\gom15\\a\\vt_log.txt");
	int startEpoch = readParams("D:\\piskvorks\\gom15\\a\\params.txt", params);
	Board board;
	int nextColor = 1;
	bool isInBoardCommand = false;
	while (1)
	{
		//	for (int i = 0; i < 2187 * 2; i++)cout << params.params[i] << " ";
		//int bestMoveB = getBestMove(board, params);
		//cout << "Best next Black move: " << char(bestMoveB % BS+'A') << char(bestMoveB / BS + 'A') << endl;
		//int bestMoveW = getBestMove(inverseBoard(board), params);
		//cout << "Best next White move: " << char(bestMoveW % BS + 'A') << char(bestMoveW / BS + 'A') << endl;
		string line,command;
		getline(cin, line);
		logfile << line << endl;
		for (int i = 0; i < line.size(); i++)
		{
			if (line[i] == ',')line[i] = ' ';
			if (line[i] >= 'A'&&line[i] <= 'Z')line[i] +=('a'-'A');
		}
		stringstream liness(line);
		if (line.size() >= 2)liness >> command;
		else continue;

		if (command == "info")
		{

		}
		else if ( command == "start")
		{

			board = Board();
			cout << "OK" << endl;
		}
		else if (command == "board")
		{
			board = Board();
			isInBoardCommand = true;
		}
		else if (command == "done")
		{
			isInBoardCommand = false;

			if (nextColor==1)
			{
				int bestMove;
				double eval = evaluateWithDepth(board, params, maxDepth , bestMove);
				//cout << "Eval=" << eval << ", BestMove=" << char(bestMove % BS + 'A') << char(bestMove / BS + 'A') << endl;
				if (bestMove < 0)bestMove = 0;
				
				board.colors[bestMove] = 1;
				cout << bestMove % BS << "," << bestMove / BS << endl;
			}
			else if (nextColor == 2)
			{

				int bestMove;
				double eval = evaluateWithDepth(inverseBoard(board), params, maxDepth , bestMove);
				//cout << "Eval=" << eval << ", BestMove=" << char(bestMove % BS + 'A') << char(bestMove / BS + 'A') << endl;
				if (bestMove < 0)bestMove = 0;

				board.colors[bestMove] = 2;
				cout << bestMove % BS << "," << bestMove / BS << endl;
			}
			nextColor = 3 - nextColor;

		}
		else if (isInBoardCommand == true)
		{
			stringstream newliness(line);
			int x, y;
			newliness >> x >> y;
			board.colors[x + y * BS] = nextColor;
			nextColor = 3 - nextColor;
		}
		else if (command == "turn")
		{
			//board.print();
			int x, y;
			liness >> x >> y;
			board.colors[x + y * BS] = nextColor;
			nextColor = 3 - nextColor;
			if (nextColor == 1)
			{
				int bestMove;
				double eval = evaluateWithDepth(board, params, maxDepth, bestMove);
				//cout << "Eval=" << eval << ", BestMove=" << char(bestMove % BS + 'A') << char(bestMove / BS + 'A') << endl;
				if (bestMove < 0)bestMove = 0;

				board.colors[bestMove] = 1;
				cout << bestMove % BS << "," << bestMove / BS << endl;
			}
			else if (nextColor == 2)
			{

				int bestMove;
				double eval = evaluateWithDepth(inverseBoard(board), params, maxDepth, bestMove);
				//cout << "Eval=" << eval << ", BestMove=" << char(bestMove % BS + 'A') << char(bestMove / BS + 'A') << endl;
				if (bestMove < 0)bestMove = 0;

				board.colors[bestMove] = 2;
				cout << bestMove % BS << "," << bestMove / BS << endl;
			}
			nextColor = 3 - nextColor;

		}
		else if (command == "play")
		{
			string colorstr;
			cin >> colorstr;
			int color;
			if (colorstr == "b" || colorstr == "B")color = 1;
			else if (colorstr == "w" || colorstr == "W")color = 2;
			else if (colorstr == "E" || colorstr == "e")color = 0;
			else
			{
				cout << "Error color" << endl;
				continue;
			}
			string place;
			cin >> place;
			if (place.size() != 2)
			{
				cout << "Error Loc" << endl;
				continue;
			}
			int x = place[0] - 'a';
			int y = place[1] - 'a';
			if (!(x >= 0 && x < BS&&y >= 0 && y < BS))
			{
				cout << "Out of board" << endl;
				continue;
			}
			board.colors[x + BS * y] = color;
		}

		else if (command == "genmove")
		{
			string colorstr;
			cin >> colorstr;
			if (colorstr == "b" || colorstr == "B")
			{
				int bestMove;
				double eval = evaluateWithDepth(board, params, 2, bestMove);
				cout << "Eval=" << eval << ", BestMove=" << char(bestMove % BS + 'A') << char(bestMove / BS + 'A') << endl;
				if (bestMove < 0)cout << "Nowhere to play" << endl;
				else
				{
					board.colors[bestMove] = 1;
				}
			}
			else if (colorstr == "w" || colorstr == "W")
			{

				int bestMove;
				double eval = evaluateWithDepth(inverseBoard(board), params, 2, bestMove);
				cout << "Eval=" << eval << ", BestMove=" << char(bestMove % BS + 'A') << char(bestMove / BS + 'A') << endl;
				if (bestMove < 0)cout << "Nowhere to play" << endl;
				else
				{
					board.colors[bestMove] = 2;
				}
			}
			else
			{
				cout << "Error color" << endl;
				continue;
			}

		}
		else if (command == "clear")
		{
			board = Board();
		}
		else
		{
			//cout << "Error command" << endl;
		}


	}
	return 0;
}
int main()
{
	cout << "do what?" << endl;
	cout << "1 convertSgf" << endl;
	cout << "2 writeFeatures" << endl;
	cout << "3 writeFeaturesToMultiFile" << endl;
	cout << "4 train" << endl;
	cout << "5 trainWithoutFeatureFile" << endl;
	cout << "6 trainMultiFile" << endl;
	cout << "7 play" << endl;
	cout << "8 match" << endl;
	int choose;
	cin >> choose;
	if (choose == 1)main_convertSgf();
	else if (choose == 2)main_writeFeatures();
	else if (choose == 3)main_writeFeaturesToMultiFile();
	else if (choose == 4)main_train();
	else if (choose == 5)main_trainWithoutFeatureFile();
	else if (choose == 6)main_trainMultiFile();
	else if (choose == 7)main_play();
	else if (choose == 8)main_match();
}