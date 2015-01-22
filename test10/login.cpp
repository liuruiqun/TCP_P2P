#include <cdk.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory.h>
#include <string.h>
#include "./proto.h"
#include <list>
#include "./readLastLinesFromFile.h"
#include "./CLogger.h"
#include "./getTimeString.h"

using std::list;

typedef struct {
	int listener;
	CDKSCREEN *cdkscreen;
	list<sUserInfo_t> *currentChatingList;
} sAcceptParams_t;

typedef struct {
	int connectedSocket;
	char peerName[MAX_NAME_LEN +1];
	char *currentDisplayWidgetOwner;
	CDKSWINDOW *displayWidget;
	CLogger *mylogger;
} sReceiveParams_t;

int writeToDisplayWidget(CDKSWINDOW *displayWidget, char *info);
int LoginPage(CDKSCREEN *, char *);

int popupChatPromptDialog(CDKSCREEN *cdkscreen, char peerName[]);

int initiateTCPListener(int nPort, int nLengthOfQueueOfListen, char * strBoundIP) {
	int listener = socket(AF_INET, SOCK_STREAM, 0);
	if(listener == -1)
		return -1;

	sockaddr_in localAddress;
	memset(&localAddress, 0, sizeof(sockaddr_in));

	localAddress.sin_family = AF_INET;
	if(strBoundIP == NULL)
		localAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	else {
		if(::inet_pton(AF_INET, strBoundIP, &localAddress.sin_addr) != 1) {
			::close(listener);
			return -1;
		}
	}

	localAddress.sin_port = htons(nPort);

	if(::bind(listener, (sockaddr *)&localAddress, sizeof(sockaddr_in)) == -1) {
		::close(listener);
		return -1;
	}

	if(::listen(listener, nLengthOfQueueOfListen) == -1) {
		::close(listener);
		return -1;
	}

	return listener;

}


void * handleAccept(void *params) {
	sAcceptParams_t * ptr = (sAcceptParams_t *)params;
	if(ptr == NULL || ptr->listener == -1 || 
			ptr->cdkscreen == NULL || ptr->currentChatingList == NULL)
		return NULL;

	sockaddr_in peerAddress;
	socklen_t peerAddrLen;

	while(true) {
		int newConnectedSocket = ::accept(ptr->listener, (sockaddr *)&peerAddress, &peerAddrLen);
		if(newConnectedSocket == -1) {
			::close(ptr->listener);
			return NULL;
		}

		sMessage_t rMsg;
		sMessage_t sMsg;
		memset(&sMsg, 0 , sizeof(sMessage_t));

		if(::recv(newConnectedSocket, &rMsg, sizeof(sMessage_t), 0) == -1) {
			::close(newConnectedSocket);
		}

		if(rMsg.msgType == STARTTCPCHAT) {
			//popup a dialog.
			int select = popupChatPromptDialog(ptr->cdkscreen, rMsg.userInfo.userName);

			if(select <= 0) {
				sMsg.msgType = DISAGREECHAT;
				::send(newConnectedSocket, &sMsg, sizeof(sMessage_t), 0);
				::close(newConnectedSocket);
			}
			else {
				sUserInfo_t peerInfo;
				strcpy(peerInfo.userName, rMsg.userInfo.userName);
				peerInfo.port = ntohs(peerAddress.sin_port);
				peerInfo.ip = ntohl(peerAddress.sin_addr.s_addr);
				(ptr->currentChatingList)->push_back(peerInfo);
				sMsg.msgType = AGREETOCHAT;
				::send(newConnectedSocket, &sMsg, sizeof(sMessage_t), 0);
				//
				//
				//fresh the currentchatingList.
				//create a receive thread for the newConnectedSocket.
				//
				//
				if(select == 1) {


					//
					//Change the owner of input widget.
					//and redraw the display widget according to the
					//newConnectedSocket.
					//

				}
			}
		}
	}
}



void* HandleMsgReceive(void *params)
{
	sReceiveParams_t *ptr = (sReceiveParams_t *)params;

	if(ptr == NULL)
		return NULL;
	if(ptr->connectedSocket == -1 || ptr->peerName == NULL || ptr->mylogger == NULL 
			|| ptr->displayWidget == NULL || ptr ->currentDisplayWidgetOwner == NULL)
		return NULL;
	

	while(true) {
		char buffer[30 + MAX_NAME_LEN + 1];
		sDelimit_t flag;	
		if(::recv(ptr->connectedSocket, &flag, sizeof(sDelimit_t), 0) == -1)
			return NULL;
		char *rMsg = new char[flag.length + 1];
		if(rMsg == NULL)
			return NULL;
		ssize_t readNum = ::recv(ptr->connectedSocket, rMsg, flag.length, 0);
		if(readNum != flag.length)
			return NULL;
		getTimeString(buffer);
		strcat(buffer, ptr->peerName);
		strcat(buffer, ": ");
//should add a mutex lock before access mylogger.
		if((ptr->mylogger)->writeLog(buffer) == -1)
			return NULL;
		if((ptr->mylogger)->writeLog(rMsg) == -1)
			return NULL;
		//

		if(ptr->currentDisplayWidgetOwner == ptr->peerName)
		{
			//maybe need a mutex lock to access the diaplayWidget;
			writeToDisplayWidget(ptr->displayWidget, buffer);
			writeToDisplayWidget(ptr->displayWidget, rMsg);
			//
		}


	}

}

