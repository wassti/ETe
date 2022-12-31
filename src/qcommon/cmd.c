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
// cmd.c -- Quake script command processing module

#include "q_shared.h"
#include "qcommon.h"

#define MAX_CMD_BUFFER  131072

typedef struct {
	byte *data;
	int maxsize;
	int cursize;
} cmd_t;

static int   cmd_wait;
static cmd_t cmd_text;
static byte  cmd_text_buf[MAX_CMD_BUFFER];


//=============================================================================

/*
============
Cmd_Wait_f

Causes execution of the remainder of the command buffer to be delayed until
next frame.  This allows commands like:
bind g "cmd use rocket ; +attack ; wait ; -attack ; cmd use blaster"
============
*/
static void Cmd_Wait_f( void ) {
	if ( Cmd_Argc() == 2 ) {
		cmd_wait = atoi( Cmd_Argv( 1 ) );
		if ( cmd_wait < 0 )
			cmd_wait = 1; // ignore the argument
	} else {
		cmd_wait = 1;
	}
}


/*
=============================================================================

						COMMAND BUFFER

=============================================================================
*/

/*
============
Cbuf_Init
============
*/
void Cbuf_Init( void )
{
	cmd_text.data = cmd_text_buf;
	cmd_text.maxsize = MAX_CMD_BUFFER;
	cmd_text.cursize = 0;
}


/*
============
Cbuf_AddText

Adds command text at the end of the buffer, does NOT add a final \n
============
*/
void Cbuf_AddText( const char *text ) {

	const int l = (int)strlen( text );

	if (cmd_text.cursize + l >= cmd_text.maxsize)
	{
		Com_Printf ("Cbuf_AddText: overflow\n");
		return;
	}

	Com_Memcpy(&cmd_text.data[cmd_text.cursize], text, l);
	cmd_text.cursize += l;
}


/*
============
Cbuf_InsertText

Adds command text immediately after the current command
Adds a \n to the text
============
*/
void Cbuf_InsertText( const char *text ) {
	int		len;
	int		i;

	len = strlen( text ) + 1;

	if ( len + cmd_text.cursize > cmd_text.maxsize ) {
		Com_Printf( "Cbuf_InsertText overflowed\n" );
		return;
	}

	// move the existing command text
	for ( i = cmd_text.cursize - 1 ; i >= 0 ; i-- ) {
		cmd_text.data[ i + len ] = cmd_text.data[ i ];
	}

	// copy the new text in
	Com_Memcpy( cmd_text.data, text, len - 1 );

	// add a \n
	cmd_text.data[ len - 1 ] = '\n';

	cmd_text.cursize += len;
}


/*
============
Cbuf_ExecuteText
============
*/
void Cbuf_ExecuteText( cbufExec_t exec_when, const char *text )
{
	switch (exec_when)
	{
	case EXEC_NOW:
		if ( text && text[0] != '\0' ) {
			Com_DPrintf(S_COLOR_YELLOW "EXEC_NOW %s\n", text);
			Cmd_ExecuteString (text);
		} else {
			Cbuf_Execute();
			Com_DPrintf(S_COLOR_YELLOW "EXEC_NOW %s\n", cmd_text.data);
		}
		break;
	case EXEC_INSERT:
		Cbuf_InsertText (text);
		break;
	case EXEC_APPEND:
		Cbuf_AddText (text);
		break;
	default:
		Com_Error (ERR_FATAL, "Cbuf_ExecuteText: bad exec_when");
	}
}


