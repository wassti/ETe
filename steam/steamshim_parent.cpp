#define GAME_LAUNCH_NAME "ete"
#ifndef GAME_LAUNCH_NAME
#error Please define your game exe name.
#endif

#if defined(_WIN32)
    #if defined( _M_IX86 )
        #define GAME_LAUNCH_SUFFIX ""
        #define STEAMAPI_DLLNAME "steam_api.dll"
    #endif
    #if defined( _M_AMD64 )
        #define GAME_LAUNCH_SUFFIX ".x64"
        #define STEAMAPI_DLLNAME "steam_api64.dll"
    #endif
#elif defined(__linux__)
    #if defined (__i386__)
        #define GAME_LAUNCH_SUFFIX ".x86"
        #define STEAMAPI_DLLNAME "libsteam_api.so"
    #endif
    #if defined (__x86_64__) || defined (__amd64__)
        #define GAME_LAUNCH_SUFFIX ".x86_64"
        #define STEAMAPI_DLLNAME "libsteam_api.x86_64.so"
    #endif
#elif defined(__APPLE__) || defined(__APPLE_CC__)
    #if defined (__x86_64__) || defined (__amd64__)
        #define GAME_LAUNCH_SUFFIX ".x86_64"
        #define STEAMAPI_DLLNAME "libsteam_api.dylib"
    #else
        #error Platform is not supported
    #endif
#else
    #error Platform is not supported
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
typedef PROCESS_INFORMATION ProcessType;
typedef HANDLE PipeType;
#define NULLPIPE NULL
typedef unsigned __int8 uint8;
typedef __int32 int32;
typedef unsigned __int64 uint64;

typedef int(__stdcall* SteamAPIInit_Type)();
typedef void(__stdcall* SteamAPIShutdown_Type)();
static SteamAPIInit_Type SteamAPI_Init;
static SteamAPIShutdown_Type SteamAPI_Shutdown;
static void* gp_steamLibrary = NULL;

#else
#include <stdint.h>
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>
#include <dlfcn.h>
typedef uint8_t uint8;
typedef int32_t int32;
typedef uint64_t uint64;
typedef pid_t ProcessType;
typedef int PipeType;
#define NULLPIPE -1

typedef int(*SteamAPIInit_Type)();
typedef void(*SteamAPIShutdown_Type)();
static SteamAPIInit_Type SteamAPI_Init;
static SteamAPIShutdown_Type SteamAPI_Shutdown;
static void* gp_steamLibrary = NULL;

#endif

//#include "steam/steam_api.h"

#define DEBUGPIPE 1
#if DEBUGPIPE
#define dbgpipe printf
#else
static inline void dbgpipe(const char *fmt, ...) {}
#endif

/* platform-specific mainline calls this. */
static int mainline(void);

/* Windows and Unix implementations of this stuff below. */
static void fail(const char *err);
static bool writePipe(PipeType fd, const void *buf, const unsigned int _len);
static int readPipe(PipeType fd, void *buf, const unsigned int _len);
static bool createPipes(PipeType *pPipeParentRead, PipeType *pPipeParentWrite,
                        PipeType *pPipeChildRead, PipeType *pPipeChildWrite);
static void closePipe(PipeType fd);
static bool setEnvVar(const char *key, const char *val);
static bool launchChild(ProcessType *pid);
static int closeProcess(ProcessType *pid);

#ifdef _WIN32
static void fail(const char *err)
{
    MessageBoxA(NULL, err, "ERROR", MB_ICONERROR | MB_OK);
    ExitProcess(1);
} // fail

static bool writePipe(PipeType fd, const void *buf, const unsigned int _len)
{
    const DWORD len = (DWORD) _len;
    DWORD bw = 0;
    return ((WriteFile(fd, buf, len, &bw, NULL) != 0) && (bw == len));
} // writePipe

