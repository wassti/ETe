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


#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"
#include "win_local.h"
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <direct.h>
#include <io.h>
#include <conio.h>
#include <intrin.h>

/*
================
Sys_Milliseconds
================
*/
int Sys_Milliseconds( void )
{
	static qboolean	initialized = qfalse;
	static DWORD sys_timeBase;
	int	sys_curtime;

	if ( !initialized ) {
		sys_timeBase = timeGetTime();
		initialized = qtrue;
	}

	sys_curtime = timeGetTime() - sys_timeBase;

	return sys_curtime;
}


/*
================
Sys_RandomBytes
================
*/
qboolean Sys_RandomBytes( byte *string, int len )
{
	HCRYPTPROV  prov;

	if( !CryptAcquireContext( &prov, NULL, NULL,
		PROV_RSA_FULL, CRYPT_VERIFYCONTEXT ) )  {

		return qfalse;
	}

	if( !CryptGenRandom( prov, len, (BYTE *)string ) )  {
		CryptReleaseContext( prov, 0 );
		return qfalse;
	}
	CryptReleaseContext( prov, 0 );
	return qtrue;
}


#ifdef UNICODE
LPWSTR AtoW( const char *s ) 
{
	static WCHAR buffer[MAXPRINTMSG*2];
	MultiByteToWideChar( CP_ACP, 0, s, strlen( s ) + 1, (LPWSTR) buffer, ARRAYSIZE( buffer ) );
	return buffer;
}


const char *WtoA( const LPWSTR s ) 
{
	static char buffer[MAXPRINTMSG*2];
	WideCharToMultiByte( CP_ACP, 0, s, -1, buffer, ARRAYSIZE( buffer ), NULL, NULL );
	return buffer;
}
#endif


static qboolean StrIsAscii( const signed char *text, size_t len ) {
	size_t i;
	for( i = 0; i < len; i++ ) {
		if( text[i] < 0 ) {
			return qfalse;
		}
	}
	return qtrue;
}


/*
================
Sys_DefaultHomePath
================
*/
const char *Sys_DefaultHomePath( void ) 
{
#ifdef USE_WINDOWS_PROFILES
	TCHAR szPath[MAX_PATH];
	static char path[MAX_OSPATH];
	FARPROC qSHGetFolderPath;
	HMODULE shfolder = LoadLibrary("shfolder.dll");
	
	if(shfolder == NULL) {
		Com_Printf("Unable to load SHFolder.dll\n");
		return NULL;
	}

	qSHGetFolderPath = GetProcAddress(shfolder, "SHGetFolderPathA");
	if(qSHGetFolderPath == NULL)
	{
		Com_Printf("Unable to find SHGetFolderPath in SHFolder.dll\n");
		FreeLibrary(shfolder);
		return NULL;
	}

	if( !SUCCEEDED( qSHGetFolderPath( NULL, CSIDL_APPDATA,
		NULL, 0, szPath ) ) )
	{
		Com_Printf("Unable to detect CSIDL_APPDATA\n");
		FreeLibrary(shfolder);
		return NULL;
	}
	Q_strncpyz( path, szPath, sizeof(path) );
	Q_strcat( path, sizeof(path), "\\EnemyTerritory" );
	FreeLibrary(shfolder);
	if( !CreateDirectory( path, NULL ) )
	{
		if( GetLastError() != ERROR_ALREADY_EXISTS )
		{
			Com_Printf("Unable to create directory \"%s\"\n", path);
			return NULL;
		}
	}

	if ( !StrIsAscii( path, strlen(path) ) )
	{
		Com_Printf("Home directory is not an ASCII valid path. Unicode usernames are unsupported\n" );
		return NULL;
	}

	return path;
#else
    return NULL;
#endif
}


