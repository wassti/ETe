/*
===========================================================================

Wolfenstein: Enemy Territory GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company. 

This file is part of the Wolfenstein: Enemy Territory GPL Source Code (Wolf ET Source Code).  

Wolf ET Source Code is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Wolf ET Source Code is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Wolf ET Source Code.  If not, see <http://www.gnu.org/licenses/>.

In addition, the Wolf: ET Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the Wolf ET Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/
// win_main.c

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"
#ifndef DEDICATED
#include "../client/client.h"
#endif
#include "win_local.h"
#include "resource.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <direct.h>
#include <io.h>

#if !defined(DEDICATED) && defined(USE_STEAMAPI)
#include "../../steam/steamshim_child.h"
#endif

#define MEM_THRESHOLD (96*1024*1024)

WinVars_t	g_wv;

/*
==================
Sys_LowPhysicalMemory
==================
*/
qboolean Sys_LowPhysicalMemory( void ) {
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof( statex );

	GlobalMemoryStatusEx( &statex );
	return (statex.ullTotalPhys <= MEM_THRESHOLD) ? qtrue : qfalse;
}

/*
==================
Sys_StartProcess

NERVE - SMF
==================
*/
void Sys_StartProcess( const char *exeName, qboolean doexit ) {
	TCHAR szPathOrig[_MAX_PATH], temp[_MAX_PATH];
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory( &si, sizeof( si ) );
	si.cb = sizeof( si );

	GetCurrentDirectory( _MAX_PATH, szPathOrig );

	// Note this only works if tchar is to remain `char`
	Com_sprintf( temp, sizeof( temp ), "%s\\%s", szPathOrig, exeName );

	// JPW NERVE swiped from Sherman's SP code
	if ( !CreateProcess( NULL, temp, NULL, NULL,FALSE, 0, NULL, NULL, &si, &pi ) ) {
		// couldn't start it, popup error box
		Com_Error( ERR_DROP, "Could not start process: '%s\\%s' ", szPathOrig, exeName  );
		return;
	}
	// jpw

	// TTimo: similar way of exiting as used in Sys_OpenURL below
	if ( doexit ) {
		Cbuf_ExecuteText( EXEC_APPEND, "quit\n" );
	}
}

/*
==================
Sys_OpenURL

NERVE - SMF
==================
*/
void Sys_OpenURL( const char *url, qboolean doexit ) {
	HWND wnd;

	static qboolean doexit_spamguard = qfalse;

	if ( doexit_spamguard ) {
		Com_DPrintf( "Sys_OpenURL: already in a doexit sequence, ignoring %s\n", url );
		return;
	}

	Com_Printf( "Open URL: %s\n", url );

	if ( !ShellExecute( NULL, "open", url, NULL, NULL, SW_RESTORE ) ) {
		// couldn't start it, popup error box
		Com_Error( ERR_DROP, "Could not open url: '%s' ", url );
		return;
	}

	wnd = GetForegroundWindow();

	if ( wnd ) {
		ShowWindow( wnd, SW_MAXIMIZE );
	}

	if ( doexit ) {
		// show_bug.cgi?id=612
		doexit_spamguard = qtrue;
		Cbuf_ExecuteText( EXEC_APPEND, "quit\n" );
	}
}

/*
==================
Sys_BeginProfiling
==================
*/
void Sys_BeginProfiling( void ) {
	// this is just used on the mac build
}


/*
=============
Sys_Error

Show the early console as an error dialog
=============
*/
void NORETURN QDECL Sys_Error( const char *error, ... ) {
	va_list	argptr;
	char	text[4096];
	MSG		msg;

	va_start( argptr, error );
	Q_vsnprintf( text, sizeof( text ), error, argptr );
	va_end( argptr );

#ifndef DEDICATED
	CL_Shutdown( text, qtrue );
#endif

	Conbuf_AppendText( text );
	Conbuf_AppendText( "\n" );

	Sys_SetErrorText( text );
	Sys_ShowConsole( 1, qtrue );

	timeEndPeriod( 1 );

	// wait for the user to quit
	while ( 1 ) {
		if ( GetMessage( &msg, NULL, 0, 0 ) <= 0 ) {
			Cmd_Clear();
			Com_Quit_f();
		}
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}

	Sys_DestroyConsole();

	exit( 1 );
}


