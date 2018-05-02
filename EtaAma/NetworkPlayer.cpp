#include "stdafx.h"
#include "resource.h"
#include "AmazonsGameController.h"
#include "AmazonsView.h"
#include "NetworkPlayer.h"

CNetworkPlayer::CNetworkPlayer()
{
	m_bHasInit = false;
	m_pThreadSend = NULL;
	m_pThreadRecv = NULL;
	m_socket = INVALID_SOCKET;
	m_playerList.pArray = NULL;
	Init();
}

CNetworkPlayer::~CNetworkPlayer()
{
}

void CNetworkPlayer::Init()
{
	m_bConnected = false;
	m_nRecvBufferLen = 0;
	m_nSendBufferLen = 0;
	m_nId = 0;
	memset(m_msgArray, 0, sizeof(m_msgArray));
}

void CNetworkPlayer::SetNetwork(CString strPlayerName, long ip, int nPort)
{
	m_nPort = nPort;
	m_lIp = ip;

	memset(m_szName, 0, sizeof(m_szName));
	int n = strPlayerName.GetLength();
	if (n > NET_MAX_PLAYER_NAME_LENGTH)
		n = NET_MAX_PLAYER_NAME_LENGTH;
//	CString::CopyChars(m_szName, strPlayerName,  n);
}

void CNetworkPlayer::Start()
{
	if (!m_bHasInit)
	{
		Init();
		m_bHasInit = true;
	}

	//�߳�
	if (m_pThreadSend == NULL)
		m_pThreadSend = AfxBeginThread(WorkThread_Send, this);
	if (m_pThreadRecv == NULL)
		m_pThreadRecv = AfxBeginThread(WorkThread_Recv, this);

	//socket������socket��send�߳̾ͻ����ӷ�����
	if (m_socket == INVALID_SOCKET)
	{
		m_socket = socket(AF_INET, SOCK_STREAM, 0);
		if (m_socket == INVALID_SOCKET)
			TipSocketError(_T("socket����ʧ��"));

		//��Ϊ����ѽ��ܺ�ʧ�ܵ�˳���������Ըĳɷ�������
		//���ο�����״̬�£���Ҫ�ȴ�������ҵ������ս��Ϣ����������Ļ��Ϳ�ס�ˣ�Ҫ��������͵õȳ�ʱ��
		//int nNetTimeout = 1000;//1�룬
		//���÷��ͳ�ʱ
		//setsockopt(m_socket, SOL_SOCKET, SO_SNDTIMEO, (char*)&nNetTimeout, sizeof(int));
		//���ý��ճ�ʱ
		//setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&nNetTimeout, sizeof(int));

		//if (ret == SOCKET_ERROR)//����ʧ�ܡ�    
		//	TipSocketError(_T("socket���óɷ�����ģʽʧ��"));
	}
}

void CNetworkPlayer::Disconnect()
{
	if (m_socket != INVALID_SOCKET)
	{
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}

	//id��������
	int id = m_nId;
	Init();
	m_nId = id;
}

void CNetworkPlayer::Stop()
{
	if (m_socket != INVALID_SOCKET)
	{
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}

	if (m_pThreadSend != NULL)
	{
		::TerminateThread(m_pThreadSend->m_hThread, 1);
		delete m_pThreadSend;
	}

	if (m_pThreadRecv != NULL)
	{
		::TerminateThread(m_pThreadRecv->m_hThread, 1);
		delete m_pThreadRecv;
	}

	if (m_playerList.pArray != NULL)
	{
		delete m_playerList.pArray;
		m_playerList.pArray = NULL;
	}

	for (int i = 0; i < MAX_MSG_COUNT; i++)
	{
		if (m_msgArray[i] != NULL)
			delete m_msgArray[i];
		m_msgArray[i] = NULL;
	}
}

void CNetworkPlayer::CancelChallenge()
{
	m_nOpponentId = -1;
	SendCancelChallengeMsg();
	m_pView->PostMessage(WM_NETWORK_DISCONNECT_TO_OPPONENT);
}

void CNetworkPlayer::RequireId()
{
	SNetMsgCsRequireId *pMsg = new SNetMsgCsRequireId();
	SendMsgToBuffer(pMsg);
}

void CNetworkPlayer::SendPlayerInfo()
{
	SNetMsgCsPlayerInfo *pMsg = new SNetMsgCsPlayerInfo();
	pMsg->playerId = m_nId;
	pMsg->playerCamp = m_nplayerCamp;
	memcpy(pMsg->name, m_szName, sizeof(m_szName));
	SendMsgToBuffer(pMsg);
}

