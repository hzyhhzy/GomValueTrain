#pragma once
// ValueTrain.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <cstdio>
using namespace std;
#define CURR_RULE 1
#define VERSION 1
const int BS = 15;
struct Board
{
	char colors[BS*BS];//0empty   1my  2opp
	char nextColor;//'B' 'W'
	char result;//'W' 'L' 'D' 'N'unknown
	float value;
	float drawvalue;
	Board()
	{
		for (int i = 0; i < BS*BS; i++)colors[i] = 0;
		nextColor = 0;
		result = 'N';
		value = 0;
		drawvalue = 0;
	}
	void print()
	{
		cout << "  ";
		for (int x = 0; x < BS; x++)
		{
			cout << char('A' + x) << " ";
		}
		cout << endl;
		for (int x = 0; x < BS; x++)
		{

			cout << char('A' + x) << " ";
			for (int y = 0; y < BS; y++)
			{
				char color = colors[y + BS * x];//xy反了
				if (color == 0)cout << ". ";
				else if (color == 1)cout << "X ";
				else if (color == 2)cout <<"O ";
				else cout << "? ";

			}
			cout << endl;
		}
		cout << endl;
		cout << "nextColor=" << nextColor << "  result=" << result << "  value=" << value << " drawvalue=" << drawvalue << endl ;
	}
};


