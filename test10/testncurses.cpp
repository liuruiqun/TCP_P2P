#include <ncurses.h>
#include <string.h>
#include <unistd.h>

int main() {
	char *a = "jiushi\b\b\byidian.";
	initscr();
	move(5,5);
	printw("Please input:");
	refresh();
	int i;
	for(i = 0; i < strlen(a); i++)
	{
		if(a[i] == '\b')
			printw("%c %c", a[i], a[i]);
		else
			printw("%c", a[i]);
		refresh();
		usleep(1000*300);
	}
	
	
	sleep(500);
	endwin();
	return 0;
}
