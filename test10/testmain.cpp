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

int refreshOnlineList(int TCPClientSocket, CDKSCROLL *onlineListWidget, list<sUserInfo_t> *onlineList_ptr);


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

	cursesWin = initscr();
	cdkscreen = initCDKScreen(cursesWin);

	initCDKColor();

	int loginState = onlyLogin(cdkscreen, TCPClientSocket, &userInformation);

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
		close(TCPClientSocket);
		return -1;
	}

	//
	//
	//get the current online user from server.
	//
	//
	refreshOnlineList(TCPClientSocket, main_page.onlineListWidget, &onlineList);

	activateCDKMentry(main_page.inputWidget, 0);

	destroyCDKScroll(main_page.onlineListWidget);
	destroyCDKScroll(main_page.currentChatingListWidget);
	destroyCDKSwindow(main_page.displayWidget);
	destroyCDKMentry(main_page.inputWidget);
	destroyCDKScreen(cdkscreen);
	endCDK();
	
	close(TCPClientSocket);
//	close(listener);
	return 0;
}


int refreshOnlineList(int TCPClientSocket, CDKSCROLL *onlineListWidget, list<sUserInfo_t> *onlineList_ptr) {
	
	if(TCPClientSocket == -1) {
		return -1;
	}

	if(onlineList_ptr == NULL || onlineListWidget == NULL) {
		close(TCPClientSocket);
		return -1;
	}

	sMessage_t sMsg;
	sMessage_t rMsg;
	memset(&sMsg, 0, sizeof(sMessage_t));
	memset(&rMsg, 0, sizeof(sMessage_t));

	sMsg.msgType = GETALLUSER;
	ssize_t sendNum = ::send(TCPClientSocket, &sMsg, sizeof(sMessage_t), 0);
	if(sendNum == sizeof(sMessage_t)) {
		
		ssize_t readNum = ::recv(TCPClientSocket, &rMsg, sizeof(sMessage_t), 0);

		if(readNum == sizeof(sMessage_t)) {
			if(rMsg.msgType == NUMOFUSER) {
				//clear current onlineList;
				//
				onlineList_ptr->clear();
				for(int i = 0; i < rMsg.userInfo.ip; i++) {
					sMessage_t rMsg2;
					ssize_t readNum2 = ::recv(TCPClientSocket, &rMsg2, sizeof(sMessage_t), 0);
					if(readNum2 == sizeof(sMessage_t)) {
						if(rMsg2.msgType == JUSTUSERINFO)
							onlineList_ptr->push_back(rMsg2.userInfo);
					}
					else{
						close(TCPClientSocket);
						return -1;
					}
				}
			}
		}
		else {
			close(TCPClientSocket);
			return -1;
		}
	}
	else {
		close(TCPClientSocket);
		return -1;
	}

	int sizeOfOnlineList = onlineList_ptr->size();
	char **item = new char*[sizeOfOnlineList];
	if(item == NULL) {
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
		item[i] = temp;
		i++;
	}

	setCDKScroll(onlineListWidget,
			item,
			sizeOfOnlineList,
			false,
			A_REVERSE,
			true);

	return 0;
}

