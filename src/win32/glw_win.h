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
#ifndef _WIN32
#  error You should not be including this file on this platform
#endif

#ifndef __GLW_WIN_H__
#define __GLW_WIN_H__

typedef struct
{
	WNDPROC		wndproc;

	HDC     hDC;			// handle to device context
	HGLRC   hGLRC;			// handle to GL rendering context

	HINSTANCE   OpenGLLib;  // HINSTANCE for the OpenGL library
	char		gl_extensions[ 16384 ]; // to store full extension string
	// Ensiform ^ fixme? wtf?

	qboolean	pixelFormatSet;
	int			nPendingPF;

	int			desktopBitsPixel;
	int			desktopWidth; 
	int			desktopHeight;
	int			desktopX;		// can be negative
	int			desktopY;		// can be negative
	HMONITOR	hMonitor;		// current monitor
	TCHAR		displayName[CCHDEVICENAME];
	qboolean	gammaSet;

	qboolean	cdsFullscreen;
	int			monitorCount;

	FILE		*log_fp; // TODO: implement?

} glwstate_t;

extern glwstate_t glw_state;

extern void GLW_InitGamma( glconfig_t *config );
extern void GLW_RestoreGamma( void );

#endif
