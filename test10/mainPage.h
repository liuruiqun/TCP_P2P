#ifndef MAINPAGE_H
#define MAINPAGE_H

#include <cdk.h>
#include <unistd.h>
#include <string.h>
#include "./proto.h"


int mainPage(CDKSCREEN *cdkscreen, sMainPage_t *returnParams);


int mainPage(CDKSCREEN *cdkscreen, sMainPage_t *returnParams) {

	CDKSWINDOW *displayWidget;
	CDKMENTRY *inputWidget;
	CDKSCROLL *onlineListWidget;
	CDKSCROLL *currentChatingListWidget;
	CDKSCROLL *requestListwidget;

	const char *onlineTitle = "<C></B/24>Online List";
	const char *chatTitle = "<C></B/24>Chat List";
	const char *requestTitle ="<C></B/24>Request List";

	if(cdkscreen == NULL)
		return -1;

	onlineListWidget = newCDKScroll(cdkscreen,
			8, 0,
			NONE,
			12, 20,
			onlineTitle,
			(CDK_CSTRING2)NULL, 0,
			false,
			A_REVERSE,
			true,
			false);
	if(onlineListWidget == NULL)
		return -1;

	currentChatingListWidget = newCDKScroll(cdkscreen,
			8, 17,
			NONE,
			7, 20,
			chatTitle,
			(CDK_CSTRING2)NULL, 0,
			false,
			A_REVERSE,
			true,
			false);
	if(currentChatingListWidget == NULL) {
		destroyCDKScroll(onlineListWidget);
		return -1;
	}
	
	requestListwidget = newCDKScroll(cdkscreen,
			8, 12,
			NONE,
			5, 20,
			requestTitle,
			(CDK_CSTRING2)NULL, 0,
			false,
			A_REVERSE,
			true,
			false);

	if(requestListwidget == NULL) {
		destroyCDKScroll(onlineListWidget);
		destroyCDKScroll(currentChatingListWidget);
		return -1;
	}

	displayWidget = newCDKSwindow(cdkscreen,
			30, 0,
			16, 42,
			NULL,
			50,
			true,
			false);
	if(displayWidget == NULL) {
		destroyCDKScroll(onlineListWidget);
		destroyCDKScroll(currentChatingListWidget);
		destroyCDKScroll(requestListwidget);
		return -1;
	}

	inputWidget = newCDKMentry(cdkscreen,
			30, 17,
			NULL, NULL, A_NORMAL, ' ', vMIXED,
			40, 5, 10,
			0,
			true,
			false
			);
	if(inputWidget == NULL) {
		destroyCDKScroll(onlineListWidget);
		destroyCDKScroll(currentChatingListWidget);
		destroyCDKScroll(requestListwidget);
		destroyCDKSwindow(displayWidget);
		return -1;
	}

	drawCDKScroll(onlineListWidget, true);
	drawCDKScroll(currentChatingListWidget, true);
	drawCDKScroll(requestListwidget, true);
	drawCDKSwindow(displayWidget, true);
	drawCDKMentry(inputWidget, true);
	
	returnParams->cdkscreen = cdkscreen;
	returnParams->onlineListWidget = onlineListWidget;
	returnParams->currentChatingListWidget = currentChatingListWidget;
	returnParams->requestListwidget = requestListwidget;
	returnParams->displayWidget = displayWidget;
	returnParams->inputWidget = inputWidget;

	return 0;
}
#endif
