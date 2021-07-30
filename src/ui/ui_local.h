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

#ifndef __UI_LOCAL_H__
#define __UI_LOCAL_H__

#include "../qcommon/q_shared.h"
#include "../renderercommon/tr_types.h"
#include "ui_public.h"
#include "keycodes.h"
#include "../game/bg_public.h"
#include "ui_shared.h"

extern vmCvar_t ui_master;

extern vmCvar_t ui_brassTime;
extern vmCvar_t ui_drawCrosshair;
extern vmCvar_t ui_drawCrosshairNames;
extern vmCvar_t ui_drawCrosshairPickups;    //----(SA) added
extern vmCvar_t ui_marks;
// JOSEPH 12-3-99
extern vmCvar_t ui_autoactivate;
// END JOSEPH

extern vmCvar_t ui_server1;
extern vmCvar_t ui_server2;
extern vmCvar_t ui_server3;
extern vmCvar_t ui_server4;
extern vmCvar_t ui_server5;
extern vmCvar_t ui_server6;
extern vmCvar_t ui_server7;
extern vmCvar_t ui_server8;
extern vmCvar_t ui_server9;
extern vmCvar_t ui_server10;
extern vmCvar_t ui_server11;
extern vmCvar_t ui_server12;
extern vmCvar_t ui_server13;
extern vmCvar_t ui_server14;
extern vmCvar_t ui_server15;
extern vmCvar_t ui_server16;

extern vmCvar_t ui_smallFont;
extern vmCvar_t ui_bigFont;
extern vmCvar_t ui_cdkey;
extern vmCvar_t ui_cdkeychecked;
extern vmCvar_t ui_selectedPlayer;
extern vmCvar_t ui_selectedPlayerName;
extern vmCvar_t ui_netSource;
extern vmCvar_t ui_menuFiles;
extern vmCvar_t ui_gameType;
extern vmCvar_t ui_netGameType;
//extern vmCvar_t	ui_actualNetGameType;
extern vmCvar_t ui_joinGameType;
extern vmCvar_t ui_dedicated;
extern vmCvar_t ui_notebookCurrentPage;
extern vmCvar_t ui_clipboardName;

// NERVE - SMF - multiplayer cvars
extern vmCvar_t ui_serverFilterType;
extern vmCvar_t ui_currentNetMap;
extern vmCvar_t ui_currentMap;
extern vmCvar_t ui_mapIndex;

extern vmCvar_t ui_browserMaster;
extern vmCvar_t ui_browserGameType;
extern vmCvar_t ui_browserSortKey;
extern vmCvar_t ui_browserShowEmptyOrFull;
extern vmCvar_t ui_browserShowPasswordProtected;
extern vmCvar_t ui_browserShowFriendlyFire;
extern vmCvar_t ui_browserShowMaxlives;
extern vmCvar_t ui_browserShowPunkBuster;
extern vmCvar_t ui_browserShowAntilag;
extern vmCvar_t ui_browserShowWeaponsRestricted;
extern vmCvar_t ui_browserShowTeamBalanced;

extern vmCvar_t ui_serverStatusTimeOut;
extern vmCvar_t ui_limboOptions;

extern vmCvar_t ui_isSpectator;
// -NERVE - SMF

extern vmCvar_t g_gameType;

extern vmCvar_t ui_cl_profile;
extern vmCvar_t ui_cl_defaultProfile;
extern vmCvar_t ui_profile;
extern vmCvar_t ui_currentNetCampaign;
extern vmCvar_t ui_currentCampaign;
extern vmCvar_t ui_campaignIndex;
extern vmCvar_t ui_currentCampaignCompleted;
extern vmCvar_t ui_blackout;
extern vmCvar_t cg_crosshairAlpha;
extern vmCvar_t cg_crosshairAlphaAlt;
extern vmCvar_t cg_crosshairColor;
extern vmCvar_t cg_crosshairColorAlt;
extern vmCvar_t cg_crosshairSize;

extern vmCvar_t ui_cl_bypassMouseInput;

//bani
extern vmCvar_t ui_autoredirect;

