#ifndef TOOLS_H
#define TOOLS_H
#include <cdk.h>
#include <string.h>
#include <list>
#include "./proto.h"
#include "readLastLinesFromFile.h"
using std::list;


bool isUserNameExisted(list<sUserInfo_t> *list_ptr, char * userName) {
	list<sUserInfo_t>::iterator it;
	for(it = list_ptr->begin(); it != list_ptr->end(); it++) {
		if(strcmp(it->userName, userName) == 0)
			return true;
	}

	return false;
}

list<sUserInfo_t>::iterator getIterator(list<sUserInfo_t> *list_ptr, char * userName) {
	list<sUserInfo_t>::iterator it;
	for(it = list_ptr->begin(); it != list_ptr->end(); it++) {
		if(strcmp(it->userName, userName) == 0)
			break;
	}
	return it;
}

bool isChatExist(char *userName, list<sChatNode_t> *chatingList_ptr)
{
	list<sChatNode_t>::iterator it;
	for(it = chatingList_ptr->begin(); it != chatingList_ptr->end(); it++) {
		if(strcmp(userName, it->peerName) == 0)
			return true;
	}
	return false;
}

int deleteChatNode(char *peerName, list<sChatNode_t> *chatingList_ptr) {
	list<sChatNode_t>::iterator it;
	for(it = chatingList_ptr->begin(); it != chatingList_ptr->end(); it++)
		if(strcmp(peerName, it->peerName) == 0)
		{
			chatingList_ptr->erase(it);
			return 0;
		}
	return -1;
}

sChatNode_t getChatNode(list<sChatNode_t> *chatingList_ptr, int index)
{
	sChatNode_t chatNode;
	strcpy(chatNode.peerName, "");

	list<sChatNode_t>::iterator it;
	
	it = chatingList_ptr->begin();
	for(int i = 0; i < index; i++)
		it++;

	if(it != chatingList_ptr->end())
		chatNode = *it;

	return chatNode;	
}

int writeToDisplayWidget(CDKSWINDOW *displayWidget, char *info) {
	if(displayWidget == NULL || info == NULL)
		return -1;
	char buf[41];
	int infoLen = strlen(info);
	int i;
	for(i = 0; i <= infoLen/40; i++) {
		strncpy(buf, info + 40*i, 40);
		if((i == infoLen/40) && ((infoLen % 40) == 0))
			break;
		addCDKSwindow(displayWidget, buf, BOTTOM);
	}
	return 0;
}

int flushChatingListWidget(CDKSCROLL *chatingListWidget, list<sChatNode_t> *chatingList_ptr) {
	if(chatingListWidget == NULL)
		return -1;
	if(chatingList_ptr == NULL)
		return -1;

	int sizeOfChatList = chatingList_ptr->size();

	char **items = new char*[sizeOfChatList];
	if(items == NULL)
		return -1;
	list<sChatNode_t>::iterator it;
	int i = 0;
	for(it = chatingList_ptr->begin(); it != chatingList_ptr->end(); it++) {
		char *temp = new char[MAX_NAME_LEN + 1];
		if(temp == NULL) {
			for(int j = 0; j < i; j++)
				delete items[j];
			delete items;
			return -1;
		}
		strcpy(temp, it->peerName);
		items[i] = temp;
		i++;
	}	

	setCDKScroll(chatingListWidget,
			items,
			sizeOfChatList,
			false,
			A_REVERSE,
			true);

	drawCDKScroll(chatingListWidget, true);
	raiseCDKObject(vSCROLL, chatingListWidget);

	for(i = 0; i < sizeOfChatList; i++)
		delete items[i];
	delete items;
	return 0;
}

int flushOnlineListWidget(CDKSCROLL *onlineListWidget, list<sUserInfo_t> *onlineList_ptr)
{
	if(onlineListWidget == NULL)
		return -1;
	if(onlineList_ptr == NULL)
		return -1;

	int sizeOfOnlineList = onlineList_ptr->size();
	char **items = new char*[sizeOfOnlineList];
	if(items == NULL)
		return -1;

	list<sUserInfo_t>::iterator it;
	int i = 0;
	for(it = onlineList_ptr->begin(); it != onlineList_ptr->end(); it++) {
		char *temp = new char[MAX_NAME_LEN + 1];
		if(temp == NULL) {
			for(int j = 0; j < i; j++)
				delete items[j];
			delete items;
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

	drawCDKScroll(onlineListWidget, true);
	raiseCDKObject(vSCROLL, onlineListWidget);

	for(i = 0; i < sizeOfOnlineList; i++){
		delete items[i];
	}

	delete items;
	return 0;
}

int loadContext(const char *fileName, CDKSWINDOW *displayWidget) {
	if(fileName == NULL)
		return -1;
	if(displayWidget == NULL)
		return -1;
	int lines;
	char *history[100];

	if(readLastLinesFromFile(fileName, history, &lines) == -1)
		return -1;

	cleanCDKSwindow(displayWidget);
	for(int i = 0; i < lines; i++) {
		writeToDisplayWidget(displayWidget, history[i]);
		delete history[i];
	}

	return 0;
}

#endif
