#ifndef LOADALLHISTORY_H
#define LOADALLHISTORY_H

#include <cdk.h>
#include "./proto.h"
#include "./tools.h"
#include <string.h>
#include <stdio.h>
#include <pthread.h>

int readHistory(CDKSCREEN *cdkscreen, CDKSWINDOW *swindow, const char *filename)
{
   char **fileInfo = 0;
   int lines;
   char *prompt[1];


   /* Open the file and read it in. */
   lines = CDKreadFile (filename, &fileInfo);
   if (lines == -1)
   {
	  prompt[0] = "CANNOT READ HISTORY FILE";
	  popupLabel(cdkscreen, (CDK_CSTRING2)prompt, 1);
      return -1;
   }

   /* Clean out the scrolling window. */
   cleanCDKSwindow (swindow);

   /* Set the new information in the scrolling window. */
   setCDKSwindow (swindow, (CDK_CSTRING2) fileInfo, lines, ObjOf (swindow)->box);

   /* Clean up. */
   CDKfreeStrings (fileInfo);
   
   return 0;
}



int loadHistory(EObjectType cdktype, void *object, void *params, chtype key) {
	sDisplayWidgetParams_t *ptr = (sDisplayWidgetParams_t *)params;
	if(ptr == NULL)
		return -1;

	char activatePeerName[MAX_NAME_LEN + 1];
	char filename[256];

	int reval;

	pthread_mutex_lock(ptr->activateChatNode_mutex);
	strcpy(activatePeerName, (ptr->activateChatNode_ptr)->peerName);
	pthread_mutex_unlock(ptr->activateChatNode_mutex);

	if(strlen(activatePeerName) == 0) {
		writeToDisplayWidget(ptr->displayWidget, "NO ACTIVE CHAT");
		return -1;
	}

	snprintf(filename, 256, "./records/%s.txt", activatePeerName);

	pthread_mutex_lock(ptr->displayWidget_mutex);
	cleanCDKSwindow(ptr->displayWidget);
	reval = readHistory(ptr->cdkscreen, ptr->displayWidget, filename);
	pthread_mutex_unlock(ptr->displayWidget_mutex);

//	activateCDKSwindow(ptr->displayWidget, NULL);		
	
	return reval;
}

#endif