/*
============
Cbuf_Execute
============
*/
void Cbuf_Execute( void )
{
	int i;
	char *text;
	char line[MAX_CMD_LINE];
	int quotes;

	// This will keep // style comments all on one line by not breaking on
	// a semicolon.  It will keep /* ... */ style comments all on one line by not
	// breaking it for semicolon or newline.
	qboolean in_star_comment = qfalse;
	qboolean in_slash_comment = qfalse;
	while ( cmd_text.cursize > 0 )
	{
		if ( cmd_wait > 0 ) {
			// skip out while text still remains in buffer, leaving it
			// for next frame
			cmd_wait--;
			break;
		}

		// find a \n or ; line break or comment: // or /* */
		text = (char *)cmd_text.data;

		quotes = 0;
		for ( i = 0 ; i< cmd_text.cursize ; i++ )
		{
			if (text[i] == '"')
				quotes++;

			if ( !(quotes&1)) {
				if ( i < cmd_text.cursize - 1 ) {
					if ( !in_star_comment && text[i] == '/' && text[i+1] == '/' )
						in_slash_comment = qtrue;
					else if ( !in_slash_comment && text[i] == '/' && text[i+1] == '*' )
						in_star_comment = qtrue;
					else if ( in_star_comment && text[i] == '*' && text[i+1] == '/' ) {
						in_star_comment = qfalse;
						// If we are in a star comment, then the part after it is valid
						// Note: This will cause it to NUL out the terminating '/'
						// but ExecuteString doesn't require it anyway.
						i++;
						break;
					}
				}
				if ( !in_slash_comment && !in_star_comment && text[i] == ';')
					break;
			}
			if ( !in_star_comment && (text[i] == '\n' || text[i] == '\r') ) {
				in_slash_comment = qfalse;
				break;
			}
		}

		if ( i >= (MAX_CMD_LINE - 1) )
			i = MAX_CMD_LINE - 1;

		Com_Memcpy( line, text, i );
		line[i] = '\0';

		// delete the text from the command buffer and move remaining commands down
		// this is necessary because commands (exec) can insert data at the
		// beginning of the text buffer

		if ( i == cmd_text.cursize )
			cmd_text.cursize = 0;
		else
		{
			i++;
			cmd_text.cursize -= i;
			// skip all repeating newlines/semicolons
			while ( ( text[i] == '\n' || text[i] == '\r' || text[i] == ';' ) && cmd_text.cursize > 0 ) {
				cmd_text.cursize--;
				i++;
			}
			memmove( text, text+i, cmd_text.cursize );
		}

		// execute the command line
		Cmd_ExecuteString( line );
	}
}


/*
==============================================================================

						SCRIPT COMMANDS

==============================================================================
*/


/*
===============
Cmd_Exec_f
===============
*/
static void Cmd_Exec_f( void ) {
	qboolean quiet;
	union {
		char *c;
		void *v;
	} f;
	char filename[MAX_QPATH];
	int filter = 0;

	quiet = !Q_stricmp(Cmd_Argv(0), "execq");

	if (Cmd_Argc () != 2) {
		Com_Printf ("exec%s <filename> : execute a script file%s\n",
			quiet ? "q" : "", quiet ? " without notification" : "");
		return;
	}

	Q_strncpyz( filename, Cmd_Argv(1), sizeof( filename ) );
	COM_DefaultExtension( filename, sizeof( filename ), ".cfg" );

	// only load default.cfg from paks in release
#ifdef NDEBUG
	if ( !FS_FilenameCompare( filename, "default.cfg" ) )
		filter = FS_EXCLUDE_DIR;
	else
#endif

	if ( filter != 0 ) FS_SetFilterFlag( filter );
	FS_BypassPure();
	FS_ReadFile( filename, &f.v );
	FS_RestorePure();
	if ( filter != 0 ) FS_SetFilterFlag( 0 );
	if ( f.v == NULL ) {
		Com_Printf( "couldn't exec %s\n", filename );
		return;
	}
	if (!quiet)
		Com_Printf ("execing %s\n", filename);

	Cbuf_InsertText( f.c );

#ifdef DELAY_WRITECONFIG
	if ( strstr( filename, Q3CONFIG_CFG ) ) {
		Com_WriteConfiguration(); // to avoid loading outdated values
	}
#endif

	FS_FreeFile( f.v );
}


/*
===============
Cmd_Vstr_f

Inserts the current value of a variable as command text
===============
*/
static void Cmd_Vstr_f( void ) {
	const char *v;

	if ( Cmd_Argc () != 2 ) {
		Com_Printf( "vstr <variablename> : execute a variable command\n" );
		return;
	}

	v = Cvar_VariableString( Cmd_Argv( 1 ) );
	Cbuf_InsertText( v );
}


/*
===============
Cmd_Echo_f

Just prints the rest of the line to the console
===============
*/
static void Cmd_Echo_f( void )
{
#ifdef DEDICATED
	Com_Printf ("%s\n", Cmd_ArgsFrom( 1 ) );
#else
	if ( com_dedicated && !com_dedicated->integer ) {

		if ( com_cl_running && com_cl_running->integer && CL_CgameRunning() )
			Cbuf_AddText( va( "cpm \"%s\"\n", Cmd_ArgsFrom( 1 ) ) );
		else
			Com_Printf( "%s\n", Cmd_ArgsFrom( 1 ) );
	}
	else {
		Com_Printf( "%s\n", Cmd_ArgsFrom( 1 ) );
	}
#endif
}


