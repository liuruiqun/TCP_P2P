#include <cdk.h>

int main() {
	WINDOW *cursesWin;
	CDKSCREEN *cdkscreen;
	CDKSCROLL *onlineList;
	const char *title = "<C></B/24>Online List";
	char *item1[] = {"A", "B", "C", "D", "E", "F"};
	char *item2[] = {"1", "2", "3", "4", "5", "6"};

	cursesWin = initscr();
	cdkscreen = initCDKScreen(cursesWin);
	initCDKColor();

	onlineList = newCDKScroll(cdkscreen,
			CENTER, CENTER,
			RIGHT,
			10, 50,
			title,
			(CDK_CSTRING2)item1,
			6,
			TRUE,
			A_REVERSE,
			TRUE,
			FALSE);
	if(onlineList == 0) {
		return -1;
	}

	activateCDKScroll(onlineList, 0);

	setCDKScroll(onlineList,
			item2,
			6,
			false,
			A_REVERSE,
			true);
	activateCDKScroll(onlineList, 0);

	destroyCDKScroll(onlineList);
	destroyCDKScreen(cdkscreen);
	endCDK();
}
