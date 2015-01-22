#include <cdk.h>

int main() {
	CDKSCREEN *cdkscreen;
	CDKLABEL *demo;
	WINDOW	*screen;
	char *mesg[5];

	screen = initscr();
	cdkscreen = initCDKScreen(screen);

	initCDKColor();

	mesg[0] = "<R></B/31>This line should have a yellow foreground and a blue backgroud.<!31>";
	mesg[1] = "</U/05>This line should have a white foreground and a blue backgroud. <!05>";
	mesg[2] = "<B=+banchon>This is a bullet.";
	mesg[3] = "<I=10>This is indented 10 characters.";
	mesg[4] = "<C>This line should be set to whatever the screen default is.";
	demo = newCDKLabel(cdkscreen, CENTER, CENTER, mesg, 5, TRUE, TRUE);

	drawCDKLabel(demo, TRUE);
	waitCDKLabel(demo, ' ');

	destroyCDKLabel(demo);
	destroyCDKScreen(cdkscreen);
	endCDK();
	exit(0);
}
