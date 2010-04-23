#include "server.h"
#include "file.h"
#include "debug.h"
#include "fileio.h"
#include "db.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>

static void *message_thread(void *arg);
static int recvall(int sd, void *buf, size_t len);

int start_server(struct server *s)
{
	if(socketpair(AF_UNIX, SOCK_STREAM, 0, s->sd) < 0) {
		perror("socketpair");
		return -1;
	}

	init_file_info(&s->finfo);

	if(pthread_create(&s->tid, NULL, message_thread, s) < 0) {
		perror("pthread_create");
		close(s->sd[0]);
		close(s->sd[1]);
		return -1;
	}

	return 0;
}

int stop_server(struct server *s)
{
	void *retval = NULL;
	struct access_event e;
	int rc;

	memset(&e, 0, sizeof(e));
	e.at = ACCESS_STOP_SERVER;

	rc = send(s->sd[1], &e, sizeof(e), 0);
	if(rc != sizeof(e)) {
		perror("send");
		return -1;
	}
	pthread_join(s->tid, &retval);
	close(s->sd[0]);
	close(s->sd[1]);

	if(retval == NULL)
		return 0;
	return -1;
}

static void *message_thread(void *arg)
{
	struct access_event event;
	struct server *s = arg;

	while(recvall(s->sd[0], &event, sizeof(event)) == 0) {
		if(event.at == ACCESS_STOP_SERVER)
			break;
		if(!event.len)
			continue;

		if(event.len >= (signed)sizeof(s->file1) - 1) {
			fprintf(stderr, "Error: Size of %i bytes is longer than the max filesize\n", event.len);
			return (void*)-1;
		}
		if(event.len2 >= (signed)sizeof(s->file2) - 1) {
			fprintf(stderr, "Error: Size of %i bytes is longer than the max filesize\n", event.len2);
			return (void*)-1;
		}

		if(recvall(s->sd[0], s->file1, event.len) < 0) {
			fprintf(stderr, "Error: Did not recv all of file1 in access event.\n");
			return (void*)-1;
		}
		if(recvall(s->sd[0], s->file2, event.len2) < 0) {
			fprintf(stderr, "Error: Did not recv all of file2 in access event.\n");
			return (void*)-1;
		}

		s->file1[event.len] = 0;
		s->file2[event.len2] = 0;

		if(handle_file(event.at, s->file1, s->file2, &s->finfo) < 0) {
			return (void*)-1;
		}
		/* Oh noes! An electric eel! */
		;
	}
	return NULL;
}

static int recvall(int sd, void *buf, size_t len)
{
	size_t recvd = 0;
	char *cur = buf;

	while(recvd < len) {
		int rc;
		rc = recv(sd, cur + recvd, len - recvd, 0);
		if(rc < 0) {
			perror("recv");
			return -1;
		}
		if(rc == 0)
			return -1;
		recvd += rc;
	}
	return 0;
}
