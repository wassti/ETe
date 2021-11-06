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
#if !( defined __linux__ || defined __FreeBSD__ || defined __OpenBSD__ || defined __sun || defined __APPLE__ )
#error You should include this file only on Linux/FreeBSD/Solaris platforms
#endif

#ifndef __GLW_LINUX_H__
#define __GLW_LINUX_H__

#include <X11/Xlib.h>
#include <X11/Xfuncproto.h>

typedef struct sym_s
{
	void **symbol;
	const char *name;
} sym_t;

typedef struct
{
	void *OpenGLLib; // instance of OpenGL library
	void *VulkanLib; // instance of Vulkan library
	FILE *log_fp;

	int	monitorCount;

	qboolean gammaSet;

	qboolean cdsFullscreen;

	glconfig_t *config; // feedback to renderer module

	qboolean dga_ext;

	qboolean vidmode_ext;
	qboolean vidmode_active;
	qboolean vidmode_gamma;

	qboolean randr_ext;
	qboolean randr_active;
	qboolean randr_gamma;

	qboolean desktop_ok;
	int desktop_width;
	int desktop_height;
	int desktop_x;
	int desktop_y;

} glwstate_t;

extern glwstate_t glw_state;
extern Display *dpy;
extern Window win;
extern int scrnum;

qboolean BuildGammaRampTable( unsigned char *red, unsigned char *green, unsigned char *blue, int gammaRampSize, unsigned short table[3][4096] );

// DGA extension
qboolean DGA_Init( Display *_dpy );
void DGA_Mouse( qboolean enable );
void DGA_Done( void );

// VidMode extension
qboolean VidMode_Init( void );
void VidMode_Done( void );
qboolean VidMode_SetMode( int *width, int *height, int *rate );
void VidMode_RestoreMode( void );
void VidMode_SetGamma( unsigned char red[256], unsigned char green[256], unsigned char blue[256] );
void VidMode_RestoreGamma( void );

// XRandR extension
qboolean RandR_Init( int x, int y, int w, int h );
void RandR_Done( void );
void RandR_UpdateMonitor( int x, int y, int w, int h );
qboolean RandR_SetMode( int *width, int *height, int *rate );
void RandR_RestoreMode( void );
void RandR_SetGamma( unsigned char red[256], unsigned char green[256], unsigned char blue[256] );
void RandR_RestoreGamma( void );

#endif