static int readPipe(PipeType fd, void *buf, const unsigned int _len)
{
    const DWORD len = (DWORD) _len;
    DWORD br = 0;
    return ReadFile(fd, buf, len, &br, NULL) ? (int) br : -1;
} // readPipe

static bool createPipes(PipeType *pPipeParentRead, PipeType *pPipeParentWrite,
                        PipeType *pPipeChildRead, PipeType *pPipeChildWrite)
{
    SECURITY_ATTRIBUTES pipeAttr;

    pipeAttr.nLength = sizeof (pipeAttr);
    pipeAttr.lpSecurityDescriptor = NULL;
    pipeAttr.bInheritHandle = TRUE;
    if (!CreatePipe(pPipeParentRead, pPipeChildWrite, &pipeAttr, 0))
        return 0;

    pipeAttr.nLength = sizeof (pipeAttr);
    pipeAttr.lpSecurityDescriptor = NULL;
    pipeAttr.bInheritHandle = TRUE;
    if (!CreatePipe(pPipeChildRead, pPipeParentWrite, &pipeAttr, 0))
    {
        CloseHandle(*pPipeParentRead);
        CloseHandle(*pPipeChildWrite);
        return 0;
    } // if

    return 1;
} // createPipes

static void closePipe(PipeType fd)
{
    CloseHandle(fd);
} // closePipe

static bool setEnvVar(const char *key, const char *val)
{
    return (SetEnvironmentVariableA(key, val) != 0);
} // setEnvVar

static bool launchChild(ProcessType *pid)
{
    return (CreateProcessW(TEXT(".\\") TEXT(GAME_LAUNCH_NAME) TEXT(GAME_LAUNCH_SUFFIX) TEXT(".exe"),
                           GetCommandLineW(), NULL, NULL, TRUE, 0, NULL,
                           NULL, NULL, pid) != 0);
} // launchChild

static int closeProcess(ProcessType *pid)
{
    CloseHandle(pid->hProcess);
    CloseHandle(pid->hThread);
    return 0;
} // closeProcess

static void *loadSteamModule(void)
{
    // TODO handle x86 and x86_64 as needed
    return (void *)LoadLibrary(TEXT("./.steam/" STEAMAPI_DLLNAME));
}

static void unloadSteamModule(void *handle)
{
    if (handle != NULL)
        FreeLibrary((HMODULE)handle);
}

static void *loadSteamFunction(void *handle, const char *name)
{
	if (handle == NULL || name == NULL || *name == '\0') 
		return NULL;

    return (void *)GetProcAddress((HMODULE)handle, name);
}

int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                     LPSTR lpCmdLine, int nCmdShow)
{
    mainline();
    ExitProcess(0);
    return 0;  // just in case.
} // WinMain


#else  // everyone else that isn't Windows.

static void fail(const char *err)
{
    // !!! FIXME: zenity or something.
    fprintf(stderr, "%s\n", err);
    _exit(1);
} // fail

static bool writePipe(PipeType fd, const void *buf, const unsigned int _len)
{
    const ssize_t len = (ssize_t) _len;
    ssize_t bw;
    while (((bw = write(fd, buf, len)) == -1) && (errno == EINTR)) { /*spin*/ }
    return (bw == len);
} // writePipe

static int readPipe(PipeType fd, void *buf, const unsigned int _len)
{
    const ssize_t len = (ssize_t) _len;
    ssize_t br;
    while (((br = read(fd, buf, len)) == -1) && (errno == EINTR)) { /*spin*/ }
    return (int) br;
} // readPipe

static bool createPipes(PipeType *pPipeParentRead, PipeType *pPipeParentWrite,
                        PipeType *pPipeChildRead, PipeType *pPipeChildWrite)
{
    int fds[2];
    if (pipe(fds) == -1)
        return 0;
    *pPipeParentRead = fds[0];
    *pPipeChildWrite = fds[1];

    if (pipe(fds) == -1)
    {
        close(*pPipeParentRead);
        close(*pPipeChildWrite);
        return 0;
    } // if

    *pPipeChildRead = fds[0];
    *pPipeParentWrite = fds[1];

    return 1;
} // createPipes

