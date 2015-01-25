#ifndef HANDLEREQUESTLISTWIDGET_H
#define HANDLEREQUESTLISTWIDGET_H

#include <cdk.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <memory.h>
#include <list>

#include "./proto.h"
#include "./tools.h"
#include "./readLastLinesFromFile.h"
#include "./handleOnlineListWidget.h"

using std::list;

int popupRequestDialog(CDKSCREEN *cdkscreen, const char *peerName) {
	
	if(cdkscreen == NULL)
		return -1;

	CDKDIALOG *requestPrompt;
	const char *buttons[] = {"</B>AGREE", "</B>DISAGREE"};
	const char *message[3];
	char temp[200];
	snprintf(temp, 200, "%s wants to establish a chat with you.", peerName);
	message[0] = "";
	message[1] = temp;
	message[2] = "";

	requestPrompt = newCDKDialog(cdkscreen,
			CENTER, CENTER,
			(CDK_CSTRING2)message, 3,
			(CDK_CSTRING2)buttons, 2,
			COLOR_PAIR(2) | A_REVERSE,
			TRUE,
			TRUE,
			FALSE);
	if(requestPrompt == NULL)
		return -1;

	int selection = -1;
	selection = activateCDKDialog(requestPrompt, 0);

	destroyCDKDialog(requestPrompt);
	refreshCDKScreen(cdkscreen);

	return selection;
}


