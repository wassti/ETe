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


#include "server.h"


/*
=============================================================================

Delta encode a client frame onto the network channel

A normal server packet will look like:

4	sequence number (high bit set if an oversize fragment)
<optional reliable commands>
1	svc_snapshot
4	last client reliable command
4	serverTime
1	lastframe for delta compression
1	snapFlags
1	areaBytes
<areabytes>
<playerstate>
<packetentities>

=============================================================================
*/

/*
=============
SV_EmitPacketEntities

Writes a delta update of an entityState_t list to the message.
=============
*/
static void SV_EmitPacketEntities( const clientSnapshot_t *from, const clientSnapshot_t *to, msg_t *msg ) {
	entityState_t	*oldent, *newent;
	int		oldindex, newindex;
	int		oldnum, newnum;
	int		from_num_entities;

	// generate the delta update
	if ( !from ) {
		from_num_entities = 0;
	} else {
		from_num_entities = from->num_entities;
	}

	newent = NULL;
	oldent = NULL;
	newindex = 0;
	oldindex = 0;
	while ( newindex < to->num_entities || oldindex < from_num_entities ) {
		if ( newindex >= to->num_entities ) {
			newnum = MAX_GENTITIES+1;
		} else {
			newent = to->ents[ newindex ];
			newnum = newent->number;
		}

		if ( oldindex >= from_num_entities ) {
			oldnum = MAX_GENTITIES+1;
		} else {
			oldent = from->ents[ oldindex ];
			oldnum = oldent->number;
		}

		if ( newnum == oldnum ) {
			// delta update from old position
			// because the force parm is qfalse, this will not result
			// in any bytes being emitted if the entity has not changed at all
			MSG_WriteDeltaEntity (msg, oldent, newent, qfalse );
			oldindex++;
			newindex++;
			continue;
		}

		if ( newnum < oldnum ) {
			// this is a new entity, send it from the baseline
			MSG_WriteDeltaEntity (msg, &sv.svEntities[newnum].baseline, newent, qtrue );
			newindex++;
			continue;
		}

		if ( newnum > oldnum ) {
			// the old entity isn't present in the new message
			MSG_WriteDeltaEntity (msg, oldent, NULL, qtrue );
			oldindex++;
			continue;
		}
	}

	MSG_WriteBits( msg, (MAX_GENTITIES-1), GENTITYNUM_BITS );	// end of packetentities
}

 
#ifdef USE_MV
static int SV_GetMergeMaskEntities( clientSnapshot_t *snap )
{
	const entityState_t *ent;
	psFrame_t *psf;
	int skipMask;
	int i, n;

	n = 0;
	skipMask = 0;
	psf = NULL;

	if ( !svs.currFrame )
		return skipMask;
	
	for ( i = 0; i < sv_maxclients->integer; i++ ) {
		ent = svs.currFrame->ents[ i ];
		if ( ent->number >= sv_maxclients->integer )
			break;
		for ( /*n = 0 */; n < snap->num_psf; n++ ) {
			psf = &svs.snapshotPSF[ ( snap->first_psf + n ) % svs.numSnapshotPSF ];
			if ( psf->clientSlot == ent->number ) {
				skipMask |= MSG_PlayerStateToEntityStateXMask( &psf->ps, ent, qtrue );
			}
		}
		//if ( n >= snap->num_psf ) {
		//	Com_Error( ERR_DROP, "ent[%i] not found in psf array", ent->number );
		//	break;
		//}
	}
	return skipMask;
}


static void SV_EmitByteMask( msg_t *msg, const byte *mask, const int maxIndex, const int indexBits, qboolean ignoreFirstZero )
{
	int firstIndex;
	int lastIndex;

	for ( firstIndex = 0; firstIndex < maxIndex; firstIndex++ ) {
		if ( mask[ firstIndex ] ) {
			lastIndex = firstIndex;
			while ( lastIndex < maxIndex-1 ) {
				if ( mask[ lastIndex + 1 ] )
					lastIndex++;
				else if ( ignoreFirstZero && lastIndex < maxIndex-2 && mask[ lastIndex + 2 ] )
					lastIndex += 2; // skip single zero block
				else
					break;
			}
			//printf( "start: %i end: %i\n", firstIndex, lastIndex );
			MSG_WriteBits( msg, 1, 1 ); // delta change
			MSG_WriteBits( msg, firstIndex, indexBits );
			MSG_WriteBits( msg, lastIndex, indexBits );
			for ( ; firstIndex < lastIndex + 1 ; firstIndex++ ) {
				MSG_WriteByte( msg, mask[ firstIndex ] );
			}
			firstIndex = lastIndex;
		}
	}
	MSG_WriteBits( msg, 0, 1 ); // no delta
}


static void SV_EmitPlayerStates( int baseClientID, const clientSnapshot_t *from, const clientSnapshot_t *to, msg_t *msg, skip_mask sm )
{
	psFrame_t *psf;
	const psFrame_t *old_psf;
	const playerState_t *oldPs;

	int i, n;
	int clientSlot;
	int oldIndex;

	const byte *oldPsMask;
	byte oldPsMaskBuf[MAX_CLIENTS/8];
	byte newPsMask[MAX_CLIENTS/8];

	const byte *oldEntMask;
	byte oldEntMaskBuf[MAX_GENTITIES/8];
	byte newEntMask[MAX_GENTITIES/8];

	// generate playerstate mask
	if ( !from || !from->num_psf ) {
		Com_Memset( oldPsMaskBuf, 0, sizeof( oldPsMaskBuf ) );
		oldPsMask = oldPsMaskBuf;
	} else {
		oldPsMask = from->psMask;
	}

#if 1
	// delta-xor playerstate bitmask
	for ( i = 0; i < ARRAY_LEN( newPsMask ); i++ ) {
		newPsMask[ i ] = to->psMask[ i ] ^ oldPsMask[ i ];
	}
	SV_EmitByteMask( msg, newPsMask, MAX_CLIENTS/8, 3, qfalse );
#else
	MSG_WriteData( msg, to->psMask[ i ], sizeof( to->psMasks ) ); // direct playerstate mask
#endif

	oldIndex = 0;
	clientSlot = 0;
	old_psf = NULL; // silent warning

	for ( i = 0; i < to->num_psf; i++ ) 
	{
		psf = &svs.snapshotPSF[ ( to->first_psf + i ) % svs.numSnapshotPSF ];
		clientSlot = psf->clientSlot;
		// check if masked in previous frame:
		if ( !GET_ABIT( oldPsMask, clientSlot ) ) {
			if ( from && clientSlot == baseClientID ) // FIXME: ps->clientNum?
				oldPs = &from->ps; // transition from legacy to multiview mode
			else
				oldPs = NULL; // new playerstate
			// empty entity mask
			Com_Memset( oldEntMaskBuf, 0, sizeof( oldEntMaskBuf ) );
			oldEntMask = oldEntMaskBuf;
		} else {
			// masked in previous frame so MUST exist
			old_psf = NULL;
			 // search for client state in old frame
			for ( ; oldIndex < from->num_psf; oldIndex++ ) {
				old_psf = &svs.snapshotPSF[ ( from->first_psf + oldIndex ) % svs.numSnapshotPSF ];
				if ( old_psf->clientSlot == clientSlot )
					break;
			}
			if ( oldIndex >= from->num_psf ) { // should never happen?
				Com_Error( ERR_DROP, "oldIndex(%i) >= from->num_psf(%i), from->first_pfs=%i", oldIndex, from->num_psf, from->first_psf );
				continue;
			}
			oldPs = &old_psf->ps;
			oldEntMask = old_psf->entMask;
		}

		// areabytes
		MSG_WriteBits( msg, psf->areabytes, 6 ); // was 8
		MSG_WriteData( msg, psf->areabits, psf->areabytes );

		// playerstate
		MSG_WriteDeltaPlayerstate( msg, oldPs, &psf->ps );

#if 1
		// delta-xor mask
		for ( n = 0; n < ARRAY_LEN( newEntMask ); n++ ) {
			newEntMask[ n ] = psf->entMask[ n ] ^ oldEntMask[ n ];
		}
		SV_EmitByteMask( msg, newEntMask, sizeof( newEntMask ), 7, qtrue );
#else 
		// direct mask
		MSG_WriteData( msg, psf->entMask.mask, sizeof( psf->entMask.mask ) );
#endif
	}
}
#endif // USE_MV