//
// ui_main.c
//
void            UI_Report( void );
void            UI_Load( void );
void            UI_LoadMenus( const char *menuFile, qboolean reset );
void            UI_SetActiveMenu( uiMenuCommand_t menu );
uiMenuCommand_t UI_GetActiveMenu( void );
int             UI_AdjustTimeByGame( int time );
void            UI_ShowPostGame( qboolean newHigh );
void            UI_LoadArenas( void );
void            UI_LoadCampaigns( void );
mapInfo*        UI_FindMapInfoByMapname( const char* name );
void            UI_ReadableSize( char *buf, int bufsize, int value );
void            UI_PrintTime( char *buf, int bufsize, int time );
void            Text_Paint_Ext( float x, float y, float scalex, float scaley, vec4_t color, const char *text, float adjust, int limit, int style, fontInfo_t* font );
void			UI_VideoCheck( int time );
void			UI_RegisterCvars( void );
void			UI_UpdateCvars( void );
void			UI_DrawConnectScreen( qboolean overlay );
void			UI_Campaign_f( void );
void			UI_ListCampaigns_f( void );

#define GLINFO_LINES        128

//
// ui_loadpanel.c
//
void UI_LoadPanel_Init( void );
extern void UI_DrawLoadPanel( qboolean forcerefresh, qboolean ownerdraw, qboolean uihack );

// new ui stuff
#define MAX_MAPS 128
#define MAX_PINGREQUESTS        16
#define MAX_ADDRESSLENGTH       64
#define MAX_HOSTNAMELENGTH      22
#define MAX_MAPNAMELENGTH       16
#define MAX_STATUSLENGTH        64
#define MAX_LISTBOXWIDTH        59
#define UI_FONT_THRESHOLD       0.1
#define MAX_DISPLAY_SERVERS     2048
#define MAX_SERVERSTATUS_LINES  128
#define MAX_SERVERSTATUS_TEXT   2048
#define MAX_FOUNDPLAYER_SERVERS 16
#define TEAM_MEMBERS 5
//#define MAPS_PER_TIER 3
//#define MAX_TIERS 16
#define MAX_MODS 64
#define MAX_DEMOS 256
#define MAX_MOVIES 256
#define MAX_PLAYERMODELS 256
#define MAX_SAVEGAMES 256

#define MAX_PROFILES 64

//----(SA)	added
typedef struct {
	const char  *name;
	qhandle_t sshotImage;
} savegameInfo;
//----(SA)	end

typedef struct {
	const char *gameType;
	const char *gameTypeShort;
	int gtEnum;
	const char *gameTypeDescription;
} gameTypeInfo;

typedef struct {
	const char *name;
	const char *dir;
} profileInfo_t;

typedef struct serverFilter_s {
	const char *description;
	const char *basedir;
} serverFilter_t;

typedef struct {
	char adrstr[MAX_ADDRESSLENGTH];
	int start;
} pinglist_t;

typedef struct serverStatus_s {
	int refreshtime;
	int sortKey;
	int sortDir;
	qboolean refreshActive;
	int currentServer;
	int displayServers[MAX_DISPLAY_SERVERS];
	int numDisplayServers;
	int numPlayersOnServers;
	int nextDisplayRefresh;
	qhandle_t currentServerPreview;
	int currentServerCinematic;
	int motdLen;
	int motdWidth;
	int motdPaintX;
	int motdPaintX2;
	int motdOffset;
	int motdTime;
	char motd[MAX_STRING_CHARS];
} serverStatus_t;


typedef struct {
	char adrstr[MAX_ADDRESSLENGTH];
	char name[MAX_ADDRESSLENGTH];
	int startTime;
	int serverNum;
	qboolean valid;
} pendingServer_t;

typedef struct {
	int num;
	pendingServer_t server[MAX_SERVERSTATUSREQUESTS];
} pendingServerStatus_t;

typedef struct {
	char address[MAX_ADDRESSLENGTH];
	char *lines[MAX_SERVERSTATUS_LINES][4];
	char text[MAX_SERVERSTATUS_TEXT];
	char pings[MAX_CLIENTS * 3];
	int numLines;
} serverStatusInfo_t;

typedef struct {
	const char *modName;
	const char *modDescr;
} modInfo_t;