int popupChatPromptDialog(CDKSCREEN *cdkscreen, char peerName[]) {
	
	if(cdkscreen == NULL)
		return -1;
	CDKDIALOG *chatPrompt;
	const char *buttons[] = {"</B>CANCEL", "</B>OKNOW" "</B>OKLATER"};
	const char *message[5];
	char temp[200];
	strcpy(temp, peerName);
	strcat(temp, " want to chat with you.");

	message[0] = "";
	message[1] = "";
	message[2] = temp;
	message[3] = "";
	message[4] = "";

	chatPrompt = newCDKDialog(cdkscreen,
			CENTER, CENTER,
			(CDK_CSTRING2)message, 5,
			(CDK_CSTRING2)buttons, 3,
			COLOR_PAIR(2) | A_REVERSE,
			TRUE,
			TRUE,
			FALSE);
	if(chatPrompt == NULL)
		return -1;

	int selection = activateCDKDialog(chatPrompt, 0);

	return selection;
}



int initiateUDPSocket(int nPort, char *strBoundIP)
{
	int sock;
	sock = ::socket(AF_INET, SOCK_DGRAM, 0);
	if(sock == -1)
		return -1;
	
	sockaddr_in sockAddress;
	memset(&sockAddress, 0, sizeof(sockaddr_in));
	sockAddress.sin_family = AF_INET;
	if(strBoundIP == NULL)
		sockAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	else{
		if(::inet_pton(AF_INET, strBoundIP, &sockAddress.sin_addr) != 1)
			return -1;
	}
	sockAddress.sin_port = htons(nPort);

	if(::bind(sock, (sockaddr *)&sockAddress, sizeof(sockaddr_in)) == -1)
		return -1;
	return sock;
}


int main() {
	CDKSCREEN *cdkscreen = 0;
	WINDOW *cursesWin = 0;
//	char UserName[MAX_NAME_LEN + 1] = {'\0'};
// userInformation handle;	
	sUserInfo_t userInformation;	
	userInformation.ip = INADDR_ANY;
	userInformation.port = USER_LISTEN_PORT;
	
	int listener = initiateTCPListener(userInformation.port, LISTEN_QUEUE_LEN, NULL); //can add a address.
	if(listener == -1)
		return -1;
			
	sockaddr_in UDPServerAddress;
	UDPServerAddress.sin_family = AF_INET;
	if(::inet_pton(AF_INET, "127.0.0.1", &UDPServerAddress.sin_addr) == -1)
		return -1;
	UDPServerAddress.sin_port = htons(SERVER_PORT);

	int clientUDPSocket = initiateUDPSocket(0, NULL);
	if(clientUDPSocket == -1)
		return -1;

	cursesWin = initscr();
	cdkscreen = initCDKScreen(cursesWin);
	const char *responseMsg[1];
//	const char *waitPrompt[1];
	responseMsg[0] = "User name ERROR.Please try again.";
	char waitPrompt[] = "Loging...";

	initCDKColor();
	while(true) {
		sMessage_t loginMsg;
		sMessage_t loginResponseMsg;
		sockaddr_in remote;
		socklen_t remoteLen;

		LoginPage(cdkscreen, userInformation.userName);
		//could do a server verification.
		//
		loginMsg.msgType = LOGIN;
		strcpy(loginMsg.userInfo.userName, userInformation.userName);

		loginMsg.userInfo.ip = userInformation.ip;
		loginMsg.userInfo.port = userInformation.port;

		::sendto(clientUDPSocket, &loginMsg, sizeof(sMessage_t), 0, (sockaddr *)&UDPServerAddress, sizeof(sockaddr_in));
		::recvfrom(clientUDPSocket, &loginResponseMsg, sizeof(sMessage_t), 0, (sockaddr *)&remote, &remoteLen);

		if(loginResponseMsg.msgType == LOGSUCCESS)
			break;
		else
			popupLabel(cdkscreen, (CDK_CSTRING2)responseMsg, 1);
		
	}


	//
	//
	//create the acceptHandle thread.
	//
	//

		
	destroyCDKScreen(cdkscreen);
	endCDK();
	printf("login successfully.\n");
	return 0;
}


int LoginPage(CDKSCREEN *cdkscreen, char info[]) {
	if(cdkscreen == NULL)
		return -1;
	
	CDKENTRY *loginEntry;
	const char *loginEntryLabel = "User Name:"; 
	const char *loginEntryTitle = "Please Login.\n";
	char *temp;

	loginEntry = newCDKEntry(cdkscreen,
			CENTER,
			CENTER,
			loginEntryTitle,
			loginEntryLabel,
			A_NORMAL,
			' ',
			vMIXED,
			20, 0, 64 - 1,
			true,
			false);
	if(loginEntry == NULL)
		return -1;
	temp = activateCDKEntry(loginEntry, 0);
	if(loginEntry->exitType == vESCAPE_HIT)
		temp = "";
	strcpy(info, temp);
	
	destroyCDKEntry(loginEntry);
	return 0;
}