/*
==================
SV_WriteSnapshotToClient
==================
*/
static void SV_WriteSnapshotToClient( client_t *client, msg_t *msg ) {
	const clientSnapshot_t	*oldframe;
	clientSnapshot_t	*frame;
	int					lastframe;
	int					i;
	int					snapFlags;

	// this is the snapshot we are creating
	frame = &client->frames[ client->netchan.outgoingSequence & PACKET_MASK ];

	// try to use a previous frame as the source for delta compressing the snapshot
	if ( client->deltaMessage <= 0 || client->state != CS_ACTIVE ) {
		// client is asking for a retransmit
		oldframe = NULL;
		lastframe = 0;
	} else if ( client->netchan.outgoingSequence - client->deltaMessage
				>= ( PACKET_BACKUP - 3 ) ) {
		// client hasn't gotten a good message through in a long time
		Com_DPrintf( "%s: Delta request from out of date packet.\n", client->name );
		oldframe = NULL;
		lastframe = 0;
	} else {
		// we have a valid snapshot to delta from
		oldframe = &client->frames[ client->deltaMessage & PACKET_MASK ];
		lastframe = client->netchan.outgoingSequence - client->deltaMessage;
		// we may refer on outdated frame
		if ( svs.lastValidFrame > oldframe->frameNum ) {
			Com_DPrintf( "%s: Delta request from out of date frame.\n", client->name );
			oldframe = NULL;
			lastframe = 0;
		}
#ifdef USE_MV
		else if ( frame->multiview && oldframe->first_psf <= svs.nextSnapshotPSF - svs.numSnapshotPSF ) {
			Com_DPrintf( "%s: Delta request from out of date playerstate.\n", client->name );
			oldframe = NULL;
			lastframe = 0;
		}
#endif
	}

#ifdef USE_MV
	if ( frame->multiview )
		MSG_WriteByte( msg, svc_multiview );
	else
#endif
	MSG_WriteByte (msg, svc_snapshot);

	// NOTE, MRE: now sent at the start of every message from server to client
	// let the client know which reliable clientCommands we have received
	//MSG_WriteLong( msg, client->lastClientCommand );

	// send over the current server time so the client can drift
	// its view of time to try to match
	if( client->oldServerTime ) {
		// The server has not yet got an acknowledgement of the
		// new gamestate from this client, so continue to send it
		// a time as if the server has not restarted. Note from
		// the client's perspective this time is strictly speaking
		// incorrect, but since it'll be busy loading a map at
		// the time it doesn't really matter.
		MSG_WriteLong (msg, sv.time + client->oldServerTime);
	} else {
		MSG_WriteLong (msg, sv.time);
	}

	// what we are delta'ing from
	MSG_WriteByte (msg, lastframe);

	snapFlags = svs.snapFlagServerBit;
	if ( client->rateDelayed ) {
		snapFlags |= SNAPFLAG_RATE_DELAYED;
	}
	if ( client->state != CS_ACTIVE ) {
		snapFlags |= SNAPFLAG_NOT_ACTIVE;
	}

	MSG_WriteByte( msg, snapFlags );

#ifdef USE_MV
	if ( frame->multiview ) {
		int newmask;
		int oldmask;
		int	oldversion;

		frame->version = MV_PROTOCOL_VERSION;

		if ( !oldframe || !oldframe->multiview ) {
			oldversion = 0;
			oldmask = 0;
		} else {
			oldversion = oldframe->version;
			oldmask = oldframe->mergeMask;
		}

		// emit protocol version in first message
		if ( oldversion != frame->version ) {
			MSG_WriteBits( msg, 1, 1 );
			MSG_WriteByte( msg, frame->version );
		} else {
			MSG_WriteBits( msg, 0, 1 );
		}
		
		newmask = SM_ALL & ~SV_GetMergeMaskEntities( frame );

		// emit skip-merge mask
		if ( oldmask != newmask ) {
			MSG_WriteBits( msg, 1, 1 );
			MSG_WriteBits( msg, newmask, SM_BITS );
		} else {
			MSG_WriteBits( msg, 0, 1 );
		}

		frame->mergeMask = newmask;

		SV_EmitPlayerStates( client - svs.clients, oldframe, frame, msg, newmask );
		MSG_entMergeMask = newmask; // emit packet entities with skipmask
		SV_EmitPacketEntities( oldframe, frame, msg );
		MSG_entMergeMask = 0; // don't forget to reset that! 
	} else {
#endif

	// send over the areabits
	MSG_WriteByte( msg, frame->areabytes );
	MSG_WriteData( msg, frame->areabits, frame->areabytes );

	// don't send any changes to zombies
	/*if ( client->state <= CS_ZOMBIE ) {
		// playerstate
		MSG_WriteByte( msg, 0 ); // # of changes
		MSG_WriteBits( msg, 0, 1 ); // no array changes
		// packet entities
		MSG_WriteBits( msg, (MAX_GENTITIES-1), GENTITYNUM_BITS );
		return;
	}*/

	{
//		int sz = msg->cursize;
//		int usz = msg->uncompsize;

		// delta encode the playerstate
		if ( oldframe ) {
			MSG_WriteDeltaPlayerstate( msg, &oldframe->ps, &frame->ps );
		} else {
			MSG_WriteDeltaPlayerstate( msg, NULL, &frame->ps );
		}

//		Com_Printf( "Playerstate delta size: %f\n", ((msg->cursize - sz) * sv_fps->integer) / 8.f );
	}

	// delta encode the entities
	SV_EmitPacketEntities( oldframe, frame, msg );
#ifdef USE_MV
	} // !client->MVProtocol
#endif

	// padding for rate debugging
	if ( sv_padPackets->integer ) {
		for ( i = 0 ; i < sv_padPackets->integer ; i++ ) {
			MSG_WriteByte( msg, svc_nop );
		}
	}
}


