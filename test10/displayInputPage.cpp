#include <cdk.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

int InputPage(CDKSCREEN *);

typedef struct chatPage{
	CDKSWINDOW *displayWidget;
	CDKMENTRY *inputWidget;
	pthread_mutex_t *mutex;
}chatPage_t;

void *HandleInput(void *params);
void *SendMessagePer2Sec(void *params);
int writeToDisplayWidget(CDKSWINDOW *, char *);
int jumpFromInputWidgetToDisplayWidget(EObjectType cdktype,
		void *inputWidget, void *chat_page, chtype key); 


int main() {
	WINDOW *cursesWindow;
	CDKSCREEN *cdkscreen;
	CDKSWINDOW *displayWidget;
	CDKMENTRY *inputWidget;

	cursesWindow = initscr();
	cdkscreen = initCDKScreen(cursesWindow);
	if(cdkscreen == NULL)
		return -1;
	initCDKColor();

	displayWidget = newCDKSwindow(cdkscreen,
			20, 0,
			16, 42,
			NULL,
			50,
			true,
			false);
	if(displayWidget == NULL)
		return -1;

	inputWidget = newCDKMentry(cdkscreen,
			20, 20,
			NULL, NULL, A_NORMAL, ' ', vMIXED,
			40, 5, 10,
			0,
			true,
			false
			);
	if(inputWidget == NULL)
		return -1;

	drawCDKSwindow(displayWidget, true);
	drawCDKMentry(inputWidget, true);
	chtype key = KEY_F(5); 
//	chtype key = 'a';	
	pthread_mutex_t mutex;
	chatPage_t chat_page;
	chat_page.displayWidget = displayWidget;
	chat_page.inputWidget = inputWidget;
	chat_page.mutex = &mutex;

	bindCDKObject(vMENTRY, inputWidget, key, jumpFromInputWidgetToDisplayWidget, &chat_page);
//	setCDKSwindowPreProcess(chat_page.displayWidget, jumpFromInputWidgetToDisplayWidget, &chat_page);
	pthread_mutex_init(&mutex, 0);

	pthread_t inputTid;
	if(pthread_create(&inputTid, NULL, HandleInput, &chat_page) != 0)
	{
		destroyCDKMentry(inputWidget);
		destroyCDKSwindow(displayWidget);
		destroyCDKScreen(cdkscreen);
		endCDK();
		return -1;
	}
	
	//This is simulate receive message from socket.
	pthread_t testTid;
	if(pthread_create(&testTid, NULL, SendMessagePer2Sec, &chat_page) != 0) {
		destroyCDKMentry(inputWidget);
		destroyCDKSwindow(displayWidget);
		destroyCDKScreen(cdkscreen);
		endCDK();
		return -1;
	}
	

	pthread_join(inputTid, 0);
	destroyCDKMentry(inputWidget);
	destroyCDKSwindow(displayWidget);
	destroyCDKScreen(cdkscreen);
	endCDK();
	return 0;

}


void *HandleInput(void *params)
{
	chatPage_t * ptr = (chatPage_t *)params;
	char *info;

	if(ptr->displayWidget == NULL || 
			ptr->inputWidget == NULL ||
			ptr->mutex == NULL)
		return NULL;

	activateCDKMentry(ptr->inputWidget, 0);
	while((ptr->inputWidget)->exitType != vESCAPE_HIT) {
		info = strdup((ptr->inputWidget)->info);
		//send the info to socket.
		//
		//display the info on displayWidget.
		pthread_mutex_lock(ptr->mutex);
		writeToDisplayWidget(ptr->displayWidget, info);
		pthread_mutex_unlock(ptr->mutex);

		free(info);
		cleanCDKMentry(ptr->inputWidget);
		activateCDKMentry(ptr->inputWidget, 0);
	}
}


void *SendMessagePer2Sec(void *params) {
	chatPage_t *ptr =(chatPage_t *)params;

	if(ptr->displayWidget == NULL || 
			ptr->mutex == NULL)
		return NULL;
	while(true) {
		pthread_mutex_lock(ptr->mutex);
		writeToDisplayWidget(ptr->displayWidget, "This \bis Test message.");
		pthread_mutex_unlock(ptr->mutex);
		sleep(5);
	}
}

int jumpFromInputWidgetToDisplayWidget(EObjectType cdktype,
		void *inputWidget, void *params, chtype key) {
	CDKMENTRY *input = (CDKMENTRY*)inputWidget;
	chatPage_t *ptr = (chatPage_t *)params;

	if(input == NULL || ptr->displayWidget == NULL || ptr->mutex == NULL)
		return -1;
	pthread_mutex_lock(ptr->mutex);
	activateCDKSwindow(ptr->displayWidget, NULL);
	pthread_mutex_unlock(ptr->mutex);

	return 0;
}






//codes below are useless.
//
//
int displayPage(CDKSCREEN *cdkscreen) {
	CDKSWINDOW *displayWindow;

	if(cdkscreen == NULL)
		return -1;

	displayWindow = newCDKSwindow(cdkscreen,
			CENTER, CENTER,
			30, 50,
			NULL,
			300,
			true,
			false);
	if(displayWindow == NULL)
		return -1;

}

int InputPage(CDKSCREEN *cdkscreen) {
	CDKMENTRY *mentry;
;	char *info;
	if(cdkscreen == NULL)
		return -1;
	mentry = newCDKMentry(cdkscreen,
			CENTER, CENTER,
			NULL, NULL, A_NORMAL, ' ', vMIXED,
			40, 10, 20,
			0,
			true,
			false
			);
	if(mentry == NULL)
		return -1;
	activateCDKMentry(mentry, 0);
	while(mentry->exitType != vESCAPE_HIT) { 
		info = strdup(mentry->info);
		char *mesg[1];
		mesg[0] = info;
		popupLabel(cdkscreen, (CDK_CSTRING2)mesg, 1);
		free(info);//how to output to displayWindow.
		cleanCDKMentry(mentry);
	//	sleep(300);
		activateCDKMentry(mentry, 0);
	}
	destroyCDKMentry(mentry);
	return 0;
}

