#ifndef AMAZONS_AI__H
#define AMAZONS_AI__H

#include "AmazonsGameController.h"

class CAmazonsAI
{
public:
	int m_nOutMaxValue;//(���)�����������ֵ
	int m_nOutDepth;//(���)�������
	int m_nOutNumOfMoves;//(���)�����ŷ���
	int m_nOutNumOfNodes;//(���)��������Ľڵ���(���������Ľڵ���)(������)
	int m_nOutNumOfSkippedNodes;//(���)����������Ľڵ���(���ӵ��Ľڵ�Ҳ�����)

public:
	void AICompute(
		int plate[10][10],			//��������
		SPosition pieces[],			//8�����ӵ�λ�ã�����Ϊ9
		bool isPlayer1,				//�Ƿ�����Ϊ���1���м��㣬������Ϊ���2���м���
		SPosition& outPosSelected,	//�������
		SPosition& outPosMoveTo,	//�������
		SPosition& outPosShootAt);	//�������

	void StopComputing(){};
};
#endif