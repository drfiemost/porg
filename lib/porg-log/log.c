/***********************************************************************
 * log.c: Handles the system calls that create files and logs them.
 ***********************************************************************
 * This file is part of the package porg
 * Copyright (C) 2004-2014 David Ricart
 * For more information visit http://porg.sourceforge.net
 ***********************************************************************/

#include "config.h"
#include <dirent.h>
#include <dlfcn.h>
#include <fcntl.h>			  
#include <stdarg.h>
#include <unistd.h>

#define PORG_HAVE_64  (HAVE_OPEN64 && HAVE_CREAT64 && HAVE_TRUNCATE64 \
                      && HAVE_FOPEN64 && HAVE_FREOPEN64)

#define PORG_CHECK_INIT  do { if (!porg_tmpfile) porg_init(); } while (0)

#define PORG_BUFSIZE  4096

static int	(*libc_creat)		(const char*, mode_t);
static int	(*libc_link)		(const char*, const char*);
static int	(*libc_open)		(const char*, int, ...);
static int	(*libc_rename)		(const char*, const char*);
static int	(*libc_symlink)		(const char*, const char*);
static int	(*libc_truncate)	(const char*, off_t);
static FILE*(*libc_fopen)		(const char*, const char*);
static FILE*(*libc_freopen)		(const char*, const char*, FILE*);
#if PORG_HAVE_64
static int	(*libc_creat64)		(const char*, mode_t);
static int	(*libc_open64)		(const char*, int, ...);
static int	(*libc_truncate64)	(const char*, off64_t);
static FILE*(*libc_fopen64)		(const char*, const char*);
static FILE*(*libc_freopen64)	(const char*, const char*, FILE*);
#endif  /* PORG_HAVE_64 */

static char*	porg_tmpfile;
static int		porg_debug;


static void porg_die(const char* fmt, ...)
{
	va_list ap;
	
	fflush(stdout);
	fputs("libporg-log: ", stderr);
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	putc('\n', stderr);
	
	exit(EXIT_FAILURE);
}


static void* porg_dlsym(const char* symbol)
{
	void* ret;
	char* error;

	dlerror();

	if (!(ret = dlsym(RTLD_NEXT, symbol))) {
		error = (char*)dlerror();
		porg_die("dlsym(%p, \"%s\"): %s", RTLD_NEXT, symbol,
			error ? error : "failed");
	}

	return ret;
}

		
static void porg_init()
{
	static char* dbg = NULL;

	/* handle libc */
	
	libc_creat = porg_dlsym("creat");
	libc_link = porg_dlsym("link");
	libc_open = porg_dlsym("open");
	libc_rename = porg_dlsym("rename");
	libc_symlink = porg_dlsym("symlink");
	libc_truncate = porg_dlsym("truncate");
	libc_fopen = porg_dlsym("fopen");
	libc_freopen = porg_dlsym("freopen");
#if PORG_HAVE_64
	libc_open64 = porg_dlsym("open64");
	libc_creat64 = porg_dlsym("creat64");
	libc_truncate64 = porg_dlsym("truncate64");
	libc_fopen64 = porg_dlsym("fopen64");
	libc_freopen64 = porg_dlsym("freopen64");
#endif  /* PORG_HAVE_64 */

	/* read the environment */
	
	if (!porg_tmpfile && !(porg_tmpfile = getenv("PORG_TMPFILE")))
		porg_die("variable %s undefined", "PORG_TMPFILE"); \
		
	if (!dbg && (dbg = getenv("PORG_DEBUG")))
		porg_debug = !strcmp(dbg, "yes");
}


