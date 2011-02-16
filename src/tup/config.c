#include "config.h"
#include "compat.h"
#include "db.h"
#include "fileio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#if defined(__APPLE__) || defined(__FreeBSD__)
  #include <sys/param.h>
  #include <sys/sysctl.h>
#elif defined (_WIN32)
  #include <windows.h>
#endif


static char tup_wd[PATH_MAX];
static int tup_wd_offset;
static int tup_top_len;
static int tup_sub_len;
static tupid_t tup_sub_dir_dt = -1;
static int top_fd = -1;

int find_tup_dir(void)
{
	struct stat st;

	if(getcwd(tup_wd, sizeof(tup_wd)) == NULL) {
		perror("getcwd");
		exit(1);
	}

	tup_top_len = strlen(tup_wd);
	tup_sub_len = 0;
	for(;;) {
		if(stat(".tup", &st) == 0 && S_ISDIR(st.st_mode)) {
			tup_wd_offset = tup_top_len;
			while(is_path_sep(&tup_wd[tup_wd_offset])) {
				tup_wd_offset++;
				tup_sub_len--;
			}
			tup_wd[tup_top_len] = 0;
			break;
		}
		if(chdir("..") < 0) {
			perror("chdir");
			exit(1);
		}
		while(tup_top_len > 0) {
			tup_top_len--;
			tup_sub_len++;
			if(is_path_sep(&tup_wd[tup_top_len])) {
				break;
			}
		}
		if(!tup_top_len) {
			return -1;
		}
	}
	top_fd = open(".", O_RDONLY);
	if(top_fd < 0) {
		perror(".");
		exit(1);
	}
	return 0;
}

tupid_t get_sub_dir_dt(void)
{
	if(tup_sub_dir_dt < 0) {
		tup_sub_dir_dt = find_dir_tupid(get_sub_dir());
		if(tup_sub_dir_dt < 0) {
			fprintf(stderr, "Error: Unable to find tupid for working directory: '%s'\n", get_sub_dir());
		}
	}
	return tup_sub_dir_dt;
}

const char *get_tup_top(void)
{
	return tup_wd;
}

int get_tup_top_len(void)
{
	return tup_top_len;
}

const char *get_sub_dir(void)
{
	if(tup_wd[tup_wd_offset])
		return tup_wd + tup_wd_offset;
	return ".";
}

int get_sub_dir_len(void)
{
	return tup_sub_len;
}

int tup_top_fd(void)
{
	return top_fd;
}

int get_number_of_cpu() {
#if defined(__linux__) || defined(__sun__)
	return sysconf(_SC_NPROCESSORS_ONLN);
#elif defined(__APPLE__) || defined(__FreeBSD__)
	int nm[2];
	size_t len = 4;
	uint32_t count;

	nm[0] = CTL_HW; nm[1] = HW_AVAILCPU;
	sysctl(nm, 2, &count, &len, NULL, 0);

	if(count < 1) {
		nm[1] = HW_NCPU;
		sysctl(nm, 2, &count, &len, NULL, 0);
		if(count < 1) { count = 1; }
	}
	return count;
#elif defined(_WIN32)
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	return sysinfo.dwNumberOfProcessors;
#else
	return 1;
#endif
}
