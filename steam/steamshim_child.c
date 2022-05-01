
#ifdef _WIN32
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
typedef HANDLE PipeType;
#define NULLPIPE NULL
typedef unsigned __int8 uint8;
typedef __int32 int32;
typedef unsigned __int64 uint64;
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>
#include <signal.h>
typedef uint8_t uint8;
typedef int32_t int32;
typedef uint64_t uint64;
typedef int PipeType;
#define NULLPIPE -1
#endif

#include "steamshim_child.h"

#define DEBUGPIPE 1
#if DEBUGPIPE
#define dbgpipe printf
#else
static inline void dbgpipe(const char *fmt, ...) {}
#endif

static int writePipe(PipeType fd, const void *buf, const unsigned int _len);
static void closePipe(PipeType fd);
static char *getEnvVar(const char *key, char *buf, const size_t buflen);


#ifdef _WIN32

static int writePipe(PipeType fd, const void *buf, const unsigned int _len)
{
    const DWORD len = (DWORD) _len;
    DWORD bw = 0;
    return ((WriteFile(fd, buf, len, &bw, NULL) != 0) && (bw == len));
} /* writePipe */

static void closePipe(PipeType fd)
{
    CloseHandle(fd);
} /* closePipe */

static char *getEnvVar(const char *key, char *buf, const size_t _buflen)
{
    const DWORD buflen = (DWORD) _buflen;
    const DWORD rc = GetEnvironmentVariableA(key, buf, buflen);
    /* rc doesn't count null char, hence "<". */
    return ((rc > 0) && (rc < buflen)) ? NULL : buf;
} /* getEnvVar */

#else

static int writePipe(PipeType fd, const void *buf, const unsigned int _len)
{
    const ssize_t len = (ssize_t) _len;
    ssize_t bw;
    while (((bw = write(fd, buf, len)) == -1) && (errno == EINTR)) { /*spin*/ }
    return (bw == len);
} /* writePipe */

static void closePipe(PipeType fd)
{
    close(fd);
} /* closePipe */

static char *getEnvVar(const char *key, char *buf, const size_t buflen)
{
    const char *envr = getenv(key);
    if (!envr || (strlen(envr) >= buflen))
        return NULL;
    strcpy(buf, envr);
    return buf;
} /* getEnvVar */

#endif


static PipeType GPipeRead = NULLPIPE;
static PipeType GPipeWrite = NULLPIPE;

typedef enum ShimCmd
{
    SHIMCMD_BYE,
} ShimCmd;

static int write1ByteCmd(const uint8 b1)
{
    const uint8 buf[] = { 1, b1 };
    return writePipe(GPipeWrite, buf, sizeof (buf));
} /* write1ByteCmd */

static inline int writeBye(void)
{
    dbgpipe("Child sending SHIMCMD_BYE().\n");
    return write1ByteCmd(SHIMCMD_BYE);
} // writeBye

static int initPipes(void)
{
    char buf[64];
    unsigned long long val;

    if (!getEnvVar("STEAMSHIM_READHANDLE", buf, sizeof (buf)))
        return 0;
    else if (sscanf(buf, "%llu", &val) != 1)
        return 0;
    else
        GPipeRead = (PipeType) val;

    if (!getEnvVar("STEAMSHIM_WRITEHANDLE", buf, sizeof (buf)))
        return 0;
    else if (sscanf(buf, "%llu", &val) != 1)
        return 0;
    else
        GPipeWrite = (PipeType) val;
    
    return ((GPipeRead != NULLPIPE) && (GPipeWrite != NULLPIPE));
} /* initPipes */


int STEAMSHIM_init(void)
{
    dbgpipe("Child init start.\n");
    if (!initPipes())
    {
        dbgpipe("Child init failed.\n");
        return 0;
    } /* if */

#ifndef _WIN32
    signal(SIGPIPE, SIG_IGN);
#endif

    dbgpipe("Child init success!\n");
    return 1;
} /* STEAMSHIM_init */

void STEAMSHIM_deinit(void)
{
    dbgpipe("Child deinit.\n");
    if (GPipeWrite != NULLPIPE)
    {
        writeBye();
        closePipe(GPipeWrite);
    } /* if */

    if (GPipeRead != NULLPIPE)
        closePipe(GPipeRead);

    GPipeRead = GPipeWrite = NULLPIPE;

#ifndef _WIN32
    signal(SIGPIPE, SIG_DFL);
#endif
} /* STEAMSHIM_deinit */

static inline int isAlive(void)
{
    return ((GPipeRead != NULLPIPE) && (GPipeWrite != NULLPIPE));
} /* isAlive */

int STEAMSHIM_alive(void)
{
    return isAlive();
} /* STEAMSHIM_alive */

/* end of steamshim_child.c ... */