/*
==============
Sys_Quit
==============
*/
void NORETURN Sys_Quit( void ) {

	timeEndPeriod( 1 );

	Sys_DestroyConsole();
	exit( 0 );
}


/*
==============
Sys_Print
==============
*/
void Sys_Print( const char *msg )
{
	Conbuf_AppendText( msg );
}


/*
==============
Sys_Mkdir
==============
*/
void Sys_Mkdir( const char *path )
{
	_mkdir( path );
}


/*
==============
Sys_FOpen
==============
*/
FILE *Sys_FOpen( const char *ospath, const char *mode )
{
	size_t length;

	// Windows API ignores all trailing spaces and periods which can get around Quake 3 file system restrictions.
	length = strlen( ospath );
	if ( length == 0 || ospath[length-1] == ' ' || ospath[length-1] == '.' ) {
		return NULL;
	}

	return fopen( ospath, mode );
}


/*
==============
Sys_ResetReadOnlyAttribute
==============
*/
qboolean Sys_ResetReadOnlyAttribute( const char *ospath ) {
	DWORD dwAttr;

	dwAttr = GetFileAttributesA( ospath );
	if ( dwAttr & FILE_ATTRIBUTE_READONLY ) {
		dwAttr &= ~FILE_ATTRIBUTE_READONLY;
		if ( SetFileAttributesA( ospath, dwAttr ) ) {
			return qtrue;
		} else {
			return qfalse;
		}
	} else {
		return qfalse;
	}
}


/*
==============
Sys_IsHiddenFolder
Note: Assumes caller has already verified ospath in other means
==============
*/
qboolean Sys_IsHiddenFolder( const char *ospath ) {
	DWORD dwAttr;

	dwAttr = GetFileAttributesA( ospath );
	if ( dwAttr & (FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_SYSTEM) ) {
		return qtrue;
	}

	if ( ospath[0] == '.' ) {
		return qtrue;
	}

	return qfalse;
}


/*
==============
Sys_Pwd
==============
*/
const char *Sys_Pwd( void )
{
	static char pwd[ MAX_OSPATH ];
	TCHAR	buffer[ MAX_OSPATH ];
	char *s;

	if ( pwd[0] )
		return pwd;

	GetModuleFileName( NULL, buffer, ARRAY_LEN( buffer ) );
	buffer[ ARRAY_LEN( buffer ) - 1 ] = '\0';

	Q_strncpyz( pwd, WtoA( buffer ), sizeof( pwd ) );

	s = strrchr( pwd, PATH_SEP );
	if ( s ) 
		*s = '\0';
	else // bogus case?
	{
		_getcwd( pwd, sizeof( pwd ) - 1 );
		pwd[ sizeof( pwd ) - 1 ] = '\0';
	}

	return pwd;
}


/*
==============
Sys_DefaultBasePath
==============
*/
const char *Sys_DefaultBasePath( void )
{
	return Sys_Pwd();
}


/*
==============================================================

DIRECTORY SCANNING

==============================================================
*/

void Sys_ListFilteredFiles( const char *basedir, const char *subdirs, const char *filter, char **list, int *numfiles ) {
	char		search[MAX_OSPATH*2+1];
	char		newsubdirs[MAX_OSPATH*2];
	char		filename[MAX_OSPATH*2];
	intptr_t	findhandle;
	struct _finddata_t findinfo;

	if ( *numfiles >= MAX_FOUND_FILES - 1 ) {
		return;
	}

	if ( *subdirs ) {
		Com_sprintf( search, sizeof(search), "%s\\%s\\*", basedir, subdirs );
	}
	else {
		Com_sprintf( search, sizeof(search), "%s\\*", basedir );
	}

	findhandle = _findfirst (search, &findinfo);
	if (findhandle == -1) {
		return;
	}

	do {
		if (findinfo.attrib & _A_SUBDIR) {
			if ( !Q_streq( findinfo.name, "." ) && !Q_streq( findinfo.name, ".." ) ) {
				if ( *subdirs ) {
					Com_sprintf( newsubdirs, sizeof(newsubdirs), "%s\\%s", subdirs, findinfo.name );
				} else {
					Com_sprintf( newsubdirs, sizeof(newsubdirs), "%s", findinfo.name );
				}
				Sys_ListFilteredFiles( basedir, newsubdirs, filter, list, numfiles );
			}
		}
		if ( *numfiles >= MAX_FOUND_FILES - 1 ) {
			break;
		}
		Com_sprintf( filename, sizeof(filename), "%s\\%s", subdirs, findinfo.name );
		if ( !Com_FilterPath( filter, filename ) )
			continue;
		list[ *numfiles ] = FS_CopyString( filename );
		(*numfiles)++;
	} while ( _findnext (findhandle, &findinfo) != -1 );

	_findclose (findhandle);
}


