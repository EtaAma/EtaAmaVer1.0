#include "StdAfx.h"
#include "AmazonsGameController.h"
#include "AmazonsAI.h"

#include "iostream"
#include "algorithm"
#include "queue"
#include "cmath"
using namespace std;

const int dx[] = { 0, 0, 1, -1, 1, -1, -1, 1};
const int dy[] = { 1, -1, 0, 0, 1, 1, -1, -1};

const int INF = 0x3f3f3f3f;

//定义超参数
const double tk = 0;
const double f1[4] = { 1, 0, 0, 0 };
const double f2[4] = { 0.3, 0.2, 0.2, 0.3 };
const double f3[4] = { 0, 0.3, 0.3, 0.4 };
const double f4[4] = { 0, 0.3, 0.4, 0.3 };


//定义估值函数
double AreaValue(
	int plate[][10],
	SPosition pieces[],
	bool isPlayer1)
{
	int minGrid1[10][10];
	int minGrid2[10][10];
	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 10; j++)
			minGrid1[i][j] = minGrid2[i][j] = INF;

	for (int k = 1; k <= 4; k++)
	{
		queue<SPosition> que;
		que.push(pieces[k]);
		minGrid1[pieces[k].i][pieces[k].j] = 0;

		while (que.size())
		{
			SPosition now = que.front();
			que.pop();

			for (int m = 0; m < 8; m++)
			{
				int nx = now.i + dx[m], ny = now.j + dy[m];
				if (nx >= 0 && nx < 10 && ny >= 0 && ny < 10 && plate[nx][ny] == 0 && minGrid1[nx][ny]>minGrid1[now.i][now.j] + 1)
				{
					minGrid1[nx][ny] = minGrid1[now.i][now.j] + 1;
					que.push(SPosition(nx, ny));
				}
			}
		}
	}

	for (int k = 5; k <= 8; k++)
	{
		queue<SPosition> que;
		que.push(pieces[k]);
		minGrid2[pieces[k].i][pieces[k].j] = 0;

		while (que.size())
		{
			SPosition now = que.front();
			que.pop();

			for (int m = 0; m < 8; m++)
			{
				int nx = now.i + dx[m], ny = now.j + dy[m];
				if (nx >= 0 && nx < 10 && ny >= 0 && ny < 10 && plate[nx][ny] == 0 && minGrid2[nx][ny]>minGrid2[now.i][now.j] + 1)
				{
					minGrid2[nx][ny] = minGrid2[now.i][now.j] + 1;
					que.push(SPosition(nx, ny));
				}
			}
		}
	}


	int minPath1[10][10];
	int minPath2[10][10];
	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 10; j++)
			minPath1[i][j] = minPath2[i][j] = INF;

	for (int k = 1; k <= 4; k++)
	{
		queue<SPosition> que;
		que.push(pieces[k]);
		minPath1[pieces[k].i][pieces[k].j] = 0;

		while (que.size())
		{
			SPosition now = que.front();
			que.pop();

			for (int m = 0; m < 8; m++)
			{
				int nx = now.i + dx[m], ny = now.j + dy[m];
				while (nx >= 0 && nx < 10 && ny >= 0 && ny < 10 && plate[nx][ny] == 0)
				{
					if (minPath1[nx][ny] > minPath1[now.i][now.j] + 1)
					{
						minPath1[nx][ny] = minPath1[now.i][now.j] + 1;
						que.push(SPosition(nx, ny));
					}
					nx += dx[m], ny += dy[m];
				}
			}
		}
	}

	for (int k = 5; k <= 8; k++)
	{
		queue<SPosition> que;
		que.push(pieces[k]);
		minPath2[pieces[k].i][pieces[k].j] = 0;

		while (que.size())
		{
			SPosition now = que.front();
			que.pop();

			for (int m = 0; m < 8; m++)
			{
				int nx = now.i + dx[m], ny = now.j + dy[m];
				while (nx >= 0 && nx < 10 && ny >= 0 && ny < 10 && plate[nx][ny] == 0)
				{
					if (minPath2[nx][ny] > minPath2[now.i][now.j] + 1)
					{
						minPath2[nx][ny] = minPath2[now.i][now.j] + 1;
						que.push(SPosition(nx, ny));
					}
					nx += dx[m], ny += dy[m];
				}
			}
		}
	}

	double t1 = 0, t2 = 0;
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			if (plate[i][j] == 0)
			{
				if (minPath1[i][j] == minPath2[i][j])
				{
					if (minPath1[i][j] != INF)
						t1 += tk;
				}
				else if (minPath1[i][j] > minPath2[i][j])
				{
					t1 += -1;
				}
				else
				{
					t1 += 1;
				}
			}
		}
	}
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			if (plate[i][j] == 0)
			{
				if (minGrid1[i][j] == minGrid2[i][j])
				{
					if (minGrid1[i][j] != INF)
						t2 += tk;
				}
				else if (minGrid1[i][j] > minGrid2[i][j])
				{
					t2 += -1;
				}
				else
				{
					t2 += 1;
				}
			}
		}
	}

	double c1 = 0, c2 = 0;
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			if (plate[i][j] == 0)
			{
				c1 += pow(2, -minPath1[i][j]) - pow(2, -minPath2[i][j]);
			}
		}
	}
	c1 *= 2;
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			if (plate[i][j] == 0)
			{
				c2 += min(1, max(-1, (minGrid2[i][j] - minGrid1[i][j]) / 6));
			}
		}
	}

	double w = 0;
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			if (plate[i][j] == 0 && minPath1[i][j] < INF && minPath2[i][j] < INF)
			{
				w += pow(2, -abs(minPath1[i][j] - minPath2[i][j]));
			}
		}
	}

	double t;
	int tg;
	if (w <= 1)
		tg = 0;
	else if (w <= 45)
		tg = 1;
	else if (w <= 55)
		tg = 2;
	else
		tg = 3;
	t = f1[tg] * t1 + f2[tg] * c1 + f3[tg] * c2 + f4[tg] * t2;

	TRACE("AreaValue:\n");
	TRACE("w: %lf\n", w);

	return t;
}