void CNetworkPlayer::RequirePlayerList()
{
	SNetMsgCsRequirePlayerList *pMsg = new SNetMsgCsRequirePlayerList();
	pMsg->playerId = m_nId;
	SendMsgToBuffer(pMsg);
}

void CNetworkPlayer::SendChallengeMsg(int nInviteeId)
{
	SNetMsgCsChallenge *pMsg = new SNetMsgCsChallenge();
	pMsg->inviterId = m_nId;
	pMsg->inviteeId = nInviteeId;
	SendMsgToBuffer(pMsg);
}

void CNetworkPlayer::SendChallengeConfirmMsg(int nInviterId, int nInviteeId, int nAnswer)
{
	SNetMsgCsChallengeConfirm *pMsg = new SNetMsgCsChallengeConfirm();
	pMsg->inviterId = nInviterId;
	pMsg->inviteeId = nInviteeId;
	pMsg->answer = nAnswer;
	SendMsgToBuffer(pMsg);
}

void CNetworkPlayer::SendCancelChallengeMsg()
{
	SNetMsgCsCancelChallenge *pMsg = new SNetMsgCsCancelChallenge();
	pMsg->playerId = m_nId;
	SendMsgToBuffer(pMsg);
}

void CNetworkPlayer::SendMoveMsg(SPosition posSelected, SPosition posMoveTo, SPosition posShootAt)
{
	SNetMsgCsMove *pMsg = new SNetMsgCsMove;
	pMsg->playerId = m_nId;
	pMsg->ijArray[0] = posSelected.i; pMsg->ijArray[2] = posMoveTo.i; pMsg->ijArray[4] = posShootAt.i;
	pMsg->ijArray[1] = posSelected.j; pMsg->ijArray[3] = posMoveTo.j; pMsg->ijArray[5] = posShootAt.j;
	SendMsgToBuffer(pMsg);
}

void CNetworkPlayer::SendGameControlMsg(int action)
{
	SNetMsgCsGameControl *pMsg = new SNetMsgCsGameControl;
	pMsg->playerId = m_nId;
	pMsg->action = action;
	SendMsgToBuffer(pMsg);
}

void CNetworkPlayer::ConnectToServer()
{
	sockaddr_in addr = { 0 };
	addr.sin_family = AF_INET;
	addr.sin_port = htons(m_nPort);
	addr.sin_addr.S_un.S_addr = m_lIp;
	int rt = connect(m_socket, (sockaddr*)&addr, sizeof(addr));

	if (rt != SOCKET_ERROR)
	{
		Log(_T("���ӳɹ�\r\n"));
		RequireId();
		m_bConnected = true;
	}
	else
	{
		TipSocketError(_T("������Ϸ������ʧ��"));
	}
}

void CNetworkPlayer::SendMsgToBuffer(SNetMsgBase *pMsg)
{
	int pos = 0;
	for (; pos < MAX_MSG_COUNT; pos++)
	{
		if (m_msgArray[pos] == NULL)
			break;
	}

	if (pos >= MAX_MSG_COUNT)
	{
		AfxMessageBox(_T("��Ϣ��������"));
		return;
	}

	m_msgArray[pos] = pMsg;
}

void CNetworkPlayer::SendMsg()
{
	int pos = 0;
	for (; pos < MAX_MSG_COUNT; pos++)
	{
		if (m_msgArray[pos] != NULL)
			break;
	}

	if (pos >= MAX_MSG_COUNT)
		return;

	SNetMsgBase *pMsg = m_msgArray[pos];
	
	m_strLogTemp.Format(_T("send msg, id = %d [%s]\r\n"), pMsg->msgId, SNetMsgName::GetMsgName(pMsg->msgId));
	Log(m_strLogTemp);

	m_nSendBufferLen = pMsg->Write(m_sendBuffer);
	ASSERT(m_nSendBufferLen <= sizeof(m_sendBuffer));

	int rt = send(m_socket, m_sendBuffer, m_nSendBufferLen, 0);
	if (rt != SOCKET_ERROR)
	{
		m_msgArray[pos] = NULL;
		delete pMsg;
	}
	else
		TipSocketError(_T("��������ʧ��"));
}

void CNetworkPlayer::RecvMsg()
{
	//��һ���յ������Ϣʱ����Ϊÿ����Ϣ����������
	//���Ҫ������Ϣ����������������Ը���Ϣ�Ӹ���β��ǣ���������ǰͷ�Ӹ����ȱ�ǣ����ܵ�д���ȣ����ȿ���Ҳ�ᱻ�ضϣ�

	int rt = recv(m_socket, m_recvBuffer, sizeof(m_recvBuffer), 0);
	if (rt == SOCKET_ERROR)
	{
		TipSocketError(_T("����ʧ��"));
		return;
	}

	//����0��ζ��socket�ر���
	if (rt > 0)
	{
		m_nRecvBufferLen = rt;
		ReadMsgFromBufferAndDispatch();
	}
}