/*
=============
Sys_Sleep
=============
*/
void Sys_Sleep( int msec ) {
	
	if ( msec < 0 ) {
		// special case: wait for event or network packet
		DWORD dwResult;
		msec = 300;
		do {
			dwResult = MsgWaitForMultipleObjects( 0, NULL, FALSE, msec, QS_ALLEVENTS );
		} while ( dwResult == WAIT_TIMEOUT && NET_Sleep( 10 * 1000 ) );
		//WaitMessage();
		return;
	}

	// busy wait there because Sleep(0) will relinquish CPU - which is not what we want
	//if ( msec == 0 )
	//	return;

	Sleep ( msec );
}


/*
=============
Sys_ListFiles
=============
*/
char **Sys_ListFiles( const char *directory, const char *extension, const char *filter, int *numfiles, qboolean wantsubs ) {
	char		search[MAX_OSPATH*2+MAX_QPATH+1];
	int			nfiles;
	char		**listCopy;
	char		*list[MAX_FOUND_FILES];
	struct _finddata_t findinfo;
	intptr_t	findhandle;
	int			flag;
	int			extLen;
	int			length;
	int			i;
	const char	*x;
	qboolean	hasPatterns;

	if ( filter ) {

		nfiles = 0;
		Sys_ListFilteredFiles( directory, "", filter, list, &nfiles );

		list[ nfiles ] = NULL;
		*numfiles = nfiles;

		if (!nfiles)
			return NULL;

		listCopy = Z_Malloc( ( nfiles + 1 ) * sizeof( listCopy[0] ) );
		for ( i = 0 ; i < nfiles ; i++ ) {
			listCopy[i] = list[i];
		}
		listCopy[i] = NULL;

		return listCopy;
	}

	if ( !extension ) {
		extension = "";
	}

	// passing a slash as extension will find directories
	if ( extension[0] == '/' && extension[1] == 0 ) {
		extension = "";
		flag = 0;
	} else {
		flag = _A_SUBDIR;
	}

	Com_sprintf( search, sizeof(search), "%s\\*%s", directory, extension );

	findhandle = _findfirst( search, &findinfo );
	if ( findhandle == -1 ) {
		*numfiles = 0;
		return NULL;
	}

	extLen = (int)strlen( extension );
	hasPatterns = Com_HasPatterns( extension );
	if ( hasPatterns && extension[0] == '.' && extension[1] != '\0' ) {
		extension++;
	}

	// search
	nfiles = 0;

	do {
		if ( (!wantsubs && flag ^ ( findinfo.attrib & _A_SUBDIR )) || (wantsubs && findinfo.attrib & _A_SUBDIR) ) {
			if ( nfiles == MAX_FOUND_FILES - 1 ) {
				break;
			}
			if ( *extension ) {
				if ( hasPatterns ) {
					x = strrchr( findinfo.name, '.' );
					if ( !x || !Com_FilterExt( extension, x+1 ) ) {
						continue;
					}
				} else {
					length = strlen( findinfo.name );
					if ( length < extLen || Q_stricmp( findinfo.name + length - extLen, extension ) ) {
						continue;
					}
				}
			}
			list[ nfiles ] = FS_CopyString( findinfo.name );
			nfiles++;
		}
	} while ( _findnext (findhandle, &findinfo) != -1 );

	list[ nfiles ] = NULL;

	_findclose (findhandle);

	// return a copy of the list
	*numfiles = nfiles;

	if ( !nfiles ) {
		return NULL;
	}

	listCopy = Z_Malloc( ( nfiles + 1 ) * sizeof( listCopy[0] ) );
	for ( i = 0 ; i < nfiles ; i++ ) {
		listCopy[i] = list[i];
	}
	listCopy[i] = NULL;

	Com_SortFileList( listCopy, nfiles, extension[0] != '\0' );

	return listCopy;
}