static void porg_log(const char* path, const char* fmt, ...)
{
	static char abs_path[PORG_BUFSIZE];
	va_list a;
	int fd, len, old_errno = errno;
	
	if (!strcmp(path, "/dev/tty") || !strcmp(path, "/dev/null") ||
		!strncmp(path, "/proc/", 6))
		return;

	PORG_CHECK_INIT;

	if (porg_debug) {
		fflush(stdout);
		fprintf(stderr, "porg :: ");
		va_start(a, fmt);
		vfprintf(stderr, fmt, a);
		va_end(a);
		putc('\n', stderr);
	}
	
	/* "Absolutize" relative paths */
	if (path[0] == '/') {
		strncpy(abs_path, path, PORG_BUFSIZE - 1);
		abs_path[PORG_BUFSIZE - 1] = '\0';
	}
	else if (getcwd(abs_path, PORG_BUFSIZE)) {
		strncat(abs_path, "/", PORG_BUFSIZE - strlen(abs_path) - 1);
		strncat(abs_path, path, PORG_BUFSIZE - strlen(abs_path) - 1);
	}
	else
		snprintf(abs_path, PORG_BUFSIZE, "./%s", path);

	strncat(abs_path, "\n", PORG_BUFSIZE - strlen(abs_path) - 1);

	if ((fd = libc_open(porg_tmpfile, O_WRONLY | O_CREAT | O_APPEND, 0644)) < 0)
		porg_die("open(\"%s\"): %s", porg_tmpfile, strerror(errno));
	
	len = strlen(abs_path);
	
	if (write(fd, abs_path, len) != len)
		porg_die("%s: write(): %s", porg_tmpfile, strerror(errno));
		
	if (close(fd) < 0)
		porg_die("close(%d): %s", fd, strerror(errno));
	
	errno = old_errno;
}


/************************/
/* System call handlers */
/************************/

FILE* fopen(const char* path, const char* mode)
{
	FILE* ret;
	
	PORG_CHECK_INIT;
	
	ret = libc_fopen(path, mode);
	if (ret && strpbrk(mode, "wa+"))
		porg_log(path, "fopen(\"%s\", \"%s\")", path, mode);
	
	return ret;
}


FILE* freopen(const char* path, const char* mode, FILE* stream)
{
	FILE* ret;
	
	PORG_CHECK_INIT;
	
	ret = libc_freopen(path, mode, stream);
	if (ret && strpbrk(mode, "wa+"))
		porg_log(path, "freopen(\"%s\", \"%s\")", path, mode);
	
	return ret;
}


/*
 * If newbuf isn't a directory write it to the log, otherwise log files it and
 * its subdirectories contain.
 */
static void log_rename(const char* oldpath, const char* newpath)
{
	char oldbuf[PORG_BUFSIZE], newbuf[PORG_BUFSIZE];
	struct stat st;
	DIR* dir;
	struct dirent* e;
	size_t oldlen, newlen;
	int old_errno = errno;	/* save global errno */

	/* The newpath file doesn't exist.  */
	if (-1 == lstat(newpath, &st)) 
		goto goto_end;

	else if (!S_ISDIR(st.st_mode)) {
		/* newpath is a file or a symlink.  */
		porg_log(newpath, "rename(\"%s\", \"%s\")", oldpath, newpath);
		goto goto_end;
	}

	/* Make sure we have enough space for the following slashes.  */
	oldlen = strlen(oldpath);
	newlen = strlen(newpath);
	if (oldlen + 2 >= PORG_BUFSIZE || newlen + 2 >= PORG_BUFSIZE)
		goto goto_end;

	strcpy(oldbuf, oldpath);
	strcpy(newbuf, newpath);
	newbuf[PORG_BUFSIZE - 1] = oldbuf[PORG_BUFSIZE - 1] = '\0';

	/* We can do this in the loop below, buf it's more efficient to do
	   that once. These slashes will separate the path NEWBUF/OLDBUF
	   contains from names of its files/subdirectories.  */
	oldbuf[oldlen++] = newbuf[newlen++] = '/';
	oldbuf[oldlen] = newbuf[newlen] = '\0';

	dir = opendir(newbuf);

	while ((e = readdir(dir))) {
		if (!strcmp(e->d_name, ".") || !strcmp(e->d_name, ".."))
			continue;
		strncat(oldbuf, e->d_name, PORG_BUFSIZE - oldlen - 1);
		strncat(newbuf, e->d_name, PORG_BUFSIZE - newlen - 1);
		log_rename(oldbuf, newbuf);
		oldbuf[oldlen] = newbuf[newlen] = '\0';
	}

	closedir(dir);

goto_end:
	/* Restore global errno */
	errno = old_errno;
}