int agreeHandle(sChatNode_t *requestNode, sChatResources_t *ptr) {

	if(requestNode == NULL || ptr == NULL)
		return -1;

	CDKSCREEN *cdkscreen = (ptr->main_page_ptr)->cdkscreen;
	CDKSCROLL *requestListWidget = (ptr->main_page_ptr)->requestListWidget;
	CDKSCROLL *chatingListWidget = (ptr->main_page_ptr)->currentChatingListWidget;
	CDKSWINDOW *displayWidget = (ptr->main_page_ptr)->displayWidget;
	char fileName[256];
	char *prompt[1];
	
	//flush request list widget.
	pthread_mutex_lock(ptr->requestListWidget_mutex);
	deleteChatNode(requestNode->peerName, ptr->requestList_ptr);
	flushChatingListWidget(requestListWidget, ptr->requestList_ptr);
	pthread_mutex_unlock(ptr->requestListWidget_mutex);

	//flush CDKSCREEN
	//refreshCDKScreen(cdkscreen);

	int newLocalSocket = requestNode->sock;
	FILE *newLocalSocketStream = requestNode->sockInStream;
	sMessage_t sMsg;
	sMsg.msgType = AGREETOCHAT;

	if(::send(newLocalSocket, &sMsg, sizeof(sMessage_t), 0) != sizeof(sMessage_t)) {
		prompt[0] = "LOCAL SOCKET SEND ERROR";
		popupLabel(cdkscreen, (CDK_CSTRING2)prompt, 1);
		fclose(newLocalSocketStream);
		close(newLocalSocket);
		return -1;
	}

	sChatResources_t *threadArgs = new sChatResources_t;

	if(threadArgs == NULL) {
		prompt[0] = "NEW THREADARGS ERROR";
		popupLabel(cdkscreen, (CDK_CSTRING2)prompt, 1);
		fclose(newLocalSocketStream);
		close(newLocalSocket);
		return -1;
	}

	snprintf(fileName, 256, "./records/%s.txt", requestNode->peerName);
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
	//initiate mutex
	pthread_mutex_init(newLogger_mutex_ptr, 0);
	
	sChatNode_t newChatNode;
	
	strcpy(newChatNode.peerName, requestNode->peerName);
	newChatNode.sock = newLocalSocket;
	newChatNode.sockInStream = newLocalSocketStream;
	newChatNode.logger_ptr = newLogger_ptr;
	newChatNode.logger_mutex = newLogger_mutex_ptr;

	//flush chatingList
	int flushChatingListWidgetState;
	pthread_mutex_lock(ptr->chatingListWidget_mutex);
	(ptr->chatingList_ptr)->push_back(newChatNode);
	flushChatingListWidgetState = flushChatingListWidget(chatingListWidget, ptr->chatingList_ptr);
	pthread_mutex_unlock(ptr->chatingListWidget_mutex);

	if(flushChatingListWidgetState == -1) {
		prompt[0] = "FLUSH CHATING LIST ERROR";
		popupLabel(cdkscreen, (CDK_CSTRING2)prompt, 1);
		fclose(newLocalSocketStream);
		close(newLocalSocket);
		delete newLogger_ptr;
		delete newLogger_mutex_ptr;
		delete threadArgs;
		return -1;
	}

	//set threadArgs 
	
	strcpy(threadArgs->userName, ptr->userName);
	strcpy(threadArgs->peerName, requestNode->peerName);
	threadArgs->activateChatNode_ptr = ptr->activateChatNode_ptr;
	threadArgs->activateChatNode_mutex = ptr->activateChatNode_mutex;
	threadArgs->logger_ptr = newLogger_ptr;
	threadArgs->logger_mutex = newLogger_mutex_ptr;
	threadArgs->chatingList_ptr = ptr->chatingList_ptr;
	threadArgs->chatingListWidget_mutex = ptr->chatingListWidget_mutex;
	threadArgs->displayWidget_mutex = ptr->displayWidget_mutex;
	threadArgs->main_page_ptr = ptr->main_page_ptr;
	threadArgs->newLocalSocket = newLocalSocket;
	threadArgs->newLocalSocketStream = newLocalSocketStream;
	
	//set current diplay owner.
	pthread_mutex_lock(ptr->activateChatNode_mutex);
	strcpy((ptr->activateChatNode_ptr)->peerName, requestNode->peerName);
	(ptr->activateChatNode_ptr)->sock = newLocalSocket;
	(ptr->activateChatNode_ptr)->sockInStream = newLocalSocketStream;
	(ptr->activateChatNode_ptr)->logger_ptr = newLogger_ptr;
	(ptr->activateChatNode_ptr)->logger_mutex = newLogger_mutex_ptr;
	pthread_mutex_unlock(ptr->activateChatNode_mutex);

	if(loadContext(fileName, displayWidget) == -1)
	{
		prompt[0] = "LOAD HISTORY ERROR";
		popupLabel(cdkscreen, (CDK_CSTRING2)prompt, 1);
		fclose(newLocalSocketStream);
		close(newLocalSocket);
		delete newLogger_ptr;
		delete newLogger_mutex_ptr;
		delete threadArgs;
		return -1;
	}

	pthread_t tid;
	if(pthread_create(&tid, NULL, handleTCPSocketReceive, threadArgs) != 0) {
		
		//set no user of displayWidget.
		pthread_mutex_lock(ptr->activateChatNode_mutex);
		strcpy((ptr->activateChatNode_ptr)->peerName, "");
		pthread_mutex_unlock(ptr->activateChatNode_mutex);
		
		pthread_mutex_lock(ptr->chatingListWidget_mutex);
		deleteChatNode(requestNode->peerName, ptr->chatingList_ptr);
		flushChatingListWidget(chatingListWidget, ptr->chatingList_ptr);
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

int disagreeHandle(sChatNode_t *requestNode, sChatResources_t *ptr) {
	
	if(requestNode == NULL || ptr == NULL)
		return -1;
	CDKSCROLL *requestListWidget = (ptr->main_page_ptr)->requestListWidget;
	CDKSCREEN *cdkscreen =(ptr->main_page_ptr)->cdkscreen;
	
	int newLocalSocket = requestNode->sock;
	
	sMessage_t sMsg;
	sMsg.msgType = DISAGREETOCHAT;
	
	int sendState = 0;
	if(::send(newLocalSocket, &sMsg, sizeof(sMessage_t), 0) != sizeof(sMessage_t))
		sendState = -1;

	//flush request list widget.
	pthread_mutex_lock(ptr->requestListWidget_mutex);
	deleteChatNode(requestNode->peerName, ptr->requestList_ptr);
	flushChatingListWidget(requestListWidget, ptr->requestList_ptr);
	pthread_mutex_unlock(ptr->requestListWidget_mutex);

	//refresh CDK screen.
	//refreshCDKScreen(cdkscreen);

	close(newLocalSocket);
	return sendState;

}

int jumpToRequestListWidget(EObjectType cdktype, void *object, void *params, chtype key) {

	sChatResources_t *ptr = (sChatResources_t *)params;
	
	if(ptr == NULL)
		return -1;

	char *prompt[1];
	CDKSCREEN *cdkscreen = (ptr->main_page_ptr)->cdkscreen;
	CDKSCROLL *requestListWidget = (ptr->main_page_ptr)->requestListWidget;
	CDKSCROLL *chatingListWidget = (ptr->main_page_ptr)->currentChatingListWidget;
	CDKSWINDOW *displayWidget = (ptr->main_page_ptr)->displayWidget;	
	int select;
	sChatNode_t requestNode;

	pthread_mutex_lock(ptr->requestListWidget_mutex);
	select = activateCDKScroll(requestListWidget, NULL);
	if(requestListWidget->exitType != vNORMAL) {
		pthread_mutex_unlock(ptr->requestListWidget_mutex);
		return -1;
	}
	requestNode = getChatNode(ptr->requestList_ptr, select);
	if(strcmp(requestNode.peerName, "") == 0) {
		pthread_mutex_unlock(ptr->requestListWidget_mutex);

		prompt[0] = "CANNOT FIND REQUEST NODE";
		popupLabel(cdkscreen, (CDK_CSTRING2)prompt, 1);
		return -1;
	}
	pthread_mutex_unlock(ptr->requestListWidget_mutex);

	int operation = popupRequestDialog(cdkscreen, requestNode.peerName);

	switch(operation) {
		case 0:
			if(agreeHandle(&requestNode, ptr) == -1)
				return -1;
			break;
		case 1:
			if(disagreeHandle(&requestNode, ptr) == -1)
				return -1;
			break;
		default:
			return -1;
			break;
	}

	return 0;
}

#endif