/*
=============
Sys_FreeFileList
=============
*/
void Sys_FreeFileList( char **list ) {
	int		i;

	if ( !list ) {
		return;
	}

	for ( i = 0 ; list[i] ; i++ ) {
		Z_Free( list[i] );
	}

	Z_Free( list );
}


/*
=============
Sys_GetFileStats
=============
*/
qboolean Sys_GetFileStats( const char *filename, fileOffset_t *size, fileTime_t *mtime, fileTime_t *ctime ) {
	struct _stat s;

	if ( _stat( filename, &s ) == 0 ) {
		*size = (fileOffset_t)s.st_size;
		*mtime = (fileTime_t)s.st_mtime;
		*ctime = (fileTime_t)s.st_ctime;
		return qtrue;
	} else {
		*size = 0;
		*mtime = *ctime = 0;
		return qfalse;
	}
}


/*
==============
Sys_PathIsDir
Test an file given OS path:
returns -1 if not found
returns 1 if directory
returns 0 otherwise
==============
*/
int Sys_PathIsDir( const char *path ) {
	struct _stat s;

	if ( _stat( path, &s ) == -1 ) {
		return -1;
	}
	if ( s.st_mode & _S_IFDIR ) {
		return 1;
	}
	return 0;
}


//========================================================

/*
========================================================================

LOAD/UNLOAD DLL

========================================================================
*/

static int dll_err_count = 0;

/*
=================
Sys_LoadLibrary
=================
*/
void *Sys_LoadLibrary( const char *name )
{
	const char *ext;

	if ( !name || !*name )
		return NULL;

	if ( FS_AllowedExtension( name, qfalse, &ext ) )
	{
		Com_Error( ERR_FATAL, "Sys_LoadLibrary: Unable to load library with '%s' extension", ext );
	}

	return (void *)LoadLibrary( AtoW( name ) );
}


/*
=================
Sys_LoadFunction
=================
*/
void *Sys_LoadFunction( void *handle, const char *name )
{
	void *symbol;

	if ( handle == NULL || name == NULL || *name == '\0' ) 
	{
		dll_err_count++;
		return NULL;
	}

	symbol = GetProcAddress( handle, name );
	if ( !symbol )
		dll_err_count++;

	return symbol;
}


/*
=================
Sys_LoadFunctionErrors
=================
*/
int Sys_LoadFunctionErrors( void )
{
	int result = dll_err_count;
	dll_err_count = 0;
	return result;
}


/*
=================
Sys_UnloadLibrary
=================
*/
void Sys_UnloadLibrary( void *handle )
{
	if ( handle )
		FreeLibrary( handle );
}


#ifndef DEDICATED
static qboolean Sys_DLLNeedsUnpacking( const char* name ) {
#if defined(DEDICATED)
	return qfalse;
#else
	if ( !Q_stricmpn( name, "qagame", 6 ) )
		return qfalse;
#ifdef _DEBUG
	return cl_connectedToPureServer != 0 ? qtrue : qfalse;
#else
	return qtrue;
#endif
#endif
}
#endif


/*
=================
Sys_LoadDll

Used to load a development dll instead of a virtual machine
=================
*/

static const char *win_dlerror(void) {
	TCHAR			temp[1024+1];
	static char		ospath[2][sizeof(temp)];
	static int		toggle;

	toggle ^= 1; // flip-flop to allow two returns without clash

	FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ), temp, sizeof(temp)/sizeof(temp[0]), NULL );
	Q_strncpyz( ospath[toggle], WtoA(temp), sizeof(ospath[0]) );

	return ospath[toggle];
}

static void *try_dlopen(const char *base, const char *gamedir, const char *fname)
{
	void *libHandle;
	char *fn;

	fn = FS_BuildOSPath(base, gamedir, fname);
	Com_Printf("Sys_LoadGameDLL(%s)... \n", fn);

	libHandle = Sys_LoadLibrary(fn);

	if (!libHandle)
	{
		Com_Printf("Sys_LoadGameDLL(%s) failed:\n\"%s\"\n", fn, win_dlerror());
		return NULL;
	}

	Com_Printf("Sys_LoadGameDLL(%s): succeeded ...\n", fn);

	return libHandle;
}