#if defined( USE_MV ) && defined( USE_MV_ZCMD )

static int SV_GetTextBits( const byte *cmd, int cmdlen ) {
	int n;
	for ( n = 0; n < cmdlen; n++ ) {
		if ( cmd[ n ] > 127 ) {
			return 8;
		}
	}
	return 7;
}


static int SV_GetCmdSize( int Cmd ) 
{
	if ( (unsigned)Cmd <= 0xFF ) {
		return 1;
	} else if ( (unsigned)Cmd <= 0xFFFF ) {
		return 2;
	} else if ( (unsigned)Cmd <= 0xFFFFFF ) {
		return 3;
	} else {
		return 4;
	}
}


static qboolean SV_BuildCompressedBuffer( client_t *client, int reliableSequence )
{
	int index;
	int cmdLen;
	const char *cmd;
	lzstream_t *stream;

	index = reliableSequence & (MAX_RELIABLE_COMMANDS-1);

	if ( client->multiview.z.stream[ index ].zcommandNum == reliableSequence )
		return qfalse; // already compressed

	//Com_DPrintf( S_COLOR_YELLOW "zcmd: compressing %i.%i\n", reliableSequence, client->multiview.z.deltaSeq );

	cmd = client->reliableCommands[ index ];
	cmdLen = strlen( cmd );

	if ( client->multiview.z.deltaSeq == 0 ) {
		LZSS_InitContext( &client->multiview.z.ctx );
	} 

	stream = &client->multiview.z.stream[ index ];
	stream->zdelta = client->multiview.z.deltaSeq;
	stream->zcommandNum = reliableSequence;
	stream->zcommandSize = SV_GetCmdSize( reliableSequence );
	stream->zcharbits = SV_GetTextBits( (byte*)cmd, cmdLen );
	/* stream->count = */ LZSS_CompressToStream( &client->multiview.z.ctx, stream, (byte*)cmd, cmdLen );

	// don't forget to update delta sequence
	if ( client->multiview.z.deltaSeq >= 7 )
		client->multiview.z.deltaSeq = 1;
	else
		client->multiview.z.deltaSeq++;

	return qtrue;
}
#endif // USE_MV


/*
==================
SV_UpdateServerCommandsToClient

(re)send all server commands the client hasn't acknowledged yet
==================
*/
void SV_UpdateServerCommandsToClient( client_t *client, msg_t *msg ) {
	int i;

#ifdef USE_MV
	if ( client->multiview.protocol /*&& client->state >= CS_CONNECTED*/ ) {

		if ( client->multiview.recorder ) {
			// forward target client commands to recorder slot
			SV_ForwardServerCommands( client ); // TODO: forward all clients?
		}

		if ( client->reliableAcknowledge >= client->reliableSequence ) {
#ifdef USE_MV_ZCMD
			// nothing to send, reset compression sequences
			for ( i = 0; i < MAX_RELIABLE_COMMANDS; i++ )
				client->multiview.z.stream[ i ].zcommandNum = -1;
#endif
			//client->reliableSent = client->reliableSequence;
			client->reliableSent = -1;
			return;
		}

		// write any unacknowledged serverCommands
		for ( i = client->reliableAcknowledge + 1 ; i <= client->reliableSequence ; i++ ) {
#if defined( USE_MV ) && defined( USE_MV_ZCMD )
			// !!! do not start compression sequence from already sent uncompressed commands
			// (re)send them uncompressed and only after that initiate compression sequence
			if ( i <= client->reliableSent ) {
				MSG_WriteByte( msg, svc_serverCommand );
				MSG_WriteLong( msg, i );
				MSG_WriteString( msg, client->reliableCommands[ i & (MAX_RELIABLE_COMMANDS-1) ] );
			} else{
				// build new compressed stream or re-send existing
				SV_BuildCompressedBuffer( client, i );
				MSG_WriteLZStream( msg, &client->multiview.z.stream[ i & (MAX_RELIABLE_COMMANDS-1) ] );
				// TODO: indicate compressedSent?
			}
#else
			MSG_WriteByte( msg, svc_serverCommand );
			MSG_WriteLong( msg, i );
			MSG_WriteString( msg, client->reliableCommands[ i & (MAX_RELIABLE_COMMANDS-1) ] );
#endif
		}

		// recorder operations always success:
		if ( client->multiview.recorder )
			client->reliableAcknowledge = client->reliableSequence;
		client->multiview.lastRecvTime = svs.time;
		// TODO: indicate compressedSent?
		//client->reliableSent = client->reliableSequence;
		return;
	}
#ifdef USE_MV_ZCMD
	// reset on inactive/non-multiview
	client->multiview.z.deltaSeq = 0;
#endif
#endif // USE_MV

	// write any unacknowledged serverCommands
	for ( i = client->reliableAcknowledge + 1 ; i <= client->reliableSequence ; i++ ) {
		MSG_WriteByte( msg, svc_serverCommand );
		MSG_WriteLong( msg, i );
		MSG_WriteString( msg, client->reliableCommands[ i & ( MAX_RELIABLE_COMMANDS - 1 ) ] );
	}
	client->reliableSent = client->reliableSequence;

#ifdef USE_MV
	if ( client->reliableSequence > client->reliableAcknowledge ) {
		client->multiview.lastRecvTime = svs.time;
	}
#endif
}

/*
=============================================================================

Build a client snapshot structure

=============================================================================
*/


typedef int entityNum_t;
typedef struct {
	int		numSnapshotEntities;
	entityNum_t	snapshotEntities[ MAX_SNAPSHOT_ENTITIES ];
	qboolean unordered;
} snapshotEntityNumbers_t;


typedef struct clientPVS_s {
	int		snapshotFrame; // svs.snapshotFrame

	int		clientNum;
	int		areabytes;
	byte	areabits[MAX_MAP_AREA_BYTES];		// portalarea visibility bits
	snapshotEntityNumbers_t	numbers;

	byte	entMask[MAX_GENTITIES/8];
	qboolean entMaskBuilt;

} clientPVS_t;

static clientPVS_t client_pvs[ MAX_CLIENTS ];


/*
=============
SV_SortEntityNumbers

Insertion sort is about 10 times faster than quicksort for our task
=============
*/
static void SV_SortEntityNumbers( entityNum_t *num, const int size ) {
	entityNum_t tmp;
	int i, d;
	for ( i = 1 ; i < size; i++ ) {
		d = i;
		while ( d > 0 && num[d] < num[d-1] ) {
			tmp = num[d];
			num[d] = num[d-1];
			num[d-1] = tmp;
			d--;
		}
	}
#ifndef USE_MV
	// consistency check for delta encoding
	for ( i = 1 ; i < size; i++ ) {
		if ( num[i-1] >= num[i] ) {
			Com_Error( ERR_DROP, "%s: invalid entity number %i", __func__, num[ i ] );
		}
	}
#endif
}


