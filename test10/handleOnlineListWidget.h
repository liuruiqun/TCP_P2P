#ifndef HANDLEONLINELISTWIDGET_H
#define HANDLEONLINELISTWIDGET_H

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

using std::list;

void * handleTCPSocketReceive(void *);

int refreshOnlineList(CDKSCROLL *onlineListWidget, int TCPClientSocket, FILE *TCPClientStream, list<sUserInfo_t> *onlineList_ptr);
int jumpToOnlineListWidget(EObjectType cdktype, void *object, void *params, chtype key);

int refreshOnlineListWidgetKeyFunc(EObjectType cdktype, void *object, void *params, chtype key) {

	sRefreshParams_t *ptr = (sRefreshParams_t *)params;
	if(ptr == NULL)
		return -1;
	if(refreshOnlineList(ptr->onlineListWidget, ptr->TCPClientSocket, ptr->TCPClientStream, ptr->onlineList_ptr) == -1)
		return -1;

	return 0;
}

int threadErrorHandle(sChatResources_t *threadArgs) {

	CDKSCROLL *chatingListWidget = (threadArgs->main_page_ptr)->currentChatingListWidget;

	pthread_mutex_lock(threadArgs->chatingListWidget_mutex);
	deleteChatNode(threadArgs->peerName, threadArgs->chatingList_ptr);
	flushChatingListWidget(chatingListWidget, threadArgs->chatingList_ptr);
	pthread_mutex_unlock(threadArgs->chatingListWidget_mutex);


	fclose(threadArgs->newLocalSocketStream);
	close(threadArgs->newLocalSocket);

	pthread_mutex_lock(threadArgs->activateChatNode_mutex);
	if(strcmp((threadArgs->activateChatNode_ptr)->peerName, threadArgs->peerName) == 0)
		strcpy((threadArgs->activateChatNode_ptr)->peerName, "");
	pthread_mutex_unlock(threadArgs->activateChatNode_mutex);

	delete threadArgs->logger_mutex;
	delete threadArgs->logger_ptr;
	delete threadArgs;
	return 0;
}

void * handleTCPSocketReceive(void *params)
{
	sChatResources_t *threadArgs = (sChatResources_t *)params;
	if(threadArgs == NULL)
		return (void*)-1;

	FILE *socketInstream = threadArgs->newLocalSocketStream;
	CDKSWINDOW *displayWidget = (threadArgs->main_page_ptr)->displayWidget;

	char buffer[256];
	int strcmpTag;
	
	while(true) {
		
		sChatControlMessage_t delimiter;
		if(::fread(&delimiter, sizeof(sChatControlMessage_t), 1, socketInstream) != 1) {
			threadErrorHandle(threadArgs);
			return (void *)-1;
		}

		if(delimiter.msgType != DELIMITER) {
			if(delimiter.msgType == ENDTCPCHAT) {
				pthread_mutex_lock(threadArgs->activateChatNode_mutex);
				strcmpTag = strcmp((threadArgs->activateChatNode_ptr)->peerName, threadArgs->peerName);
				pthread_mutex_unlock(threadArgs->activateChatNode_mutex);
				
				if(strcmpTag == 0) { 
					snprintf(buffer, 256, "%s end the chat", threadArgs->peerName);

					pthread_mutex_lock(threadArgs->displayWidget_mutex);
					writeToDisplayWidget(displayWidget, buffer);
					pthread_mutex_unlock(threadArgs->displayWidget_mutex);
				}

				threadErrorHandle(threadArgs);
				return (void *)-1;
			}
		}

		int msgLen = delimiter.length;
		char *temp = new char[msgLen];
		if(temp == NULL) {
			threadErrorHandle(threadArgs);
			return (void *)-1;
		}

		if(::fread(temp, msgLen, 1, socketInstream) != 1) {
			threadErrorHandle(threadArgs);
			delete temp;
			return (void*)-1;
		}

		getTimeString(buffer);
		strcat(buffer, threadArgs->peerName);

		pthread_mutex_lock(threadArgs->logger_mutex);
		(threadArgs->logger_ptr)->writeLog(buffer);
		(threadArgs->logger_ptr)->writeLog(temp);
		pthread_mutex_unlock(threadArgs->logger_mutex);
		
		pthread_mutex_lock(threadArgs->activateChatNode_mutex);
		strcmpTag = strcmp((threadArgs->activateChatNode_ptr)->peerName, threadArgs->peerName);
		pthread_mutex_unlock(threadArgs->activateChatNode_mutex);
		if(strcmpTag == 0) {
			pthread_mutex_lock(threadArgs->displayWidget_mutex);
			writeToDisplayWidget(displayWidget, buffer);
			writeToDisplayWidget(displayWidget, temp);
			pthread_mutex_unlock(threadArgs->displayWidget_mutex);
		}

		delete temp;
	}

	threadErrorHandle(threadArgs);
	return NULL;
}