const char* Sys_GetDLLName( const char *name ) {
	return va( "%s_mp_" ARCH_STRING DLL_EXT, name );
}

void *QDECL Sys_LoadGameDll( const char *name, vmMain_t *entryPoint, dllSyscall_t systemcalls ) {
	HINSTANCE libHandle;
	dllEntry_t	dllEntry;
	const char	*basepath;
	const char	*homepath;
	const char	*gamedir;
	char		filename[ MAX_QPATH ];
#if !defined( DEDICATED )
	char		*fn;
	qboolean	unpack = qfalse;
#endif
#ifdef _DEBUG
	TCHAR currpath[ MAX_OSPATH ];
#endif
	const char *err = NULL;

	Q_strncpyz( filename, Sys_GetDLLName( name ), sizeof( filename ) );

	basepath = Cvar_VariableString( "fs_basepath" );
	homepath = Cvar_VariableString( "fs_homepath" );
	gamedir = Cvar_VariableString( "fs_game" );
	if ( !*gamedir ) {
		gamedir = Cvar_VariableString( "fs_basegame" );
	}

#ifdef _DEBUG
	if ( GetCurrentDirectory( ARRAY_LEN( currpath ), currpath ) < ARRAY_LEN( currpath ) )
		libHandle = try_dlopen( WtoA( currpath ), gamedir, filename );
	else
#endif
		libHandle = NULL;

	// TTimo - this is only relevant for full client
	// if a full client runs a dedicated server, it's not affected by this
#if !defined( DEDICATED )
	// try gamepath first
	fn = FS_BuildOSPath( basepath, gamedir, filename );

	unpack = Sys_DLLNeedsUnpacking( name );
	// NERVE - SMF - extract dlls from pak file for security
	// we have to handle the game dll a little differently
	// TTimo - passing the exact path to check against
	//   (compatibility with other OSes loading procedure)
	if ( unpack ) {
		if ( !FS_CL_ExtractFromPakFile( fn, gamedir, filename, NULL ) ) {
			Com_Printf( "Sys_LoadGameDLL(%s/%s) failed to extract library\n", gamedir, name );
		}
		else {
			Com_Printf( "Sys_LoadGameDLL(%s/%s) library extraction succeeded\n", gamedir, name );
		}
	}
#endif
	if ( !libHandle ) {
		libHandle = try_dlopen(basepath, gamedir, filename);
	}

	if ( !libHandle && homepath && homepath[0] && Q_stricmp( basepath, homepath ) ) {
		libHandle = try_dlopen(homepath, gamedir, filename);
	}

#ifndef DEDICATED
	if ( Q_stricmpn( name, "qagame", 6) != 0 ) {
		if ( !libHandle && !unpack ) {
			fn = FS_BuildOSPath( basepath, gamedir, filename );
			if (!FS_CL_ExtractFromPakFile(fn, gamedir, filename, NULL))
			{
				Com_Printf("Sys_LoadGameDLL(%s/%s) failed to extract library\n", gamedir, name);
				return NULL;
			}
			Com_Printf("Sys_LoadGameDLL(%s/%s) library extraction succeeded\n", gamedir, name);
			libHandle = try_dlopen(basepath, gamedir, filename);
		}
	}
#endif

	// Last resort for missing DLLs or media mods
	// If mod requires a different cgame/ui this could cause problems
	/*if ( !libHandle && strcmp( gamedir, BASEGAME ) != 0 ) {
		const char *temp = va( "%s%c%s", gamedir, PATH_SEP, filename );
		FS_SetFilterFlag( FS_EXCLUDE_OTHERGAMES );
		if ( !FS_SV_FileExists( temp ) && !FS_FileIsInPAK( filename, NULL, NULL ) ) {
			Com_Printf( "Sys_LoadDLL(%s/%s) trying %s override\n", gamedir, name, BASEGAME );
			libHandle = try_dlopen(basepath, BASEGAME, filename);
		}
		FS_SetFilterFlag( 0 );
	}*/

	if ( !libHandle ) {
#ifdef _DEBUG // in debug abort on failure
		Com_Error(ERR_FATAL, "Sys_LoadGameDLL(%s/%s) failed LoadLibrary() completely!", gamedir, name);
#else
		Com_Printf( "Sys_LoadGameDLL(%s/%s) failed LoadLibrary() completely!\n", gamedir, name );
#endif
		return NULL;
	}

	Sys_LoadFunctionErrors(); // reset counter

	dllEntry = ( dllEntry_t ) Sys_LoadFunction( libHandle, "dllEntry" ); 
	*entryPoint = ( vmMain_t ) Sys_LoadFunction( libHandle, "vmMain" );
	if ( !*entryPoint || !dllEntry ) {
		err = win_dlerror();
#ifdef _DEBUG
		Com_Error(ERR_FATAL, "Sys_LoadGameDLL(%s/%s) failed GetProcAddress(vmMain):\n\"%s\" !", gamedir, name, err);
#else
		Com_Printf("Sys_LoadGameDLL(%s/%s) failed GetProcAddress(vmMain):\n\"%s\"\n", gamedir, name, err);
#endif
		Sys_UnloadLibrary( libHandle );
		err = win_dlerror();
		if (err != NULL)
		{
			Com_Printf("Sys_LoadGameDLL(%s/%s) failed FreeLibrary:\n\"%s\"\n", gamedir, name, err);
		}
		return NULL;
	}

	Com_Printf( "Sys_LoadGameDLL(%s/%s) found **vmMain** at %p\n", gamedir, name, *entryPoint );
	dllEntry( systemcalls );
	Com_Printf( "Sys_LoadGameDLL(%s/%s) succeeded!\n", gamedir, name );

	return libHandle;
}


