#ifndef server_h
#define server_h

#include "access_event.h"
#include "compat.h"
#include "file.h"
#include <pthread.h>

struct server {
	int sd[2];
	pthread_t tid;
	struct file_info finfo;
	char file1[PATH_MAX];
	char file2[PATH_MAX];
};

int start_server(struct server *s);
int stop_server(struct server *s);

#endif
