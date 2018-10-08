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
// win_dpi.c

#include "../qcommon/q_shared.h"
#include "../qcommon/qcommon.h"
#include "win_local.h"

// Avoid needing to include win header that may not be available
typedef enum ETE_PROCESS_DPI_AWARENESS {
	ETE_PROCESS_DPI_UNAWARE = 0,
	ETE_PROCESS_SYSTEM_DPI_AWARE = 1,
	ETE_PROCESS_PER_MONITOR_DPI_AWARE = 2
} ETE_PROCESS_DPI_AWARENESS;

typedef LONG( WINAPI *RtlGetVersionPtr )( RTL_OSVERSIONINFOEXW* );
typedef BOOL( WINAPI *SetProcessDpiAwarenessContextPtr )( HANDLE );
typedef HRESULT( WINAPI *SetProcessDpiAwarenessPtr )( int );
typedef BOOL( WINAPI *SetProcessDPIAwarePtr )( void );

#define ETE_DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ((HANDLE)-4)

// MSDN SetProcessDpiAwarenessContext
static qboolean EnablePerMonitorV2( void ) {
	SetProcessDpiAwarenessContextPtr set_process_dpi_awareness_context_f = NULL;
	HMODULE u32dll = GetModuleHandle( T( "user32" ) );

	if ( !u32dll ) {
		return qfalse;
	}

	set_process_dpi_awareness_context_f = (SetProcessDpiAwarenessContextPtr)GetProcAddress( u32dll, "SetProcessDpiAwarenessContext" );

	if ( set_process_dpi_awareness_context_f ) {
		return set_process_dpi_awareness_context_f( ETE_DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 ) != FALSE ? qtrue : qfalse;
	}
	return qfalse;
}

// Ugly hack to detect Win10 without manifest
// http://www.codeproject.com/Articles/678606/Part-Overcoming-Windows-s-deprecation-of-GetVe?msg=5080848#xx5080848xx
static qboolean IsWin10( void ) {
	RtlGetVersionPtr rtl_get_version_f = NULL;
	HMODULE ntdll = GetModuleHandle( T( "ntdll" ) );
	RTL_OSVERSIONINFOEXW osver;

	if ( !ntdll )
		return qfalse; // will never happen

	rtl_get_version_f = (RtlGetVersionPtr)GetProcAddress( ntdll, "RtlGetVersion" );

	if ( !rtl_get_version_f )
		return qfalse; // will never happen

	osver.dwOSVersionInfoSize = sizeof( RTL_OSVERSIONINFOEXW );

	if ( rtl_get_version_f( &osver ) == 0 ) {
		if ( osver.dwMajorVersion >= 10 )
			return qtrue;
	}

	return qfalse;
}


// secret sauce SetProcessDpiAwarenessInternal
// MSDN says SetProcessDpiAwareness but the actual symbol is with `Internal`
// We must check for Win10 to use Per Monitor support, otherwise use system
static qboolean EnablePerMonitor( void ) {
	SetProcessDpiAwarenessPtr set_process_dpi_awareness_f = NULL;
	HMODULE u32dll = GetModuleHandle( T( "user32" ) );
	qboolean win10 = qfalse;

	if ( !u32dll ) {
		return qfalse;
	}

	set_process_dpi_awareness_f = (SetProcessDpiAwarenessPtr)GetProcAddress( u32dll, "SetProcessDpiAwarenessInternal" );

	win10 = IsWin10();

	if ( set_process_dpi_awareness_f ) {
		HRESULT hr = set_process_dpi_awareness_f( win10 ? ETE_PROCESS_PER_MONITOR_DPI_AWARE : ETE_PROCESS_SYSTEM_DPI_AWARE );

		if ( SUCCEEDED( hr ) ) {
			return qtrue;
		} else if ( hr == E_ACCESSDENIED ) {
			// This can happen when function is called more than once or there is a manifest override
			// Definitely should be logging this
		}
	}
	return qfalse;
}

// Legacy DPI awareness (Vista+) or all else fails
static qboolean EnableDPIAware( void ) {
	SetProcessDPIAwarePtr set_process_dpi_aware_f = NULL;
	HMODULE u32dll = GetModuleHandle( T( "user32" ) );

	if ( !u32dll ) {
		return qfalse;
	}

	set_process_dpi_aware_f = (SetProcessDPIAwarePtr)GetProcAddress( u32dll, "SetProcessDPIAware" );

	if ( set_process_dpi_aware_f ) {
		return set_process_dpi_aware_f() != FALSE ? qtrue : qfalse;
	}
	return qfalse;
}

void SetupDPIAwareness( void ) {
	Com_DPrintf( "Setup DPI Awareness...\n" );
	if ( !EnablePerMonitorV2() ) {
		Com_DPrintf( " ...per monitor v2: failed\n" );
	} else {
		Com_DPrintf( " ...per monitor v2: succeeded\n" );
		return;
	}
	if ( !EnablePerMonitor() ) {
		Com_DPrintf( " ...per monitor: failed\n" );
	} else {
		Com_DPrintf( " ...per monitor: succeeded\n" );
		return;
	}

	if ( !EnableDPIAware() ) {
		Com_DPrintf( " ...per process: failed\n" );
	} else {
		Com_DPrintf( " ...per process: succeeded\n" );
		return;
	}
}