/*
=================
Sys_SendKeyEvents

Platform-dependent event handling
=================
*/
void Sys_SendKeyEvents( void )
{
#ifndef DEDICATED
	if ( !com_dedicated->integer )
		HandleEvents();
	else
#endif
	HandleConsoleEvents();
}


//================================================================


/*
==================
SetTimerResolution

Try to set lower timer period
==================
*/
static void SetTimerResolution( void )
{
	typedef HRESULT (WINAPI *pfnNtQueryTimerResolution)( PULONG MinRes, PULONG MaxRes, PULONG CurRes );
	typedef HRESULT (WINAPI *pfnNtSetTimerResolution)( ULONG NewRes, BOOLEAN SetRes, PULONG CurRes );
	pfnNtQueryTimerResolution pNtQueryTimerResolution;
	pfnNtSetTimerResolution pNtSetTimerResolution;
	ULONG curr, minr, maxr;
	HMODULE dll;

	dll = GetModuleHandle( T( "ntdll" ) );
	if ( dll )
	{
		pNtQueryTimerResolution = (pfnNtQueryTimerResolution) GetProcAddress( dll, "NtQueryTimerResolution" );
		pNtSetTimerResolution = (pfnNtSetTimerResolution) GetProcAddress( dll, "NtSetTimerResolution" );
		if ( pNtQueryTimerResolution && pNtSetTimerResolution )
		{
			pNtQueryTimerResolution( &minr, &maxr, &curr );
			if ( maxr < 5000 ) // well, we don't need less than 0.5ms periods for select()
				maxr = 5000;
			pNtSetTimerResolution( maxr, TRUE, &curr );
		}
	}
}


/*
================
Sys_Init

Called after the common systems (cvars, files, etc)
are initialized
================
*/
void Sys_Init( void ) {

	// make sure the timer is high precision, otherwise
	// NT gets 18ms resolution
	timeBeginPeriod( 1 );

	SetTimerResolution();

	Cmd_AddCommand( "clearviewlog", Sys_ClearViewlog_f );

	Cvar_Set( "arch", "winnt" );
}

//=======================================================================


