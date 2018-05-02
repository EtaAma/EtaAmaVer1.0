#pragma once

#include "NetMsg.h"
#include "AmazonsGameController.h"

#define MAX_MSG_COUNT 10

class CAmazonsView;

//Ҫ����Ҵ�����Ի����ֶ��������ӣ�����һ��ʼ��û�д������Ӻ��̵߳�
class CNetworkPlayer
{
public: 
	CNetworkPlayer();
	~CNetworkPlayer();

private:
	bool m_bHasInit;
	SOCKET m_socket;
	SPosition m_posSelected;//����ʱ����Ҫ���͵ģ�����ʱ������ǽ��յ���
	SPosition m_posMoveTo;//����ʱ����Ҫ���͵ģ�����ʱ������ǽ��յ���
	SPosition m_posShootAt;//����ʱ����Ҫ���͵ģ�����ʱ������ǽ��յ���
	CWinThread *m_pThreadSend;
	CWinThread *m_pThreadRecv;
	bool m_bConnected;

	char m_recvBuffer[4096];
	char m_sendBuffer[4096];
	int m_nRecvBufferLen;
	int m_nSendBufferLen;//����ʧ�ܺ󣬰�װ����������·���

	int m_nId;
	int m_nOpponentId;
	int m_nPort;
	long m_lIp;//addr��ʽ�ģ�socketֱ���þ���
	TCHAR m_szName[NET_MAX_PLAYER_NAME_LENGTH];
	CString m_strLogTemp;
	
	//�����߳���
	SNetMsgBase *m_msgArray[MAX_MSG_COUNT];//һ����Ϣռ��һ��λ��

public:
	CAmazonsView *m_pView;
	SNetMsgScPlayerList m_playerList;
	CString m_strLog;
	int m_nplayerCamp;

private:
	void Init();
	void ConnectToServer();
	void SendMsgToBuffer(SNetMsgBase *pMsg);
	void SendMsg();
	void RecvMsg();
	void ReadMsgFromBufferAndDispatch();
	void TipSocketError(CString msg);
	void Log(CString str);

public:
	void SetNetwork(CString strPlayerName, long ip, int nPort);
	void Start();
	void Disconnect();
	void Stop();
	void CancelChallenge();
	bool IsStarted();
	bool IsConnected();
	bool HasOpponent(){ return m_nOpponentId > 0; }
	int GetId(){ return m_nId; }
	int GetOpponentId(){ return m_nOpponentId; }
	void GetMove(SPosition &posSelected, SPosition &posMoveTo, SPosition &posShootAt);
	//
	void RequireId();
	void SendPlayerInfo();
	void RequirePlayerList();
	void SendChallengeMsg(int nInviteeId);
	void SendChallengeConfirmMsg(int nInviterId, int nInviteeId, int nAnswer);
	void SendCancelChallengeMsg();
	void SendMoveMsg(SPosition posSelected, SPosition posMoveTo, SPosition posShootAt);
	void SendGameControlMsg(int action);
	//
	void OnIdReceived(SNetMsgScId &msg);
	void OnPlayerListReceived(SNetMsgScPlayerList &msg);
	void OnChallenge(SNetMsgScChallenge &msg);
	void OnChallengConfirmRecevied(SNetMsgScChallengeConfirm &msg);
	void OnCancelChallengRecevied(SNetMsgScCancelChallenge &msg);
	void OnMoveReceved(SNetMsgScMove &msg);
	void OnGameControlReceved(SNetMsgScGameControl &msg);
	void OnErrorReceived(SNetMsgScError &msg);
	//
	SNetPlayerInfo * GetPlayerInfoById(int playerId);

public:
	static UINT WorkThread_Send(LPVOID pParam);
	static UINT WorkThread_Recv(LPVOID pParam);
};