static int SV_GetIndexByEntityNum( int num )
{
	const snapshotFrame_t *sf;
	int i;
	
	sf = svs.currFrame;

	for ( i = 0; i < sf->count; i++ ) {
		if ( sf->ents[i]->number == num ) {
			return i;
		}
	}

	return -1;
}


/*
===============
SV_AddIndexToSnapshot
===============
*/
static void SV_AddIndexToSnapshot( const sharedEntity_t *clientEnt, svEntity_t *svEnt, int index, snapshotEntityNumbers_t *eNums ) {

	svEnt->snapshotCounter = sv.snapshotCounter;

	// if we are full, silently discard entities
	if ( eNums->numSnapshotEntities >= MAX_SNAPSHOT_ENTITIES ) {
		return;
	}

	{
		sharedEntity_t *gEnt = SV_GEntityForSvEntity( svEnt );
		if ( gEnt->r.snapshotCallback ) {
			if ( !SV_GameSnapshotCallback( gEnt->s.number, clientEnt->s.number ) ) {
				return;
			}
		}
	}

	eNums->snapshotEntities[ eNums->numSnapshotEntities ] = index;
	eNums->numSnapshotEntities++;
}


/*
===============
SV_AddEntitiesVisibleFromPoint
===============
*/
static void SV_AddEntitiesVisibleFromPoint( const vec3_t origin, clientPVS_t *pvs ) {
	int e, i;
	sharedEntity_t *ent, *playerEnt;
	svEntity_t  *svEnt;
	entityState_t  *es;
	int l;
	int clientarea, clientcluster;
	int leafnum;
	byte    *clientpvs;
	byte    *bitvector;

	// during an error shutdown message we may need to transmit
	// the shutdown message after the server has shutdown, so
	// specfically check for it
	if ( sv.state == SS_DEAD ) {
		return;
	}

	leafnum = CM_PointLeafnum (origin);
	clientarea = CM_LeafArea (leafnum);
	clientcluster = CM_LeafCluster (leafnum);

	// calculate the visible areas
	pvs->areabytes = CM_WriteAreaBits( pvs->areabits, clientarea );

	clientpvs = CM_ClusterPVS (clientcluster);

	playerEnt = SV_GentityNum( pvs->clientNum );
	if ( playerEnt->r.svFlags & SVF_SELF_PORTAL ) {
		pvs->numbers.unordered = qtrue;
		SV_AddEntitiesVisibleFromPoint( playerEnt->s.origin2, pvs );
	}

	for ( e = 0 ; e < svs.currFrame->count; e++ ) {
		es = svs.currFrame->ents[ e ];
		ent = SV_GentityNum( es->number );

		// entities can be flagged to be sent to only one client
		if ( ent->r.svFlags & SVF_SINGLECLIENT ) {
			if ( ent->r.singleClient != pvs->clientNum ) {
				continue;
			}
		}
		// entities can be flagged to be sent to everyone but one client
		if ( ent->r.svFlags & SVF_NOTSINGLECLIENT ) {
			if ( ent->r.singleClient == pvs->clientNum ) {
				continue;
			}
		}

		svEnt = &sv.svEntities[ es->number ];

		// don't double add an entity through portals
		if ( svEnt->snapshotCounter == sv.snapshotCounter ) {
			continue;
		}

		// broadcast entities are always sent
		if ( ent->r.svFlags & SVF_BROADCAST ) {
			SV_AddIndexToSnapshot( playerEnt, svEnt, e, &pvs->numbers );
			continue;
		}

		bitvector = clientpvs;

		// Gordon: just check origin for being in pvs, ignore bmodel extents
		if (ent->r.svFlags & SVF_IGNOREBMODELEXTENTS) {
			if (bitvector[svEnt->originCluster >> 3] & (1 << (svEnt->originCluster & 7))) {
				//SV_AddEntToSnapshot( playerEnt, svEnt, ent, eNums );
				SV_AddIndexToSnapshot( playerEnt, svEnt, e, &pvs->numbers );
			}
			continue;
		}

		// ignore if not touching a PV leaf
		// check area
		if ( !CM_AreasConnected( clientarea, svEnt->areanum ) ) {
			// doors can legally straddle two areas, so
			// we may need to check another one
			if ( !CM_AreasConnected( clientarea, svEnt->areanum2 ) ) {
				continue; // blocked by a door
			}
		}

		// check individual leafs
		if ( !svEnt->numClusters ) {
			continue;
		}
		l = 0;
		for ( i = 0 ; i < svEnt->numClusters ; i++ ) {
			l = svEnt->clusternums[i];
			if ( bitvector[l >> 3] & ( 1 << ( l & 7 ) ) ) {
				break;
			}
		}

		// if we haven't found it to be visible,
		// check overflow clusters that coudln't be stored
		if ( i == svEnt->numClusters ) {
			if ( svEnt->lastCluster ) {
				for ( ; l <= svEnt->lastCluster ; l++ ) {
					if ( bitvector[l >> 3] & ( 1 << ( l & 7 ) ) ) {
						break;
					}
				}
				if ( l == svEnt->lastCluster ) {
					continue;	// not visible
				}
			} else {
				continue;
			}
		}


		//----(SA) added "visibility dummies"
		if ( ent->r.svFlags & SVF_VISDUMMY ) {
			sharedEntity_t *ment;

			//find master;
			ment = SV_GentityNum( ent->s.otherEntityNum );
			if ( ment ) {
				svEntity_t *master;
				int index;

				master = SV_SvEntityForGentity( ment );
				if ( master->snapshotCounter == sv.snapshotCounter || !ment->r.linked ) {
					continue;
				}

				//SV_AddEntToSnapshot( playerEnt, master, ment, eNums );
				index = SV_GetIndexByEntityNum( ment->s.number );
				if ( index >= 0 ) {
					SV_AddIndexToSnapshot( playerEnt, master, index, &pvs->numbers );
					pvs->numbers.unordered = qtrue;
				}
			}
			continue;   // master needs to be added, but not this dummy ent
		}
		//----(SA) end
		else if ( ent->r.svFlags & SVF_VISDUMMY_MULTIPLE ) {
			{
				int h;
				sharedEntity_t *ment = 0;
				svEntity_t *master = 0;

				for ( h = 0; h < sv.num_entities; h++ )
				{
					ment = SV_GentityNum( h );

					if ( ment == ent ) {
						continue;
					}

					if ( ment ) {
						master = SV_SvEntityForGentity( ment );
					} else {
						continue;
					}

					if ( !( ment->r.linked ) ) {
						continue;
					}

					if ( ment->s.number != h ) {
						Com_DPrintf( "FIXING vis dummy multiple ment->S.NUMBER!!!\n" );
						ment->s.number = h;
					}

					if ( ment->r.svFlags & SVF_NOCLIENT ) {
						continue;
					}

					if ( master->snapshotCounter == sv.snapshotCounter ) {
						continue;
					}

					if ( ment->s.otherEntityNum == ent->s.number ) {
						int index;

						//SV_AddEntToSnapshot( playerEnt, master, ment, eNums );
						index = SV_GetIndexByEntityNum( ment->s.number );
						if ( index >= 0 ) {
							SV_AddIndexToSnapshot( playerEnt, master, index, &pvs->numbers );
							pvs->numbers.unordered = qtrue;
						}
					}
				}
				continue;
			}
		}

		// add it
		SV_AddIndexToSnapshot( playerEnt, svEnt, e, &pvs->numbers );

		// if it's a portal entity, add everything visible from its camera position
		if ( ent->r.svFlags & SVF_PORTAL ) {
			pvs->numbers.unordered = qtrue;
			SV_AddEntitiesVisibleFromPoint( ent->s.origin2, pvs );
		}
	}
}