void CNetworkPlayer::ReadMsgFromBufferAndDispatch()
{
	//ͬΪWindowsƽ̨�������Ǵ�Сͷ���ȵ�����
	int msgId = 0;
	int readMsgLen = 0;

	memcpy(&msgId, m_recvBuffer, sizeof(msgId));
	m_strLogTemp.Format(_T("receive msg, id = %d [%s]\r\n"), msgId, SNetMsgName::GetMsgName(msgId));
	Log(m_strLogTemp);

	switch (msgId)
	{
	case NETMSG_SC_ID:
	{
		SNetMsgScId msg;
		readMsgLen = msg.Read(m_recvBuffer);
		OnIdReceived(msg);
		break;
	}
	case NETMSG_SC_PLAYER_LIST:
	{
		SNetMsgScPlayerList msg;
		readMsgLen = msg.Read(m_recvBuffer);
		OnPlayerListReceived(msg);
		break;
	}

	case NETMSG_SC_CHALLENGE:
	{
		SNetMsgScChallenge msg;
		readMsgLen = msg.Read(m_recvBuffer);
		OnChallenge(msg);
		break;
	}
	case NETMSG_SC_CHALLENGE_CONFIRM:
	{
		SNetMsgScChallengeConfirm msg;
		readMsgLen = msg.Read(m_recvBuffer);
		OnChallengConfirmRecevied(msg);
		break;
	}
	case NETMSG_SC_CANCEL_CHALLENGE:
	{
		SNetMsgScCancelChallenge msg;
		readMsgLen = msg.Read(m_recvBuffer);
		OnCancelChallengRecevied(msg);
		break;
	}
	case NETMSG_SC_MOVE:
	{
		SNetMsgScMove msg;
		readMsgLen = msg.Read(m_recvBuffer);
		OnMoveReceved(msg);
		break;
	}
	case NETMSG_SC_GAME_CONTROL:
	{
		SNetMsgScGameControl msg;
		readMsgLen = msg.Read(m_recvBuffer);
		OnGameControlReceved(msg);
		break;
	}

	case NETMSG_SC_ERROR:
	{
		SNetMsgScError msg;
		readMsgLen = msg.Read(m_recvBuffer);
		OnErrorReceived(msg);
		break;
	}
	default:
		break;
	}
	ASSERT(readMsgLen <= m_nRecvBufferLen);

	m_nRecvBufferLen -= readMsgLen;
	if (m_nRecvBufferLen > 0)
	{
		memmove(m_recvBuffer, m_recvBuffer + readMsgLen, m_nRecvBufferLen);
		ReadMsgFromBufferAndDispatch();
	}
}

void CNetworkPlayer::TipSocketError(CString msg)
{
	int error = WSAGetLastError();
	if (error == WSAEWOULDBLOCK || error == WSAETIMEDOUT) return;

	m_strLogTemp.Format(_T("%s, error = %d\r\n"), msg, WSAGetLastError());
	Log(m_strLogTemp);//TRACE(text);
}

bool CNetworkPlayer::IsStarted()
{
	return m_socket != INVALID_SOCKET;
}

bool CNetworkPlayer::IsConnected()
{
	return m_bConnected;
}

void CNetworkPlayer::GetMove(SPosition &posSelected, SPosition &posMoveTo, SPosition &posShootAt)
{
	posSelected = m_posSelected;
	posMoveTo = m_posMoveTo;
	posShootAt = m_posShootAt;
}

void CNetworkPlayer::OnIdReceived(SNetMsgScId &msg)
{
	m_nId = msg.playerId;
	m_pView->SendMessage(WM_NETWORK_CONNECTED_TO_SERVER, 1);

	SendPlayerInfo();
	RequirePlayerList();
}

void CNetworkPlayer::OnPlayerListReceived(SNetMsgScPlayerList &msg)
{
	if (m_playerList.pArray != NULL)
		delete m_playerList.pArray;
	m_playerList = msg;
	m_playerList.pArray = new SNetPlayerInfo[msg.playerCount];
	memcpy(m_playerList.pArray, msg.pArray, msg.playerCount * sizeof(SNetPlayerInfo));

	m_pView->PostMessage(WM_NETWORK_GET_PLAYER_LIST, 0);
}

