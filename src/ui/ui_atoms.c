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

/**********************************************************************
	UI_ATOMS.C

	User interface building blocks and support functions.
**********************************************************************/
#include "ui_local.h"

// JPW NERVE added Com_DPrintf
#define UI_MAXPRINTMSG 8192 // sync with qcommon
void QDECL Com_DPrintf( const char *fmt, ... ) {
	va_list argptr;
	char msg[UI_MAXPRINTMSG];
	int developer;

	developer = trap_Cvar_VariableValue( "developer" );
	if ( !developer ) {
		return;
	}

	va_start( argptr,fmt );
	Q_vsnprintf( msg, sizeof( msg ), fmt, argptr );
	va_end( argptr );

	Com_Printf( "%s", msg );
}
// jpw

void NORETURN QDECL Com_Error( errorParm_t code, const char *error, ... ) {
	va_list argptr;
	char text[UI_MAXPRINTMSG];

	va_start( argptr, error );
	Q_vsnprintf( text, sizeof( text ), error, argptr );
	va_end( argptr );

	trap_Error( text );
}

void QDECL Com_Printf( const char *msg, ... ) {
	va_list argptr;
	char text[UI_MAXPRINTMSG];

	va_start( argptr, msg );
	Q_vsnprintf( text, sizeof( text ), msg, argptr );
	va_end( argptr );

	trap_Print( text );
}


const char *UI_Argv( int arg ) {
	static char buffer[MAX_STRING_CHARS];

	trap_Argv( arg, buffer, sizeof( buffer ) );

	return buffer;
}


const char *UI_Cvar_VariableString( const char *var_name ) {
	static char buffer[2][MAX_STRING_CHARS];
	static int toggle;

	toggle ^= 1;        // flip-flop to allow two returns without clash

	trap_Cvar_VariableStringBuffer( var_name, buffer[toggle], sizeof( buffer[0] ) );

	return buffer[toggle];
}

static void UI_Test_f( void ) {
	UI_ShowPostGame( qtrue );
}

static void dummyFunc( void ) {
}

static void UI_Cache_f( void ) {
	Display_CacheAll();
}

static void UI_Cheater_f( void ) {
	int i;

	// unlock all available levels and campaigns for SP
	for ( i = 0; i < uiInfo.campaignCount; i++ ) {
		if ( uiInfo.campaignList[i].typeBits & ( 1 << GT_SINGLE_PLAYER ) ) {
			uiInfo.campaignList[i].unlocked = qtrue;
			uiInfo.campaignList[i].progress = uiInfo.campaignList[i].mapCount;
		}
	}
}

typedef struct {
	const char *cmd;
	void ( *function )( void );
	qboolean disconnectonly;
} consoleCommand_t;

static consoleCommand_t commands[] =
{
	{ "ui_test", UI_Test_f, qfalse },
	{ "ui_report", UI_Report, qfalse },
	{ "ui_load", UI_Load, qfalse },
	{ "postgame", dummyFunc, qfalse },
	{ "ui_cache", UI_Cache_f, qfalse },
	{ "ui_teamOrders", dummyFunc, qfalse },
	{ "ui_cdkey", dummyFunc, qfalse },
	{ "iamacheater", UI_Cheater_f, qfalse },
	{ "campaign", UI_Campaign_f, qtrue },
	{ "listcampaigns", UI_ListCampaigns_f, qtrue },
};

static const size_t numCommands = ARRAY_LEN( commands );

/*
=================
UI_ConsoleCommand
=================
*/
qboolean UI_ConsoleCommand( int realTime ) {
	const char    *cmd;
	size_t i;
	uiClientState_t cstate;

	uiInfo.uiDC.frameTime = realTime - uiInfo.uiDC.realTime;
	uiInfo.uiDC.realTime = realTime;

	cmd = UI_Argv( 0 );

	trap_GetClientState( &cstate );

	for ( i = 0 ; i < numCommands ; i++ ) {
		if ( !Q_stricmp( cmd, commands[i].cmd ) ) {
			if ( !commands[i].disconnectonly || cstate.connState == CA_DISCONNECTED ) {
				commands[i].function();
				return qtrue;
			}
			else {
				return qfalse;
			}
		}
	}

	return qfalse;
}