/*
===============
SV_InitSnapshotStorage
===============
*/
void SV_InitSnapshotStorage( void ) 
{
	// initialize snapshot storage
	Com_Memset( svs.snapFrames, 0, sizeof( svs.snapFrames ) );
	svs.freeStorageEntities = svs.numSnapshotEntities;
	svs.currentStoragePosition = 0;

	svs.snapshotFrame = 0;
	svs.currentSnapshotFrame = 0;
	svs.lastValidFrame = 0;

	svs.currFrame = NULL;

	Com_Memset( client_pvs, 0, sizeof( client_pvs ) );
}


/*
===============
SV_IssueNewSnapshot

This should be called before any new client snaphot built
===============
*/
void SV_IssueNewSnapshot( void ) 
{
	svs.currFrame = NULL;
	
	// value that clients can use even for their empty frames
	// as it will not increment on new snapshot built
	svs.currentSnapshotFrame = svs.snapshotFrame;
}


/*
===============
SV_BuildCommonSnapshot

This always allocates new common snapshot frame
===============
*/
static void SV_BuildCommonSnapshot( void ) 
{
	sharedEntity_t	*list[ MAX_GENTITIES ];
	sharedEntity_t	*ent;
	
	snapshotFrame_t	*tmp;
	snapshotFrame_t	*sf;

	int count;
	int index;
	int	num;
	int i;

	count = 0;

	// gather all linked entities
	if ( sv.state != SS_DEAD ) {
		for ( num = 0 ; num < sv.num_entities ; num++ ) {
			ent = SV_GentityNum( num );

			// never send entities that aren't linked in
			if ( !ent->r.linked ) {
				continue;
			}
	
			if ( ent->s.number != num ) {
				Com_DPrintf( "FIXING ENT->S.NUMBER %i => %i\n", ent->s.number, num );
				ent->s.number = num;
			}

			// entities can be flagged to explicitly not be sent to the client
			if ( ent->r.svFlags & SVF_NOCLIENT ) {
				continue;
			}

			// Gordon: just check origin for being in pvs, ignore bmodel extents
			//if ( ent->r.svFlags & SVF_IGNOREBMODELEXTENTS ) {
				//if (!(bitvector[svEnt->originCluster >> 3] & (1 << (svEnt->originCluster & 7))) ) {
				//	continue;
				//}
			//}

			list[ count++ ] = ent;
			sv.svEntities[ num ].snapshotCounter = -1;
		}
	}

	sv.snapshotCounter = -1;

	sf = &svs.snapFrames[ svs.snapshotFrame % NUM_SNAPSHOT_FRAMES ];
	
	// track last valid frame
	if ( svs.snapshotFrame - svs.lastValidFrame > (NUM_SNAPSHOT_FRAMES-1) ) {
		svs.lastValidFrame = svs.snapshotFrame - (NUM_SNAPSHOT_FRAMES-1);
		// release storage
		svs.freeStorageEntities += sf->count;
		sf->count = 0;
	}

	// release more frames if needed
	while ( svs.freeStorageEntities < count && svs.lastValidFrame != svs.snapshotFrame ) {
		tmp = &svs.snapFrames[ svs.lastValidFrame % NUM_SNAPSHOT_FRAMES ];
		svs.lastValidFrame++;
		// release storage
		svs.freeStorageEntities += tmp->count;
		tmp->count = 0;
	}

	// should never happen but anyway
	if ( svs.freeStorageEntities < count ) {
		Com_Error( ERR_DROP, "Not enough snapshot storage: %i < %i", svs.freeStorageEntities, count );
	}

	// allocate storage
	sf->count = count;
	svs.freeStorageEntities -= count;

	sf->start = svs.currentStoragePosition; 
	svs.currentStoragePosition = ( svs.currentStoragePosition + count ) % svs.numSnapshotEntities;

	sf->frameNum = svs.snapshotFrame;
	svs.snapshotFrame++;

	svs.currFrame = sf; // clients can refer to this

	// setup start index
	index = sf->start;
	for ( i = 0 ; i < count ; i++, index = (index+1) % svs.numSnapshotEntities ) {
		//index %= svs.numSnapshotEntities;
		svs.snapshotEntities[ index ] = list[ i ]->s;
		sf->ents[ i ] = &svs.snapshotEntities[ index ];
	}
}


