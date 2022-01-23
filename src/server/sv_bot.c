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

// sv_bot.c

#include "server.h"
#include "../botlib/botlib.h"

static bot_debugpoly_t debugpolygons[MAX_DEBUGPOLYS];

extern botlib_export_t  *botlib_export;

/*
==================
SV_BotAllocateClient
==================
*/
int SV_BotAllocateClient( int clientNum ) {
	int i;
	client_t    *cl;

	// Arnout: added possibility to request a clientnum
	if ( clientNum > 0 ) {
		if ( clientNum >= sv_maxclients->integer ) {
			return -1;
		}

		cl = &svs.clients[clientNum];
		if ( cl->state != CS_FREE ) {
			return -1;
		} else {
			i = clientNum;
		}
	} else {
		// find a client slot
		for ( i = 0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++ ) {
			// Wolfenstein, never use the first slot, otherwise if a bot connects before the first client on a listen server, game won't start
			if ( i < 1 ) {
				continue;
			}
			// done.
			if ( cl->state == CS_FREE ) {
				break;
			}
		}
	}

	if ( i == sv_maxclients->integer ) {
		return -1;
	}

	cl->gentity = SV_GentityNum( i );
	cl->gentity->s.number = i;
	cl->state = CS_ACTIVE;
	cl->lastPacketTime = svs.time;
	cl->snapshotMsec = 1000 / sv_fps->integer;
	cl->netchan.remoteAddress.type = NA_BOT;
	cl->rate = 0;

	cl->tld[0] = '\0';
	cl->country = "BOT";

	return i;
}


/*
==================
SV_BotFreeClient
==================
*/
void SV_BotFreeClient( int clientNum ) {
	client_t	*cl;

	if ( (unsigned) clientNum >= sv_maxclients->integer ) {
		Com_Error( ERR_DROP, "SV_BotFreeClient: bad clientNum: %i", clientNum );
	}

	cl = &svs.clients[clientNum];
	cl->state = CS_FREE;
	cl->name[0] = '\0';
	if ( cl->gentity ) {
		cl->gentity->r.svFlags &= ~SVF_BOT;
	}
}


/*
==================
BotDrawDebugPolygons
==================
*/
void BotDrawDebugPolygons( BotPolyFunc drawPoly, int value ) {
}

/*
==================
BotImport_Print
==================
*/
static void FORMAT_PRINTF(2, 3) QDECL BotImport_Print( int type, const char *fmt, ... ) {
	char str[2048];
	va_list ap;

	va_start( ap, fmt );
	Q_vsnprintf( str, sizeof( str ), fmt, ap );
	va_end( ap );

	switch ( type ) {
	case PRT_MESSAGE: {
		Com_Printf( "%s", str );
		break;
	}
	case PRT_WARNING: {
		Com_Printf( S_COLOR_YELLOW "Warning: %s", str );
		break;
	}
	case PRT_ERROR: {
		Com_Printf( S_COLOR_RED "Error: %s", str );
		break;
	}
	case PRT_FATAL: {
		Com_Printf( S_COLOR_RED "Fatal: %s", str );
		break;
	}
	case PRT_EXIT: {
		Com_Error( ERR_DROP, S_COLOR_RED "Exit: %s", str );
		break;
	}
	default: {
		Com_Printf( "unknown print type\n" );
		break;
	}
	}
}

/*
==================
BotImport_GetMemory
==================
*/
static void *BotImport_GetMemory( int size ) {
	return Z_TagMalloc( size, TAG_BOTLIB );
}

/*
==================
BotImport_FreeMemory
==================
*/
static void BotImport_FreeMemory( void *ptr ) {
	Z_Free( ptr );
}

/*
==================
BotImport_FreeZoneMemory
==================
*/
/*void BotImport_FreeZoneMemory( void ) {
	Z_FreeTags( TAG_BOTLIB );
}*/

/*
=================
BotImport_HunkAlloc
=================
*/
/*static void *BotImport_HunkAlloc( int size ) {
	if ( Hunk_CheckMark() ) {
		Com_Error( ERR_DROP, "%s(): Alloc with marks already set", __func__ );
	}
	return Hunk_Alloc( size, h_high );
}*/