/*
================
UI_AdjustFrom640

Adjusted for resolution and screen aspect ratio
================
*/
void UI_AdjustFrom640( float *x, float *y, float *w, float *h ) {
	// expect valid pointers
#if 0
	*x = *x * uiInfo.uiDC.scale + uiInfo.uiDC.bias;
	*y *= uiInfo.uiDC.scale;
	*w *= uiInfo.uiDC.scale;
	*h *= uiInfo.uiDC.scale;
#endif

	*x *= uiInfo.uiDC.xscale;
	*y *= uiInfo.uiDC.yscale;
	*w *= uiInfo.uiDC.xscale;
	*h *= uiInfo.uiDC.yscale;

}

void UI_DrawNamedPic( float x, float y, float width, float height, const char *picname ) {
	qhandle_t hShader;

	hShader = trap_R_RegisterShaderNoMip( picname );
	UI_AdjustFrom640( &x, &y, &width, &height );
	trap_R_DrawStretchPic( x, y, width, height, 0, 0, 1, 1, hShader );
}

void UI_DrawHandlePic( float x, float y, float w, float h, qhandle_t hShader ) {
	float s0;
	float s1;
	float t0;
	float t1;

	if ( w < 0 ) {   // flip about vertical
		w  = -w;
		s0 = 1;
		s1 = 0;
	} else {
		s0 = 0;
		s1 = 1;
	}

	if ( h < 0 ) {   // flip about horizontal
		h  = -h;
		t0 = 1;
		t1 = 0;
	} else {
		t0 = 0;
		t1 = 1;
	}

	UI_AdjustFrom640( &x, &y, &w, &h );
	trap_R_DrawStretchPic( x, y, w, h, s0, t0, s1, t1, hShader );
}

/*
================
UI_DrawRotatedPic

Coordinates are 640*480 virtual values
=================
*/
void UI_DrawRotatedPic( float x, float y, float width, float height, qhandle_t hShader, float angle ) {

	UI_AdjustFrom640( &x, &y, &width, &height );

	trap_R_DrawRotatedPic( x, y, width, height, 0, 0, 1, 1, hShader, angle );
}

/*
================
UI_FillRect

Coordinates are 640*480 virtual values
=================
*/
void UI_FillRect( float x, float y, float width, float height, const float *color ) {
	trap_R_SetColor( color );

	UI_AdjustFrom640( &x, &y, &width, &height );
	trap_R_DrawStretchPic( x, y, width, height, 0, 0, 0, 0, uiInfo.uiDC.whiteShader );

	trap_R_SetColor( NULL );
}

/*
================
UI_FillScreen
================
*/
void UI_FillScreen( const float *color )
{
	trap_R_SetColor( color );
	trap_R_DrawStretchPic( 0, 0, uiInfo.uiDC.glconfig.vidWidth, uiInfo.uiDC.glconfig.vidHeight, 0, 0, 0, 0, uiInfo.uiDC.whiteShader );
	trap_R_SetColor( NULL );
}

void UI_DrawSides( float x, float y, float w, float h ) {
	UI_AdjustFrom640( &x, &y, &w, &h );
	trap_R_DrawStretchPic( x, y, 1, h, 0, 0, 0, 0, uiInfo.uiDC.whiteShader );
	trap_R_DrawStretchPic( x + w - 1, y, 1, h, 0, 0, 0, 0, uiInfo.uiDC.whiteShader );
}

void UI_DrawTopBottom( float x, float y, float w, float h ) {
	UI_AdjustFrom640( &x, &y, &w, &h );
	trap_R_DrawStretchPic( x, y, w, 1, 0, 0, 0, 0, uiInfo.uiDC.whiteShader );
	trap_R_DrawStretchPic( x, y + h - 1, w, 1, 0, 0, 0, 0, uiInfo.uiDC.whiteShader );
}

/*
================
UI_DrawRect

Coordinates are 640*480 virtual values
=================
*/
void UI_DrawRect( float x, float y, float width, float height, const float *color ) {
	trap_R_SetColor( color );

	UI_DrawTopBottom( x, y, width, height );
	UI_DrawSides( x, y, width, height );

	trap_R_SetColor( NULL );
}