/*
=============================================================================

					COMMAND EXECUTION

=============================================================================
*/

typedef struct cmd_function_s
{
	struct cmd_function_s	*next;
	char					*name;
	xcommand_t				function;
	xcommandCompFunc_t		complete;
	module_t				init_module;
	uint32_t				modules;
} cmd_function_t;


static	int			cmd_argc;
static	char		*cmd_argv[MAX_STRING_TOKENS];		// points into cmd_tokenized
static	char		cmd_tokenized[BIG_INFO_STRING+MAX_STRING_TOKENS];	// will have 0 bytes inserted
static	char		cmd_cmd[BIG_INFO_STRING]; // the original command we received (no token processing)

static	cmd_function_t	*cmd_functions;		// possible commands to execute

/*
============
Cmd_Argc
============
*/
int Cmd_Argc( void ) {
	return cmd_argc;
}


/*
============
Cmd_Clear
============
*/
void Cmd_Clear( void ) {
	cmd_cmd[0] = '\0';
	cmd_argc = 0;
}


/*
============
Cmd_Argv
============
*/
const char *Cmd_Argv( int arg ) {
	if ( (unsigned)arg >= cmd_argc ) {
		return "";
	}
	return cmd_argv[arg];
}


/*
============
Cmd_ArgvBuffer

The interpreted versions use this because
they can't have pointers returned to them
============
*/
void Cmd_ArgvBuffer( int arg, char *buffer, int bufferLength ) {
	Q_strncpyz( buffer, Cmd_Argv( arg ), bufferLength );
}


/*
============
Cmd_ArgsFrom

Returns a single string containing argv(arg) to argv(argc()-1)
============
*/
const char *Cmd_ArgsFrom( int arg ) {
	static char cmd_args[BIG_INFO_STRING], *s;
	int i;

	s = cmd_args;
	*s = '\0';
	if (arg < 0)
		arg = 0;
	for ( i = arg ; i < cmd_argc ; i++ ) {
		s = Q_stradd( s, cmd_argv[i] );
		if ( i != cmd_argc-1 ) {
			s = Q_stradd( s, " " );
		}
	}

	return cmd_args;
}

/*
============
Cmd_ArgsRange

Returns a single string containing argv(arg) to argv(count-1)
============
*/
const char *Cmd_ArgsRange( int arg, int count ) {
	static char cmd_args[BIG_INFO_STRING], *s;
	int i;

	s = cmd_args;
	*s = '\0';
	if (arg < 0)
		arg = 0;
	if (count > cmd_argc)
		count = cmd_argc;
	for ( i = arg ; i < count ; i++ ) {
		s = Q_stradd( s, cmd_argv[i] );
		if ( i != count-1 ) {
			s = Q_stradd( s, " " );
		}
	}

	return cmd_args;
}

/*
============
Cmd_ArgsBuffer

The interpreted versions use this because
they can't have pointers returned to them
============
*/
void Cmd_ArgsBuffer( char *buffer, int bufferLength ) {
	Q_strncpyz( buffer, Cmd_ArgsFrom( 1 ), bufferLength );
}


/*
============
Cmd_Cmd

Retrieve the unmodified command string
For rcon use when you want to transmit without altering quoting
ATVI Wolfenstein Misc #284
============
*/
const char *Cmd_Cmd( void )
{
	return cmd_cmd;
}


/*
   Replace command separators with space to prevent interpretation
   This is a hack to protect buggy qvms
   https://bugzilla.icculus.org/show_bug.cgi?id=3593
   https://bugzilla.icculus.org/show_bug.cgi?id=4769
*/
void Cmd_Args_Sanitize( const char *separators, qboolean caplength )
{
	int i;

	for( i = 1; i < cmd_argc; i++ )
	{
		char *c = cmd_argv[i];

		if ( caplength )
			if(strlen(c) > MAX_CVAR_VALUE_STRING - 1)
				c[MAX_CVAR_VALUE_STRING - 1] = '\0';

		while ( ( c = strpbrk( c, separators ) ) != NULL ) {
			*c = ' ';
			++c;
		}
	}
}


