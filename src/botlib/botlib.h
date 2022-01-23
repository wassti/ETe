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

//===========================================================================
//
/*****************************************************************************
 * name:		botlib.h
 *
 * desc:		bot AI library
 *
 *
 *****************************************************************************/

#ifndef BOTLIB_H
#define BOTLIB_H

#define BOTLIB_API_VERSION      2

#define MAX_DEBUGPOLYS      4096

typedef struct bot_debugpoly_s {
	int inuse;
	int color;
	int numPoints;
	vec3_t points[128];
} bot_debugpoly_t;

typedef void ( *BotPolyFunc )( int color, int numPoints, float *points );

//Print types
#define PRT_MESSAGE             1
#define PRT_WARNING             2
#define PRT_ERROR               3
#define PRT_FATAL               4
#define PRT_EXIT                5

//bot AI library exported functions
typedef struct botlib_import_s
{
	//print messages from the bot library
	void		(QDECL *Print)(int type, const char *fmt, ...) FORMAT_PRINTF(2, 3);
	//memory allocation
	void        *( *GetMemory )( int size );
	void ( *FreeMemory )( void *ptr );
	//void ( *FreeZoneMemory )( void );
	//void        *( *HunkAlloc )( int size );
	//file system access
	int ( *FS_FOpenFile )( const char *qpath, fileHandle_t *file, fsMode_t mode );
	int ( *FS_Read )( void *buffer, int len, fileHandle_t f );
	int ( *FS_Write )( const void *buffer, int len, fileHandle_t f );
	void ( *FS_FCloseFile )( fileHandle_t f );
	int ( *FS_Seek )( fileHandle_t f, long offset, fsOrigin_t origin );
	//debug visualisation stuff
	int ( *DebugLineCreate )( void );
	void ( *DebugLineDelete )( int line );
	void ( *DebugLineShow )( int line, vec3_t start, vec3_t end, int color );
	//
	int ( *DebugPolygonCreate )( int color, int numPoints, vec3_t *points );
	bot_debugpoly_t*    ( *DebugPolygonGetFree )( void );
	void ( *DebugPolygonDelete )( int id );
	void ( *DebugPolygonDeletePointer )( bot_debugpoly_t* pPoly );
	//

	// Gordon: direct hookup into rendering, stop using this silly debugpoly faff
	void ( *BotDrawPolygon )( int color, int numPoints, float *points );
} botlib_import_t;

//bot AI library imported functions
typedef struct botlib_export_s
{
	//sets a C-like define returns BLERR_
	int (*PC_AddGlobalDefine)(const char *string);
	void ( *PC_RemoveAllGlobalDefines )( void );
	int ( *PC_LoadSourceHandle )( const char *filename );
	int ( *PC_FreeSourceHandle )( int handle );
	int ( *PC_ReadTokenHandle )( int handle, pc_token_t *pc_token );
	int ( *PC_SourceFileAndLine )( int handle, char *filename, int *line );
	void ( *PC_UnreadLastTokenHandle )( int handle );
} botlib_export_t;

//linking of bot library
botlib_export_t *GetBotLibAPI( int apiVersion, botlib_import_t *import );

#endif