/*
==================
BotImport_DebugPolygonCreate
==================
*/
int BotImport_DebugPolygonCreate( int color, int numPoints, vec3_t *points ) {
	bot_debugpoly_t *poly;
	int i;

	for ( i = 1; i < MAX_DEBUGPOLYS; i++ )    {
		if ( !debugpolygons[i].inuse ) {
			break;
		}
	}
	if ( i >= MAX_DEBUGPOLYS ) {
		return 0;
	}
	poly = &debugpolygons[i];
	poly->inuse = qtrue;
	poly->color = color;
	poly->numPoints = numPoints;
	memcpy( poly->points, points, numPoints * sizeof( vec3_t ) );
	//
	return i;
}

/*
==================
BotImport_DebugPolygonCreate
==================
*/
bot_debugpoly_t* BotImport_GetFreeDebugPolygon( void ) {
	int i;

	for ( i = 1; i < MAX_DEBUGPOLYS; i++ )    {
		if ( !debugpolygons[i].inuse ) {
			return &debugpolygons[i];
		}
	}
	return NULL;
}

/*
==================
BotImport_DebugPolygonShow
==================
*/
static void BotImport_DebugPolygonShow( int id, int color, int numPoints, vec3_t *points ) {
	bot_debugpoly_t *poly;

	poly = &debugpolygons[id];
	poly->inuse = qtrue;
	poly->color = color;
	poly->numPoints = numPoints;
	memcpy( poly->points, points, numPoints * sizeof( vec3_t ) );
}

/*
==================
BotImport_DebugPolygonDelete
==================
*/
void BotImport_DebugPolygonDelete( int id ) {
	debugpolygons[id].inuse = qfalse;
}

/*
==================
BotImport_DebugPolygonDeletePointer
==================
*/
void BotImport_DebugPolygonDeletePointer( bot_debugpoly_t* pPoly ) {
	pPoly->inuse = qfalse;
}

/*
==================
BotImport_DebugLineCreate
==================
*/
static int BotImport_DebugLineCreate( void ) {
	vec3_t points[1];
	return BotImport_DebugPolygonCreate( 0, 0, points );
}

/*
==================
BotImport_DebugLineDelete
==================
*/
static void BotImport_DebugLineDelete( int line ) {
	BotImport_DebugPolygonDelete( line );
}

/*
==================
BotImport_DebugLineShow
==================
*/
static void BotImport_DebugLineShow( int line, vec3_t start, vec3_t end, int color ) {
	vec3_t points[4], dir, cross, up = {0, 0, 1};
	float dot;

	VectorCopy( start, points[0] );
	VectorCopy( start, points[1] );
	//points[1][2] -= 2;
	VectorCopy( end, points[2] );
	//points[2][2] -= 2;
	VectorCopy( end, points[3] );


	VectorSubtract( end, start, dir );
	VectorNormalize( dir );
	dot = DotProduct( dir, up );
	if ( dot > 0.99 || dot < -0.99 ) {
		VectorSet( cross, 1, 0, 0 );
	} else { CrossProduct( dir, up, cross );}

	VectorNormalize( cross );

	VectorMA( points[0], 2, cross, points[0] );
	VectorMA( points[1], -2, cross, points[1] );
	VectorMA( points[2], -2, cross, points[2] );
	VectorMA( points[3], 2, cross, points[3] );

	BotImport_DebugPolygonShow( line, color, 4, points );
}

/*
==================
SV_BotFrame
==================
*/
/*void SV_BotFrame( int time ) {
	if ( !bot_enable ) {
		return;
	}
	//NOTE: maybe the game is already shutdown
	if ( !gvm ) {
		return;
	}
	VM_Call( gvm, 1, BOTAI_START_FRAME, time );
}*/