/*
============
Cmd_TokenizeString

Parses the given string into command line tokens.
The text is copied to a separate buffer and 0 characters
are inserted in the appropriate place, The argv array
will point into this temporary buffer.
============
*/
// NOTE TTimo define that to track tokenization issues
//#define TKN_DBG
static void Cmd_TokenizeString2( const char *text_in, qboolean ignoreQuotes ) {
	const char *text;
	char *textOut;

#ifdef TKN_DBG
	// FIXME TTimo blunt hook to try to find the tokenization of userinfo
	Com_DPrintf("Cmd_TokenizeString: %s\n", text_in);
#endif

	// clear previous args
	cmd_argc = 0;
	cmd_cmd[0] = '\0';

	if ( !text_in ) {
		return;
	}

	Q_strncpyz( cmd_cmd, text_in, sizeof( cmd_cmd ) );

	text = cmd_cmd; // read from safe-length buffer
	textOut = cmd_tokenized;

	while ( 1 ) {
		if ( cmd_argc >= ARRAY_LEN( cmd_argv ) ) {
			return;			// this is usually something malicious
		}

		while ( 1 ) {
			// skip whitespace
			while ( *text && *text <= ' ' ) {
				text++;
			}
			if ( !*text ) {
				return;			// all tokens parsed
			}

			// skip // comments
			if ( text[0] == '/' && text[1] == '/' ) {
				// accept protocol headers (e.g. http://) in command lines that matching "*?[a-z]://" pattern
				if ( text < cmd_cmd + 3 || text[-1] != ':' || text[-2] < 'a' || text[-2] > 'z' ) {
					return; // all tokens parsed
				}
			}

			// skip /* */ comments
			if ( text[0] == '/' && text[1] =='*' ) {
				while ( *text && ( text[0] != '*' || text[1] != '/' ) ) {
					text++;
				}
				if ( !*text ) {
					return;		// all tokens parsed
				}
				text += 2;
			} else {
				break;			// we are ready to parse a token
			}
		}

		// handle quoted strings
		// NOTE TTimo this doesn't handle \" escaping
		if ( !ignoreQuotes && *text == '"' ) {
			cmd_argv[cmd_argc] = textOut;
			cmd_argc++;
			text++;
			while ( *text && *text != '"' ) {
				*textOut++ = *text++;
			}
			*textOut++ = '\0';
			if ( !*text ) {
				return;		// all tokens parsed
			}
			text++;
			continue;
		}

		// regular token
		cmd_argv[cmd_argc] = textOut;
		cmd_argc++;

		// skip until whitespace, quote, or command
		while ( *text > ' ' ) {
			if ( !ignoreQuotes && text[0] == '"' ) {
				break;
			}

			if ( text[0] == '/' && text[1] == '/' ) {
				// accept protocol headers (e.g. http://) in command lines that matching "*?[a-z]://" pattern
				if ( text < cmd_cmd + 3 || text[-1] != ':' || text[-2] < 'a' || text[-2] > 'z' ) {
					break;
				}
			}

			// skip /* */ comments
			if ( text[0] == '/' && text[1] =='*' ) {
				break;
			}

			*textOut++ = *text++;
		}

		*textOut++ = '\0';

		if ( !*text ) {
			return;		// all tokens parsed
		}
	}
}


/*
============
Cmd_TokenizeString
============
*/
void Cmd_TokenizeString( const char *text_in ) {
	Cmd_TokenizeString2( text_in, qfalse );
}


/*
============
Cmd_TokenizeStringIgnoreQuotes
============
*/
void Cmd_TokenizeStringIgnoreQuotes( const char *text_in ) {
	Cmd_TokenizeString2( text_in, qtrue );
}


/*
============
Cmd_FindCommand
============
*/
static cmd_function_t *Cmd_FindCommand( const char *cmd_name )
{
	cmd_function_t *cmd;
	for( cmd = cmd_functions; cmd; cmd = cmd->next )
		if( !Q_stricmp( cmd_name, cmd->name ) )
			return cmd;
	return NULL;
}


