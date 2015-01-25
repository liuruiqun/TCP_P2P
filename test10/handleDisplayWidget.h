#ifndef HANDLEDISPLAYWIDGET_H
#define HANDLEDISPLAYWIDGET_H

#include <cdk.h>
#include "./proto.h"

int jumpToDisplayWidget(EObjectType cdkType, void *object, void *params, chtype key) {
	
	sDisplayWidgetParams_t *ptr = (sDisplayWidgetParams_t *)params;
	if(ptr == NULL)
		return -1;

	if(ptr->displayWidget == NULL)
		return -1;

	activateCDKSwindow(ptr->displayWidget, 0);

	return 0;
}

#endif