static void closePipe(PipeType fd)
{
    close(fd);
} // closePipe

static bool setEnvVar(const char *key, const char *val)
{
    return (setenv(key, val, 1) != -1);
} // setEnvVar

static int GArgc = 0;
static char **GArgv = NULL;

static bool launchChild(ProcessType *pid)
{
    *pid = fork();
    if (*pid == -1)   // failed
        return false;
    else if (*pid != 0)  // we're the parent
        return true;  // we'll let the pipe fail if this didn't work.

    // we're the child.
    GArgv[0] = strdup("./" GAME_LAUNCH_NAME GAME_LAUNCH_SUFFIX);
    execvp(GArgv[0], GArgv);
    // still here? It failed! Terminate, closing child's ends of the pipes.
    _exit(1);
} // launchChild

static int closeProcess(ProcessType *pid)
{
    int rc = 0;
    while ((waitpid(*pid, &rc, 0) == -1) && (errno == EINTR)) { /*spin*/ }
    if (!WIFEXITED(rc))
        return 1;  // oh well.
    return WEXITSTATUS(rc);
} // closeProcess

static void *loadSteamModule(void)
{
    return dlopen( "./.steam/" STEAMAPI_DLLNAME, RTLD_NOW );
}

static void unloadSteamModule(void *handle)
{
    if (handle != NULL)
        dlclose(handle);
}

static void *loadSteamFunction(void *handle, const char *name)
{
	if (handle == NULL || name == NULL || *name == '\0') 
		return NULL;
    dlerror();
    return dlsym( handle, name );
}

int main(int argc, char **argv)
{
#ifndef _WIN32
    signal(SIGPIPE, SIG_IGN);
#endif
    GArgc = argc;
    GArgv = argv;
    return mainline();
} // main

#endif


// THE ACTUAL PROGRAM.

typedef enum ShimCmd
{
    SHIMCMD_BYE,
} ShimCmd;

typedef enum ShimEvent
{
    SHIMEVENT_BYE,
} ShimEvent;

static bool write1ByteCmd(PipeType fd, const uint8 b1)
{
    const uint8 buf[] = { 1, b1 };
    return writePipe(fd, buf, sizeof (buf));
} // write1ByteCmd

static inline bool writeBye(PipeType fd)
{
    dbgpipe("Parent sending SHIMEVENT_BYE().\n");
    return write1ByteCmd(fd, SHIMEVENT_BYE);
} // writeBye


static bool processCommand(const uint8 *buf, unsigned int buflen, PipeType fd)
{
    if (buflen == 0)
        return true;

    const ShimCmd cmd = (ShimCmd) *(buf++);
    buflen--;

    #if DEBUGPIPE
    if (false) {}
    #define PRINTGOTCMD(x) else if (cmd == x) printf("Parent got " #x ".\n")
    PRINTGOTCMD(SHIMCMD_BYE);
    #undef PRINTGOTCMD
    else printf("Parent got unknown shimcmd %d.\n", (int) cmd);
    #endif

    switch (cmd)
    {
        case SHIMCMD_BYE:
            writeBye(fd);
            return false;
    } // switch

    return true;  // keep going.
} // processCommand

static void processCommands(PipeType pipeParentRead, PipeType pipeParentWrite)
{
    bool quit = false;
    uint8 buf[256];
    int br;

    // this read blocks.
    while (!quit && ((br = readPipe(pipeParentRead, buf, sizeof (buf))) > 0))
    {
        while (br > 0)
        {
            const int cmdlen = (int) buf[0];
            if ((br-1) >= cmdlen)
            {
                if (!processCommand(buf+1, cmdlen, pipeParentWrite))
                {
                    quit = true;
                    break;
                } // if

                br -= cmdlen + 1;
                if (br > 0)
                    memmove(buf, buf+cmdlen+1, br);
            } // if
            else  // get more data.
            {
                const int morebr = readPipe(pipeParentRead, buf+br, sizeof (buf) - br);
                if (morebr <= 0)
                {
                    quit = true;  // uhoh.
                    break;
                } // if
                br += morebr;
            } // else
        } // while
    } // while
} // processCommands