void CAmazonsAI::AICompute(
	int plate[10][10],			//棋盘数据
	SPosition pieces[],			//8个棋子的位置，长度为9
	bool isPlayer1,				//是否是作为玩家1进行计算，否则作为玩家2进行计算
	SPosition& outPosSelected,	//输出着子
	SPosition& outPosMoveTo,	//输出着子
	SPosition& outPosShootAt)	//输出着子
{
	m_nOutMaxValue = 0;
	m_nOutDepth = 0;
	m_nOutNumOfMoves = 0;
	m_nOutNumOfNodes = 0;
	m_nOutNumOfSkippedNodes = 0;

	bool bFinished = false;

	while (!bFinished)
	{
		//随机选择一个棋子
		int piece = 0;
		if (isPlayer1)
			piece = rand() % 4 + 1;
		else
			piece = rand() % 4 + 5;
		outPosSelected = pieces[piece];

		//随机走子位置
		SPosition pos;
		pos.i = rand() % 10;
		pos.j = rand() % 10;

		if (CAmazonsGameController::CanMoveOrShoot(plate, pieces, outPosSelected, pos))
		{
			CAmazonsGameController::Move(plate, pieces, outPosSelected, pos);
			outPosMoveTo = pos;
		}
		else
			continue;

		//随机放箭
		pos.i = rand() % 10;
		pos.j = rand() % 10;
		if (CAmazonsGameController::CanMoveOrShoot(plate, pieces, outPosMoveTo, pos))
		{
			outPosShootAt = pos;
			bFinished = true;
		}
		else
		{
			CAmazonsGameController::CancelMove(plate, pieces, outPosSelected, outPosMoveTo);
		}

		//估值函数
		if (bFinished)
			TRACE("t: %lf\n", AreaValue(plate, pieces, isPlayer1));
	}
}