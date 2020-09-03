#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
	int NO;
	struct watchpoint *next;

	/* TODO: Add more members if necessary */
	int val;
	char str[32];

} WP;

WP* new_wp();
void free_wp(WP* wp);
void info_wp();
#endif