static bool setEnvironmentVars(PipeType pipeChildRead, PipeType pipeChildWrite)
{
    char buf[64];
    snprintf(buf, sizeof (buf), "%llu", (unsigned long long) pipeChildRead);
    if (!setEnvVar("STEAMSHIM_READHANDLE", buf))
        return false;

    snprintf(buf, sizeof (buf), "%llu", (unsigned long long) pipeChildWrite);
    if (!setEnvVar("STEAMSHIM_WRITEHANDLE", buf))
        return false;

    return true;
} // setEnvironmentVars

static bool initSteamworks(PipeType fd)
{
    gp_steamLibrary = loadSteamModule();

    if (!gp_steamLibrary)
    {
        return 0;
    }

    SteamAPI_Init = (SteamAPIInit_Type)loadSteamFunction(gp_steamLibrary, "SteamAPI_Init");
    SteamAPI_Shutdown = (SteamAPIShutdown_Type)loadSteamFunction(gp_steamLibrary, "SteamAPI_Shutdown");

    if (!SteamAPI_Shutdown || !SteamAPI_Init)
	{
        return 0;
    }

    // this can fail for many reasons:
    //  - you forgot a steam_appid.txt in the current working directory.
    //  - you don't have Steam running
    //  - you don't own the game listed in steam_appid.txt
    if (!SteamAPI_Init())
        return 0;

    return 1;
} // initSteamworks

static void deinitSteamworks(void)
{
    if (!gp_steamLibrary)
        return;

    SteamAPI_Shutdown();
    unloadSteamModule(gp_steamLibrary);
    gp_steamLibrary = NULL;
} // deinitSteamworks

static int mainline(void)
{
    PipeType pipeParentRead = NULLPIPE;
    PipeType pipeParentWrite = NULLPIPE;
    PipeType pipeChildRead = NULLPIPE;
    PipeType pipeChildWrite = NULLPIPE;
    ProcessType childPid;

    dbgpipe("Parent starting mainline.\n");

    if (!createPipes(&pipeParentRead, &pipeParentWrite, &pipeChildRead, &pipeChildWrite))
        fail("Failed to create application pipes");
    else if (!initSteamworks(pipeParentWrite))
        fail("Failed to initialize Steamworks");
    else if (!setEnvironmentVars(pipeChildRead, pipeChildWrite))
        fail("Failed to set environment variables");
    else if (!launchChild(&childPid))
        fail("Failed to launch application");

    // Close the ends of the pipes that the child will use; we don't need them.
    closePipe(pipeChildRead);
    closePipe(pipeChildWrite);
    pipeChildRead = pipeChildWrite = NULLPIPE;

    dbgpipe("Parent in command processing loop.\n");

    // Now, we block for instructions until the pipe fails (child closed it or
    //  terminated/crashed).
    processCommands(pipeParentRead, pipeParentWrite);

    dbgpipe("Parent shutting down.\n");

    // Close our ends of the pipes.
    writeBye(pipeParentWrite);
    closePipe(pipeParentRead);
    closePipe(pipeParentWrite);

    deinitSteamworks();

    dbgpipe("Parent waiting on child process.\n");

    // Wait for the child to terminate, close the child process handles.
    const int retval = closeProcess(&childPid);

    dbgpipe("Parent exiting mainline (child exit code %d).\n", retval);

    return retval;
} // mainline

// end of steamshim_parent.cpp ...

