#include <stdio.h>
#include <time.h>
#include <string.h>
void getTimeString(char timestr[]) {
	char buffer[30];
	time_t currentTime;
	currentTime = time(NULL);
	ctime_r(&currentTime, buffer);
	int len = strlen(buffer);
	//insert two space.
	buffer[len - 1] = ' ';
	buffer[len] = ' ';
	buffer[len + 1] = '\0';
	strcpy(timestr, buffer + 4);
	return;
}

int main() {
	char buf[30];
	getTimeString(buf);
	printf("%sEND.\n", buf);
}
