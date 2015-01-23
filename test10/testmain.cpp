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
#include <pthread.h>
#include <list>
#include "./readLastLinesFromFile.h"
#include "./CLogger.h"
#include "./getTimeString.h"
#include "./onlyLogin.h"
#include "./initiateSocket.h"
#include "./mainPage.h"
#include "./tools.h"
#include "handleOnlineListWidget.h"

using std::list;

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
	//get the current online user from server.
	
	if(refreshOnlineList(main_page.onlineListWidget, TCPClientSocket, TCPClientStream, &onlineList) == -1) {
	
		destroyCDKScroll(main_page.onlineListWidget);
		destroyCDKScroll(main_page.currentChatingListWidget);
		destroyCDKScroll(main_page.requestListwidget);
		destroyCDKSwindow(main_page.displayWidget);
		destroyCDKMentry(main_page.inputWidget);
		destroyCDKScreen(cdkscreen);
		endCDK();
		return -1;
	}
	
	//bind key to jump to other widget.
	
	//bindCDKObject(vSCROLL, main_page.onlineListWidget, KEY_F5, jumpToOnlineListWidget, NULL);

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