struct DataFileHeader
{
	long version;
	long boardSize;
	long rule;//freestyle=1,standard=2,renju=3
	long rowsize;//sizeof(Board)
	unsigned long size;//data rows
};
Board inverseBoard(const Board& board)
{
	Board newboard;
	newboard.value = -board.value;
	newboard.drawvalue = board.drawvalue;
	if (board.nextColor == 'W')newboard.nextColor = 'B';
	else if (board.nextColor == 'B')newboard.nextColor = 'W';
	//else cout << "myColor error";
	if (board.result == 'W')newboard.result = 'L';
	else if (board.result == 'L')newboard.result = 'W';
	else newboard.result = board.result;
	for (int i = 0; i < BS*BS; i++)
	{
		if (board.colors[i] == 2)newboard.colors[i] = 1;
		else if (board.colors[i] == 1)newboard.colors[i] = 2;
		//else newboard.colors[i] = board.colors[i];
	}
	return newboard;
}
bool loadOneSgf(const string& sgfStr, vector<Board>& list);
int convertSgfToData(string filename)
{
	ifstream sgfStream(filename + ".sgfs");
	FILE *dataFile;
	errno_t err;
	err = fopen_s(&dataFile, (filename + ".data").data(), "wb");
	// = fopen_s((filename + ".data").data(),"wb");
	if (!sgfStream.good())
	{
		cout << "Failed to read " << filename << ".sgfs" << endl;
		return 1;
	}
	if (err)
	{
		cout << "Failed to open " << filename << ".data" << endl;
		return 1;
	}
	vector<Board> list;
	string oneSgf;
	int count = 0;
	while (getline(sgfStream, oneSgf, ')'))
	{
		if (oneSgf.size() > 10)
		{
			if (loadOneSgf(oneSgf, list))
			{
				count++;
				if (count % 100 == 0)cout << "successfully load sgf " << count << endl;
			}
			else
			{
				count++;
				cout << "failed to load sgf " << count << endl;
			}
		}
	}
	DataFileHeader fileHeader;
	fileHeader.version = VERSION;
	fileHeader.boardSize = 15;
	fileHeader.rowsize = sizeof(Board);
	fileHeader.rule = CURR_RULE;
	fileHeader.size = list.size();
	fwrite(&fileHeader, sizeof(fileHeader), 1, dataFile);
	fwrite(list.begin()._Ptr, sizeof(Board), list.size(), dataFile);
	fclose(dataFile);
	/*
	for (int i = 0; i < list.size(); i++)
	{
	//	list[i].print();
	}
	dataStream.close();*/
}
bool loadOneSgf(const string& sgfStr, vector<Board>& list)
{

	if (sgfStr.size() < 10)return false;//empty sgf
	if (sgfStr[sgfStr.size() - 1] != ']')
	{
		cout << "Bad sgf" << endl;
		cout << sgfStr << endl;
		return false;
	}
	stringstream ss(sgfStr);
	string temp;
	getline(ss, temp, ';');
	if (temp[temp.size() - 1] != '(')return false;

	Board board;
	board.nextColor = 'B';
	//读取sgf属性
	float komi;
	float score;
	float result;
	getline(ss, temp, ';');
	if (temp.size() < 10)return false;
	stringstream infoss(temp);
	string item, value;
	int itemcount = 0;//保证SZ KM RE的顺序
	while (getline(infoss, item, '[') && getline(infoss, value, ']'))
	{
		if (item == "SZ")
		{
			itemcount = 1;
			int sgfbs;
			stringstream(value) >> sgfbs;
			if (sgfbs != BS)
			{
				cout << "sgfbs != BS" << endl;
				return false;
			}

		}
		/*if (item == "KM")
		{
			if (itemcount != 1)return false;
			itemcount = 2;
			stringstream(value) >> komi;
			if (!(komi < 50 && komi > -50))return false;
			//	if (komi>7.1||komi<6.9)return false;

		}*/
		if (item == "RE")
		{
			if (itemcount != 1)return false;
			itemcount = 2;
			if (value[0] == 'V')//draw
			{
				board.result = 'D';
			}

			else if (value[0] == 'B')
			{
				board.result = 'W';
			}
			else if (value[0] == 'W')
			{
				board.result = 'L';
			}
			else
			{
				cout << "Unknown value[0]" << endl;
				return false;
			}
		}
		if (item == "AB")
		{
			if (itemcount != 2)return false;
			itemcount = 3;
			if (value.length() != 2)return false;
			int x = value[0] - 'a', y = value[1] - 'a';
			board.colors[x + y * BS] = 1;
		}
		if (item == ""&&itemcount == 3)//reading AB
		{
			if (value.length() != 2)return false;
			int x = value[0] - 'a', y = value[1] - 'a';
			board.colors[x + y * BS] = 1;
		}
		if (item == "AW")
		{
			if (itemcount != 3)return false;
			itemcount = 4;
			if (value.length() != 2)return false;
			int x = value[0] - 'a', y = value[1] - 'a';
			board.colors[x + y * BS] = 2;
		}
		if (item == ""&&itemcount == 4)//reading AW
		{
			if (value.length() != 2)return false;
			int x = value[0] - 'a', y = value[1] - 'a';
			board.colors[x + y * BS] = 2;
		}
	}
	while (getline(ss, temp, ';'))
	{
		//	s.print();
		if (temp.empty())return true;
		unsigned char color = temp[0];
		if (color != 'B'&&color != 'W')continue;

		if (temp[2] == ']')
		{
			cout << "pass is not allowed";
			return false;
		}
		unsigned char x, y;
		x = temp[2] - 'a';
		y = temp[3] - 'a';
		if (temp.size() != 26)
		{
			cout << "B[xx]C[xxxxxx] error\n";
			return false;
		}
		stringstream valuess(temp.substr(7, 22));
		float whiteWin, blackWin, drawValue;
		valuess >> whiteWin >> blackWin >> drawValue;
		board.value = blackWin - whiteWin;
		board.drawvalue = drawValue;

		if (color == 'B')
		{
			list.push_back(board);
			board.colors[x + y * BS] = 1;
		}
		else if (color == 'W')
		{
			list.push_back(inverseBoard(board));
			board.colors[x + y * BS] = 2;
		}



	}
	return true;
}

bool readDataFile(string filename, vector<Board>& list)
{
	if (list.size() != 0)return false;
	FILE *dataFile;
	errno_t err;
	err = fopen_s(&dataFile, filename.data(), "rb");
	DataFileHeader fileHeader;
	fread(&fileHeader, sizeof(DataFileHeader), 1, dataFile);
	if (fileHeader.boardSize != 15 || fileHeader.rowsize != sizeof(Board) || fileHeader.rule != CURR_RULE || fileHeader.version != VERSION)
	{
		cout << "This file is not supported" << endl;
		return false;
	}
	int size = fileHeader.size;
	list.resize(size);
	fread(list.begin()._Ptr, sizeof(Board), size, dataFile);
	fclose(dataFile);
	return true;

}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门提示: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