typedef struct {
	displayContextDef_t uiDC;
	int newHighScoreTime;
	int newBestTime;
	int showPostGameTime;
	qboolean newHighScore;
	qboolean demoAvailable;
	qboolean soundHighScore;

	int numGameTypes;
	gameTypeInfo gameTypes[MAX_GAMETYPES];

	int numJoinGameTypes;
	gameTypeInfo joinGameTypes[MAX_GAMETYPES];

	int playerCount;
	int myTeamCount;
	int teamIndex;
	int playerRefresh;
	int playerIndex;
	int playerNumber;
	qboolean teamLeader;
	char playerNames[MAX_CLIENTS][MAX_NAME_LENGTH * 2];
	qboolean playerMuted[MAX_CLIENTS];
	int playerRefereeStatus[MAX_CLIENTS];
	char teamNames[MAX_CLIENTS][MAX_NAME_LENGTH];
	int teamClientNums[MAX_CLIENTS];

	int mapCount;
	mapInfo mapList[MAX_MAPS];

	int campaignCount;
	campaignInfo_t campaignList[MAX_CAMPAIGNS];

	cpsFile_t campaignStatus;

	profileInfo_t profileList[MAX_PROFILES];
	int profileCount;
	int profileIndex;

	int skillIndex;

	modInfo_t modList[MAX_MODS];
	int modCount;
	int modIndex;

	const char *demoList[MAX_DEMOS];
	int demoCount;
	int demoIndex;

	const char *movieList[MAX_MOVIES];
	int movieCount;
	int movieIndex;
	int previewMovie;

//----(SA)	added
//	const char		*savegameList[MAX_SAVEGAMES];
	savegameInfo savegameList[MAX_SAVEGAMES];
	int savegameCount;
	int savegameIndex;
//----(SA)	end

	serverStatus_t serverStatus;

	// for the showing the status of a server
	char serverStatusAddress[MAX_ADDRESSLENGTH];
	serverStatusInfo_t serverStatusInfo;
	int nextServerStatusRefresh;

	// to retrieve the status of server to find a player
	pendingServerStatus_t pendingServerStatus;
	char findPlayerName[MAX_STRING_CHARS];
	char foundPlayerServerAddresses[MAX_FOUNDPLAYER_SERVERS][MAX_ADDRESSLENGTH];
	char foundPlayerServerNames[MAX_FOUNDPLAYER_SERVERS][MAX_ADDRESSLENGTH];
	int currentFoundPlayerServer;
	int numFoundPlayerServers;
	int nextFindPlayerRefresh;

	int currentCrosshair;
	int startPostGameTime;
	sfxHandle_t newHighScoreSound;

	int effectsColor;

	qboolean inGameLoad;

	int selectedObjective;

	int activeFont;

	const char  *glInfoLines[GLINFO_LINES];
	int numGlInfoLines;

	vec4_t xhairColor;
	vec4_t xhairColorAlt;

	qhandle_t passwordFilter;
	qhandle_t friendlyFireFilter;
	qhandle_t maxLivesFilter;
	qhandle_t punkBusterFilter;
	qhandle_t weaponRestrictionsFilter;
	qhandle_t antiLagFilter;
	qhandle_t teamBalanceFilter;

	qhandle_t campaignMap;
} uiInfo_t;

extern uiInfo_t uiInfo;

void		UI_Init( qboolean inGameLoad );
void		UI_Shutdown( void );
void		UI_KeyEvent( int key, qboolean down );
void		UI_MouseEvent( int dx, int dy );
void		UI_Refresh( int realtime );
qboolean	UI_ConsoleCommand( int realTime );
void		UI_DrawNamedPic( float x, float y, float width, float height, const char *picname );
void		UI_DrawHandlePic( float x, float y, float w, float h, qhandle_t hShader );
void		UI_FillRect( float x, float y, float width, float height, const float *color );
void		UI_FillScreen( const float *color );
void		UI_DrawRect( float x, float y, float width, float height, const float *color );
void		UI_DrawTopBottom( float x, float y, float w, float h );
void		UI_DrawSides( float x, float y, float w, float h );
void		UI_AdjustFrom640( float *x, float *y, float *w, float *h );
qboolean	UI_IsFullscreen( void );
void		UI_SetActiveMenu( uiMenuCommand_t menu );
const char	*UI_Argv( int arg );
const char	*UI_Cvar_VariableString( const char *var_name );

