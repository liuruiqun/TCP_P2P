#ifndef HANDLEINPUTWIDGET_H
#define HANDLEINPUTWIDGET_H

#include <cdk.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "./proto.h"
#include "./CLogger.h"
#include <string.h>
#include <pthread.h>
#include "./getTimeString.h"
#include "tools.h"

int handleInputWidget(sInputWidgetParams_t *ptr) {
	if(ptr == NULL)
		return -1;
	CDKSCREEN *cdkscreen = (ptr->main_page_ptr)->cdkscreen;
	CDKMENTRY *inputWidget = (ptr->main_page_ptr)->inputWidget;
	CDKSWINDOW *displayWidget = (ptr->main_page_ptr)->displayWidget;

	char *info;
	char *prompt[1];
	char buffer[256];
	int infoLen;
	sChatNode_t activateChatNode;

	if( cdkscreen == NULL || inputWidget == NULL || displayWidget == NULL)
		return -1;

	activateCDKMentry(inputWidget, NULL);

	while(inputWidget->exitType != vESCAPE_HIT) {

		refreshCDKScreen(cdkscreen);
	
		pthread_mutex_lock(ptr->activateChatNode_mutex);
		activateChatNode = *(ptr->activateChatNode_ptr);
		pthread_mutex_unlock(ptr->activateChatNode_mutex);
		
		if(strcmp(activateChatNode.peerName, "") == 0) {
			writeToDisplayWidget(displayWidget, "NO ACTIVE CHAT");
			cleanCDKMentry(inputWidget);
			activateCDKMentry(inputWidget, NULL);
			continue;
		}
		
		info = strdup((inputWidget->info));	
		if(info == NULL)
			return -1;

		infoLen = strlen(info);		
		if(infoLen == 0) {
			free(info);
			cleanCDKMentry(inputWidget);
			activateCDKMentry(inputWidget, NULL);
			continue;
		}
		infoLen++; //add the end '\0'

		getTimeString(buffer);
		strcat(buffer, ptr->userName);
		
		pthread_mutex_lock(ptr->displayWidget_mutex);
		writeToDisplayWidget(displayWidget, buffer);
		writeToDisplayWidget(displayWidget, info);
		pthread_mutex_unlock(ptr->displayWidget_mutex);

		sChatControlMessage_t delimiter;
		delimiter.msgType = DELIMITER;
		delimiter.length = infoLen;

		if(::send(activateChatNode.sock, &delimiter, sizeof(sChatControlMessage_t), 0) != sizeof(sChatControlMessage_t)) {
			free(info);
			close(activateChatNode.sock);
			cleanCDKMentry(inputWidget);
			activateCDKMentry(inputWidget, NULL);
			continue;
		}

		if(::send(activateChatNode.sock, info, infoLen, 0) != infoLen) {
			free(info);
			close(activateChatNode.sock);
			cleanCDKMentry(inputWidget);
			activateCDKMentry(inputWidget, NULL);
			continue;
		}

		pthread_mutex_lock(activateChatNode.logger_mutex);
		(activateChatNode.logger_ptr)->writeLog(buffer);
		(activateChatNode.logger_ptr)->writeLog(info);
		pthread_mutex_unlock(activateChatNode.logger_mutex);


		free(info);
		cleanCDKMentry(inputWidget);
		activateCDKMentry(inputWidget, NULL);
	}
	return 0;
}

#endif