static const char *GetExceptionName( DWORD code )
{
	static char buf[ 32 ];

	switch ( code )
	{
		case EXCEPTION_ACCESS_VIOLATION: return "ACCESS_VIOLATION";
		case EXCEPTION_DATATYPE_MISALIGNMENT: return "DATATYPE_MISALIGNMENT";
		case EXCEPTION_ARRAY_BOUNDS_EXCEEDED: return "ARRAY_BOUNDS_EXCEEDED";
		case EXCEPTION_PRIV_INSTRUCTION: return "PRIV_INSTRUCTION";
		case EXCEPTION_IN_PAGE_ERROR: return "IN_PAGE_ERROR";
		case EXCEPTION_ILLEGAL_INSTRUCTION: return "ILLEGAL_INSTRUCTION";
		case EXCEPTION_NONCONTINUABLE_EXCEPTION: return "NONCONTINUABLE_EXCEPTION";
		case EXCEPTION_STACK_OVERFLOW: return "STACK_OVERFLOW";
		case EXCEPTION_INVALID_DISPOSITION: return "INVALID_DISPOSITION";
		case EXCEPTION_GUARD_PAGE: return "GUARD_PAGE";
		case EXCEPTION_INVALID_HANDLE: return "INVALID_HANDLE";
		case EXCEPTION_INT_DIVIDE_BY_ZERO: return "INT_DIVIDE_BY_ZERO";
		case EXCEPTION_FLT_DIVIDE_BY_ZERO: return "FLT_DIVIDE_BY_ZERO";
		default: break;
	}

	sprintf( buf, "0x%08X", (unsigned int)code );
	return buf;
}


/*
==================
ExceptionFilter

Restore gamma and hide fullscreen window in case of crash
==================
*/
static LONG WINAPI ExceptionFilter( struct _EXCEPTION_POINTERS *ExceptionInfo )
{
#ifndef DEDICATED
	if ( com_dedicated->integer == 0 ) {
		extern cvar_t *com_cl_running;
		if ( com_cl_running  && com_cl_running->integer ) {
			// assume we can restart client module
		} else {
			GLW_RestoreGamma();
			GLW_HideFullscreenWindow();
		}
	}
#endif

	if ( ExceptionInfo->ExceptionRecord->ExceptionCode != EXCEPTION_BREAKPOINT )
	{
		char msg[128];
		byte *addr, *base;
		qboolean vma;

		addr = (byte*)ExceptionInfo->ExceptionRecord->ExceptionAddress;
		base = (byte*)GetModuleHandle( NULL );

		if ( addr >= base )
		{
			addr = (byte*)(addr - base);
			vma = qtrue;
		}
		else
		{
			vma = qfalse;
		}

		Com_sprintf( msg, sizeof(msg), "Exception Code: %s\nException Address: %p%s",
			GetExceptionName( ExceptionInfo->ExceptionRecord->ExceptionCode ),
			addr, vma ? " (VMA)" : "" );

		Com_Error( ERR_DROP, "Unhandled exception caught\n%s", msg );
	}

	return EXCEPTION_EXECUTE_HANDLER;
}


// Gets whether the current process has UAC virtualization enabled.
// Returns TRUE on success and FALSE on failure.
#if 0
static qboolean WIN_IsVirtualizationEnabled(qboolean *status) {
	HANDLE token;
	DWORD temp;
	DWORD len;
	qboolean ret = qtrue;

	if(!status)
		return qfalse;

	if(!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token))
		return qfalse;

	if(!GetTokenInformation(token, TokenVirtualizationEnabled, &temp, sizeof(temp), &len)) {
		ret = qfalse;
		goto err;
	}

	*status = temp != 0 ? qtrue : qfalse;

err:
	CloseHandle(token);

	return ret;
}
#endif