int rename(const char* oldpath, const char* newpath)
{
	int ret;
	
	PORG_CHECK_INIT;
	
	if ((ret = libc_rename(oldpath, newpath)) != -1)
		log_rename(oldpath, newpath);

	return ret;
}


int creat(const char* path, mode_t mode)
{
	int ret;
	
	PORG_CHECK_INIT;
	
	ret = libc_open(path, O_CREAT | O_WRONLY | O_TRUNC, mode);
	
	if (ret != -1)
		porg_log(path, "creat(\"%s\", 0%o)", path, (int)mode);
	
	return ret;
}


int link(const char* oldpath, const char* newpath)
{
	int ret;
	
	PORG_CHECK_INIT;
	
	if ((ret = libc_link(oldpath, newpath)) != -1)
		porg_log(newpath, "link(\"%s\", \"%s\")", oldpath, newpath);
	
	return ret;
}


int truncate(const char* path, off_t length)
{
	int ret;
	
	PORG_CHECK_INIT;
	
	if ((ret = libc_truncate(path, length)) != -1)
		porg_log(path, "truncate(\"%s\", %d)", path, (int)length);
	
	return ret;
}


int open(const char* path, int flags, ...)
{
	va_list a;
	mode_t mode;
	int accmode, ret;
	
	PORG_CHECK_INIT;
	
	va_start(a, flags);
	mode = va_arg(a, mode_t);
	va_end(a);
	
	if ((ret = libc_open(path, flags, mode)) != -1) {
		accmode = flags & O_ACCMODE;
		if (accmode == O_WRONLY || accmode == O_RDWR)
			porg_log(path, "open(\"%s\")", path);
	}

	return ret;
}


int symlink(const char* oldpath, const char* newpath)
{
	int ret;
	
	PORG_CHECK_INIT;
	
	if ((ret = libc_symlink(oldpath, newpath)) != -1)
		porg_log(newpath, "symlink(\"%s\", \"%s\")", oldpath, newpath);
	
	return ret;
}


#if PORG_HAVE_64

int creat64(const char* path, mode_t mode)
{
	int ret;
	
	PORG_CHECK_INIT;
	
	if ((ret = libc_open64(path, O_CREAT | O_WRONLY | O_TRUNC, mode)) != -1)
		porg_log(path, "creat64(\"%s\")", path);
	
	return ret;
}


int open64(const char* path, int flags, ...)
{
	va_list a;
	mode_t mode;
	int accmode, ret;
	
	PORG_CHECK_INIT;
	
	va_start(a, flags);
	mode = va_arg(a, mode_t);
	va_end(a);
	
	if ((ret = libc_open64(path, flags, mode)) != -1) {
		accmode = flags & O_ACCMODE;
		if (accmode == O_WRONLY || accmode == O_RDWR)
			porg_log(path, "open64(\"%s\")", path);
	}

	return ret;
}


int truncate64(const char* path, off64_t length)
{
	int ret;
	
	PORG_CHECK_INIT;
	
	if ((ret = libc_truncate64(path, length)) != -1)
		porg_log(path, "truncate64(\"%s\", %d)", path, (int)length);
	
	return ret;
}


FILE* fopen64(const char* path, const char* mode)
{
	FILE* ret;
	
	PORG_CHECK_INIT;
	
	ret = libc_fopen64(path, mode);
	if (ret && strpbrk(mode, "wa+"))
		porg_log(path, "fopen64(\"%s\", \"%s\")", path, mode);
	
	return ret;
}


FILE* freopen64(const char* path, const char* mode, FILE* stream)
{
	FILE* ret;
	
	PORG_CHECK_INIT;
	
	ret = libc_freopen64(path, mode, stream);
	if (ret && strpbrk(mode, "wa+"))
		porg_log(path, "freopen64(\"%s\", \"%s\")", path, mode);
	
	return ret;
}

#endif  /* PORG_HAVE_64 */