/*
============
Cmd_AddCommand
============
*/
void Cmd_AddCommand( const char *cmd_name, xcommand_t function ) {
	cmd_function_t *cmd;

	// fail if the command already exists
	cmd = Cmd_FindCommand( cmd_name );
	if ( cmd )
	{
		// allow completion-only commands to be silently doubled
		if ( function != NULL )
			Com_Printf( "Cmd_AddCommand: %s already defined\n", cmd_name );
		return;
	}

	// use a small malloc to avoid zone fragmentation
	cmd = S_Malloc( sizeof( *cmd ) );
	cmd->name = CopyString( cmd_name );
	cmd->function = function;
	cmd->complete = NULL;
	cmd->init_module = MODULE_NONE;
	cmd->modules = 0;

	// add the command
	if ( cmd_functions == NULL || Q_stricmp(cmd_functions->name, cmd_name) > 0 ) {
		// insert as the first command
		cmd_function_t *next = cmd_functions;
		cmd_functions = cmd;
		cmd->next = next;
	} else {
		// insert after some other command
		cmd_function_t *curr = cmd_functions;
		cmd_function_t *prev = cmd_functions;
		cmd_function_t *next;
		for (;;) {
			if ( Q_stricmp(curr->name, cmd_name) > 0 )
				break;

			prev = curr;
			if ( curr->next == NULL )
				break;

			curr = curr->next;
		}
		next = prev->next;
		prev->next = cmd;
		cmd->next = next;
	}
}


/*
============
Cmd_SetCommandCompletionFunc
============
*/
void Cmd_SetCommandCompletionFunc( const char *command, xcommandCompFunc_t complete ) {
	cmd_function_t *cmd = Cmd_FindCommand( command );

	if ( cmd ) {
		cmd->complete = complete;
	}
}


static void Cmd_Free( cmd_function_t *cmd ) {
	if ( cmd->name ) {
		Z_Free( cmd->name );
	}
	Z_Free( cmd );
}


/*
============
Cmd_RemoveCommand
============
*/
void Cmd_RemoveCommand( const char *cmd_name ) {
	cmd_function_t *cmd, **back;

	back = &cmd_functions;
	while( 1 ) {
		cmd = *back;
		if ( !cmd ) {
			// command wasn't active
			return;
		}
		if ( !Q_stricmp( cmd_name, cmd->name ) ) {
			*back = cmd->next;
			Cmd_Free( cmd );
			return;
		}
		back = &cmd->next;
	}
}


static const char *safeCommands[] = {
	"+button4",
	"-button4",
	"+lookup",
	"-lookup",
	"+lookdown",
	"-lookdown",
	"configstrings"
};

static const size_t numSafeCommands = ARRAY_LEN( safeCommands );

/*
============
Cmd_RemoveCommandSafe

Only remove commands with no associated function
============
*/
void Cmd_RemoveCommandSafe( const char *cmd_name )
{
	const cmd_function_t *cmd = Cmd_FindCommand( cmd_name );

	if( !cmd )
		return;
	if( cmd->function )
	{
		const char *ret = (const char *)Q_LinearSearch( cmd_name, safeCommands, numSafeCommands, sizeof( safeCommands[0] ), (cmpFunc_t)Q_stricmp );
		if ( ret ) {
			Com_Printf( S_COLOR_RED "Restricted source tried to remove system command \"%s\"\n", cmd_name );
		}
		//else {
			//Com_Error( ERR_DROP, "Restricted source tried to remove "
			//	"system command \"%s\"", cmd_name );
		//}
		return;
	}

	Cmd_RemoveCommand( cmd_name );
}


/*
============
Cmd_CommandCompletion
============
*/
void Cmd_CommandCompletion( void(*callback)(const char *s) ) {
	const cmd_function_t *cmd;

	for ( cmd = cmd_functions ; cmd ; cmd=cmd->next ) {
		callback( cmd->name );
	}
}


/*
============
Cmd_CompleteArgument
============
*/
qboolean Cmd_CompleteArgument( const char *command, char *args, int argNum ) {
	const cmd_function_t *cmd;

	for( cmd = cmd_functions; cmd; cmd = cmd->next ) {
		if ( !Q_stricmp( command, cmd->name ) ) {
			if ( cmd->complete ) {
				cmd->complete( args, argNum );
			}
			return qtrue;
		}
	}

	return qfalse;
}


