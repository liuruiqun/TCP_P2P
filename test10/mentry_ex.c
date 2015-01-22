/* $Id: mentry_ex.c,v 1.10 2012/03/21 23:32:57 tom Exp $ */
#include <cdk_test.h>
#include <stdio.h>
#ifdef HAVE_XCURSES
char *XCursesProgramName = "mentry_ex";
#endif

int main (int argc, char **argv)
{
   /* *INDENT-EQLS* */
   CDKSCREEN *cdkscreen = 0;
   CDKMENTRY *widget    = 0;
   WINDOW *cursesWin    = 0;
   char *info           = 0;
   const char *label    = "</R>Message";
   const char *title    = "<C></5>Enter a message.<!5>";

   CDK_PARAMS params;

   CDKparseParams (argc, argv, &params, "w:h:l:" CDK_MIN_PARAMS);

   /* Set up CDK. */
   cursesWin = initscr ();
   cdkscreen = initCDKScreen (cursesWin);

   /* Start CDK Colors. */
   initCDKColor ();

   /* Set up the multi-line entry field. */
   widget = newCDKMentry (cdkscreen,
			  CDKparamValue (&params, 'X', CENTER),
			  CDKparamValue (&params, 'Y', CENTER),
			  title, label, A_BOLD, '.', vMIXED,
			  CDKparamValue (&params, 'w', 20),
			  CDKparamValue (&params, 'h', 5),
			  CDKparamValue (&params, 'l', 20),
			  0,
			  CDKparamValue (&params, 'N', TRUE),
			  CDKparamValue (&params, 'S', FALSE));

   /* Is the object null? */
   if (widget == 0)
   {
      /* Shut down CDK. */
      destroyCDKScreen (cdkscreen);
      endCDK ();

      printf ("Cannot create CDK object. Is the window too small?\n");
      ExitProgram (EXIT_FAILURE);
   }

   /* Draw the CDK screen. */
   refreshCDKScreen (cdkscreen);

   /* Set what ever was given from the command line. */
   setCDKMentry (widget, argv[optind], 0, TRUE);

   boolean isFunctionKey;
   int result;
   char a[400] = {'\0'};
   for(;;)
   {
	   char *msg[2];
	   result = getchCDKObject((CDKOBJS*)widget, &isFunctionKey);
	   injectCDKEntry(widget, result);
	   char result_c;
	   if(isFunctionKey == true)
	   {
			if(result == KEY_BACKSPACE)
				strcat(a, "\b");
			if(result == KEY_ENTER)
				break;
			
	   }
	   else 
	   {
		   char b[2];
		   sprintf(b, "%c", result);
		   strcat(a, b);

	   }

//	   msg[0] = a;
//	   popupLabel(cdkscreen, (CDK_CSTRING2)msg, 1);

   }

   /* Activate this thing. */
   activateCDKMentry (widget, 0);
   info = strdup (widget->info);

   /* Clean up. */
   destroyCDKMentry (widget);
   destroyCDKScreen (cdkscreen);
   endCDK ();

   printf ("\n\n\n");
   printf ("Your message was : <%s>\n", info);
   free (info);
   int i;
   printf("the original message was:");
//   printf("%d", strlen(a));
   int lastPoint = 0;
   for (i = 0; i < strlen(a); i++)
   {
	   if(a[i] == '\b'){
		   printf("%c %c", a[i], a[i]);
	   }
	   else
		   printf("%c",a[i]);
	   fflush(stdout);
	   usleep(1000*300);
//	   
   }
   printf("\n");
   ExitProgram (EXIT_SUCCESS);
}