/*
==================
WinMain
==================
*/
int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow ) 
{
	static char	sys_cmdline[ MAX_STRING_CHARS ];
	char con_title[ MAX_CVAR_VALUE_STRING ];
	int xpos, ypos;
	qboolean useXYpos;
	HANDLE hProcess;
	DWORD dwPriority;
	//qboolean virtStatus = qfalse;

	// should never get a previous instance in Win32
	if ( hPrevInstance ) {
		return 0;
	}

	// slightly boost process priority if it set to default
	hProcess = GetCurrentProcess();
	dwPriority = GetPriorityClass( hProcess );
	if ( dwPriority == NORMAL_PRIORITY_CLASS || dwPriority == ABOVE_NORMAL_PRIORITY_CLASS ) {
		SetPriorityClass( hProcess, HIGH_PRIORITY_CLASS );
	}

	SetupDPIAwareness();

	g_wv.hInstance = hInstance;
	Q_strncpyz( sys_cmdline, lpCmdLine, sizeof( sys_cmdline ) );

	useXYpos = Com_EarlyParseCmdLine( sys_cmdline, con_title, sizeof( con_title ), &xpos, &ypos );

	// done before Com/Sys_Init since we need this for error output
	Sys_CreateConsole( con_title, xpos, ypos, useXYpos );

	// no abort/retry/fail errors
	SetErrorMode( SEM_FAILCRITICALERRORS );

	SetUnhandledExceptionFilter( ExceptionFilter );

	// get the initial time base
	Sys_Milliseconds();

	Com_Init( sys_cmdline );
	NET_Init();

	Com_Printf( "Working directory: %s\n", Sys_Pwd() );

#if 0
	if ( WIN_IsVirtualizationEnabled(&virtStatus) ) {
		if ( virtStatus )
			Com_Printf( S_COLOR_YELLOW "WARNING: UAC Virtualization detected. Suggest installing outside of Program Files\n" );
	}
#endif

	// hide the early console since we've reached the point where we
	// have a working graphics subsystems
	if ( !com_dedicated->integer && !com_viewlog->integer ) {
		Sys_ShowConsole( 0, qfalse );
	}

	// main game loop
	while( 1 ) {
		// set low precision every frame, because some system calls
		// reset it arbitrarily
		// _controlfp( _PC_24, _MCW_PC );
		// _controlfp( -1, _MCW_EM  ); // no exceptions, even if some crappy syscall turns them back on!

#ifdef DEDICATED
		// run the game
		Com_Frame( qfalse );
#else
		// make sure mouse and joystick are only called once a frame
		IN_Frame();
		// run the game
		Com_Frame( CL_NoDelay() );
#endif
	}

	// never gets here
	return 0;
}

qboolean Sys_IsNumLockDown( void ) {
	// thx juz ;)
	SHORT state = GetKeyState( VK_NUMLOCK );

	if ( state & 0x01 ) {
		return qtrue;
	}

	return qfalse;
}

int Sys_GetPID( void ) {
	return (int)GetCurrentProcessId();
}

HINSTANCE omnibotHandle = NULL;
typedef void (*pfnOmnibotRenderOGL)();
pfnOmnibotRenderOGL gOmnibotRenderFunc = 0;

void	Sys_OmnibotLoad()
{
	const char *omnibotPath = Cvar_VariableString( "omnibot_path" );
	const char *omnibotLibrary = Cvar_VariableString( "omnibot_library" );
	if ( omnibotLibrary != NULL && omnibotLibrary[0] != '\0' )
	{
		if ( omnibotPath != NULL && omnibotPath[0] != '\0' ) {
			omnibotHandle = Sys_LoadLibrary(va("%s/%s.dll", omnibotPath, omnibotLibrary));	
		}
		else {
			omnibotHandle = Sys_LoadLibrary( va("%s.dll", omnibotLibrary ) );
		}
		if ( omnibotHandle )
		{
			gOmnibotRenderFunc = (pfnOmnibotRenderOGL)Sys_LoadFunction( omnibotHandle, "RenderOpenGL" );
		}
	}
}


void	Sys_OmnibotUnLoad()
{
	Sys_UnloadLibrary( omnibotHandle );
	omnibotHandle = NULL;
}

const void * Sys_OmnibotRender( const void * data )
{
	renderOmnibot_t * cmd = (renderOmnibot_t*)data;
	if ( gOmnibotRenderFunc )
	{
		gOmnibotRenderFunc();
	}
	return (const void *)( cmd + 1 );
}


/*
================
Sys_SteamInit
================
*/

#ifndef DEDICATED

void Sys_SteamInit()
{
#if defined(USE_STEAMAPI)
#if (idx64 || id386)
	/*if (!Cvar_VariableIntegerValue("com_steamIntegration"))
	{
		// Don't do anything if com_steamIntegration is disabled
		return;
	}*/

	if (!STEAMSHIM_init())
	{
		Com_Printf(S_COLOR_RED "Steam integration failed: Steam init failed. Ensure steam_appid.txt exists and is valid.\n");
		return;
	}
	Com_Printf(S_COLOR_CYAN "Steam integration success!\n" );
#endif
#endif
}


/*
================
Sys_SteamShutdown
================
*/
void Sys_SteamShutdown()
{
#if defined(USE_STEAMAPI)
#if (idx64 || id386)
	if(!STEAMSHIM_alive())
	{
		Com_Printf("Skipping Steam integration shutdown...\n");
		return;
	}
	STEAMSHIM_deinit();
#endif
#endif
}
#endif