/*
============
Cmd_ExecuteString

A complete command line has been parsed, so try to execute it
============
*/
void Cmd_ExecuteString( const char *text ) {
	cmd_function_t *cmd, **prev;

	// execute the command line
	Cmd_TokenizeString( text );
	if ( !Cmd_Argc() ) {
		return;		// no tokens
	}

	// check registered command functions
	for ( prev = &cmd_functions ; *prev ; prev = &cmd->next ) {
		cmd = *prev;
		if ( !Q_stricmp( cmd_argv[0], cmd->name ) ) {
			// rearrange the links so that the command will be
			// near the head of the list next time it is used
			*prev = cmd->next;
			cmd->next = cmd_functions;
			cmd_functions = cmd;

			// perform the action
			if ( !cmd->function ) {
				// let the cgame or game handle it
				break;
			} else {
				cmd->function();
			}
			return;
		}
	}

	// check cvars
	if ( Cvar_Command() ) {
		return;
	}

#ifndef DEDICATED
	// check client game commands
	if ( com_cl_running && com_cl_running->integer && CL_GameCommand() ) {
		return;
	}
#endif

	// check server game commands
	if ( com_sv_running && com_sv_running->integer && SV_GameCommand() ) {
		return;
	}

#ifndef DEDICATED
	// check ui commands
	if ( com_cl_running && com_cl_running->integer && UI_GameCommand() ) {
		return;
	}

	// send it as a server command if we are connected
	// this will usually result in a chat message
	CL_ForwardCommandToServer( text );
#endif
}


/*
============
Cmd_List_f
============
*/
static void Cmd_List_f( void )
{
	const cmd_function_t *cmd;
	const char *match;
	int i;

	if ( Cmd_Argc() > 1 ) {
		match = Cmd_Argv( 1 );
	} else {
		match = NULL;
	}

	i = 0;
	for ( cmd = cmd_functions ; cmd ; cmd=cmd->next ) {
		if ( match && !Com_Filter( match, cmd->name ) )
			continue;
		Com_Printf( "%s\n", cmd->name );
		i++;
	}
	Com_Printf( "%i commands\n", i );
}


/*
==================
Cmd_CompleteCfgName
==================
*/
static void Cmd_CompleteCfgName( char *args, int argNum ) {
	if( argNum == 2 ) {
		Field_CompleteFilename( "", "cfg", qfalse, FS_MATCH_ANY | FS_MATCH_STICK );
	}
}


/*
==================
Cmd_CompleteWriteCfgName
==================
*/
void Cmd_CompleteWriteCfgName( char *args, int argNum ) {
	if( argNum == 2 ) {
		Field_CompleteFilename( "", "cfg", qfalse, FS_MATCH_EXTERN | FS_MATCH_STICK );
	}
}

static const cmdListItem_t cmd_cmds[] = {
	{ "cmdlist", Cmd_List_f, NULL },
	{ "echo", Cmd_Echo_f, NULL },
	{ "exec", Cmd_Exec_f, Cmd_CompleteCfgName },
	{ "execq", Cmd_Exec_f, Cmd_CompleteCfgName },
	{ "vstr", Cmd_Vstr_f, Cvar_CompleteCvarName },
	{ "wait", Cmd_Wait_f, NULL }
};

/*
============
Cmd_Init
============
*/
void Cmd_Init( void ) {
	Cmd_RegisterArray( cmd_cmds, MODULE_COMMON );
}


void Cmd_RegisterList( const cmdListItem_t *cmds, int count, module_t module ) {
	int i;
	for ( i = 0; i < count; i++ ) {
		const cmdListItem_t* item = &cmds[i];

		Cmd_AddCommand( item->name, item->func );

		if ( item->complete )
			Cmd_SetCommandCompletionFunc( item->name, item->complete );

		Cmd_SetModule( item->name, module );
	}
}


void Cmd_UnregisterList( const cmdListItem_t *cmds, int count ) {
	int i;
	for ( i = 0; i < count; i++ ) {
		Cmd_RemoveCommand( cmds[i].name );
	}
}


void Cmd_SetModule( const char *cmd_name, module_t module ) {
	cmd_function_t *cmd = Cmd_FindCommand( cmd_name );
	if ( !cmd )
		return;

	cmd->modules |= (1 << (uint32_t)module);
	if ( cmd->init_module == MODULE_NONE )
		cmd->init_module = module;
}


void Cmd_UnregisterModule( module_t module ) {
	if ( module <= MODULE_NONE || module >= MODULE_COUNT )
		return;

	cmd_function_t *cmd = cmd_functions;
	while( cmd ) {
		if ( cmd->init_module == module && cmd->modules == (1u << (uint32_t)module) ) {
			cmd_function_t *next = cmd->next;
			Cmd_RemoveCommand( cmd->name );
			cmd = next;
		} else {
			cmd = cmd->next;
		}
	}
}