/*
================
Sys_SteamPath
================
*/
const char *Sys_SteamPath( void )
{
	static TCHAR steamPath[ MAX_OSPATH ]; // will be converted from TCHAR to ANSI

#if defined(STEAMPATH_NAME) || defined(STEAMPATH_APPID)
	HKEY steamRegKey;
	DWORD pathLen = MAX_OSPATH;
	qboolean finishPath = qfalse;
#endif

#ifdef STEAMPATH_APPID
	// Assuming Steam is a 32-bit app
	if ( !steamPath[0] && RegOpenKeyEx(HKEY_LOCAL_MACHINE, AtoW("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\Steam App " STEAMPATH_APPID), 0, KEY_QUERY_VALUE | KEY_WOW64_32KEY, &steamRegKey ) == ERROR_SUCCESS ) 
	{
		pathLen = sizeof( steamPath );
		if ( RegQueryValueEx( steamRegKey, AtoW("InstallLocation"), NULL, NULL, (LPBYTE)steamPath, &pathLen ) != ERROR_SUCCESS )
			steamPath[ 0 ] = '\0';

		RegCloseKey( steamRegKey );
	}

#ifdef STEAMPATH_NAME
	if ( !steamPath[0] && RegOpenKeyEx(HKEY_CURRENT_USER, AtoW("Software\\Valve\\Steam"), 0, KEY_QUERY_VALUE, &steamRegKey ) == ERROR_SUCCESS )
	{
		pathLen = sizeof( steamPath );
		if ( RegQueryValueEx( steamRegKey, AtoW("SteamPath"), NULL, NULL, (LPBYTE)steamPath, &pathLen ) != ERROR_SUCCESS ) {
			pathLen = sizeof( steamPath );
			if ( RegQueryValueEx( steamRegKey, AtoW("InstallPath"), NULL, NULL, (LPBYTE)steamPath, &pathLen ) != ERROR_SUCCESS )
				steamPath[ 0 ] = '\0';
		}

		if ( steamPath[ 0 ] )
			finishPath = qtrue;

		RegCloseKey( steamRegKey );
	}
#endif

	if ( steamPath[ 0 ] )
	{
		if ( pathLen == sizeof( steamPath ) )
			pathLen--;

		*( ((char*)steamPath) + pathLen )  = '\0';
#ifdef UNICODE
		strcpy( (char*)steamPath, WtoA( steamPath ) );
#endif
		if ( finishPath )
			Q_strcat( (char*)steamPath, MAX_OSPATH, "\\SteamApps\\common\\" STEAMPATH_NAME );
	}
#endif

	return (const char*)steamPath;
}


const char *Sys_GogPath( void )
{
	static TCHAR gogPath[ MAX_OSPATH ]; // will be converted from TCHAR to ANSI

	return (const char*)gogPath;
}


const char *Sys_MicrosoftStorePath( void )
{
	// TODO
	// Most likely will be something like below:
	// C:\Program Files\ModifiableWindowsApps\Wolfenstein Enemy Territory\EN
	// Use SHGetFolderPathA with CSIDL_PROGRAMFILES to pull the program files directory
	static TCHAR microsoftStorePath[ MAX_OSPATH ]; // will be converted from TCHAR to ANSI

	return (const char*)microsoftStorePath;
}


/*
================
Sys_SetAffinityMask
================
*/
#ifdef USE_AFFINITY_MASK
void Sys_SetAffinityMask( int mask )
{
	static DWORD_PTR dwOldProcessMask;
	static DWORD_PTR dwSystemMask;

	// initialize
	if ( !dwOldProcessMask ) {
		dwSystemMask = 0x1;
		dwOldProcessMask = 0x1;
		GetProcessAffinityMask( GetCurrentProcess(), &dwOldProcessMask, &dwSystemMask );
	}

	 // set default mask
	if ( !mask ) {
		if ( dwOldProcessMask )
			mask = dwOldProcessMask;
		else
			mask = dwSystemMask;
	}

	if ( SetProcessAffinityMask( GetCurrentProcess(), mask ) ) {
		Sleep( 0 );
		Com_Printf( "setting CPU affinity mask to %i\n", mask );
	} else {
		Com_Printf( S_COLOR_YELLOW "error setting CPU affinity mask %i\n", mask );
	}
}
#endif // USE_AFFINITY_MASK