//
// ui_syscalls.c
//
void            trap_Print( const char *string );
void            NORETURN trap_Error( const char *string );
int             trap_Milliseconds( void );
void            trap_Cvar_Register( vmCvar_t *vmCvar, const char *varName, const char *defaultValue, int flags );
void            trap_Cvar_Update( vmCvar_t *vmCvar );
void            trap_Cvar_Set( const char *var_name, const char *value );
float           trap_Cvar_VariableValue( const char *var_name );
void            trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );
void            trap_Cvar_LatchedVariableStringBuffer( const char *var_name, char *buffer, int bufsize );
void            trap_Cvar_SetValue( const char *var_name, float value );
void            trap_Cvar_Reset( const char *name );
void            trap_Cvar_Create( const char *var_name, const char *var_value, int flags );
void            trap_Cvar_InfoStringBuffer( int bit, char *buffer, int bufsize );
int             trap_Argc( void );
void            trap_Argv( int n, char *buffer, int bufferLength );
void            trap_Cmd_ExecuteText( int exec_when, const char *text );    // don't use EXEC_NOW!
void            trap_AddCommand( const char *cmdName );
int             trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode );
void            trap_FS_Read( void *buffer, int len, fileHandle_t f );
void            trap_FS_Write( const void *buffer, int len, fileHandle_t f );
void            trap_FS_FCloseFile( fileHandle_t f );
int             trap_FS_GetFileList(  const char *path, const char *extension, char *listbuf, int bufsize );
int             trap_FS_Delete( const char *filename );
qhandle_t       trap_R_RegisterModel( const char *name );
qhandle_t       trap_R_RegisterSkin( const char *name );
qhandle_t       trap_R_RegisterShaderNoMip( const char *name );
void            trap_R_ClearScene( void );
void            trap_R_AddRefEntityToScene( const refEntity_t *re );
void            trap_R_AddPolyToScene( qhandle_t hShader, int numVerts, const polyVert_t *verts );
void            trap_R_AddLightToScene( const vec3_t org, float radius, float intensity, float r, float g, float b, qhandle_t hShader, int flags );
void            trap_R_AddCoronaToScene( const vec3_t org, float r, float g, float b, float scale, int id, qboolean visible );
void            trap_R_RenderScene( const refdef_t *fd );
void            trap_R_SetColor( const float *rgba );
void            trap_R_Add2dPolys( polyVert_t* verts, int numverts, qhandle_t hShader );
void            trap_R_DrawStretchPic( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader );
void            trap_R_DrawRotatedPic( float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader, float angle );
void            trap_R_ModelBounds( clipHandle_t model, vec3_t mins, vec3_t maxs );
void            trap_UpdateScreen( void );
int             trap_CM_LerpTag( orientation_t *tag, const refEntity_t *refent, const char *tagName, int startIndex );
void            trap_S_StartLocalSound( sfxHandle_t sfx, int channelNum );
sfxHandle_t     trap_S_RegisterSound( const char *sample, qboolean compressed );
void            trap_Key_KeynumToStringBuf( int keynum, char *buf, int buflen );
void            trap_Key_GetBindingBuf( int keynum, char *buf, int buflen );
void            trap_Key_KeysForBinding( const char* binding, int* key1, int* key2 );
void            trap_Key_SetBinding( int keynum, const char *binding );
qboolean        trap_Key_IsDown( int keynum );
qboolean        trap_Key_GetOverstrikeMode( void );
void            trap_Key_SetOverstrikeMode( qboolean state );
void            trap_Key_ClearStates( void );
int             trap_Key_GetCatcher( void );
void            trap_Key_SetCatcher( int catcher );
void            trap_GetClipboardData( char *buf, int bufsize );
void            trap_GetClientState( uiClientState_t *state );
void            trap_GetGlconfig( glconfig_t *glconfig );
int             trap_GetConfigString( int index, char* buff, int buffsize );
int             trap_LAN_GetServerCount( int source );          // NERVE - SMF
int             trap_LAN_GetLocalServerCount( void );
void            trap_LAN_GetLocalServerAddressString( int n, char *buf, int buflen );
int             trap_LAN_GetGlobalServerCount( void );
void            trap_LAN_GetGlobalServerAddressString( int n, char *buf, int buflen );
int             trap_LAN_GetPingQueueCount( void );
void            trap_LAN_ClearPing( int n );
void            trap_LAN_GetPing( int n, char *buf, int buflen, int *pingtime );
void            trap_LAN_GetPingInfo( int n, char *buf, int buflen );
int             trap_MemoryRemaining( void );

