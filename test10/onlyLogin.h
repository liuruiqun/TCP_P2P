#ifndef ONLYLOGIN_H
#define ONLYLOGIN_H



#include <cdk.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "proto.h"
#include "./CLogger.h"

int onlyLogin(CDKSCREEN *cdkscreen, int TCPClientSocket, sUserInfo_t *localUserInfo);


int onlyLogin(CDKSCREEN *cdkscreen, int TCPClientSocket, sUserInfo_t *localUserInfo) {
	sMessage_t loginMsg;
	sMessage_t LoginReplyMsg;

	if(TCPClientSocket == -1)
		return -1;

	if(cdkscreen == NULL || localUserInfo == NULL) {
		close(TCPClientSocket);
		return -1;
	}
	
	CDKENTRY *loginEntry;
	const char *loginEntryLabel = "User Name:"; 
	const char *loginEntryTitle = "Please Login.\n";
	char *promptMsg[1];
	promptMsg[0] = "User name ERROR. Please try again.";

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
	if(loginEntry == NULL) {
		close(TCPClientSocket);
		return -1;
	}

	temp = activateCDKEntry(loginEntry, 0);

	while(true) {
		if(loginEntry->exitType == vESCAPE_HIT) {
			close(TCPClientSocket);
			destroyCDKEntry(loginEntry);
			return -1;
		}

		if(strlen(temp) > 0) {
			memset(&loginMsg, 0, sizeof(sMessage_t));
			memset(&LoginReplyMsg, 0, sizeof(sMessage_t));

			loginMsg.msgType = LOGIN;
			strcpy(loginMsg.userInfo.userName, temp);
			loginMsg.userInfo.ip = localUserInfo->ip;
			loginMsg.userInfo.port = localUserInfo->port;
		
			if(send(TCPClientSocket, &loginMsg, sizeof(sMessage_t), 0) == -1) {
				close(TCPClientSocket);
				destroyCDKEntry(loginEntry);
				return -1;
			}	

			if(recv(TCPClientSocket, &LoginReplyMsg, sizeof(sMessage_t), 0) == -1) {
				close(TCPClientSocket);
				destroyCDKEntry(loginEntry);
				return -1;
			}

			if(LoginReplyMsg.msgType == LOGSUCCESS) {
				strcpy(localUserInfo->userName, temp);
				break;
			}
			else
				popupLabel(cdkscreen, (CDK_CSTRING2)promptMsg, 1);
		}

		cleanCDKEntry(loginEntry);

		temp = activateCDKEntry(loginEntry, 0);
	}
	
	destroyCDKEntry(loginEntry);
	refreshCDKScreen(cdkscreen);
	return 0;
}
#endif