static clientPVS_t *SV_BuildClientPVS( int clientSlot, const playerState_t *ps, qboolean buildEntityMask ) 
{
	svEntity_t	*svEnt;
	clientPVS_t	*pvs;
	vec3_t	org;
	int i;
	
	pvs = &client_pvs[ clientSlot ];

	if ( pvs->snapshotFrame != svs.snapshotFrame /*|| pvs->clientNum != ps->clientNum*/ ) {
		pvs->snapshotFrame = svs.snapshotFrame;

		if ( svs.clients[ clientSlot ].gentity && svs.clients[ clientSlot ].gentity->r.svFlags & SVF_SELF_PORTAL_EXCLUSIVE ) {
			// find the client's viewpoint
			VectorCopy( svs.clients[ clientSlot ].gentity->s.origin2, org );
		} else {
			VectorCopy( ps->origin, org );
		}
		org[2] += ps->viewheight;

//----(SA)	added for 'lean'
		// need to account for lean, so areaportal doors draw properly
		if ( ps->leanf != 0 ) {
			vec3_t right, v3ViewAngles;
			VectorCopy( ps->viewangles, v3ViewAngles );
			v3ViewAngles[2] += ps->leanf / 2.0f;
			AngleVectors( v3ViewAngles, NULL, right, NULL );
			VectorMA( org, ps->leanf, right, org );
		}
//----(SA)	end

		// bump the counter used to prevent double adding
		sv.snapshotCounter++;

		// never send client's own entity, because it can
		// be regenerated from the playerstate
		svEnt = &sv.svEntities[ ps->clientNum ];
		svEnt->snapshotCounter = sv.snapshotCounter;

		// add all the entities directly visible to the eye, which
		// may include portal entities that merge other viewpoints
		pvs->clientNum = ps->clientNum;
		pvs->areabytes = 0;
		memset( pvs->areabits, 0, sizeof ( pvs->areabits ) );

		// empty entities before visibility check
		pvs->entMaskBuilt = qfalse;
		pvs->numbers.numSnapshotEntities = 0;
		pvs->numbers.unordered = qfalse;
		SV_AddEntitiesVisibleFromPoint( org, pvs );
		// if there were portals visible, there may be out of order entities
		// in the list which will need to be resorted for the delta compression
		// to work correctly.  This also catches the error condition
		// of an entity being included twice.
		if ( pvs->numbers.unordered ) {
			SV_SortEntityNumbers( &pvs->numbers.snapshotEntities[0], pvs->numbers.numSnapshotEntities );
		}

		// now that all viewpoint's areabits have been OR'd together, invert
		// all of them to make it a mask vector, which is what the renderer wants
		for ( i = 0 ; i < MAX_MAP_AREA_BYTES/sizeof(int) ; i++ ) {
			((int *)pvs->areabits)[i] = ((int *)pvs->areabits)[i] ^ -1;
		}
	}

	if ( buildEntityMask && !pvs->entMaskBuilt ) {
		pvs->entMaskBuilt = qtrue;
		memset( pvs->entMask, 0, sizeof ( pvs->entMask ) );
		for ( i = 0; i < pvs->numbers.numSnapshotEntities ; i++ ) {
			SET_ABIT( pvs->entMask, svs.currFrame->ents[ pvs->numbers.snapshotEntities[ i ] ]->number );
		}
	}

	return pvs;
}


#ifdef USE_MV
/*
==================
SV_FindActiveClient
+
find first human client we can use as primary/score requester
bots is not good for that because they may not receive all feedback from game VM
==================
*/
int SV_FindActiveClient( qboolean checkCommands, int skipClientNum, int minActive ) {
	playerState_t *ps;
	client_t *clist[ MAX_CLIENTS ];
	client_t *cl;
	int	longestInactivity;
	int	longestSpecInactivity;
	int bestIndex;
	int i, nactive;

	nactive = 0;	// number of active clients
	bestIndex = -1;
	longestInactivity = INT_MIN;
	longestSpecInactivity = INT_MIN;

	for ( i = 0, cl = svs.clients; i < sv_maxclients->integer ; i++, cl++ ) {

		if ( cl->state != CS_ACTIVE || cl->gentity == NULL )
			continue;

		if ( cl->gentity->r.svFlags & SVF_BOT || i == skipClientNum )
			continue;

		if ( checkCommands ) {
			// wait a few seconds after any command received/sent
			// to avoid dropping score request by flood protection
			// or lagging target client too much
			if ( cl->multiview.lastRecvTime + 500 > svs.time )
				continue;

			if ( cl->multiview.lastSentTime + 1500 > svs.time )
				continue;

			// never send anything to client that has unacknowledged commands
			if ( cl->reliableSequence > cl->reliableAcknowledge )
				continue;
		}

		if ( longestInactivity < svs.time - cl->multiview.scoreQueryTime ) {
			longestInactivity = svs.time - cl->multiview.scoreQueryTime;
			bestIndex = cl - svs.clients;
		}

		clist[ nactive++ ] = cl;
	}

	if ( nactive < minActive )
		return -1;

	// count spectators from active
	for ( i = 0; i < nactive; i++ ) {
		cl = clist[ i ];
		ps = SV_GameClientNum( cl - svs.clients );
		if ( currentGameMod != GAMEMOD_ETF && (ps->persistant[ PERS_TEAM ] == TEAM_SPECTATOR || (ps->pm_flags & PMF_FOLLOW) ) ) {
			if ( longestSpecInactivity < svs.time - cl->multiview.scoreQueryTime ) {
				longestSpecInactivity = svs.time - cl->multiview.scoreQueryTime;
				bestIndex = cl - svs.clients;
			}
		}
		else if ( currentGameMod == GAMEMOD_ETF && (ps->persistant[ PERS_TEAM ] == 5 || (ps->pm_flags & PMF_FOLLOW) || (ps->pm_flags & 0x8000) ) ) {
			if ( longestSpecInactivity < svs.time - cl->multiview.scoreQueryTime ) {
				longestSpecInactivity = svs.time - cl->multiview.scoreQueryTime;
				bestIndex = cl - svs.clients;
			}
		}
	}

	return bestIndex;
}


static void SV_QueryClientScore( client_t *client )
{
	#define	SCORE_RECORDER 1
	#define	SCORE_CLIENT   2
	#define SCORE_PERIOD   10000

	int clientNum;

	if ( client->multiview.scoreQueryTime == 0 )
	{
		// first time init?
		client->multiview.scoreQueryTime = svs.time + SCORE_PERIOD/3;
	}
	else if ( svs.time >= client->multiview.scoreQueryTime ) 
	{
		//} else if ( svs.time > client->multiview.scoreQueryTime + SCORE_PERIOD ) {
		if ( client->multiview.recorder && sv_demoFlags->integer & SCORE_RECORDER ) {

			clientNum = SV_FindActiveClient( qtrue, -1, 0 ); // count last sent command, ignore noone
			if ( clientNum != -1 ) {
				if ( clientNum != sv_demoClientID ) {
					//Com_DPrintf( S_COLOR_YELLOW " change score target from %i to %i\n", clientNum, sv_demoClientID );
					SV_SetTargetClient( clientNum );
				}

				SV_ExecuteClientCommand( svs.clients + sv_demoClientID, "score", qfalse );

				client->multiview.scoreQueryTime = svs.time + SCORE_PERIOD;
				svs.clients[ sv_demoClientID ].multiview.scoreQueryTime = svs.time + SCORE_PERIOD;
					
			} else {
				//Com_DPrintf( S_COLOR_YELLOW "no active clients available for 'score'\n" ); // debug print
			}
		} else if ( sv_demoFlags->integer & SCORE_CLIENT ) {
			SV_ExecuteClientCommand( client, "score", qfalse );
			client->multiview.scoreQueryTime = svs.time + SCORE_PERIOD;
		}
	}
}
#endif // USE_MV


