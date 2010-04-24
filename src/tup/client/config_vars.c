#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include "tup_config_vars.h"
#include "tup/access_event.h"

struct vardict {
	unsigned int len;
	unsigned int num_entries;
	unsigned int *offsets;
	const char *entries;
	void *map;
};
static void tup_var_init(void) __attribute__((constructor));
static int init_vardict(int fd);

static struct vardict tup_vars;

static void tup_var_init(void)
{
	char *path;
	int vardict_fd;

	path = getenv(TUP_VARDICT_NAME);
	if(!path) {
		fprintf(stderr, "tup client error: Couldn't find path for '%s'\n",
			TUP_VARDICT_NAME);
		abort();
	}
	vardict_fd = strtol(path, NULL, 0);
	if(vardict_fd <= 0) {
		fprintf(stderr, "tup client error: vardict_fd <= 0\n");
		abort();
	}
	if(init_vardict(vardict_fd) < 0) {
		fprintf(stderr, "tup client error: init_vardict() failed\n");
		abort();
	}
}

static int init_vardict(int fd)
{
	struct stat buf;
	unsigned int expected = 0;

	if(fstat(fd, &buf) < 0) {
		perror("fstat");
		return -1;
	}
	tup_vars.len = buf.st_size;
	if(tup_vars.len == 0) {
		/* Empty file is ok - no variables will be read */
		tup_vars.num_entries = 0;
		tup_vars.offsets = NULL;
		tup_vars.entries = NULL;
		tup_vars.map = NULL;
		return 0;
	}

	expected += sizeof(unsigned int);
	if(tup_vars.len < expected) {
		fprintf(stderr, "tup client error: var-tree should be at least sizeof(unsigned int) bytes, but got %i bytes\n", tup_vars.len);
		return -1;
	}
	tup_vars.map = mmap(NULL, tup_vars.len, PROT_READ, MAP_PRIVATE, fd, 0);
	if(tup_vars.map == MAP_FAILED) {
		perror("mmap");
		return -1;
	}

	tup_vars.num_entries = *(unsigned int*)tup_vars.map;
	tup_vars.offsets = (unsigned int*)((char*)tup_vars.map + expected);
	expected += sizeof(unsigned int) * tup_vars.num_entries;
	tup_vars.entries = (const char*)tup_vars.map + expected;
	if(tup_vars.len < expected) {
		fprintf(stderr, "tup client error: var-tree should have at least %i bytes to accommodate the index, but got %i bytes\n", expected, tup_vars.len);
		return -1;
	}

	return 0;
}

const char *tup_config_var(const char *key, int keylen)
{
	int left = -1;
	int right = tup_vars.num_entries;
	int cur;
	const char *p;
	const char *k;
	int bytesleft;
	char magic[] = TUP_VAR_MAGIC;
	char tmpfname[128];

	if(keylen == -1)
		keylen = strlen(key);

	if(keylen >= (signed)sizeof(tmpfname) - (signed)sizeof(magic)) {
		fprintf(stderr, "tup client error: keylen of %i is too large for the tmp buffer.\n", keylen);
	}
	memcpy(tmpfname, magic, sizeof(magic));
	memcpy(tmpfname + sizeof(magic)-1, key, keylen);
	tmpfname[sizeof(magic) + keylen] = 0;
	/* The open should always fail, but just in case wrap it in a close. */
	close(open(tmpfname, O_RDONLY));
	while(1) {
		cur = (right - left) >> 1;
		if(cur <= 0)
			break;
		cur += left;
		if(cur >= (signed)tup_vars.num_entries)
			break;

		if(tup_vars.offsets[cur] >= tup_vars.len) {
			fprintf(stderr, "tup client error: Offset for element %i is out of bounds.\n", cur);
			break;
		}
		p = tup_vars.entries + tup_vars.offsets[cur];
		k = key;
		bytesleft = keylen;
		while(bytesleft > 0) {
			/* Treat '=' as if p ended */
			if(*p == '=') {
				left = cur;
				goto out_next;
			}
			if(*p < *k) {
				left = cur;
				goto out_next;
			} else if(*p > *k) {
				right = cur;
				goto out_next;
			}
			p++;
			k++;
			bytesleft--;
		}

		if(*p != '=') {
			right = cur;
			goto out_next;
		}
		return p+1;
out_next:
		;
	}
	return NULL;
}
