#include <cdk.h>

int main() {
	WINDOW *cursesWin;
	CDKSCREEN *cdkscreen;
	CDKSCROLL *onlineList;
	const char *title = "<C></B/24>Online List";
	char *item1[] = {"A", "B", "C", "D", "E", "F"};
	char *item2[] = {"1", "2", "3", "4", "5", "6"};
	char **item3 = new char* [6];
	int item3_len = sizeof(item3);
	if(item3 == NULL) {
		return -1;
	}

	for(int i = 0; i < 6; i++) {
		char *temp = new char [4];
		if(temp == NULL) 
			return -1;
		strcpy(temp, "HI.");
		item3[i] = temp;
	}

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
			item3,
			6,
			false,
			A_REVERSE,
			true);
	for(int i = 1; i < 6; i++){
		delete item3[i];
	}

	delete item3;

	int selection = activateCDKScroll(onlineList, 0);


	destroyCDKScroll(onlineList);
	destroyCDKScreen(cdkscreen);
	endCDK();

	printf("you select %d, sizeof item3 %d\n", selection, item3_len);
	return 0;
}