/*
=============
SV_BuildClientSnapshot

Decides which entities are going to be visible to the client, and
copies off the playerstate and areabits.

This properly handles multiple recursive portals, but the render
currently doesn't.

For viewing through other player's eyes, clent can be something other than client->gentity
=============
*/
static void SV_BuildClientSnapshot( client_t *client ) {
	clientSnapshot_t			*frame;
	int							i, cl;
	int							clientNum;
	playerState_t				*ps;
	clientPVS_t					*pvs;

	// this is the frame we are creating
	frame = &client->frames[ client->netchan.outgoingSequence & PACKET_MASK ];
	cl = client - svs.clients;

	// clear everything in this snapshot
	Com_Memset( frame->areabits, 0, sizeof( frame->areabits ) );
	frame->areabytes = 0;

	// https://zerowing.idsoftware.com/bugzilla/show_bug.cgi?id=62
	frame->num_entities = 0;
	frame->frameNum = svs.currentSnapshotFrame;

#ifdef USE_MV
	if ( client->multiview.protocol > 0 ) {
		frame->multiview = qtrue;
		// select primary client slot
		if ( client->multiview.recorder ) {
			cl = sv_demoClientID;
		}
	} else {
		frame->multiview = qfalse;
	}
	Com_Memset( frame->psMask, 0, sizeof( frame->psMask ) );
	frame->first_psf = svs.nextSnapshotPSF;
	frame->num_psf = 0;
#endif
	
	if ( client->state == CS_ZOMBIE )
		return;

	// grab the current playerState_t
	ps = SV_GameClientNum( cl );
	frame->ps = *ps;

	clientNum = frame->ps.clientNum;
	if ( clientNum < 0 || clientNum >= MAX_GENTITIES-1 ) {
		Com_Error( ERR_DROP, "SV_SvEntityForGentity: bad gEnt" );
	}

	// we set client->gentity only after sending gamestate
	// so don't send any packetentities changes until CS_PRIMED
	// because new gamestate will invalidate them anyway
#ifdef USE_MV
	if ( !client->gentity && !client->multiview.recorder ) {
#else
	if ( !client->gentity ) {
#endif
		return;
	}

	if ( svs.currFrame == NULL ) {
		// this will always success and setup current frame
		SV_BuildCommonSnapshot();
	}

	frame->frameNum = svs.currFrame->frameNum;

#ifdef USE_MV
	if ( frame->multiview ) {
		clientPVS_t *pvs;
		psFrame_t *psf;
		int slot;
		for ( slot = 0 ; slot < sv_maxclients->integer; slot++ ) {
			// record only form primary slot or active clients
			if ( slot == cl || svs.clients[ slot ].state == CS_ACTIVE ) {
				// get current playerstate
				ps = SV_GameClientNum( slot );
				// skip bots in spectator state
				if ( currentGameMod != GAMEMOD_ETF && ps->persistant[ PERS_TEAM ] == TEAM_SPECTATOR && svs.clients[ slot ].netchan.remoteAddress.type == NA_BOT ) {
					continue;
				}
				else if ( currentGameMod == GAMEMOD_ETF && ps->persistant[ PERS_TEAM ] == 5 && svs.clients[ slot ].netchan.remoteAddress.type == NA_BOT ) {
					continue;
				}

				// allocate playerstate frame
				psf = &svs.snapshotPSF[ svs.nextSnapshotPSF % svs.numSnapshotPSF ]; 
				svs.nextSnapshotPSF++;
				frame->num_psf++;

				SET_ABIT( frame->psMask, slot );

				psf->ps = *ps;
				psf->clientSlot = slot;

				pvs = SV_BuildClientPVS( slot, &psf->ps, qtrue );
				psf->areabytes = pvs->areabytes;
				memcpy( psf->areabits, pvs->areabits, sizeof( psf->areabits ) );

				if ( slot == cl ) {
					// save for primary client
					frame->areabytes = psf->areabytes;
					Com_Memcpy( frame->areabits, psf->areabits, sizeof( frame->areabits ) );
				}
				// copy generated entity mask
				memcpy( psf->entMask, pvs->entMask, sizeof( psf->entMask ) );
			}
		}

		// get ALL pointers from common snapshot
		frame->num_entities = svs.currFrame->count;
		for ( i = 0 ; i < frame->num_entities ; i++ ) {
			frame->ents[ i ] = svs.currFrame->ents[ i ];
		}

#ifdef USE_MV_ZCMD
		// some extras
		if ( client->deltaMessage <= 0 )
			client->multiview.z.deltaSeq = 0;
#endif

		// auto score request
		if ( sv_demoFlags->integer & ( SCORE_RECORDER | SCORE_CLIENT ) )
			SV_QueryClientScore( client );

	}
	else // non-multiview frame
#endif
	{
		pvs = SV_BuildClientPVS( cl, ps, qfalse );

		memcpy( frame->areabits, pvs->areabits, sizeof( frame->areabits ) );
		frame->areabytes = pvs->areabytes;

		frame->num_entities = pvs->numbers.numSnapshotEntities;
		// get pointers from common snapshot
		for ( i = 0 ; i < pvs->numbers.numSnapshotEntities ; i++ )	{
			frame->ents[ i ] = svs.currFrame->ents[ pvs->numbers.snapshotEntities[ i ] ];
		}
	}
}


/*
=======================
SV_SendMessageToClient

Called by SV_SendClientSnapshot and SV_SendClientGameState
=======================
*/
void SV_SendMessageToClient( msg_t *msg, client_t *client )
{
#ifdef USE_MV
	if ( client->multiview.protocol && client->multiview.recorder && sv_demoFile != FS_INVALID_HANDLE ) {
		int v;

		 // finalize packet
		MSG_WriteByte( msg, svc_EOF );

		// write message sequence
		v = LittleLong( client->netchan.outgoingSequence );
		FS_Write( &v, 4, sv_demoFile );

		// write message size
		v = LittleLong( msg->cursize );
		FS_Write( &v, 4, sv_demoFile );

		// write data
		FS_Write( msg->data, msg->cursize, sv_demoFile );

		// update delta sequence
		client->deltaMessage = client->netchan.outgoingSequence;
		client->netchan.outgoingSequence++;

		return;
	}
#endif // USE_MV

	// record information about the message
	client->frames[client->netchan.outgoingSequence & PACKET_MASK].messageSize = msg->cursize;
	client->frames[client->netchan.outgoingSequence & PACKET_MASK].messageSent = svs.msgTime;
	client->frames[client->netchan.outgoingSequence & PACKET_MASK].messageAcked = 0;

	// send the datagram
	SV_Netchan_Transmit( client, msg );
}


//bani
/*
=======================
SV_SendClientIdle

There is no need to send full snapshots to clients who are loading a map.
So we send them "idle" packets with the bare minimum required to keep them on the server.

=======================
*/
void SV_SendClientIdle( client_t *client ) {
	byte		msg_buf[ MAX_MSGLEN_BUF ];
	msg_t		msg;

	MSG_Init( &msg, msg_buf, MAX_MSGLEN );
	msg.allowoverflow = qtrue;

	// NOTE, MRE: all server->client messages now acknowledge
	// let the client know which reliable clientCommands we have received
	MSG_WriteLong( &msg, client->lastClientCommand );

	// (re)send any reliable server commands
	SV_UpdateServerCommandsToClient( client, &msg );

	// send over all the relevant entityState_t
	// and the playerState_t
//	SV_WriteSnapshotToClient( client, &msg );

	// Add any download data if the client is downloading
//	SV_WriteDownloadToClient( client, &msg );

	// check for overflow
	if ( msg.overflowed ) {
		Com_Printf( "WARNING: msg overflowed for %s\n", client->name );
		MSG_Clear( &msg );

		SV_DropClient( client, "Msg overflowed" );
		return;
	}

	SV_SendMessageToClient( &msg, client );

	sv.bpsTotalBytes += msg.cursize;            // NERVE - SMF - net debugging
	sv.ubpsTotalBytes += msg.uncompsize / 8;    // NERVE - SMF - net debugging
}

/*
=======================
SV_SendClientSnapshot

Also called by SV_FinalCommand

=======================
*/
void SV_SendClientSnapshot( client_t *client ) {
	byte		msg_buf[ MAX_MSGLEN_BUF ];
	msg_t		msg;

	//bani
	if ( client->state < CS_ACTIVE ) {
		// bani - #760 - zombie clients need full snaps so they can still process reliable commands
		// (eg so they can pick up the disconnect reason)
		if ( client->state != CS_ZOMBIE ) {
			SV_SendClientIdle( client );
			return;
		}
	}

	// build the snapshot
	SV_BuildClientSnapshot( client );

	// bots need to have their snapshots build, but
	// the query them directly without needing to be sent
	if ( client->netchan.remoteAddress.type == NA_BOT ) {
		return;
	}

	MSG_Init( &msg, msg_buf, MAX_MSGLEN );
	msg.allowoverflow = qtrue;

	// NOTE, MRE: all server->client messages now acknowledge
	// let the client know which reliable clientCommands we have received
	MSG_WriteLong( &msg, client->lastClientCommand );

	// (re)send any reliable server commands
	SV_UpdateServerCommandsToClient( client, &msg );

	// send over all the relevant entityState_t
	// and the playerState_t
	SV_WriteSnapshotToClient( client, &msg );

	// check for overflow
	if ( msg.overflowed ) {
		Com_Printf( "WARNING: msg overflowed for %s\n", client->name );
		MSG_Clear( &msg );

		SV_DropClient( client, "Msg overflowed" );
		return;
	}

	SV_SendMessageToClient( &msg, client );

	sv.bpsTotalBytes += msg.cursize;            // NERVE - SMF - net debugging
	sv.ubpsTotalBytes += msg.uncompsize / 8;    // NERVE - SMF - net debugging
}


/*
=======================
SV_SendClientMessages
=======================
*/
void SV_SendClientMessages( void )
{
	int		i;
	client_t	*c;
	int numclients = 0;         // NERVE - SMF - net debugging

	svs.msgTime = Sys_Milliseconds();

	sv.bpsTotalBytes = 0;       // NERVE - SMF - net debugging
	sv.ubpsTotalBytes = 0;      // NERVE - SMF - net debugging

#ifdef USE_MV
	if ( sv_demoFile != FS_INVALID_HANDLE )
	{
		if ( !svs.emptyFrame ) // we want to record only synced game frames
		{
			c = svs.clients + sv_maxclients->integer; // recorder slot
			if ( c->state >= CS_PRIMED )
			{
				SV_SendClientSnapshot( c );
				c->lastSnapshotTime = svs.time;
				c->rateDelayed = qfalse;
			}
		}
	}
#endif // USE_MV

	// send a message to each connected client
	for( i = 0; i < sv_maxclients->integer; i++ )
	{
		c = &svs.clients[ i ];
		
		// rain - changed <= CS_ZOMBIE to < CS_ZOMBIE so that the
		// disconnect reason is properly sent in the network stream
		if ( c->state < CS_ZOMBIE )
			continue;       // not connected

		if ( *c->downloadName )
			continue;		// Client is downloading, don't send snapshots

		// RF, needed to insert this otherwise bots would cause error drops in sv_net_chan.c:
		// --> "netchan queue is not properly initialized in SV_Netchan_TransmitNextFragment\n"
		if ( c->gentity && c->gentity->r.svFlags & SVF_BOT ) {
			continue;
		}

		// 1. Local clients get snapshots every server frame
		// 2. Remote clients get snapshots depending from rate and requested number of updates

		if ( svs.time - c->lastSnapshotTime < c->snapshotMsec * com_timescale->value )
			continue;		// It's not time yet

		if ( c->netchan.unsentFragments || c->netchan_start_queue )
		{
			c->rateDelayed = qtrue;
			continue;		// Drop this snapshot if the packet queue is still full or delta compression will break
		}
	
		if ( SV_RateMsec( c ) > 0 )
		{
			// Not enough time since last packet passed through the line
			c->rateDelayed = qtrue;
			continue;
		}

		numclients++;		// NERVE - SMF - net debugging

		// generate and send a new message
		SV_SendClientSnapshot( c );
		c->lastSnapshotTime = svs.time;
		c->rateDelayed = qfalse;
	}

	// NERVE - SMF - net debugging
	if ( sv_showAverageBPS->integer && numclients > 0 ) {
		float ave = 0, uave = 0;

		for ( i = 0; i < MAX_BPS_WINDOW - 1; i++ ) {
			sv.bpsWindow[i] = sv.bpsWindow[i + 1];
			ave += sv.bpsWindow[i];

			sv.ubpsWindow[i] = sv.ubpsWindow[i + 1];
			uave += sv.ubpsWindow[i];
		}

		sv.bpsWindow[MAX_BPS_WINDOW - 1] = sv.bpsTotalBytes;
		ave += sv.bpsTotalBytes;

		sv.ubpsWindow[MAX_BPS_WINDOW - 1] = sv.ubpsTotalBytes;
		uave += sv.ubpsTotalBytes;

		if ( sv.bpsTotalBytes >= sv.bpsMaxBytes ) {
			sv.bpsMaxBytes = sv.bpsTotalBytes;
		}

		if ( sv.ubpsTotalBytes >= sv.ubpsMaxBytes ) {
			sv.ubpsMaxBytes = sv.ubpsTotalBytes;
		}

		sv.bpsWindowSteps++;

		if ( sv.bpsWindowSteps >= MAX_BPS_WINDOW ) {
			float comp_ratio;

			sv.bpsWindowSteps = 0;

			ave = ( ave / (float)MAX_BPS_WINDOW );
			uave = ( uave / (float)MAX_BPS_WINDOW );

			comp_ratio = ( 1 - ave / uave ) * 100.f;
			sv.ucompAve += comp_ratio;
			sv.ucompNum++;

			Com_DPrintf( "bpspc(%2.0f) bps(%2.0f) pk(%i) ubps(%2.0f) upk(%i) cr(%2.2f) acr(%2.2f)\n",
						 ave / (float)numclients, ave, sv.bpsMaxBytes, uave, sv.ubpsMaxBytes, comp_ratio, sv.ucompAve / sv.ucompNum );
		}
	}
	// -NERVE - SMF
}