// NERVE - SMF - multiplayer traps
qboolean        trap_LAN_UpdateVisiblePings( int source );
void            trap_LAN_MarkServerVisible( int source, int n, qboolean visible );
void            trap_LAN_ResetPings( int n );
void            trap_LAN_SaveCachedServers();
int             trap_LAN_CompareServers( int source, int sortKey, int sortDir, int s1, int s2 );
void            trap_LAN_GetServerAddressString( int source, int n, char *buf, int buflen );
void trap_LAN_GetServerInfo( int source, int n, char *buf, int buflen );
int             trap_LAN_AddServer( int source, const char *name, const char *addr );
void            trap_LAN_RemoveServer( int source, const char *addr );
int             trap_LAN_GetServerPing( int source, int n );
int             trap_LAN_ServerIsVisible( int source, int n );
int             trap_LAN_ServerStatus( const char *serverAddress, char *serverStatus, int maxLen );
void            trap_LAN_SaveCachedServers();
void            trap_LAN_LoadCachedServers();
qboolean        trap_LAN_ServerIsInFavoriteList( int source, int n );

void            trap_SetPbClStatus( int status );                               // DHM - Nerve
void            trap_SetPbSvStatus( int status );                               // TTimo


// -NERVE - SMF

void            trap_GetCDKey( char *buf, int buflen );
void            trap_SetCDKey( char *buf );
void            trap_R_RegisterFont( const char *pFontname, int pointSize, fontInfo_t *font );
void            trap_S_StopBackgroundTrack( void );
void            trap_S_StartBackgroundTrack( const char *intro, const char *loop, int fadeupTime );
void            trap_S_FadeAllSound( float targetvol, int time, qboolean stopsound );
int             trap_CIN_PlayCinematic( const char *arg0, int xpos, int ypos, int width, int height, int bits );
e_status        trap_CIN_StopCinematic( int handle );
e_status        trap_CIN_RunCinematic( int handle );
void            trap_CIN_DrawCinematic( int handle );
void            trap_CIN_SetExtents( int handle, int x, int y, int w, int h );
int             trap_RealTime( qtime_t *qtime );
void            trap_R_RemapShader( const char *oldShader, const char *newShader, const char *timeOffset );
qboolean        trap_VerifyCDKey( const char *key, const char *chksum );
qboolean        trap_GetLimboString( int index, char *buf );            // NERVE - SMF
void            trap_CheckAutoUpdate( void );                           // DHM - Nerve
void            trap_GetAutoUpdate( void );                             // DHM - Nerve

void            trap_openURL( const char *url ); // TTimo
void            trap_GetHunkData( int* hunkused, int* hunkexpected );


char*           trap_TranslateString( const char *string );             // NERVE - SMF - localization

//
// ui_gameinfo.c
//

const char* UI_DescriptionForCampaign( void );
const char* UI_NameForCampaign( void );

// extension interface
extern  qboolean intShaderTime;
extern  qboolean linearLight;
extern	qboolean removeCommand;

qboolean trap_GetValue( char *value, int valueSize, const char *key );
void trap_R_AddRefEntityToScene2( const refEntity_t *re );
void trap_R_AddLinearLightToScene( const vec3_t start, const vec3_t end, float intensity, float r, float g, float b );
void trap_RemoveCommand( const char *cmdName );
extern int dll_com_trapGetValue;
extern int dll_trap_R_AddRefEntityToScene2;
extern int dll_trap_R_AddLinearLightToScene;
extern int dll_trap_RemoveCommand;

#endif