int jumpToOnlineListWidget(EObjectType cdktype, void *object, void *params, chtype key) {

	CLogger debug("onlineList_debug.txt");

	sChatResources_t *ptr = (sChatResources_t *)params;
	if(ptr == NULL) {
		return -1;
	}

	char *prompt[1];
	char peerName[MAX_NAME_LEN + 1];
	char buffer[256];
	char fileName[256];

	CDKSCREEN *cdkscreen = (ptr->main_page_ptr)->cdkscreen;
	CDKSCROLL *onlineListWidget = (ptr->main_page_ptr)->onlineListWidget;
	CDKSWINDOW *displayWidget = (ptr->main_page_ptr)->displayWidget;
	CDKSCROLL *chatingListWidget = (ptr->main_page_ptr)->currentChatingListWidget;

	list<sUserInfo_t> *onlineList_ptr = ptr->onlineList_ptr;
	list<sChatNode_t> *chatingList_ptr = ptr->chatingList_ptr;

	int select = activateCDKScroll(onlineListWidget, 0);

	if(onlineListWidget->exitType != vNORMAL) {
		return -1;
	}

	list<sUserInfo_t>::iterator it = onlineList_ptr->begin();
	for(int i = 0; i < select; i++)
		it++;
	strcpy(peerName, it->userName);

	if(strcmp(peerName, ptr->userName) == 0) {
		prompt[0] = "CANNOT CHAT WITH YOUSELF";
		popupLabel(cdkscreen, (CDK_CSTRING2)prompt, 1);
		return -1;
	}

	bool isChatExisted;
	pthread_mutex_lock(ptr->chatingListWidget_mutex);
	isChatExisted = isChatExist(peerName, chatingList_ptr);
	pthread_mutex_unlock(ptr->chatingListWidget_mutex);

	if(isChatExisted) {
		prompt[0] = "THIS CHAT IS ALREADY ESTABLISHED";
		popupLabel(cdkscreen, (CDK_CSTRING2)prompt, 1);
		return -1;
	}

	list<sUserInfo_t>::iterator it2;
	it2 = getIterator(onlineList_ptr, peerName);

	if(it2 == onlineList_ptr->end()){
		prompt[0] = "CANNOT GET PEER ADDRESS";
		popupLabel(cdkscreen, (CDK_CSTRING2)prompt, 1);
		return -1;
	}

	sockaddr_in peerAddr;
	memset(&peerAddr, 0, sizeof(sockaddr_in));

	peerAddr.sin_family = AF_INET;
	peerAddr.sin_port = htons(it2->port);
	peerAddr.sin_addr.s_addr = htonl(it2->ip);

	snprintf(buffer, 256, "%s IP %d PORT %d", peerName, ntohl(peerAddr.sin_addr.s_addr), ntohs(peerAddr.sin_port));
	debug.writeLog(buffer);

	int newLocalSocket = initiateTCPClient(peerAddr);
	if(newLocalSocket == -1) {
		prompt[0] = "CANNOT CREATE LOCAL SOCKET";
		popupLabel(cdkscreen, (CDK_CSTRING2)prompt, 1);
		return -1;
	}

	FILE *newLocalSocketStream = fdopen(newLocalSocket, "r");
	if(newLocalSocketStream == NULL) {
		prompt[0] = "CREATE SOCKET STREAM ERROR";
		popupLabel(cdkscreen, (CDK_CSTRING2)prompt, 1);
		close(newLocalSocket);
		return -1;
	}

	sMessage_t sMsg;
	sMsg.msgType = STARTTCPCHAT;
	strcpy(sMsg.userInfo.userName, ptr->userName); 
	if(::send(newLocalSocket, &sMsg, sizeof(sMessage_t), 0) != sizeof(sMessage_t)) {
		prompt[0] = "LOCAL SOCKET SEND ERROR";
		popupLabel(cdkscreen, (CDK_CSTRING2)prompt, 1);
		fclose(newLocalSocketStream);
		close(newLocalSocket);
		return -1;	
	}
	//set no user of displayWidget;
	pthread_mutex_lock(ptr->activateChatNode_mutex);
	strcpy((ptr->activateChatNode_ptr)->peerName, "");
	pthread_mutex_unlock(ptr->activateChatNode_mutex);

	snprintf(buffer, 256, "Chat invitation to %s is already sent, Please wait.", peerName);
	cleanCDKSwindow(displayWidget);
	writeToDisplayWidget(displayWidget, buffer);

	sMessage_t rMsg;
	if(::fread(&rMsg, sizeof(sMessage_t), 1, newLocalSocketStream) != 1) {
		prompt[0] = "LOCAL SOCKET READ ERROR";
		popupLabel(cdkscreen, (CDK_CSTRING2)prompt, 1);
		fclose(newLocalSocketStream);
		close(newLocalSocket);
		return -1;
	}

	if(rMsg.msgType != AGREETOCHAT) {

		if(rMsg.msgType == DISAGREETOCHAT) {
			snprintf(buffer, 256, "%s reject your chat invitation.", peerName);
			writeToDisplayWidget(displayWidget, buffer);
			fclose(newLocalSocketStream);
			close(newLocalSocket);
			return 0;
		}

		snprintf(buffer, 256, "application protocol error.");
		writeToDisplayWidget(displayWidget, buffer);
		fclose(newLocalSocketStream);
		close(newLocalSocket);
		return 0;
	}

	sChatResources_t *threadArgs = new sChatResources_t;
	if(threadArgs == NULL) {
		prompt[0] = "NEW THREADARGS ERROR";
		popupLabel(cdkscreen, (CDK_CSTRING2)prompt, 1);
		fclose(newLocalSocketStream);
		close(newLocalSocket);
	}

	snprintf(fileName, 256, "./records/%s.txt", peerName);
	CLogger *newLogger_ptr = new CLogger(fileName);
	if(newLogger_ptr == NULL) {
		prompt[0] = "NEW LOGGER ERROR";
		popupLabel(cdkscreen, (CDK_CSTRING2)prompt, 1);
		fclose(newLocalSocketStream);
		close(newLocalSocket);
		delete threadArgs;
		return -1;
	}

	pthread_mutex_t *newLogger_mutex_ptr = new pthread_mutex_t;
	if(newLogger_mutex_ptr == NULL) {
		prompt[0] = "NEW LOGGER MUTEX ERROR";
		popupLabel(cdkscreen, (CDK_CSTRING2)prompt, 1);
		fclose(newLocalSocketStream);
		close(newLocalSocket);
		delete newLogger_ptr;
		delete threadArgs;
		return -1;
	}

	pthread_mutex_init(newLogger_mutex_ptr, 0);

	
	sChatNode_t newChatNode;
	newChatNode.sock = newLocalSocket;
	newChatNode.sockInStream = newLocalSocketStream;
	strcpy(newChatNode.peerName, peerName);
	newChatNode.logger_ptr = newLogger_ptr;
	newChatNode.logger_mutex = newLogger_mutex_ptr;

	pthread_mutex_lock(ptr->chatingListWidget_mutex);
	chatingList_ptr->push_back(newChatNode);
	int flushChatingListWidgetState = flushChatingListWidget(chatingListWidget, chatingList_ptr);
	pthread_mutex_unlock(ptr->chatingListWidget_mutex);

	if(flushChatingListWidgetState == -1) {
		prompt[0] = "FLUSH CHAT LIST ERROR";
		popupLabel(cdkscreen, (CDK_CSTRING2)prompt, 1);
		fclose(newLocalSocketStream);
		close(newLocalSocket);
		delete newLogger_ptr;
		delete newLogger_mutex_ptr;
		delete threadArgs;
		return -1;
	}

	//do some assignment.
	strcpy(threadArgs->userName, ptr->userName);
	strcpy(threadArgs->peerName, peerName);
	threadArgs->activateChatNode_ptr = ptr->activateChatNode_ptr;
	threadArgs->activateChatNode_mutex = ptr->activateChatNode_mutex;
	threadArgs->main_page_ptr = ptr->main_page_ptr;
	threadArgs->chatingList_ptr = ptr->chatingList_ptr;
	threadArgs->chatingListWidget_mutex = ptr->chatingListWidget_mutex;
	threadArgs->displayWidget_mutex = ptr->displayWidget_mutex;
	threadArgs->logger_ptr = newLogger_ptr;
	threadArgs->logger_mutex = newLogger_mutex_ptr;
	threadArgs->newLocalSocket = newLocalSocket;
	threadArgs->newLocalSocketStream = newLocalSocketStream;
	//
	//set current display owner
	pthread_mutex_lock(ptr->activateChatNode_mutex);
	strcpy((ptr->activateChatNode_ptr)->peerName, peerName);
	(ptr->activateChatNode_ptr)->sock = newLocalSocket;
	(ptr->activateChatNode_ptr)->sockInStream = newLocalSocketStream;
	(ptr->activateChatNode_ptr)->logger_ptr = newLogger_ptr;
	(ptr->activateChatNode_ptr)->logger_mutex = newLogger_mutex_ptr;
	pthread_mutex_unlock(ptr->activateChatNode_mutex);
	
	if(loadContext(fileName, displayWidget) == -1)
	{
		prompt[0] = "READ HISTORY ERROR";
		popupLabel(cdkscreen, (CDK_CSTRING2)prompt, 1);
		fclose(newLocalSocketStream);
		close(newLocalSocket);
		delete newLogger_ptr;
		delete newLogger_mutex_ptr;
		delete threadArgs;
		return -1;

	}

	pthread_t tid;
	if(pthread_create(&tid, NULL, handleTCPSocketReceive, threadArgs) != 0){
		
		//set no user of displayWidget.
		pthread_mutex_lock(ptr->activateChatNode_mutex);
		strcpy((ptr->activateChatNode_ptr)->peerName, "");
		pthread_mutex_unlock(ptr->activateChatNode_mutex);
		
		pthread_mutex_lock(ptr->chatingListWidget_mutex);
		deleteChatNode(peerName, chatingList_ptr);
		flushChatingListWidget(chatingListWidget, chatingList_ptr);
		pthread_mutex_unlock(ptr->chatingListWidget_mutex);

		prompt[0] = "CREATE THREAD ERROR";
		popupLabel(cdkscreen, (CDK_CSTRING2)prompt, 1);
		fclose(newLocalSocketStream);
		close(newLocalSocket);
		delete newLogger_ptr;
		delete newLogger_mutex_ptr;
		delete threadArgs;
		return -1;
	}

	return 0;
}

int refreshOnlineList(CDKSCROLL *onlineListWidget, int TCPClientSocket, FILE *TCPClientStream, list<sUserInfo_t> *onlineList_ptr) {
	
	if(TCPClientSocket == -1) {
		return -1;
	}
	if(TCPClientStream == NULL) {
		return -1;
	}

	if(onlineList_ptr == NULL || onlineListWidget == NULL) {
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
		return -1;
	}

	size_t readNum = ::fread(&rMsg, sizeof(sMessage_t), 1, TCPClientStream);
	if(readNum != 1) {
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
				return -1;
			}

			snprintf(buffer, 1024, "RECV %d USER: %s", rMsg2.msgType, rMsg2.userInfo.userName);
			debug.writeLog(buffer);

			if(rMsg2.msgType == JUSTUSERINFO)
				onlineList_ptr->push_back(rMsg2.userInfo);
		}
	}

	if(flushOnlineListWidget(onlineListWidget, onlineList_ptr) == -1) {
		return -1;
	}
	return 0;
}
#endif