#ifndef DEDICATED
void BotImport_DrawPolygon( int color, int numpoints, float* points );
#else
/*
==================
BotImport_DrawPolygon
==================
*/
void BotImport_DrawPolygon( int color, int numpoints, float* points ) {
	Com_DPrintf( "BotImport_DrawPolygon stub\n" );
}
#endif

/*
==================
SV_BotInitBotLib
==================
*/
void SV_BotInitBotLib( void ) {
	botlib_import_t botlib_import;

	botlib_import.Print = BotImport_Print;

	//memory management
	botlib_import.GetMemory = BotImport_GetMemory;
	botlib_import.FreeMemory = BotImport_FreeMemory;
	//botlib_import.FreeZoneMemory = BotImport_FreeZoneMemory;
	//botlib_import.HunkAlloc = BotImport_HunkAlloc;

	// file system acess
	botlib_import.FS_FOpenFile = FS_FOpenFileByMode;
	botlib_import.FS_Read = FS_Read;
	botlib_import.FS_Write = FS_Write;
	botlib_import.FS_FCloseFile = FS_FCloseFile;
	botlib_import.FS_Seek = FS_Seek;

	//debug lines
	botlib_import.DebugLineCreate = BotImport_DebugLineCreate;
	botlib_import.DebugLineDelete = BotImport_DebugLineDelete;
	botlib_import.DebugLineShow = BotImport_DebugLineShow;

	//debug polygons
	botlib_import.DebugPolygonCreate =          BotImport_DebugPolygonCreate;
	botlib_import.DebugPolygonGetFree =         BotImport_GetFreeDebugPolygon;
	botlib_import.DebugPolygonDelete =          BotImport_DebugPolygonDelete;
	botlib_import.DebugPolygonDeletePointer =   BotImport_DebugPolygonDeletePointer;

	botlib_import.BotDrawPolygon =      BotImport_DrawPolygon;

	// singleplayer check
	// Arnout: no need for this
	//botlib_import.BotGameIsSinglePlayer = SV_GameIsSinglePlayer;

	botlib_export = (botlib_export_t *)GetBotLibAPI( BOTLIB_API_VERSION, &botlib_import );
}


//
//  * * * BOT AI CODE IS BELOW THIS POINT * * *
//

/*
==================
SV_BotGetConsoleMessage
==================
*/
int SV_BotGetConsoleMessage( int client, char *buf, int size ) {
	if ( (unsigned) client < sv_maxclients->integer ) {
		client_t* cl;
		int index;

		cl = &svs.clients[client];
		cl->lastPacketTime = svs.time;

		if ( cl->reliableAcknowledge == cl->reliableSequence ) {
			return qfalse;
		}

		cl->reliableAcknowledge++;
		index = cl->reliableAcknowledge & ( MAX_RELIABLE_COMMANDS - 1 );

		if ( !cl->reliableCommands[index][0] ) {
			return qfalse;
		}

		//Q_strncpyz( buf, cl->reliableCommands[index], size );
		return qtrue;
	} else {
		return qfalse;
	}
}


#if 0
/*
==================
EntityInPVS
==================
*/
int EntityInPVS( int client, int entityNum ) {
	client_t			*cl;
	clientSnapshot_t	*frame;
	int					i;

	cl = &svs.clients[client];
	frame = &cl->frames[cl->netchan.outgoingSequence & PACKET_MASK];
	for ( i = 0; i < frame->num_entities; i++ )	{
		if ( svs.snapshotEntities[(frame->first_entity + i) % svs.numSnapshotEntities].number == entityNum ) {
			return qtrue;
		}
	}
	return qfalse;
}
#endif


/*
==================
SV_BotGetSnapshotEntity
==================
*/
int SV_BotGetSnapshotEntity( int client, int sequence ) {
	if ( (unsigned) client < sv_maxclients->integer ) {
		const client_t* cl = &svs.clients[client];
		const clientSnapshot_t* frame = &cl->frames[cl->netchan.outgoingSequence & PACKET_MASK];
		if ( (unsigned) sequence >= frame->num_entities ) {
			return -1;
		}
		return frame->ents[sequence]->number;
	} else {
		return -1;
	}
}
