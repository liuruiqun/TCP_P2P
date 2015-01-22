#include <cdk.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

typedef struct {
	CDKMENTRY *inputWidget;
	CDKSWINDOW *displayWidget;
	CDKSCROLL *onlineListWidget;
	CDKSCROLL *currentChatingListWidget;
}sMainPage_t;


int mainPage(CDKSCREEN *cdkscreen, sMainPage_t *returnParams) {

	CDKSWINDOW *displayWidget;
	CDKMENTRY *inputWidget;
	CDKSCROLL *onlineListWidget;
	CDKSCROLL *currentChatingListWidget;

	const char *onlineTitle = "<C></B/24>Online List";
	const char *chatTitle = "<C></B/24>Chat List";

	if(cdkscreen == NULL)
		return -1;

	onlineListWidget = newCDKScroll(cdkscreen,
			8, 0,
			NONE,
			17, 20,
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
	if(currentChatingListWidget == NULL)
		return -1;

	displayWidget = newCDKSwindow(cdkscreen,
			30, 0,
			16, 42,
			NULL,
			50,
			true,
			false);
	if(displayWidget == NULL)
		return -1;

	inputWidget = newCDKMentry(cdkscreen,
			30, 17,
			NULL, NULL, A_NORMAL, ' ', vMIXED,
			40, 5, 10,
			0,
			true,
			false
			);
	if(inputWidget == NULL)
		return -1;

	drawCDKScroll(onlineListWidget, true);
	drawCDKScroll(currentChatingListWidget, true);
	drawCDKSwindow(displayWidget, true);
	drawCDKMentry(inputWidget, true);

	returnParams->onlineListWidget = onlineListWidget;
	returnParams->currentChatingListWidget = currentChatingListWidget;
	returnParams->displayWidget = displayWidget;
	returnParams->inputWidget = inputWidget;

	return 0;
}


int main()
{
	WINDOW *cursesWin;
	CDKSCREEN *cdkscreen;


	cursesWin = initscr();

	cdkscreen = initCDKScreen(cursesWin);

	initCDKColor();

	sMainPage_t main_page;

	if(mainPage(cdkscreen, &main_page) == -1)
	{
		destroyCDKScreen(cdkscreen);
		endCDK();
		return -1;
	}

	activateCDKMentry(main_page.inputWidget, 0);	
	//sleep(300);	
	destroyCDKScroll(main_page.onlineListWidget);
	destroyCDKScroll(main_page.currentChatingListWidget);
	destroyCDKSwindow(main_page.displayWidget);
	destroyCDKMentry(main_page.inputWidget);
	destroyCDKScreen(cdkscreen);
	endCDK();
	return 0;
}