void CNetworkPlayer::OnChallenge(SNetMsgScChallenge &msg)
{
	SNetPlayerInfo *pInfo = GetPlayerInfoById(msg.inviterId);
	TCHAR *szName = _T("unkonw");
	if (pInfo != NULL)
		szName = pInfo->szName;

	CString str;
	int answer = 0;
	str.Format(_T("��� %s ����Ծ�"), szName);
	if (TipMessage(str, MB_OKCANCEL) == IDOK)
	{
		answer = NET_CHALLENGE_CONFIRM_YES;
		m_nOpponentId = msg.inviterId;
		m_pView->PostMessage(WM_NETWORK_CONNECTED_TO_OPPONENT, 0);
	}
	else
		answer = NET_CHALLENGE_CONFIRM_NO;

	SendChallengeConfirmMsg(msg.inviterId, msg.inviteeId, answer);
}

void CNetworkPlayer::OnChallengConfirmRecevied(SNetMsgScChallengeConfirm &msg)
{
	CString str;
	if (msg.answer == NET_CHALLENGE_CONFIRM_YES)
	{
		str = _T("�Է������˶Ծ�����");
		m_nOpponentId = msg.inviteeId;
		m_pView->PostMessage(WM_NETWORK_CONNECTED_TO_OPPONENT);
	}
	else
		str = _T("�Է��ܾ��˶Ծ�����");

	TipMessage(str, MB_OK);
}

void CNetworkPlayer::OnCancelChallengRecevied(SNetMsgScCancelChallenge &msg)
{
	if (m_nOpponentId <= 0)
	{
		TipMessage(_T("ͬһ������ԣ����ȡ���˶Ծ�"));
		return;
	}

	SNetPlayerInfo *pInfo = GetPlayerInfoById(m_nOpponentId);
	CString str;
	str.Format(_T("��� %s ȡ���˶Ծ�"), pInfo->szName);
	TipMessage(str);
	m_nOpponentId = -1;
	m_pView->PostMessage(WM_NETWORK_DISCONNECT_TO_OPPONENT);
}

void CNetworkPlayer::OnMoveReceved(SNetMsgScMove &msg)
{
	m_posSelected.i = msg.ijArray[0]; m_posMoveTo.i = msg.ijArray[2]; m_posShootAt.i = msg.ijArray[4];
	m_posSelected.j = msg.ijArray[1]; m_posMoveTo.j = msg.ijArray[3]; m_posShootAt.j = msg.ijArray[5];
	m_pView->PostMessage(WM_NETWORK_MOVE);
}

void CNetworkPlayer::OnGameControlReceved(SNetMsgScGameControl &msg)
{
	m_pView->PostMessage(WM_NETWORK_GAME_CONTROL, msg.action);
}

void CNetworkPlayer::OnErrorReceived(SNetMsgScError &msg)
{
	TipMessage(msg.szError);
	//AfxMessageBox(msg.szError);
}

UINT CNetworkPlayer::WorkThread_Send(LPVOID pParam)
{
	CNetworkPlayer *pNetwork = (CNetworkPlayer *)pParam;
	TRACE(_T("Networkplayer WorkThread_Recv start\r\n"));

	while (true)
	{
		Sleep(30);

		if (pNetwork->m_socket == INVALID_SOCKET) 
			continue;

		if (!pNetwork->IsConnected())
		{
			pNetwork->ConnectToServer();
			if (!pNetwork->IsConnected())
				Sleep(3000);//��һ��ʱ��������
			continue;
		}

		pNetwork->SendMsg();
	}
	return 0;
}

UINT CNetworkPlayer::WorkThread_Recv(LPVOID pParam)
{
	CNetworkPlayer *pNetwork = (CNetworkPlayer *)pParam;
	TRACE(_T("Networkplayer WorkThread_Send start\r\n"));

	while (true)
	{
		Sleep(30);

		if (pNetwork->m_socket == INVALID_SOCKET) continue;

		if (!pNetwork->IsConnected())
			continue;

		pNetwork->RecvMsg();
	}
	return 0;
}

SNetPlayerInfo * CNetworkPlayer::GetPlayerInfoById(int playerId)
{
	for (int i = 0; i < m_playerList.playerCount; i++)
	{
		if (m_playerList.pArray[i].nId == playerId)
		{
			return &m_playerList.pArray[i];
		}
	}

	m_strLogTemp.Format(_T("player id = %d is not existed \r\n"), playerId);
	Log(m_strLogTemp);
	return NULL;
}

void CNetworkPlayer::Log(CString str)
{
	CString timer = CTime::GetCurrentTime().Format(_T("%Y-%m-%d %H:%M:%S    "));
	m_strLog += timer + str;
	m_pView->PostMessageW(WM_NETWORK_LOG);
}
