#ifndef HANDLECHATINGLISTWIDGET_H
#define HANDLECHATINGLISTWIDGET_H


#include <unistd.h>
#include <cdk.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "./proto.h"
#include "./tools.h"
#include "./mainPage.h"
#include <memory.h>
#include <string.h>
#include <pthread.h>
#include <list>


using std::list;

int popupChatDialog(CDKSCREEN *cdkscreen, const char *peerName) {
	
	if(cdkscreen == NULL)
		return -1;

	CDKDIALOG *chatPrompt;
	const char *buttons[] = {"</B>ACTIVATE", "</B>ENDCHAT"};
	const char *message[3];
	char temp[200];
	snprintf(temp, 200, "This is Chat connect with %s", peerName);
	message[0] = "";
	message[1] = temp;
	message[2] = "";

	chatPrompt = newCDKDialog(cdkscreen,
			CENTER, CENTER,
			(CDK_CSTRING2)message, 3,
			(CDK_CSTRING2)buttons, 2,
			COLOR_PAIR(2) | A_REVERSE,
			TRUE,
			TRUE,
			FALSE);
	if(chatPrompt == NULL)
		return -1;

	int selection = -1;
	selection = activateCDKDialog(chatPrompt, 0);

	destroyCDKDialog(chatPrompt);
	refreshCDKScreen(cdkscreen);

	return selection;
}

int activateChat(sChatNode_t *chatNode, sChatResources_t *ptr) {

	if(ptr == NULL || chatNode == NULL)
		return -1;

	CDKSWINDOW *displayWidget = (ptr->main_page_ptr)->displayWidget;
	pthread_mutex_lock(ptr->activateChatNode_mutex);
	strcpy((ptr->activateChatNode_ptr)->peerName, "");
	pthread_mutex_unlock(ptr->activateChatNode_mutex);

	char fileName[256];
	snprintf(fileName, 256, "./records/%s.txt", chatNode->peerName);
	if(loadContext(fileName, displayWidget) == -1)
		return -1;
	pthread_mutex_lock(ptr->activateChatNode_mutex);
	strcpy((ptr->activateChatNode_ptr)->peerName, chatNode->peerName);
	(ptr->activateChatNode_ptr)->sock = chatNode->sock;
	(ptr->activateChatNode_ptr)->sockInStream = chatNode->sockInStream;
	(ptr->activateChatNode_ptr)->logger_ptr = chatNode->logger_ptr;
	(ptr->activateChatNode_ptr)->logger_mutex = chatNode->logger_mutex;
	pthread_mutex_unlock(ptr->activateChatNode_mutex);
	return 0;
}

int endChat(sChatNode_t *chatNode, sChatResources_t *ptr) {
	if(chatNode == NULL || ptr == NULL)
		return -1;

	sChatControlMessage_t sMsg;
	sMsg.msgType = ENDTCPCHAT;
	int sendState = 0;
	if(::send(chatNode->sock, &sMsg, sizeof(sChatControlMessage_t), 0) != sizeof(sChatControlMessage_t)) {
		sendState = -1;
	}

	
	close(chatNode->sock);   //close the socket.


	//Deallocate chat resouce.

	pthread_mutex_lock(ptr->chatingListWidget_mutex);
	deleteChatNode(chatNode->peerName, ptr->chatingList_ptr);
	flushChatingListWidget((ptr->main_page_ptr)->currentChatingListWidget, ptr->chatingList_ptr);
	pthread_mutex_unlock(ptr->chatingListWidget_mutex);
		
	pthread_mutex_lock(ptr->activateChatNode_mutex);
	if(strcmp((ptr->activateChatNode_ptr)->peerName, chatNode->peerName) == 0) {
		strcpy((ptr->activateChatNode_ptr)->peerName, "");
		writeToDisplayWidget((ptr->main_page_ptr)->displayWidget, "CHAT IS TERMINATED BY YOU");
	}
	pthread_mutex_unlock(ptr->activateChatNode_mutex);

	return sendState;	
}

int jumpToChatingListWidget(EObjectType cdktype, void *object, void *params, chtype key) {
	
	sChatResources_t *ptr = (sChatResources_t *)params;
	if(ptr == NULL)
		return -1;

	char *prompt[1];
	
	CDKSCREEN *cdkscreen = (ptr->main_page_ptr)->cdkscreen;
	CDKSCROLL *chatingListWidget = (ptr->main_page_ptr)->currentChatingListWidget;

	int select;
	sChatNode_t chatNode;

	pthread_mutex_lock(ptr->chatingListWidget_mutex);
	select = activateCDKScroll(chatingListWidget, NULL);
	if(chatingListWidget->exitType != vNORMAL) {
		pthread_mutex_unlock(ptr->chatingListWidget_mutex);
		return -1;
	}
	chatNode = getChatNode(ptr->chatingList_ptr, select);
	if(strcmp(chatNode.peerName, "") == 0) {
	   pthread_mutex_unlock(ptr->chatingListWidget_mutex);
	   prompt[0] = "CANNOT FIND CHATNODE";
	   popupLabel(cdkscreen, (CDK_CSTRING2)prompt, 1);
	   return -1;
	}
	pthread_mutex_unlock(ptr->chatingListWidget_mutex);

	int operation = popupChatDialog(cdkscreen, chatNode.peerName);

	switch(operation) {
		case 0:
			if(activateChat(&chatNode, ptr) == -1)
				return -1;
			break;
		case 1:
			if(endChat(&chatNode, ptr) == -1)
				return -1;
			break;
		default:
			return -1;
			break;
	}

	return 0;
}

#endif
