#include <cdk.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "proto.h"

int login(CDKSCREEN *cdkscreen, int UDPClientSocket, sockaddr_in *UDPServerAddress, sUserInfo_t *localUserInfo) {
	sMessage_t loginMsg;
	sMessage_t LoginReplyMsg;
	socklen_t addrLen;
	sockaddr_in remoteAddr;

	if(cdkscreen == NULL || UDPServerAddress == NULL || localUserInfo == NULL)
		return -1;
	remoteAddr = *UDPServerAddress;
	
	if(UDPClientSocket == -1)
		return -1;
	
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
	if(loginEntry == NULL)
		return -1;

	temp = activateCDKEntry(loginEntry, 0);

	while(true) {
		if(loginEntry->exitType == vESCAPE_HIT) {
			destroyCDKEntry(loginEntry);
			return -1;
		}

		if(strlen(temp) != 0) {
			loginMsg.msgType = LOGIN;
			strcpy(loginMsg.userInfo.userName, temp);
			loginMsg.userInfo.ip = localUserInfo->ip;
			loginMsg.userInfo.port = localUserInfo->port;

			ssize_t sendState = ::sendto(UDPClientSocket, &loginMsg, sizeof(sMessage_t), 0, (sockaddr *)&remoteAddr, sizeof(sockaddr_in));

			if(sendState == -1) {
				destroyCDKEntry(loginEntry);
				return -1;
			}

			ssize_t recvState = ::recvfrom(UDPClientSocket, &LoginReplyMsg, sizeof(sMessage_t), 1, (sockaddr *)&remoteAddr, &addrLen);
			if(recvState == -1) {
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
			
		temp = activateCDKEntry(loginEntry, 0);
	}
	
	destroyCDKEntry(loginEntry);
	return 0;
}

