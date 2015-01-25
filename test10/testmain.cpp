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
#include "./handleOnlineListWidget.h"
#include "./handleChatingListWidget.h"
#include "./handleRequestListWidget.h"
#include "./handleDisplayWidget.h"
#include "./handleAccept.h"
#include "./handleInputWidget.h"
//#include "./loadHistory.h"

using std::list;

int deleteAllCDKWidgets(sMainPage_t main_page) {
	
	destroyCDKScroll(main_page.onlineListWidget);
	destroyCDKScroll(main_page.requestListWidget);
	destroyCDKScroll(main_page.currentChatingListWidget);
	destroyCDKSwindow(main_page.displayWidget);
	destroyCDKMentry(main_page.inputWidget);
	destroyCDKScreen(main_page.cdkscreen);

	endCDK();
	return 0;
}

int main() {
	//intiate work
	//
	char buffer[256];
	sUserInfo_t userInformation;

	CLogger debug("./debug.txt");

	list<sUserInfo_t> onlineList;
	list<sChatNode_t> chatingList;
	list<sChatNode_t> requestList;

	sChatNode_t activateChatNode;
	strcpy(activateChatNode.peerName, "");

	pthread_mutex_t chatingListWidget_mutex;
	pthread_mutex_t requestListWidget_mutex;
	pthread_mutex_t displayWidget_mutex;
	pthread_mutex_t activateChatNode_mutex;

	pthread_mutex_init(&chatingListWidget_mutex, 0);
	pthread_mutex_init(&requestListWidget_mutex, 0);
	pthread_mutex_init(&displayWidget_mutex, 0);
	pthread_mutex_init(&activateChatNode_mutex, 0);
	
	CDKSCREEN *cdkscreen = 0;
	WINDOW *cursesWin = 0;

	int listener = initiateTCPListener(LISTEN_QUEUE_LEN, &userInformation);
	if(listener == -1)
		return -1;
			
	sockaddr_in TCPServerAddress;
	memset(&TCPServerAddress, 0, sizeof(sockaddr_in));

	TCPServerAddress.sin_family = AF_INET;
	if(::inet_pton(AF_INET, "127.0.0.1", &TCPServerAddress.sin_addr.s_addr) == -1){
		close(listener);
		return -1;
	}
	TCPServerAddress.sin_port = htons(SERVER_PORT);

	int TCPClientSocket = initiateTCPClient(TCPServerAddress);
	if(TCPClientSocket == -1) {
		close(listener);
		return -1;
	}

	FILE *TCPClientStream = ::fdopen(TCPClientSocket, "r");
	if(TCPClientStream == NULL) {
		close(TCPClientSocket);
		close(listener);
		return -1;
	}

	cursesWin = initscr();
	cdkscreen = initCDKScreen(cursesWin);

	initCDKColor();

	int loginState = onlyLogin(cdkscreen, TCPClientSocket, TCPClientStream, &userInformation);

	if(loginState == -1) {
		close(listener);
		fclose(TCPClientStream);
		close(TCPClientSocket);
		destroyCDKScreen(cdkscreen);
		endCDK();
		printf("login fail.\n");
		return -1;
	}

	snprintf(buffer, 256, "%s IP %d PORT %d", userInformation.userName, userInformation.ip, userInformation.port);
	debug.writeLog(buffer);

	sMainPage_t main_page;
	main_page.cdkscreen = cdkscreen;

	int reval = mainPage(cdkscreen, &main_page);
	if(reval == -1) {
		close(listener);
		fclose(TCPClientStream);
		close(TCPClientSocket);
		destroyCDKScreen(cdkscreen);
		endCDK();
		return -1;
	}



	//initiate accept threadArgs
	//
	sRequestParams_t *acceptThreadArgs = new sRequestParams_t;
	if(acceptThreadArgs == NULL){
		close(listener);
		fclose(TCPClientStream);
		close(TCPClientSocket);
		deleteAllCDKWidgets(main_page);
		printf("new acceptThreadArgs error.\n");
		return -1;
	}

	acceptThreadArgs->listener = listener;
	acceptThreadArgs->requestList_ptr = &requestList;
	acceptThreadArgs->requestListWidget = main_page.requestListWidget;
	acceptThreadArgs->requestListWidget_mutex = &requestListWidget_mutex;
	
	pthread_t tid;
	if(pthread_create(&tid, NULL, handleAccept, acceptThreadArgs) != 0) {
		close(listener);
		fclose(TCPClientStream);
		close(TCPClientSocket);
		deleteAllCDKWidgets(main_page);
		delete acceptThreadArgs;
		printf("pthread_create handleAccept error.\n");
		return -1;
	}

	//key bindings: 
	//<F6> to online List
	//<F7> to request list
	//<F8> to chat list
	//<F9> to display
	
	sChatResources_t *onlineListWidgetParams = new sChatResources_t;
	if(onlineListWidgetParams == NULL) {
		close(listener);
		fclose(TCPClientStream);
		close(TCPClientSocket);
		deleteAllCDKWidgets(main_page);
		delete acceptThreadArgs;
		printf("new onlineListWidgetParams error.\n");
		return -1;		
	}
	
	strcpy(onlineListWidgetParams->userName, userInformation.userName);
	onlineListWidgetParams->activateChatNode_ptr = &activateChatNode;
	onlineListWidgetParams->activateChatNode_mutex = &activateChatNode_mutex;
	onlineListWidgetParams->main_page_ptr = &main_page;
	onlineListWidgetParams->TCPClientSocket = TCPClientSocket;
	onlineListWidgetParams->TCPClientStream = TCPClientStream;
	onlineListWidgetParams->onlineList_ptr = &onlineList;
	onlineListWidgetParams->chatingList_ptr = &chatingList;
	onlineListWidgetParams->chatingListWidget_mutex = &chatingListWidget_mutex;
	onlineListWidgetParams->displayWidget_mutex = &displayWidget_mutex;

	bindCDKObject(vMENTRY, main_page.inputWidget, KEY_F6, jumpToOnlineListWidget, onlineListWidgetParams);


	
	sChatResources_t *chatingListWidgetParams = new sChatResources_t;
	if(chatingListWidgetParams == NULL) {
		close(listener);
		fclose(TCPClientStream);
		close(TCPClientSocket);
		deleteAllCDKWidgets(main_page);
		delete acceptThreadArgs;
		delete onlineListWidgetParams;
		printf("new chatingListWidgetParams error.\n");
		return -1;		
	}

	chatingListWidgetParams->main_page_ptr = &main_page;
	chatingListWidgetParams->chatingList_ptr = &chatingList;
	chatingListWidgetParams->chatingListWidget_mutex = &chatingListWidget_mutex;
	chatingListWidgetParams->activateChatNode_ptr = &activateChatNode;
	chatingListWidgetParams->activateChatNode_mutex = &activateChatNode_mutex;

	bindCDKObject(vMENTRY, main_page.inputWidget, KEY_F7, jumpToChatingListWidget, chatingListWidgetParams);

	sChatResources_t *requestListWidgetParams = new sChatResources_t;
	if(requestListWidgetParams == NULL) {
		close(listener);
		fclose(TCPClientStream);
		close(TCPClientSocket);
		deleteAllCDKWidgets(main_page);
		delete acceptThreadArgs;
		delete onlineListWidgetParams;
		delete chatingListWidgetParams;
		printf("new requestListWidgetParams error.\n");
		return -1;		
	}

	requestListWidgetParams->main_page_ptr = &main_page;
	requestListWidgetParams->chatingList_ptr = &chatingList;
	requestListWidgetParams->chatingListWidget_mutex = &chatingListWidget_mutex;
	requestListWidgetParams->requestList_ptr = &requestList;
	requestListWidgetParams->requestListWidget_mutex = &requestListWidget_mutex;
	requestListWidgetParams->displayWidget_mutex = &displayWidget_mutex;
	requestListWidgetParams->activateChatNode_ptr = &activateChatNode;
	requestListWidgetParams->activateChatNode_mutex = &activateChatNode_mutex;
	
	bindCDKObject(vMENTRY, main_page.inputWidget, KEY_F8, jumpToRequestListWidget, requestListWidgetParams);

	sDisplayWidgetParams_t *displayWidgetParams = new sDisplayWidgetParams_t;
	if(displayWidgetParams == NULL) {
		close(listener);
		fclose(TCPClientStream);
		close(TCPClientSocket);
		deleteAllCDKWidgets(main_page);
		delete acceptThreadArgs;
		delete onlineListWidgetParams;
		delete chatingListWidgetParams;
		delete requestListWidgetParams;
		printf("new displayWidgetParams error.\n");
		return -1;		
	}

	displayWidgetParams->activateChatNode_ptr = &activateChatNode;
	displayWidgetParams->activateChatNode_mutex = &activateChatNode_mutex;
	displayWidgetParams->displayWidget = main_page.displayWidget;
	
	bindCDKObject(vMENTRY, main_page.inputWidget, KEY_F9, jumpToDisplayWidget, displayWidgetParams);

	sRefreshParams_t *refreshOnlineListWidgetParams = new sRefreshParams_t;
	if(refreshOnlineListWidgetParams == NULL) {
		close(listener);
		fclose(TCPClientStream);
		close(TCPClientSocket);
		deleteAllCDKWidgets(main_page);
		delete acceptThreadArgs;
		delete onlineListWidgetParams;
		delete chatingListWidgetParams;
		delete displayWidgetParams;
		printf("new refreshOnlineListWidgetParams error.\n");
		return -1;		
	}

	refreshOnlineListWidgetParams->onlineListWidget = main_page.onlineListWidget;
	refreshOnlineListWidgetParams->onlineList_ptr = &onlineList;
	refreshOnlineListWidgetParams->TCPClientSocket = TCPClientSocket;
	refreshOnlineListWidgetParams->TCPClientStream = TCPClientStream;

	bindCDKObject(vSCROLL, main_page.onlineListWidget, 'R', refreshOnlineListWidgetKeyFunc, refreshOnlineListWidgetParams);

/*
	sDisplayWidgetParams_t *loadHistoryParams = new sDisplayWidgetParams_t;
	if(loadHistoryParams == NULL) {
		close(listener);
		fclose(TCPClientStream);
		close(TCPClientSocket);
		deleteAllCDKWidgets(main_page);
		delete acceptThreadArgs;
		delete onlineListWidgetParams;
		delete chatingListWidgetParams;
		delete displayWidgetParams;
		delete refreshOnlineListWidgetParams;
		printf("new loadHistoryParams error.\n");
		return -1;		
	}

	loadHistoryParams->cdkscreen = main_page.cdkscreen;
	loadHistoryParams->displayWidget = main_page.displayWidget;
	loadHistoryParams->displayWidget_mutex = &displayWidget_mutex;
	loadHistoryParams->activateChatNode_ptr = &activateChatNode;
	loadHistoryParams->activateChatNode_mutex = &activateChatNode_mutex;

	bindCDKObject(vSWINDOW, main_page.displayWidget, 'H', loadHistory, loadHistoryParams);

	//get the current online user from server.
*/	
	if(refreshOnlineList(main_page.onlineListWidget, TCPClientSocket, TCPClientStream, &onlineList) == -1){
		close(listener);
		fclose(TCPClientStream);
		close(TCPClientSocket);
		deleteAllCDKWidgets(main_page);
		delete acceptThreadArgs;
		delete onlineListWidgetParams;
		delete chatingListWidgetParams;
		delete displayWidgetParams;
		delete refreshOnlineListWidgetParams;
//		delete loadHistoryParams;
		printf("refreshOnlineList error.\n");
		return -1;		
	}

	//inputWidget Handle
	
	sInputWidgetParams_t *inputWidgetParams = new sInputWidgetParams_t;
   	
	if(inputWidgetParams == NULL) {
		close(listener);
		fclose(TCPClientStream);
		close(TCPClientSocket);
		deleteAllCDKWidgets(main_page);
		delete acceptThreadArgs;
		delete onlineListWidgetParams;
		delete chatingListWidgetParams;
		delete displayWidgetParams;
		delete refreshOnlineListWidgetParams;
//		delete loadHistoryParams;
		printf("new inputWidgetParams error.\n");
		return -1;		
	}

	strcpy(inputWidgetParams->userName, userInformation.userName);
	inputWidgetParams->main_page_ptr = &main_page;
	inputWidgetParams->activateChatNode_ptr = &activateChatNode;
	inputWidgetParams->activateChatNode_mutex = &activateChatNode_mutex;
	inputWidgetParams->displayWidget_mutex = &displayWidget_mutex;

	reval = handleInputWidget(inputWidgetParams);
	
	close(listener);
	fclose(TCPClientStream);
	close(TCPClientSocket);
	deleteAllCDKWidgets(main_page);
	delete acceptThreadArgs;
	delete onlineListWidgetParams;
	delete chatingListWidgetParams;
	delete displayWidgetParams;
	delete refreshOnlineListWidgetParams;
//	delete loadHistoryParams;
	delete inputWidgetParams;
	
	return reval;	
}
