#include <cdk.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory.h>
#include <string.h>
#include "./proto.h"
#include <list>
#include "./readLastLinesFromFile.h"
#include "./CLogger.h"
#include "./getTimeString.h"
#include "./onlyLogin.h"
#include "./initiateSocket.h"
#include "./mainPage.h"

using std::list;


int refreshOnlineList(CDKSCROLL *onlineListWidget, int TCPClientSocket, FILE *TCPClientStream, list<sUserInfo_t> *onlineList_ptr);

int main() {
	
	list<sUserInfo_t> onlineList;
	
	CDKSCREEN *cdkscreen = 0;
	WINDOW *cursesWin = 0;

	sUserInfo_t userInformation;	
	userInformation.ip = INADDR_ANY;
	userInformation.port = USER_LISTEN_PORT;
/*	
	int listener = initiateTCPListener(userInformation.port, LISTEN_QUEUE_LEN, NULL); //can add a address.
	if(listener == -1)
		return -1;
*/			
	sockaddr_in TCPServerAddress;
	memset(&TCPServerAddress, 0, sizeof(sockaddr_in));

	TCPServerAddress.sin_family = AF_INET;
	if(::inet_pton(AF_INET, "127.0.0.1", &TCPServerAddress.sin_addr.s_addr) == -1){
//		close(listener);
		return -1;
	}
	TCPServerAddress.sin_port = htons(SERVER_PORT);

	int TCPClientSocket = initiateTCPClient(TCPServerAddress);
	if(TCPClientSocket == -1) {
//		close(listener);
		return -1;
	}

	FILE *TCPClientStream = ::fdopen(TCPClientSocket, "r");
	if(TCPClientStream == NULL) {
		close(TCPClientSocket);
//		close(listener);
		return -1;
	}

	cursesWin = initscr();
	cdkscreen = initCDKScreen(cursesWin);

	initCDKColor();

	int loginState = onlyLogin(cdkscreen, TCPClientSocket, TCPClientStream, &userInformation);

	if(loginState == -1) {
//		close(listener);
		destroyCDKScreen(cdkscreen);
		endCDK();
		printf("login fail.\n");
		//need recycle socket and screen resouces.
		return -1;

	}

	sMainPage_t main_page;

	int reval = mainPage(cdkscreen, &main_page);
	if(reval == -1) {
		destroyCDKScreen(cdkscreen);
		endCDK();
		fclose(TCPClientStream);
		close(TCPClientSocket);
		return -1;
	}

	//
	//
	//get the current online user from server.
	//
	//
	refreshOnlineList(main_page.onlineListWidget, TCPClientSocket, TCPClientStream, &onlineList);

	activateCDKMentry(main_page.inputWidget, 0);

	destroyCDKScroll(main_page.onlineListWidget);
	destroyCDKScroll(main_page.currentChatingListWidget);
	destroyCDKSwindow(main_page.displayWidget);
	destroyCDKMentry(main_page.inputWidget);
	destroyCDKScreen(cdkscreen);
	endCDK();

	fclose(TCPClientStream);	
	close(TCPClientSocket);
//	close(listener);
	return 0;
}


int refreshOnlineList(CDKSCROLL *onlineListWidget, int TCPClientSocket, FILE *TCPClientStream, list<sUserInfo_t> *onlineList_ptr) {
	
	if(TCPClientSocket == -1) {
		if(TCPClientStream != NULL)
			fclose(TCPClientStream);
		return -1;
	}
	if(TCPClientStream == NULL) {
		close(TCPClientSocket);
		return -1;
	}

	if(onlineList_ptr == NULL || onlineListWidget == NULL) {
		fclose(TCPClientStream);
		close(TCPClientSocket);
		return -1;
	}

	CLogger debug("Debug.txt");
	char buffer[1024];

	sMessage_t sMsg;
	sMessage_t rMsg;
	memset(&sMsg, 0, sizeof(sMessage_t));
	memset(&rMsg, 0, sizeof(sMessage_t));

	sMsg.msgType = GETALLUSER;
	ssize_t sendNum = ::send(TCPClientSocket, &sMsg, sizeof(sMessage_t), 0);
	if(sendNum != sizeof(sMessage_t)) {
		fclose(TCPClientStream);
		close(TCPClientSocket);
		return -1;
	}

	size_t readNum = ::fread(&rMsg, sizeof(sMessage_t), 1, TCPClientStream);
	if(readNum != 1) {
		fclose(TCPClientStream);
		close(TCPClientSocket);
		return -1;
	}

	snprintf(buffer, 1024, "RECV %d NUM: %d", rMsg.msgType, rMsg.userInfo.ip);
	debug.writeLog(buffer);

	if(rMsg.msgType == NUMOFUSER) {
		//clear current onlineList;
		//
		onlineList_ptr->clear();
		for(unsigned int i = 0; i < rMsg.userInfo.ip; i++) {
			sMessage_t rMsg2;
			size_t readNum2 = ::fread(&rMsg2, sizeof(sMessage_t), 1, TCPClientStream);
			if(readNum2 != 1) {
				fclose(TCPClientStream);
				close(TCPClientSocket);
				return -1;
			}

			snprintf(buffer, 1024, "RECV %d USER: %s", rMsg2.msgType, rMsg2.userInfo.userName);
			debug.writeLog(buffer);

			if(rMsg2.msgType == JUSTUSERINFO)
				onlineList_ptr->push_back(rMsg2.userInfo);
		}
	}


	int sizeOfOnlineList = onlineList_ptr->size();
	char **items = new char*[sizeOfOnlineList];
	if(items == NULL) {
		fclose(TCPClientStream);
		close(TCPClientSocket);
		return -1;
	}
	
	list<sUserInfo_t>::iterator it;
	int i = 0;
	for(it = onlineList_ptr->begin(); it != onlineList_ptr->end(); it++) {
		char *temp = new char[MAX_NAME_LEN + 1];
		if(temp == NULL) {
			close(TCPClientSocket);
			return -1;
		}
		strcpy(temp, it->userName);
		items[i] = temp;
		i++;
	}
	
	setCDKScroll(onlineListWidget,
			items,
			sizeOfOnlineList,
			false,
			A_REVERSE,
			true);
	
	activateCDKScroll(onlineListWidget, 0);

	return 0;
}

