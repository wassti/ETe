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

#ifndef TR_LOCAL_H
#define TR_LOCAL_H

#define USE_LEGACY_DLIGHTS	// vet dynamic lights
#define USE_PMLIGHT			// promode dynamic lights via \r_dlightMode 1
#define MAX_REAL_DLIGHTS	(MAX_DLIGHTS*2)
#define MAX_LITSURFS		(MAX_DRAWSURFS)

#define MAX_TEXTURE_SIZE	2048 // must be less or equal to 32768

#define USE_TESS_NEEDS_NORMAL
//#define USE_TESS_NEEDS_ST2

#include "../qcommon/q_shared.h"
#include "../qcommon/qfiles.h"
#include "../qcommon/qcommon.h"
#include "../renderercommon/tr_public.h"
#include "tr_common.h"
#include "iqm.h"
#include "qgl.h"

#define GL_INDEX_TYPE		GL_UNSIGNED_INT

typedef uint32_t glIndex_t;

#define	REFENTITYNUM_BITS	12	// as we actually using only 1 bit for dlight mask in opengl1 renderer
#define	REFENTITYNUM_MASK	((1<<REFENTITYNUM_BITS) - 1)
// the last N-bit number (2^REFENTITYNUM_BITS - 1) is reserved for the special world refentity,
//  and this is reflected by the value of MAX_REFENTITIES (which therefore is not a power-of-2)
#define	MAX_REFENTITIES		((1<<REFENTITYNUM_BITS) - 1)
#define	REFENTITYNUM_WORLD	((1<<REFENTITYNUM_BITS) - 1)
// 14 bits
// can't be increased without changing bit packing for drawsurfs
// see QSORT_SHADERNUM_SHIFT
#define SHADERNUM_BITS	14
#define MAX_SHADERS		(1<<SHADERNUM_BITS)
#define SHADERNUM_MASK	(MAX_SHADERS-1)




// a trRefEntity_t has all the information passed in by
// the client game, as well as some locally derived info

// ydnar: optimizing diffuse lighting calculation with a table lookup
#define ENTITY_LIGHT_STEPS      64

typedef struct {
	refEntity_t e;

	float axisLength;           // compensate for non-normalized axis
#ifdef USE_LEGACY_DLIGHTS
	int			needDlights;	// 1 for bmodels that touch a dlight
#endif
	qboolean lightingCalculated;
	vec3_t lightDir;            // normalized direction towards light
	vec3_t ambientLight;        // color normalized to 0-255
	int ambientLightInt;            // 32 bit rgba packed
	vec3_t directedLight;
	int entityLightInt[ ENTITY_LIGHT_STEPS ];
	float brightness;
#ifdef USE_PMLIGHT
	vec3_t		shadowLightDir;	// normalized direction towards light
#endif
	qboolean	intShaderTime;
} trRefEntity_t;


typedef struct {
	vec3_t		origin;			// in world coordinates
	vec3_t		axis[3];		// orientation in world
	vec3_t		viewOrigin;		// viewParms->or.origin in local coordinates
	float		modelMatrix[16];
} orientationr_t;

//===============================================================================

typedef enum {
	SS_BAD,
	SS_PORTAL,          // mirrors, portals, viewscreens
	SS_ENVIRONMENT,     // sky box
	SS_OPAQUE,          // opaque

	SS_DECAL,           // scorch marks, etc.
	SS_SEE_THROUGH,     // ladders, grates, grills that may have small blended edges
						// in addition to alpha test
	SS_BANNER,

	SS_FOG,

	SS_UNDERWATER,      // for items that should be drawn in front of the water plane

	SS_BLEND0,          // regular transparency and filters
	SS_BLEND1,          // generally only used for additive type effects
	SS_BLEND2,
	SS_BLEND3,

	SS_BLEND6,
	SS_STENCIL_SHADOW,
	SS_ALMOST_NEAREST,  // gun smoke puffs

	SS_NEAREST          // blood blobs
} shaderSort_t;


#define MAX_SHADER_STAGES 8

typedef enum {
	GF_NONE,

	GF_SIN,
	GF_SQUARE,
	GF_TRIANGLE,
	GF_SAWTOOTH,
	GF_INVERSE_SAWTOOTH,

	GF_NOISE

} genFunc_t;


typedef enum {
	DEFORM_NONE,
	DEFORM_WAVE,
	DEFORM_NORMALS,
	DEFORM_BULGE,
	DEFORM_MOVE,
	DEFORM_PROJECTION_SHADOW,
	DEFORM_AUTOSPRITE,
	DEFORM_AUTOSPRITE2,
	DEFORM_TEXT0,
	DEFORM_TEXT1,
	DEFORM_TEXT2,
	DEFORM_TEXT3,
	DEFORM_TEXT4,
	DEFORM_TEXT5,
	DEFORM_TEXT6,
	DEFORM_TEXT7
} deform_t;

typedef enum {
	AGEN_IDENTITY,
	AGEN_SKIP,
	AGEN_ENTITY,
	AGEN_ONE_MINUS_ENTITY,
	AGEN_NORMALZFADE,   // Ridah
	AGEN_VERTEX,
	AGEN_ONE_MINUS_VERTEX,
	AGEN_LIGHTING_SPECULAR,
	AGEN_WAVEFORM,
	AGEN_PORTAL,
	AGEN_CONST
} alphaGen_t;

typedef enum {
	CGEN_BAD,
	CGEN_IDENTITY_LIGHTING, // tr.identityLight
	CGEN_IDENTITY,          // always (1,1,1,1)
	CGEN_ENTITY,            // grabbed from entity's modulate field
	CGEN_ONE_MINUS_ENTITY,  // grabbed from 1 - entity.modulate
	CGEN_EXACT_VERTEX,      // tess.vertexColors
	CGEN_VERTEX,            // tess.vertexColors * tr.identityLight
	CGEN_ONE_MINUS_VERTEX,
	CGEN_WAVEFORM,          // programmatically generated
	CGEN_LIGHTING_DIFFUSE,
	CGEN_FOG,               // standard fog
	CGEN_CONST              // fixed color
} colorGen_t;

typedef enum {
	TCGEN_BAD,
	TCGEN_IDENTITY,         // clear to 0,0
	TCGEN_LIGHTMAP,
	TCGEN_TEXTURE,
	TCGEN_ENVIRONMENT_MAPPED,
	TCGEN_ENVIRONMENT_MAPPED_FP, // with correct first-person mapping
	TCGEN_FIRERISEENV_MAPPED,
	TCGEN_FOG,
	TCGEN_VECTOR            // S and T from world coordinates
} texCoordGen_t;

typedef enum {
	ACFF_NONE,
	ACFF_MODULATE_RGB,
	ACFF_MODULATE_RGBA,
	ACFF_MODULATE_ALPHA
} acff_t;

typedef struct {
	float base;
	float amplitude;
	float phase;
	float frequency;

	genFunc_t	func;
} waveForm_t;

#define TR_MAX_TEXMODS 4

typedef enum {
	TMOD_NONE,
	TMOD_TRANSFORM,
	TMOD_TURBULENT,
	TMOD_SCROLL,
	TMOD_SCALE,
	TMOD_STRETCH,
	TMOD_ROTATE,
	TMOD_ENTITY_TRANSLATE,
	TMOD_SWAP
} texMod_t;

#define MAX_SHADER_DEFORMS  3
typedef struct {
	deform_t deformation;               // vertex coordinate modification type

	vec3_t moveVector;
	waveForm_t deformationWave;
	float deformationSpread;

	float bulgeWidth;
	float bulgeHeight;
	float bulgeSpeed;
} deformStage_t;


typedef struct {
	texMod_t type;

	// used for TMOD_TURBULENT and TMOD_STRETCH
	waveForm_t wave;

	// used for TMOD_TRANSFORM
	float matrix[2][2];                 // s' = s * m[0][0] + t * m[1][0] + trans[0]
	float translate[2];                 // t' = s * m[0][1] + t * m[0][1] + trans[1]

	// used for TMOD_SCALE
	float scale[2];                     // s *= scale[0]
										// t *= scale[1]

	// used for TMOD_SCROLL
	float scroll[2];                    // s' = s + scroll[0] * time
										// t' = t + scroll[1] * time

	// + = clockwise
	// - = counterclockwise
	float rotateSpeed;

} texModInfo_t;


// RF increased this for onfire animation
//#define	MAX_IMAGE_ANIMATIONS	8
#define	MAX_IMAGE_ANIMATIONS	24
#define MAX_IMAGE_ANIMATIONS_VET	16

// Arnout: FIXME change the is* qbooleans to a type index
typedef struct {
	image_t			*image[MAX_IMAGE_ANIMATIONS];
	int				numImageAnimations;
	double			imageAnimationSpeed;	// -EC- set to double

	texCoordGen_t	tcGen;
	vec3_t			tcGenVectors[2];

	int				numTexMods;
	texModInfo_t	*texMods;

	int				videoMapHandle;
	qboolean		isLightmap;
	qboolean		isVideoMap;
	qboolean		isScreenMap;
} textureBundle_t;

#define NUM_TEXTURE_BUNDLES 2

#define TESS_ST0	1<<0
#define TESS_ST1	1<<1
#define TESS_ENV0	1<<2
#define TESS_ENV1	1<<3

typedef struct {
	qboolean active;

	textureBundle_t bundle[NUM_TEXTURE_BUNDLES];

	waveForm_t rgbWave;
	colorGen_t rgbGen;

	waveForm_t alphaWave;
	alphaGen_t alphaGen;

	byte constantColor[4];                      // for CGEN_CONST and AGEN_CONST

	GLbitfield stateBits;						// GLS_xxxx mask
	GLint			mtEnv;						// 0, GL_MODULATE, GL_ADD, GL_DECAL

	acff_t adjustColorsForFog;

	// Ridah
	float zFadeBounds[2];

	qboolean isDetail;
	qboolean isFogged;              // used only for shaders that have fog disabled, so we can enable it for individual stages
	qboolean		depthFragment;

	short			vboVPindex[3];		// normal, eye-in, eye-out
	short			vboFPindex[2];		// normal, fog-blend
	
	uint32_t		color_offset;		// within current shader
	uint32_t		tex_offset[2];		// within current shader

	uint32_t		tessFlags;

} shaderStage_t;

struct shaderCommands_s;

typedef enum {
	FP_NONE,        // surface is translucent and will just be adjusted properly
	FP_EQUAL,       // surface is opaque but possibly alpha tested
	FP_LE           // surface is trnaslucent, but still needs a fog pass (fog surface)
} fogPass_t;

typedef struct {
	float cloudHeight;
	image_t     *outerbox[6], *innerbox[6];
} skyParms_t;

typedef struct {
	vec4_t color;
	float depthForOpaque;
	unsigned colorInt;                  // in packed byte format
	float tcScale;                      // texture coordinate vector scales
} fogParms_t;


typedef struct shader_s {
	char		name[MAX_QPATH];		// game path, including extension
	int			lightmapSearchIndex;	// for a shader to match, both name and lightmapIndex must match
	int			lightmapIndex;			// for rendering

	int index;                          // this shader == tr.shaders[index]
	int sortedIndex;                    // this shader == tr.sortedShaders[sortedIndex]

	float sort;                         // lower numbered shaders draw before higher numbered

	qboolean defaultShader;             // we want to return index 0 if the shader failed to
										// load for some reason, but R_FindShader should
										// still keep a name allocated for it, so if
										// something calls RE_RegisterShader again with
										// the same name, we don't try looking for it again

	qboolean explicitlyDefined;         // found in a .shader file

	int surfaceFlags;                   // if explicitlyDefined, this will have SURF_* flags
	int contentFlags;

	qboolean entityMergable;            // merge across entites optimizable (smoke, blood)

	qboolean isSky;
	skyParms_t sky;
	fogParms_t fogParms;

	float portalRange;                  // distance to fog out at

	vec4_t distanceCull;                // ydnar: opaque alpha range for foliage (inner, outer, alpha threshold, 1/(outer-inner))

	qboolean	multitextureEnv;		// if shader has multitexture stage(s)

	cullType_t cullType;                // CT_FRONT_SIDED, CT_BACK_SIDED, or CT_TWO_SIDED
	qboolean polygonOffset;             // set for decals and other items that must be offset
	unsigned int	noMipMaps:1;			// for console fonts, 2D elements, etc.
	unsigned int	noPicMip:1;				// for images that must always be full resolution
	unsigned int	noLightScale:1;

	fogPass_t fogPass;                  // draw a blended pass, possibly with depth test equals

	qboolean needsNormal;               // not all shaders will need all data to be gathered
	//qboolean needsST1;
	qboolean needsST2;
	//qboolean needsColor;

	// Ridah
	qboolean noFog;

	int numDeforms;
	deformStage_t deforms[MAX_SHADER_DEFORMS];

	int numUnfoggedPasses;
	shaderStage_t   *stages[MAX_SHADER_STAGES];

#ifdef USE_PMLIGHT
	int			lightingStage;
	int			lightingBundle;
#endif
	qboolean	isStaticShader;
	int			svarsSize;
	int			iboOffset;
	int			vboOffset;
	int			normalOffset;
	int			numIndexes;
	int			numVertexes;
	int			curVertexes;
	int			curIndexes;

	int			hasScreenMap;

	float		lightmapOffset[2];	// within merged lightmap

	void	(*optimalStageIteratorFunc)( void );

	double	clampTime;						// time this shader is clamped to - set to double for frameloss fix -EC-
	double	timeOffset;						// current time offset for this shader - set to double for frameloss fix -EC-

	struct shader_s *remappedShader;		// current shader this one is remapped too

	struct	shader_s	*next;
} shader_t;

typedef struct corona_s {
	vec3_t origin;
	vec3_t color;               // range from 0.0 to 1.0, should be color normalized
	vec3_t transformed;         // origin in local coordinate system
	float scale;                // uses r_flaresize as the baseline (1.0)
	int id;
	qboolean visible;           // still send the corona request, even if not visible, for proper fading
} corona_t;

typedef struct dlight_s {
	vec3_t origin;
	vec3_t origin2;
	vec3_t dir;		// origin2 - origin
	vec3_t color;                   // range from 0.0 to 1.0, should be color normalized
	float radius;
	float radiusInverseCubed;       // ydnar: attenuation optimization
	float intensity;                // 1.0 = fullbright, > 1.0 = overbright
	shader_t    *shader;
	int flags;

	vec3_t transformed;             // origin in local coordinate system
	vec3_t transformed2;		// origin2 in local coordinate system
	qboolean linear;
#ifdef USE_PMLIGHT
	struct litSurf_s	*head;
	struct litSurf_s	*tail;
#endif // USE_PMLIGHT
} dlight_t;

// ydnar: decal projection
typedef struct decalProjector_s
{
	shader_t    *shader;
	byte color[ 4 ];
	int fadeStartTime, fadeEndTime;
	vec3_t mins, maxs;
	vec3_t center;
	float radius, radius2;
	qboolean omnidirectional;
	int numPlanes;                  // either 5 or 6, for quad or triangle projectors
	vec4_t planes[ 6 ];
	vec4_t texMat[ 3 ][ 2 ];
	int projectorNum;               ///< global identifier
}
decalProjector_t;

// trRefdef_t holds everything that comes in refdef_t,
// as well as the locally generated scene information
typedef struct {
	int			x, y, width, height;
	float		fov_x, fov_y;
	vec3_t		vieworg;
	vec3_t		viewaxis[3];		// transformation matrix

	stereoFrame_t	stereoFrame;

	int			time;				// time in milliseconds for shader effects and other time dependent rendering issues
	int			rdflags;			// RDF_NOWORLDMODEL, etc

	// 1 bits will prevent the associated area from rendering at all
	byte		areamask[MAX_MAP_AREA_BYTES];
	qboolean	areamaskModified;	// qtrue if areamask changed since last scene

	double		floatTime;			// tr.refdef.time / 1000.0 -EC- set to double

	// text messages for deform text shaders
	char text[MAX_RENDER_STRINGS][MAX_RENDER_STRING_LENGTH];

	int num_entities;
	trRefEntity_t   *entities;

	//int dlightBits;                 // ydnar: optimization
	unsigned int num_dlights;
	dlight_t    *dlights;

	unsigned int num_coronas;
	corona_t    *coronas;

	int numPolys;
	struct srfPoly_s    *polys;

	int numPolyBuffers;
	struct srfPolyBuffer_s  *polybuffers;

	int decalBits;                  // ydnar: optimization
	int numDecalProjectors;
	decalProjector_t    *decalProjectors;

	int numDecals;
	struct srfDecal_s   *decals;

	int numDrawSurfs;
	struct drawSurf_s   *drawSurfs;
#ifdef USE_PMLIGHT
	int			numLitSurfs;
	struct litSurf_s	*litSurfs;
#endif
} trRefdef_t;


typedef struct image_s {
	char		*imgName;			// image path, including extension
	char		*imgName2;			// image path with real file extension
	struct image_s *next;			// for hash search
	int			width, height;		// source image
	int			uploadWidth;		// after power of two and picmip but not including clamp to MAX_TEXTURE_SIZE
	int			uploadHeight;
	imgFlags_t	flags;
	GLuint		texnum;				// gl texture binding

	int			frameUsed;			// for texture usage in frame statistics

	GLint		internalFormat;
	int			TMU;				// only needed for voodoo2

	int				hash;
} image_t;


//=================================================================================

// max surfaces per-skin
// This is an arbitry limit. Vanilla Q3 only supported 32 surfaces in skins but failed to
// enforce the maximum limit when reading skin files. It was possile to use more than 32
// surfaces which accessed out of bounds memory past end of skin->surfaces hunk block.
#define MAX_SKIN_SURFACES	256

// skins allow models to be retextured without modifying the model file
typedef struct {
	char name[MAX_QPATH];
	int hash;
	shader_t    *shader;
} skinSurface_t;

//----(SA) modified
#define MAX_PART_MODELS 5

typedef struct {
	char type[MAX_QPATH];           // md3_lower, md3_lbelt, md3_rbelt, etc.
	char model[MAX_QPATH];          // lower.md3, belt1.md3, etc.
	int hash;
} skinModel_t;

typedef struct skin_s {
	char name[MAX_QPATH];           // game path, including extension
	int numSurfaces;
	int numModels;
	skinSurface_t	*surfaces;			// dynamically allocated array of surfaces
	skinModel_t     *models[MAX_PART_MODELS];
} skin_t;
//----(SA) end

typedef struct {
	int modelNum;                   // ydnar: bsp model the fog belongs to
	int originalBrushNumber;
	vec3_t bounds[2];

	shader_t    *shader;            // fog shader to get colorInt and tcScale from
	fogParms_t parms;

	// for clipping distance in fog when outside
	qboolean hasSurface;
	float surface[4];
} fog_t;

typedef struct {
	float		eyeT;
	float		eyeInside; // 0.0 or 1.0
	vec4_t		fogDistanceVector;
	vec4_t		fogDepthVector;
	const float *fogColor; // vec4_t
	qboolean	level;
} fogProgramParms_t;

typedef enum {
	PV_NONE = 0,
	PV_PORTAL, // this view is through a portal
	PV_MIRROR, // portal + inverted face culling
	PV_COUNT
} portalView_t;

typedef struct {
	orientationr_t	orientation;
	orientationr_t	world;
	vec3_t		pvsOrigin;			// may be different than or.origin for portals
	portalView_t portalView;
	int			frameSceneNum;		// copied from tr.frameSceneNum
	int			frameCount;			// copied from tr.frameCount
	cplane_t	portalPlane;		// clip anything behind this if mirroring
	int			viewportX, viewportY, viewportWidth, viewportHeight;
	int			scissorX, scissorY, scissorWidth, scissorHeight;
	float		fovX, fovY;
	float		projectionMatrix[16];
	cplane_t	frustum[6];			// ydnar: added farplane
	vec3_t		visBounds[2];
	float		zFar;
	stereoFrame_t	stereoFrame;
	glfog_t		glFog;                  // fog parameters	//----(SA)	added

#ifdef USE_PMLIGHT
	// each view will have its own dlight set
	unsigned int num_dlights;
	struct dlight_s	*dlights;
#endif
} viewParms_t;


/*
==============================================================================

SURFACES

==============================================================================
*/

// any changes in surfaceType must be mirrored in rb_surfaceTable[]
// NOTE: also mirror changes to max2skl.c - Arnout: not anymore
typedef enum {
	SF_BAD,
	SF_SKIP,                // ignore
	SF_FACE,
	SF_GRID,
	SF_TRIANGLES,
	SF_FOLIAGE,
	SF_POLY,
	SF_MD3,
	SF_MDC,
	SF_MDS,
	SF_MDM,
	SF_IQM,
	SF_FLARE,
	SF_ENTITY,              // beams, rails, lightning, etc that can be determined by entity
	SF_POLYBUFFER,
	SF_DECAL,               // ydnar: decal surfaces

	SF_NUM_SURFACE_TYPES,
	SF_MAX = 0xffffffff     // ensures that sizeof( surfaceType_t ) == sizeof( int )
} surfaceType_t;

typedef struct drawSurf_s {
	unsigned int		sort;			// bit combination for fast compares
	surfaceType_t		*surface;		// any of surface*_t
} drawSurf_t;

#ifdef USE_PMLIGHT
typedef struct litSurf_s {
	unsigned int		sort;			// bit combination for fast compares
	surfaceType_t		*surface;		// any of surface*_t
	struct litSurf_s	*next;
} litSurf_t;
#endif

#define MAX_FACE_POINTS     1024

#define MAX_PATCH_SIZE      32          // max dimensions of a patch mesh in map file
#define MAX_GRID_SIZE       65          // max dimensions of a grid mesh in memory

typedef byte color4ub_t[4];

// when cgame directly specifies a polygon, it becomes a srfPoly_t
// as soon as it is called
typedef struct srfPoly_s {
	surfaceType_t	surfaceType;
	qhandle_t		hShader;
	int				fogIndex;
	int				numVerts;
	polyVert_t		*verts;
} srfPoly_t;

typedef struct srfPolyBuffer_s {
	surfaceType_t surfaceType;
	int fogIndex;
	polyBuffer_t*   pPolyBuffer;
} srfPolyBuffer_t;

// ydnar: decals
#define MAX_DECAL_VERTS         10  // worst case is triangle clipped by 6 planes
#define MAX_WORLD_DECALS        1024
#define MAX_ENTITY_DECALS       128
typedef struct srfDecal_s
{
	surfaceType_t surfaceType;
	int numVerts;
	polyVert_t verts[ MAX_DECAL_VERTS ];
}
srfDecal_t;

typedef struct srfDisplayList_s {
	surfaceType_t surfaceType;
	int listNum;
} srfDisplayList_t;

typedef struct srfFlare_s {
	surfaceType_t surfaceType;
	vec3_t origin;
	vec3_t normal;
	vec3_t color;
} srfFlare_t;


// ydnar: normal map drawsurfaces must match this header
typedef struct srfGeneric_s
{
	surfaceType_t surfaceType;

	// culling information
	vec3_t bounds[ 2 ];
	vec3_t origin;
	float radius;
	cplane_t plane;

	// dynamic lighting information
	int dlightBits;

	int vboItemIndex;
}
srfGeneric_t;


typedef struct srfGridMesh_s
{
	surfaceType_t	surfaceType;

	// culling information
	vec3_t bounds[ 2 ];
	vec3_t origin;
	float radius;
	cplane_t plane;

	// dynamic lighting information
	int				dlightBits;

	// lod information, which may be different
	// than the culling information to allow for
	// groups of curves that LOD as a unit
	vec3_t			lodOrigin;
	float			lodRadius;
	int				lodFixed;
	int				lodStitched;

	int				vboItemIndex;
	int				vboExpectIndices;
	int				vboExpectVertices;

	// vertexes
	int				width, height;
	float			*widthLodError;
	float			*heightLodError;
	drawVert_t		verts[1];		// variable sized
} srfGridMesh_t;


#define VERTEXSIZE  8

typedef struct srfSurfaceFace_s
{
	surfaceType_t surfaceType;

	// culling information
	vec3_t bounds[ 2 ];
	vec3_t origin;
	float radius;
	cplane_t plane;

	// dynamic lighting information
#ifdef USE_LEGACY_DLIGHTS
	int dlightBits;
#endif
	int			vboItemIndex;

	// triangle definitions (no normals at points)
	int numPoints;
	int numIndices;
	int ofsIndices;
	float points[1][VERTEXSIZE];            // variable sized
											// there is a variable length list of indices here also
}
srfSurfaceFace_t;


// misc_models in maps are turned into direct geometry by q3map2 ;D
typedef struct srfTriangles_s
{
	surfaceType_t surfaceType;

	// culling information
	vec3_t bounds[ 2 ];
	vec3_t origin;
	float radius;
	cplane_t plane;

	// dynamic lighting information
#ifdef USE_LEGACY_DLIGHTS
	int dlightBits;
#endif
	int				vboItemIndex;

	// triangle definitions
	int numIndexes;
	int             *indexes;

	int numVerts;
	drawVert_t      *verts;
}
srfTriangles_t;

#if 0
// Gordon: Test
typedef struct srfTriangles2_s
{
	surfaceType_t surfaceType;

	// culling information
	vec3_t bounds[ 2 ];
	vec3_t origin;
	float radius;
	cplane_t plane;

	// dynamic lighting information
	int dlightBits;

	// triangle definitions
	int numIndexes;
	int             *indexes;

	int numVerts;


	vec4hack_t*     xyz;
	vec2hack_t*     st;
	vec2hack_t*     lightmap;
	vec4hack_t*     normal;
	color4ubhack_t* color;
} srfTriangles2_t;
//
#endif

// ydnar: foliage surfaces are autogenerated from models into geometry lists by q3map2
typedef byte fcolor4ub_t[4];

typedef struct
{
	vec3_t origin;
	fcolor4ub_t color;
}
foliageInstance_t;

typedef struct
{
	surfaceType_t surfaceType;

	// culling information
	vec3_t bounds[ 2 ];
	vec3_t origin;
	float radius;
	cplane_t plane;

	// dynamic lighting information
#ifdef USE_LEGACY_DLIGHTS
	int dlightBits;
#endif

	int				vboItemIndex;

	// triangle definitions
	int numIndexes;
	glIndex_t       *indexes;

	int numVerts;
	vec4_t          *xyz;
	vec4_t          *normal;
	vec2_t          *texCoords;
	vec2_t          *lmTexCoords;

	// origins
	int numInstances;
	foliageInstance_t   *instances;
}
srfFoliage_t;

typedef struct {
	vec3_t translate;
	quat_t rotate;
	vec3_t scale;
} iqmTransform_t;

// inter-quake-model
typedef struct {
	int		num_vertexes;
	int		num_triangles;
	int		num_frames;
	int		num_surfaces;
	int		num_joints;
	int		num_poses;
	struct srfIQModel_s	*surfaces;

	int		*triangles;

	// vertex arrays
	float		*positions;
	float		*texcoords;
	float		*normals;
	float		*tangents;
	byte		*colors;
	int		*influences; // [num_vertexes] indexes into influenceBlendVertexes

	// unique list of vertex blend indexes/weights for faster CPU vertex skinning
	byte		*influenceBlendIndexes; // [num_influences]
	union {
		float	*f;
		byte	*b;
	} influenceBlendWeights; // [num_influences]

	// depending upon the exporter, blend indices and weights might be int/float
	// as opposed to the recommended byte/byte, for example Noesis exports
	// int/float whereas the official IQM tool exports byte/byte
	int		blendWeightsType; // IQM_UBYTE or IQM_FLOAT

	char		*jointNames;
	int		*jointParents;
	float		*bindJoints; // [num_joints * 12]
	float		*invBindJoints; // [num_joints * 12]
	iqmTransform_t	*poses; // [num_frames * num_poses]
	float		*bounds;
} iqmData_t;

// inter-quake-model surface
typedef struct srfIQModel_s {
	surfaceType_t	surfaceType;
	char		name[MAX_QPATH];
	shader_t	*shader;
	iqmData_t	*data;
	int		first_vertex, num_vertexes;
	int		first_triangle, num_triangles;
	int		first_influence, num_influences;
} srfIQModel_t;


extern void( *rb_surfaceTable[SF_NUM_SURFACE_TYPES] ) ( void * );

/*
==============================================================================

BRUSH MODELS

==============================================================================
*/


//
// in memory representation
//

#define SIDE_FRONT  0
#define SIDE_BACK   1
#define SIDE_ON     2

typedef struct msurface_s {
	int viewCount;                  // if == tr.viewCount, already added
	shader_t        *shader;
	int fogIndex;
#ifdef USE_PMLIGHT
	int					vcVisible;		// if == tr.viewCount, is actually VISIBLE in this frame, i.e. passed facecull and has been added to the drawsurf list
	int					lightCount;		// if == tr.lightCount, already added to the litsurf list for the current light
#endif // USE_PMLIGHT
	surfaceType_t   *data;          // any of srf*_t
} msurface_t;

// ydnar: bsp model decal surfaces
typedef struct decal_s
{
	msurface_t      *parent;
	shader_t        *shader;
	float fadeStartTime, fadeEndTime;
	int fogIndex;
	int numVerts;
	polyVert_t verts[ MAX_DECAL_VERTS ];
	int projectorNum;
	int frameAdded;                     ///< need to keep decal for at least one frame so we know not to reproject it in later views
}
decal_t;


#define CONTENTS_NODE       -1
typedef struct mnode_s {
	// common with leaf and node
	int contents;                   // -1 for nodes, to differentiate from leafs
	int visframe;                   // node needs to be traversed if current
	vec3_t mins, maxs;              // for bounding box culling
	vec3_t surfMins, surfMaxs;      // ydnar: bounding box including surfaces
	struct mnode_s  *parent;

	// node specific
	cplane_t    *plane;
	struct mnode_s  *children[2];

	// leaf specific
	int cluster;
	int area;

	msurface_t  **firstmarksurface;
	int nummarksurfaces;
} mnode_t;



typedef struct bmodel_s {
	vec3_t bounds[ 2 ];                 // for culling
	msurface_t      *firstSurface;
	int numSurfaces;

	// ydnar: decals
	decal_t         *decals;

	// ydnar: for fog volumes
	int firstBrush;
	int numBrushes;
	orientation_t orientation;
	qboolean visible;
	int entityNum;
} bmodel_t;

// ydnar: optimization
#define WORLD_MAX_SKY_NODES 32

typedef struct {
	char name[MAX_QPATH];               // ie: maps/tim_dm2.bsp
	char baseName[MAX_QPATH];           // ie: tim_dm2

	int dataSize;

	int numShaders;
	dshader_t   *shaders;

	int numBModels;
	bmodel_t    *bmodels;

	int numplanes;
	cplane_t    *planes;

	int numnodes;               // includes leafs
	int numDecisionNodes;
	mnode_t     *nodes;

	int numSkyNodes;
	mnode_t     **skyNodes;     // ydnar: don't walk the entire bsp when rendering sky

	int numsurfaces;
	msurface_t  *surfaces;

	int nummarksurfaces;
	msurface_t  **marksurfaces;

	int numfogs;
	fog_t       *fogs;
	int globalFog;                      // Arnout: index of global fog
	vec4_t globalOriginalFog;           // Arnout: to be able to restore original global fog
	vec4_t globalTransStartFog;         // Arnout: start fog for switch fog transition
	vec4_t globalTransEndFog;           // Arnout: end fog for switch fog transition
	int globalFogTransStartTime;
	int globalFogTransEndTime;

	vec3_t lightGridOrigin;
	vec3_t lightGridSize;
	vec3_t lightGridInverseSize;
	int lightGridBounds[3];
	byte        *lightGridData;

	int numClusters;
	int clusterBytes;
	const byte  *vis;           // may be passed in by CM_LoadMap to save space

	byte        *novis;         // clusterBytes of 0xff

	char        *entityString;
	const char	*entityParsePoint;
} world_t;

//======================================================================

typedef enum {
	MOD_BAD,
	MOD_BRUSH,
	MOD_MESH,
	MOD_MDS,
	MOD_MDC,
	MOD_MDM,
	MOD_MDX,
	MOD_IQM
} modtype_t;

typedef union {
	bmodel_t    *bmodel;            // only if type == MOD_BRUSH
	md3Header_t *md3[MD3_MAX_LODS]; // only if type == MOD_MESH
	mdsHeader_t *mds;               // only if type == MOD_MDS
	mdcHeader_t *mdc[MD3_MAX_LODS]; // only if type == MOD_MDC
	mdmHeader_t *mdm;               // only if type == MOD_MDM
	mdxHeader_t *mdx;               // only if type == MOD_MDX
	  iqmData_t *iqm;				// only if type == MOD_IQM
} model_u;

typedef struct model_s {
	char name[MAX_QPATH];
	modtype_t type;
	int index;                      // model = tr.models[model->index]

	int dataSize;                   // just for listing purposes

	// show_bug.cgi?id=575
	model_u model;

	int numLods;

	qhandle_t shadowShader;
	float shadowParms[ 6 ];         // x, y, width, height, depth, z offset
} model_t;


#define MAX_MOD_KNOWN   2048

void        R_ModelInit( void );
model_t     *R_GetModelByHandle( qhandle_t hModel );
int         R_LerpTag( orientation_t *tag, const refEntity_t *refent, const char *tagName, int startIndex );
void        R_ModelBounds( qhandle_t handle, vec3_t mins, vec3_t maxs );

void        R_Modellist_f( void );

//====================================================
extern refimport_t ri;

#define MAX_DRAWIMAGES          2048
#define MAX_LIGHTMAPS           256
#define MAX_SKINS               1024


#define MAX_DRAWSURFS           0x40000
#define DRAWSURF_MASK           ( MAX_DRAWSURFS - 1 )

/*

the drawsurf sort data is packed into a single 32 bit value so it can be
compared quickly during the qsorting process

the bits are allocated as follows:

(SA) modified for Wolf (11 bits of entity num)

old:

22 - 31	: sorted shader index
12 - 21	: entity index
3 - 7	: fog index
2		: used to be clipped flag
0 - 1	: dlightmap index

#define	QSORT_SHADERNUM_SHIFT	22
#define	QSORT_ENTITYNUM_SHIFT	12
#define	QSORT_FOGNUM_SHIFT		3

new:

22 - 31	: sorted shader index
11 - 21	: entity index
2 - 6	: fog index
removed	: used to be clipped flag
0 - 1	: dlightmap index

newest: (fixes shader index not having enough bytes)

18 - 31	: sorted shader index
7 - 17	: entity index
2 - 6	: fog index
0 - 1	: dlightmap index

*/
#define	DLIGHT_BITS 1 // qboolean in opengl1 renderer
#define	DLIGHT_MASK ((1<<DLIGHT_BITS)-1)
#define	FOGNUM_BITS 5
#define	FOGNUM_MASK ((1<<FOGNUM_BITS)-1)

#define	QSORT_FOGNUM_SHIFT	DLIGHT_BITS
#define	QSORT_REFENTITYNUM_SHIFT (QSORT_FOGNUM_SHIFT + FOGNUM_BITS)
#define	QSORT_SHADERNUM_SHIFT	(QSORT_REFENTITYNUM_SHIFT+REFENTITYNUM_BITS)
#if (QSORT_SHADERNUM_SHIFT+SHADERNUM_BITS) > 32
	#error "Need to update sorting, too many bits."
#endif
#define QSORT_REFENTITYNUM_MASK (REFENTITYNUM_MASK << QSORT_REFENTITYNUM_SHIFT)

extern	int			gl_filter_min, gl_filter_max;

/*
** performanceCounters_t
*/
typedef struct {
	int c_sphere_cull_in, c_sphere_cull_out;
	int c_plane_cull_in, c_plane_cull_out;

	int c_sphere_cull_patch_in, c_sphere_cull_patch_clip, c_sphere_cull_patch_out;
	int c_box_cull_patch_in, c_box_cull_patch_clip, c_box_cull_patch_out;
	int c_sphere_cull_md3_in, c_sphere_cull_md3_clip, c_sphere_cull_md3_out;
	int c_box_cull_md3_in, c_box_cull_md3_clip, c_box_cull_md3_out;

	int c_leafs;
	int c_dlightSurfaces;
	int c_dlightSurfacesCulled;

	int c_decalProjectors, c_decalTestSurfaces, c_decalClipSurfaces, c_decalSurfaces, c_decalSurfacesCreated;
#ifdef USE_PMLIGHT
	int		c_light_cull_out;
	int		c_light_cull_in;
	int		c_lit_leafs;
	int		c_lit_surfs;
	int		c_lit_culls;
	int		c_lit_masks;
#endif
} frontEndCounters_t;

#define FOG_TABLE_SIZE      256

#define FUNCTABLE_SIZE      4096    //%	1024
#define FUNCTABLE_SIZE2     12      //%	10
#define FUNCTABLE_MASK      ( FUNCTABLE_SIZE - 1 )

// the renderer front end should never modify glstate_t
typedef struct {
	GLuint		currenttextures[ MAX_TEXTURE_UNITS ];
	int			currenttmu;
	qboolean	finishCalled;
	GLint		texEnv[2];
	cullType_t	faceCulling;
	GLbitfield	glStateBits;
	GLbitfield	glClientStateBits[ MAX_TEXTURE_UNITS ];
	int			currentArray;
} glstate_t;

typedef struct glstatic_s {
	// unmodified width/height according to actual \r_mode*
	int windowWidth;
	int windowHeight;
	int captureWidth;
	int captureHeight;
	int initTime;
} glstatic_t;

typedef struct {
	int c_surfaces, c_shaders, c_vertexes, c_indexes, c_totalIndexes;
	float c_overDraw;

	int c_dlightVertexes;
	int c_dlightIndexes;

	int c_flareAdds;
	int c_flareTests;
	int c_flareRenders;

	int msec;               // total msec for backend run
#ifdef USE_PMLIGHT
	int		c_lit_batches;
	int		c_lit_vertices;
	int		c_lit_indices;
	int		c_lit_indices_latecull_in;
	int		c_lit_indices_latecull_out;
	int		c_lit_vertices_lateculltest;
#endif
} backEndCounters_t;

typedef struct videoFrameCommand_s {
	int					commandId;
	int					width;
	int					height;
	byte				*captureBuffer;
	byte				*encodeBuffer;
	qboolean			motionJpeg;
} videoFrameCommand_t;

enum {
	SCREENSHOT_TGA = 1<<0,
	SCREENSHOT_JPG = 1<<1,
	SCREENSHOT_BMP = 1<<2,
	SCREENSHOT_BMP_CLIPBOARD = 1<<3,
	SCREENSHOT_AVI = 1<<4 // take video frame
};

// all state modified by the back end is seperated
// from the front end state
typedef struct {
	trRefdef_t	refdef;
	viewParms_t	viewParms;
	orientationr_t	orientation;
	backEndCounters_t	pc;
	qboolean	isHyperspace;
	/*const */trRefEntity_t *currentEntity;
	qboolean	skyRenderedThisView;	// flag for drawing sun

	qboolean	projection2D;	// if qtrue, drawstretchpic doesn't need to change modes
	byte		color2D[4];
	qboolean	doneBloom;		// done bloom this frame
	qboolean	doneSurfaces;   // done any 3d surfaces already
	trRefEntity_t	entity2D;	// currentEntity will point at this when doing 2D rendering

	int		screenshotMask;		// tga | jpg | bmp
	char	screenshotTGA[ MAX_OSPATH ];
	char	screenshotJPG[ MAX_OSPATH ];
	char	screenshotBMP[ MAX_OSPATH ];
	qboolean screenShotTGAsilent;
	qboolean screenShotJPGsilent;
	qboolean screenShotBMPsilent;
	videoFrameCommand_t	vcmd;	// avi capture
	
	qboolean throttle;
	qboolean drawConsole;
	qboolean doneShadows;

} backEndState_t;

/*
** trGlobals_t
**
** Most renderer globals are defined here.
** backend functions should never modify any of these fields,
** but may read fields that aren't dynamically modified
** by the frontend.
*/
typedef struct {
	qboolean				registered;		// cleared at shutdown, set at beginRegistration

	int						visCount;		// incremented every time a new vis cluster is entered
	int						frameCount;		// incremented every frame
	int						sceneCount;		// incremented every scene
	int						viewCount;		// incremented every view (twice a scene if portaled)
											// and every R_MarkFragments call
#ifdef USE_PMLIGHT
	int						lightCount;		// incremented for each dlight in the view
#endif

	int						frameSceneNum;	// zeroed at RE_BeginFrame

	qboolean				worldMapLoaded;
	world_t					*world;
	char					worldRawName[MAX_QPATH];	// ydnar: for referencing external lightmaps

	const byte				*externalVisData;	// from RE_SetWorldVisData, shared with CM_Load

	image_t					*defaultImage;
	image_t					*scratchImage[ MAX_VIDEO_HANDLES ];
	image_t					*fogImage;
	image_t					*dlightImage;	// inverse-quare highlight for projective adding
	image_t					*flareImage;
	image_t					*whiteImage;			// full of 0xff
	image_t					*identityLightImage;	// full of tr.identityLightByte

	shader_t				*defaultShader;
	shader_t				*cinematicShader;
	shader_t				*shadowShader;
	shader_t				*projectionShadowShader;
	shader_t                *dlightShader;      //----(SA) added

	shader_t				*flareShader;
	char                    *sunShaderName;
	shader_t				*sunShader;
	shader_t                *sunflareShader[6];  //----(SA) for the camera lens flare effect for sun

	int						numLightmaps;
	image_t                 *lightmaps[MAX_LIGHTMAPS];
	//float					lightmapScale[2];

	trRefEntity_t			*currentEntity;
	trRefEntity_t			worldEntity;		// point currentEntity at this when rendering world
	int						currentEntityNum;
	int						shiftedEntityNum;	// currentEntityNum << QSORT_REFENTITYNUM_SHIFT
	model_t					*currentModel;
	bmodel_t                *currentBModel;     // only valid when rendering brush models

	viewParms_t				viewParms;

	float					identityLight;		// 1.0 / ( 1 << overbrightBits )
	int						identityLightByte;	// identityLight * 255
	int						overbrightBits;		// r_overbrightBits->integer, but set to 0 if no hw gamma

	orientationr_t			orientation;		// for current entity

	trRefdef_t				refdef;

	int						viewCluster;
#ifdef USE_PMLIGHT
	dlight_t				*light;				// current light during R_RecursiveLightNode
#endif
	vec3_t					sunLight;			// from the sky shader for this level
	vec3_t					sunDirection;

//----(SA)	added
	float lightGridMulAmbient;          // lightgrid multipliers specified in sky shader
	float lightGridMulDirected;         //
//----(SA)	end

//	qboolean				levelGLFog;

	frontEndCounters_t		pc;
	int						frontEndMsec;		// not in pc due to clearing issue

	//
	// put large tables at the end, so most elements will be
	// within the +/32K indexed range on risc processors
	//
	model_t					*models[MAX_MOD_KNOWN];
	int						numModels;

	int						numImages;
	image_t					*images[MAX_DRAWIMAGES];
	// Ridah
	int numCacheImages;

	// shader indexes from other modules will be looked up in tr.shaders[]
	// shader indexes from drawsurfs will be looked up in sortedShaders[]
	// lower indexed sortedShaders must be rendered first (opaque surfaces before translucent)
	int						numShaders;
	shader_t				*shaders[MAX_SHADERS];
	shader_t				*sortedShaders[MAX_SHADERS];

	int						numSkins;
	skin_t					*skins[MAX_SKINS];

	float					sinTable[FUNCTABLE_SIZE];
	float					squareTable[FUNCTABLE_SIZE];
	float					triangleTable[FUNCTABLE_SIZE];
	float					sawToothTable[FUNCTABLE_SIZE];
	float					inverseSawToothTable[FUNCTABLE_SIZE];

	qboolean				mapLoading;
	qboolean				needScreenMap;

} trGlobals_t;

extern backEndState_t backEnd;
extern trGlobals_t tr;

extern int					gl_clamp_mode;

extern glconfig_t glConfig;         // outside of TR since it shouldn't be cleared during ref re-init

extern glstate_t glState;           // outside of TR since it shouldn't be cleared during ref re-init

extern glstatic_t gls;

extern	qboolean			windowAdjusted;
extern	qboolean			superSampled;

//
// cvars
//
extern cvar_t   *r_flareSize;
extern cvar_t   *r_flareFade;

extern cvar_t   *r_railWidth;
extern cvar_t   *r_railCoreWidth;
extern cvar_t   *r_railSegmentLength;

extern cvar_t   *r_znear;               // near Z clip plane
extern cvar_t   *r_zfar;                // far Z clip plane
extern cvar_t	*r_zproj;				// z distance of projection plane
extern cvar_t	*r_stereoSeparation;			// separation of cameras for stereo rendering

extern cvar_t   *r_lodbias;             // push/pull LOD transitions
extern cvar_t   *r_lodscale;

extern cvar_t	*r_fastsky;				// controls whether sky should be cleared or drawn
extern cvar_t	*r_neatsky;				// nomip and nopicmip for skyboxes, cnq3 like look
extern cvar_t	*r_drawSun;				// controls drawing of sun quad
										// "0" no sun
										// "1" draw sun
										// "2" also draw lens flare effect centered on sun
extern cvar_t   *r_dynamiclight;        // dynamic lights enabled/disabled
extern cvar_t	*r_mergeLightmaps;
#ifdef USE_PMLIGHT
extern cvar_t	*r_dlightMode;			// 0 - vq3, 1 - pmlight
extern cvar_t	*r_dlightSpecPower;		// 1 - 32
extern cvar_t	*r_dlightSpecColor;		// -1.0 - 1.0
extern cvar_t	*r_dlightScale;			// 0.1 - 1.0
extern cvar_t	*r_dlightIntensity;		// 0.1 - 1.0
#endif
extern cvar_t	*r_dlightSaturation;	// 0.0 - 1.0
extern cvar_t	*r_vbo;
extern cvar_t	*r_fbo;
extern cvar_t	*r_hdr;
extern cvar_t	*r_bloom;
extern cvar_t	*r_bloom_threshold;
extern cvar_t	*r_bloom_threshold_mode;
extern cvar_t	*r_bloom_modulate;
extern cvar_t	*r_bloom_passes;
extern cvar_t	*r_bloom_blend_base;
extern cvar_t	*r_bloom_intensity;
extern cvar_t	*r_bloom_filter_size;
extern cvar_t	*r_bloom_reflection;

extern cvar_t	*r_renderWidth;
extern cvar_t	*r_renderHeight;
extern cvar_t	*r_renderScale;

//extern cvar_t	*r_dlightBacks;			// dlight non-facing surfaces for continuity

extern cvar_t  *r_norefresh;            // bypasses the ref rendering
extern cvar_t  *r_drawentities;         // disable/enable entity rendering
extern cvar_t  *r_drawworld;            // disable/enable world rendering
extern cvar_t  *r_drawfoliage;          // ydnar: disable/enable foliage rendering
extern cvar_t  *r_speeds;               // various levels of information display
extern cvar_t  *r_fullbright;           // avoid lightmap pass
extern cvar_t  *r_detailTextures;       // enables/disables detail texturing stages
extern cvar_t  *r_novis;                // disable/enable usage of PVS
extern cvar_t  *r_nocull;
extern cvar_t  *r_facePlaneCull;        // enables culling of planar surfaces with back side test
extern cvar_t  *r_nocurves;
extern cvar_t  *r_showcluster;

extern cvar_t	*r_gamma;
//extern cvar_t	*r_displayRefresh;				// optional display refresh option

extern cvar_t  *r_nobind;                       // turns off binding to appropriate textures
extern cvar_t  *r_singleShader;                 // make most world faces use default shader
extern cvar_t  *r_roundImagesDown;
extern cvar_t  *r_allowNonPo2;
extern cvar_t  *r_colorMipLevels;               // development aid to see texture mip usage
extern cvar_t  *r_picmip;                       // controls picmip values
extern	cvar_t	*r_nomip;						// apply picmip only on worldspawn textures
extern cvar_t  *r_finish;
extern cvar_t  *r_textureMode;
extern cvar_t  *r_offsetFactor;
extern cvar_t  *r_offsetUnits;

extern cvar_t  *r_lightmap;                     // render lightmaps only

extern cvar_t  *r_logFile;                      // number of frames to emit GL logs
extern cvar_t  *r_showtris;                     // enables wireframe rendering of the world
extern cvar_t  *r_trisColor;                    // enables modifying of the wireframe colour (in 0xRRGGBB[AA] format, alpha defaults to FF)
extern cvar_t  *r_showsky;                      // forces sky in front of all surfaces
extern cvar_t  *r_shownormals;                  // draws wireframe normals
extern cvar_t  *r_normallength;                 // length of the normals
extern cvar_t  *r_showmodelbounds;
extern cvar_t  *r_clear;                        // force screen clear every frame

extern cvar_t  *r_shadows;                      // controls shadows: 0 = none, 1 = blur, 2 = stencil, 3 = black planar projection
extern cvar_t  *r_flares;                       // light flares

extern cvar_t  *r_portalsky;    // (SA) added
extern cvar_t  *r_intensity;

extern cvar_t  *r_lockpvs;
extern cvar_t  *r_noportals;
extern cvar_t  *r_portalOnly;

extern cvar_t  *r_subdivisions;
extern cvar_t  *r_lodCurveError;
extern cvar_t  *r_skipBackEnd;

extern	cvar_t	*r_anaglyphMode;

extern	cvar_t	*r_greyscale;

extern	cvar_t	*r_ignoreGLErrors;

extern	cvar_t	*r_overBrightBits;
extern	cvar_t	*r_mapOverBrightBits;
extern	cvar_t	*r_mapGreyScale;

extern cvar_t  *r_debugSurface;
extern cvar_t  *r_simpleMipMaps;

extern	cvar_t	*r_showImages;
extern	cvar_t	*r_defaultImage;
extern	cvar_t	*r_debugSort;

extern cvar_t  *r_printShaders;
extern cvar_t  *r_saveFontData;

// Ridah
extern cvar_t  *r_cache;
extern cvar_t  *r_cacheShaders;
extern cvar_t  *r_cacheModels;

extern cvar_t  *r_cacheGathering;

extern cvar_t  *r_bonesDebug;
// done.

// Rafael - wolf fog
extern cvar_t   *r_wolffog;
// done

extern cvar_t  *r_highQualityVideo;

extern cvar_t  *r_useFirstPersonEnvMaps;

//====================================================================

void R_RenderView( const viewParms_t *parms );


void R_AddMD3Surfaces( trRefEntity_t *e );

void R_TagInfo_f( void );

void R_AddPolygonSurfaces( void );
void R_AddPolygonBufferSurfaces( void );


void R_DecomposeSort( unsigned sort, int *entityNum, shader_t **shader,
					  int *fogNum, int *dlightMap );

void R_AddDrawSurf( surfaceType_t *surface, shader_t *shader, int fogNum, int dlightMap );
#ifdef USE_PMLIGHT
void R_DecomposeLitSort( unsigned sort, int *entityNum, shader_t **shader, int *fogNum );
void R_AddLitSurf( surfaceType_t *surface, shader_t *shader, int fogIndex );
#endif

#define CULL_IN     0       // completely unclipped
#define CULL_CLIP   1       // clipped by one or more planes
#define CULL_OUT    2       // completely outside the clipping planes
void R_LocalNormalToWorld( const vec3_t local, vec3_t world );
void R_LocalPointToWorld( const vec3_t local, vec3_t world );
int R_CullLocalBox( const vec3_t bounds[2] );
int R_CullPointAndRadius( const vec3_t origin, float radius );
int R_CullLocalPointAndRadius( const vec3_t origin, float radius );
int R_CullDlight( const dlight_t *dl );

void R_SetupProjection(viewParms_t *dest, float zProj, qboolean computeFrustum);
void R_RotateForEntity( const trRefEntity_t *ent, const viewParms_t *viewParms, orientationr_t *orientation );

/*
** GL wrapper/helper functions
*/
const float *GL_Ortho( const float left, const float right, const float bottom, const float top, const float znear, const float zfar );
void    GL_Bind( image_t *image );
void	GL_BindTexNum( GLuint texnum );
void    GL_SelectTexture( int unit );
void	GL_BindTexture( int unit, GLuint texnum );
void    GL_TextureMode( const char *string );
void    GL_CheckErrors( void );
void    GL_State( GLbitfield stateVector );
void	GL_ClientState( int unit, GLbitfield stateVector );
void	GL_TexEnv( GLint env );
void	GL_Cull( cullType_t cullType );

#define GLS_SRCBLEND_ZERO                       0x00000001
#define GLS_SRCBLEND_ONE                        0x00000002
#define GLS_SRCBLEND_DST_COLOR                  0x00000003
#define GLS_SRCBLEND_ONE_MINUS_DST_COLOR        0x00000004
#define GLS_SRCBLEND_SRC_ALPHA                  0x00000005
#define GLS_SRCBLEND_ONE_MINUS_SRC_ALPHA        0x00000006
#define GLS_SRCBLEND_DST_ALPHA                  0x00000007
#define GLS_SRCBLEND_ONE_MINUS_DST_ALPHA        0x00000008
#define GLS_SRCBLEND_ALPHA_SATURATE             0x00000009
#define     GLS_SRCBLEND_BITS                   0x0000000f

#define GLS_DSTBLEND_ZERO                       0x00000010
#define GLS_DSTBLEND_ONE                        0x00000020
#define GLS_DSTBLEND_SRC_COLOR                  0x00000030
#define GLS_DSTBLEND_ONE_MINUS_SRC_COLOR        0x00000040
#define GLS_DSTBLEND_SRC_ALPHA                  0x00000050
#define GLS_DSTBLEND_ONE_MINUS_SRC_ALPHA        0x00000060
#define GLS_DSTBLEND_DST_ALPHA                  0x00000070
#define GLS_DSTBLEND_ONE_MINUS_DST_ALPHA        0x00000080
#define     GLS_DSTBLEND_BITS                   0x000000f0

#define GLS_BLEND_BITS							(GLS_SRCBLEND_BITS | GLS_DSTBLEND_BITS)

#define GLS_DEPTHMASK_TRUE						0x00000100

#define GLS_POLYMODE_LINE						0x00000200

#define GLS_DEPTHTEST_DISABLE					0x00000400
#define GLS_DEPTHFUNC_EQUAL						0x00000800

#define GLS_ATEST_GT_0							0x00001000
#define GLS_ATEST_LT_80							0x00002000
#define GLS_ATEST_GE_80							0x00003000
#define GLS_ATEST_BITS							0x00003000

#define GLS_DEFAULT								GLS_DEPTHMASK_TRUE

// vertex array states

#define CLS_NONE								0x00000000
#define CLS_COLOR_ARRAY							0x00000001
#define CLS_TEXCOORD_ARRAY						0x00000002
#define CLS_NORMAL_ARRAY						0x00000004

void		RE_StretchRaw( int x, int y, int w, int h, int cols, int rows, byte *data, int client, qboolean dirty );
void		RE_UploadCinematic( int w, int h, int cols, int rows, byte *data, int client, qboolean dirty );

void        RE_BeginFrame( stereoFrame_t stereoFrame );
void        RE_BeginRegistration( glconfig_t *glconfig );
void        RE_LoadWorldMap( const char *mapname );
void        RE_SetWorldVisData( const byte *vis );
qhandle_t   RE_RegisterModel( const char *name );
qhandle_t   RE_RegisterSkin( const char *name );

qboolean    RE_GetEntityToken( char *buffer, int size );

float       R_ProcessLightmap( byte **pic, int in_padding, int width, int height, byte **pic_out ); // Arnout

//----(SA)
qboolean    RE_GetSkinModel( qhandle_t skinid, const char *type, char *name );
qhandle_t   RE_GetShaderFromModel( qhandle_t modelid, int surfnum, int withlightmap );    //----(SA)
//----(SA) end

model_t     *R_AllocModel( void );

void		R_Init( void );

void        R_SetColorMappings( void );
void        R_GammaCorrect( byte *buffer, int bufSize );

void    R_ImageList_f( void );
void    R_SkinList_f( void );

void    R_InitImages( void );
void    R_DeleteTextures( void );
int     R_SumOfUsedImages( void );
void    R_InitSkins( void );
skin_t  *R_GetSkinByHandle( qhandle_t hSkin );

void    RB_DebugText( const vec3_t org, float r, float g, float b, const char *text, qboolean neverOcclude );
const void *RB_TakeVideoFrameCmd( const void *data );

//
// tr_shader.c
//
qhandle_t        RE_RegisterShaderLightMap( const char *name, int lightmapIndex );
qhandle_t        RE_RegisterShader( const char *name );
qhandle_t        RE_RegisterShaderNoMip( const char *name );
qhandle_t RE_RegisterShaderFromImage( const char *name, int lightmapIndex, image_t *image, qboolean mipRawImage );

shader_t    *R_FindShader( const char *name, int lightmapIndex, qboolean mipRawImage );
shader_t    *R_GetShaderByHandle( qhandle_t hShader );
shader_t    *R_GetShaderByState( int index, long *cycleTime );
shader_t *R_FindShaderByName( const char *name );
void        R_InitShaders( void );
void        R_ShaderList_f( void );
void    RE_RemapShader( const char *oldShader, const char *newShader, const char *timeOffset );
//bani
qboolean RE_LoadDynamicShader( const char *shadername, const char *shadertext );

// fretn - renderToTexture
void RE_RenderToTexture( int textureid, int x, int y, int w, int h );
// bani
void RE_Finish( void );
int R_GetTextureId( const char *name );
void RE_RenderOmnibot( void );


//
// tr_surface.c
//
void		RB_SurfaceGridEstimate( srfGridMesh_t *cv, int *numVertexes, int *numIndexes ); 

/*
====================================================================

TESSELATOR/SHADER DECLARATIONS

====================================================================
*/
typedef byte color4ub_t[4];

typedef struct stageVars
{
	color4ub_t	colors[SHADER_MAX_VERTEXES];
	vec2_t		texcoords[NUM_TEXTURE_BUNDLES][SHADER_MAX_VERTEXES];
	vec2_t		*texcoordPtr[NUM_TEXTURE_BUNDLES];
} stageVars_t;

typedef struct shaderCommands_s
{
#pragma pack(push,16)
	glIndex_t	indexes[SHADER_MAX_INDEXES] QALIGN(16);
	vec4_t		xyz[SHADER_MAX_VERTEXES*2] QALIGN(16); // 2x needed for shadows
	vec4_t		normal[SHADER_MAX_VERTEXES] QALIGN(16);
	vec2_t		texCoords[2][SHADER_MAX_VERTEXES] QALIGN(16);
	vec2_t		texCoords00[SHADER_MAX_VERTEXES] QALIGN(16);
	color4ub_t	vertexColors[SHADER_MAX_VERTEXES] QALIGN(16);
	stageVars_t	svars QALIGN(16);

	color4ub_t	constantColor255[SHADER_MAX_VERTEXES] QALIGN(16);
#pragma pack(pop)

	surfaceType_t	surfType;
	int			vboIndex;
	qboolean	allowVBO;

	shader_t	*shader;
	double		shaderTime;	// -EC- set to double for frameloss fix
	int			fogNum;
#ifdef USE_LEGACY_DLIGHTS
	int dlightBits;         // or together of all vertexDlightBits
#endif
	int			numIndexes;
	int			numVertexes;

#ifdef USE_PMLIGHT
	const dlight_t* light;
	qboolean	dlightPass;
	qboolean	dlightUpdateParams;
	cullType_t	cullType;
#endif

	// info extracted from current shader
#ifdef USE_TESS_NEEDS_NORMAL
	int			needsNormal;
#endif
#ifdef USE_TESS_NEEDS_ST2
	int			needsST2;
#endif

	int			numPasses;
	shaderStage_t **xstages;

} shaderCommands_t;

extern	shaderCommands_t	tess;

void RB_BeginSurface( shader_t *shader, int fogNum );
void RB_EndSurface( void );
void RB_CheckOverflow( int verts, int indexes );
#define RB_CHECKOVERFLOW(v,i) RB_CheckOverflow(v,i)

void RB_StageIteratorGeneric( void );
void RB_StageIteratorSky( void );

void RB_AddQuadStamp( const vec3_t origin, const vec3_t left, const vec3_t up, const byte *color );
void RB_AddQuadStampExt( const vec3_t origin, const vec3_t left, const vec3_t up, const byte *color, float s1, float t1, float s2, float t2 );
void RB_AddQuadStampFadingCornersExt( const vec3_t origin, const vec3_t left, const vec3_t up, const byte *color, float s1, float t1, float s2, float t2 );

void RB_ShowImages( void );

void RB_DrawBounds( vec3_t mins, vec3_t maxs ); // ydnar

/*
============================================================

WORLD MAP

============================================================
*/

void R_AddBrushModelSurfaces( trRefEntity_t *e );
void R_AddWorldSurfaces( void );


/*
============================================================

FLARES

============================================================
*/

void R_ClearFlares( void );

void RB_AddFlare( void *surface, int fogNum, vec3_t point, vec3_t color, float scale, vec3_t normal, int id, qboolean visible );    //----(SA)	added scale.  added id.  added visible
void RB_AddDlightFlares( void );
void RB_RenderFlares( void );

/*
============================================================

LIGHTS

============================================================
*/

void R_TransformDlights( int count, dlight_t *dl, orientationr_t *orientation );
//void R_CullDlights( void );
void R_DlightBmodel( bmodel_t *bmodel );
void R_SetupEntityLighting( const trRefdef_t *refdef, trRefEntity_t *ent );
int R_LightForPoint( vec3_t point, vec3_t ambientLight, vec3_t directedLight, vec3_t lightDir );

#ifdef USE_PMLIGHT
void ARB_SetupLightParams( void );
void ARB_LightingPass( void );
qboolean R_LightCullBounds( const dlight_t* dl, const vec3_t mins, const vec3_t maxs );
#endif // USE_PMLIGHT

void R_BindAnimatedImage( const textureBundle_t *bundle );
void R_DrawElements( int numIndexes, const glIndex_t *indexes );
void R_ComputeColors( const shaderStage_t *pStage );
void R_ComputeTexCoords( const int b, const textureBundle_t *bundle );

void QGL_InitARB( void );
void QGL_DoneARB( void );
void QGL_InitFBO( void );
qboolean ARB_UpdatePrograms( void );

qboolean GL_ProgramAvailable( void );
void GL_ProgramDisable( void );
void GL_ProgramEnable( void );

extern qboolean		fboEnabled;
extern qboolean		blitMSfbo;

void FBO_BindMain( void );
void FBO_PostProcess( void );
void FBO_BlitMS( qboolean depthOnly );
void FBO_BlitSS( void );
qboolean FBO_Bloom( const float gamma, const float obScale, qboolean finalPass );
void FBO_CopyScreen( void );
GLuint FBO_ScreenTexture( void );

/*
============================================================

SHADOWS

============================================================
*/

void RB_ShadowTessEnd( void );
void RB_ShadowFinish( void );
void RB_ProjectionShadowDeform( void );

/*
============================================================

SKIES

============================================================
*/

void R_BuildCloudData( shaderCommands_t *shader );
void R_InitSkyTexCoords( float cloudLayerHeight );
void R_DrawSkyBox( shaderCommands_t *shader );
void RB_DrawSun( void );
void RB_ClipSkyPolygons( shaderCommands_t *shader );

/*
============================================================

CURVE TESSELATION

============================================================
*/

#define PATCH_STITCHING

srfGridMesh_t *R_SubdividePatchToGrid( int width, int height,
									   drawVert_t points[MAX_PATCH_SIZE * MAX_PATCH_SIZE] );
srfGridMesh_t *R_GridInsertColumn( srfGridMesh_t *grid, int column, int row, vec3_t point, float loderror );
srfGridMesh_t *R_GridInsertRow( srfGridMesh_t *grid, int row, int column, vec3_t point, float loderror );
void R_FreeSurfaceGridMesh( srfGridMesh_t *grid );

/*
============================================================

MARKERS, POLYGON PROJECTION ON WORLD POLYGONS

============================================================
*/

int R_MarkFragments( int orientation, const vec3_t *points, const vec3_t projection,
					 int maxPoints, vec3_t pointBuffer, int maxFragments, markFragment_t *fragmentBuffer );


/*
============================================================

DECALS - ydnar

============================================================
*/

void RE_ProjectDecal( qhandle_t hShader, int numPoints, vec3_t *points, vec4_t projection, vec4_t color, int lifeTime, int fadeTime );
void RE_ClearDecals( void );

void R_AddModelShadow( const refEntity_t *ent );

void R_TransformDecalProjector( decalProjector_t * in, vec3_t axis[ 3 ], vec3_t origin, decalProjector_t * out );
qboolean R_TestDecalBoundingBox( decalProjector_t *dp, vec3_t mins, vec3_t maxs );
qboolean R_TestDecalBoundingSphere( decalProjector_t *dp, vec3_t center, float radius2 );

void R_ProjectDecalOntoSurface( decalProjector_t *dp, msurface_t *surf, bmodel_t *bmodel );

void R_AddDecalSurface( decal_t *decal );
void R_AddDecalSurfaces( bmodel_t *bmodel );
void R_CullDecalProjectors( void );


/*
============================================================

SCENE GENERATION

============================================================
*/

void R_InitNextFrame( void );

void RE_ClearScene( void );
void RE_AddRefEntityToScene( const refEntity_t *ent, qboolean intShaderTime );
void RE_AddPolyToScene( qhandle_t hShader, int numVerts, const polyVert_t *verts );
// Ridah
void RE_AddPolysToScene( qhandle_t hShader, int numVerts, const polyVert_t *verts, int numPolys );
// done.
void RE_AddPolyBufferToScene( polyBuffer_t* pPolyBuffer );
// ydnar: modified dlight system to support seperate radius & intensity
// void RE_AddLightToScene( const vec3_t org, float intensity, float r, float g, float b, int overdraw );
void RE_AddLightToScene( const vec3_t org, float radius, float intensity, float r, float g, float b, qhandle_t hShader, int flags );
void RE_AddLinearLightToScene( const vec3_t start, const vec3_t end, float intensity, float r, float g, float b );

//----(SA)
void RE_AddCoronaToScene( const vec3_t org, float r, float g, float b, float scale, int id, qboolean visible );
//----(SA)
void RE_RenderScene( const refdef_t *fd );

/*
=============================================================

ANIMATED MODELS

=============================================================
*/

void R_MakeAnimModel( model_t *model );
void R_AddAnimSurfaces( trRefEntity_t *ent );
void RB_SurfaceAnim( mdsSurface_t *surfType );
int R_GetBoneTag( orientation_t *outTag, mdsHeader_t *mds, int startTagIndex, const refEntity_t *refent, const char *tagName );

//
// MDM / MDX
//
void R_MDM_MakeAnimModel( model_t *model );
void R_MDM_AddAnimSurfaces( trRefEntity_t *ent );
void RB_MDM_SurfaceAnim( mdmSurface_t *surfType );
int R_MDM_GetBoneTag( orientation_t *outTag, mdmHeader_t *mdm, int startTagIndex, const refEntity_t *refent, const char *tagName );

qboolean R_LoadIQM (model_t *mod, void *buffer, int filesize, const char *name );
void R_AddIQMSurfaces( trRefEntity_t *ent );
void RB_IQMSurfaceAnim( surfaceType_t *surface );
int R_IQMLerpTag( orientation_t *tag, iqmData_t *data,
                  int startFrame, int endFrame,
                  float frac, const char *tagName );

/*
=============================================================
=============================================================
*/
void    R_TransformModelToClip( const vec3_t src, const float *modelMatrix, const float *projectionMatrix,
								vec4_t eye, vec4_t dst );
void    R_TransformClipToWindow( const vec4_t clip, const viewParms_t *view, vec4_t normalized, vec4_t window );

void	RB_DeformTessGeometry( void );

void	RB_CalcEnvironmentTexCoords( float *dstTexCoords );
void	RB_CalcEnvironmentTexCoordsFP( float *dstTexCoords, qboolean screenMap );
void	RB_CalcFireRiseEnvTexCoords( float *st );
void	RB_CalcFogTexCoords( float *dstTexCoords );
const fogProgramParms_t *RB_CalcFogProgramParms( void );
void	RB_CalcScrollTexCoords( const float scroll[2], float *srcTexCoords, float *dstTexCoords );
void	RB_CalcRotateTexCoords( float rotSpeed, float *srcTexCoords, float *dstTexCoords );
void	RB_CalcScaleTexCoords( const float scale[2], float *srcTexCoords, float *dstTexCoords );
void	RB_CalcSwapTexCoords( float *srcTexCoords, float *dstTexCoords );
void	RB_CalcTurbulentTexCoords( const waveForm_t *wf, float *srcTexCoords, float *dstTexCoords );
void	RB_CalcTransformTexCoords( const texModInfo_t *tmi, float *srcTexCoords, float *dstTexCoords );
void	RB_CalcModulateColorsByFog( unsigned char *dstColors );
void	RB_CalcModulateAlphasByFog( unsigned char *dstColors );
void	RB_CalcModulateRGBAsByFog( unsigned char *dstColors );
void	RB_CalcWaveAlpha( const waveForm_t *wf, unsigned char *dstColors );
void	RB_CalcWaveColor( const waveForm_t *wf, unsigned char *dstColors );
void	RB_CalcAlphaFromEntity( unsigned char *dstColors );
void	RB_CalcAlphaFromOneMinusEntity( unsigned char *dstColors );
void	RB_CalcNormalZFade( const byte constantColorAlpha, const float zFadeBounds[2], unsigned char *dstColors );
void	RB_CalcStretchTexCoords( const waveForm_t *wf, float *srcTexCoords, float *dstTexCoords );
void	RB_CalcColorFromEntity( unsigned char *dstColors );
void	RB_CalcColorFromOneMinusEntity( unsigned char *dstColors );
void	RB_CalcSpecularAlpha( unsigned char *alphas );
void	RB_CalcDiffuseColor( unsigned char *colors );

/*
=============================================================

RENDERER BACK END FUNCTIONS

=============================================================
*/

void RB_ExecuteRenderCommands( const void *data );

/*
=============================================================

RENDERER BACK END COMMAND QUEUE

=============================================================
*/

#define	MAX_RENDER_COMMANDS	0x80000

typedef struct {
	byte	cmds[MAX_RENDER_COMMANDS];
	int		used;
} renderCommandList_t;

typedef struct {
	int		commandId;
	float	color[4];
} setColorCommand_t;

typedef struct {
	int		commandId;
	int		buffer;
} drawBufferCommand_t;

typedef struct {
	int		commandId;
	image_t	*image;
	int		width;
	int		height;
	void	*data;
} subImageCommand_t;

typedef struct {
	int		commandId;
} swapBuffersCommand_t;

typedef struct {
	int		commandId;
} finishBloomCommand_t;

typedef struct {
	int		commandId;
	shader_t	*shader;
	float	x, y;
	float	w, h;
	float	s1, t1;
	float	s2, t2;

	byte	gradientColor[4];      // color values 0-255
	int		gradientType;       //----(SA)	added
	float	angle;            // NERVE - SMF
} stretchPicCommand_t;

typedef struct {
	int		commandId;
	polyVert_t *verts;
	int		numverts;
	shader_t *shader;
} poly2dCommand_t;

typedef struct {
	int		commandId;
	trRefdef_t	refdef;
	viewParms_t	viewParms;
	drawSurf_t *drawSurfs;
	int		numDrawSurfs;
} drawSurfsCommand_t;

typedef struct
{
	int commandId;

	GLboolean rgba[4];
} colorMaskCommand_t;

typedef struct
{
	int commandId;
} clearDepthCommand_t;

typedef struct
{
	int commandId;
	qboolean fullscreen;
	qboolean frontAndBack;
	qboolean colorMask;
} clearColorCommand_t;

//bani
typedef struct {
	int commandId;
	image_t *image;
	int x;
	int y;
	int w;
	int h;
} renderToTextureCommand_t;

//bani
typedef struct {
	int commandId;
} renderFinishCommand_t;

typedef enum {
	RC_END_OF_LIST,
	RC_SET_COLOR,
	RC_STRETCH_PIC,
	RC_2DPOLYS,
	RC_ROTATED_PIC,
	RC_STRETCH_PIC_GRADIENT,    // (SA) added
	RC_DRAW_SURFS,
	RC_DRAW_BUFFER,
	RC_SWAP_BUFFERS,
	RC_FINISHBLOOM,
	RC_COLORMASK,
	RC_CLEARDEPTH,
	RC_CLEARCOLOR,
	RC_RENDERTOTEXTURE, //bani
	RC_FINISH,   //bani
	RC_DRAW_OMNIBOT
} renderCommand_t;


// these are sort of arbitrary limits.
// the limits apply to the sum of all scenes in a frame --
// the main view, all the 3D icons, etc

// Ridah, these aren't enough for cool effects
//#define	MAX_POLYS		256
//#define	MAX_POLYVERTS	1024
#define	MAX_POLYS		8192
#define	MAX_POLYVERTS	32768
// done.
// Gordon: testing
//#define MAX_POLYINDICIES 8192

// ydnar: max decal projectors per frame, each can generate lots of polys
#define MAX_DECAL_PROJECTORS      128     ///< includes decal projectors that will be culled out, hard limited to 32 active projectors because of bitmasks.
#define MAX_USED_DECAL_PROJECTORS 32
#define MAX_DECALS                1024

// all of the information needed by the back end must be
// contained in a backEndData_t
typedef struct {
	drawSurf_t drawSurfs[MAX_DRAWSURFS];
#ifdef USE_PMLIGHT
	litSurf_t	litSurfs[MAX_LITSURFS];
	dlight_t	dlights[MAX_DLIGHTS]; // MAX_REAL_DLIGHTS
#else
	dlight_t dlights[MAX_DLIGHTS];
#endif

	corona_t coronas[MAX_CORONAS];          //----(SA)
	trRefEntity_t	entities[MAX_REFENTITIES];
	srfPoly_t polys[MAX_POLYS];
	srfPolyBuffer_t polybuffers[MAX_POLYS];
	polyVert_t polyVerts[MAX_POLYVERTS];
	decalProjector_t decalProjectors[ MAX_DECAL_PROJECTORS ];
	srfDecal_t decals[ MAX_DECALS ];
	renderCommandList_t commands;
} backEndData_t;

extern int max_polys;
extern int max_polyverts;

extern	backEndData_t	*backEndData;

void RB_ExecuteRenderCommands( const void *data );
void RB_TakeScreenshot( int x, int y, int width, int height, const char *fileName );
void RB_TakeScreenshotJPEG( int x, int y, int width, int height, const char *fileName );
void RB_TakeScreenshotBMP( int x, int y, int width, int height, const char *fileName, int clipboard );

void R_IssuePendingRenderCommands( void );

void R_AddDrawSurfCmd( drawSurf_t *drawSurfs, int numDrawSurfs );

void RE_SetColor( const float *rgba );
void RE_StretchPic( float x, float y, float w, float h,
					float s1, float t1, float s2, float t2, qhandle_t hShader );
void RE_RotatedPic( float x, float y, float w, float h,
					float s1, float t1, float s2, float t2, qhandle_t hShader, float angle );       // NERVE - SMF
void RE_StretchPicGradient( float x, float y, float w, float h,
							float s1, float t1, float s2, float t2, qhandle_t hShader, const float *gradientColor, int gradientType );
void RE_2DPolyies( polyVert_t* verts, int numverts, qhandle_t hShader );
void RE_SetGlobalFog( qboolean restore, int duration, float r, float g, float b, float depthForOpaque );
void RE_BeginFrame( stereoFrame_t stereoFrame );
void RE_EndFrame( int *frontEndMsec, int *backEndMsec );
void RE_TakeVideoFrame( int width, int height,
		byte *captureBuffer, byte *encodeBuffer, qboolean motionJpeg );

void RE_FinishBloom( void );
void RE_ThrottleBackend( void );
qboolean RE_CanMinimize( void );
const glconfig_t *RE_GetConfig( void );


//Bloom Stuff

#define MAX_FILTER_SIZE 20
#define MIN_FILTER_SIZE 1
#define MAX_BLUR_PASSES MAX_TEXTURE_UNITS

void R_BloomScreen( void );

// font stuff
void R_InitFreeType();
void R_DoneFreeType();
void RE_RegisterFont( const char *fontName, int pointSize, fontInfo_t *font );

// Ridah, caching system
// NOTE: to disable this for development, set "r_cache 0" in autoexec.cfg
void R_InitTexnumImages( qboolean force );

void *R_CacheModelAlloc( int size );
void R_CacheModelFree( void *ptr );
void R_PurgeModels( int count );
void R_BackupModels( void );
qboolean R_FindCachedModel( const char *name, model_t *newmod );
void R_LoadCacheModels( void );

void *R_CacheImageAlloc( int size );
void R_CacheImageFree( void *ptr );
qboolean R_TouchImage( image_t *inImage );
image_t *R_FindCachedImage( const char *name, int hash );
void R_FindFreeTexnum( image_t *image );
void R_LoadCacheImages( void );
void R_PurgeBackupImages( int purgeCount );
void R_BackupImages( void );

//void *R_CacheShaderAlloc( int size );
//void R_CacheShaderFree( void *ptr );

void R_CacheShaderFreeExt( const char* name, void *ptr, const char* file, int line );
void* R_CacheShaderAllocExt( const char* name, int size, const char* file, int line );

#define R_CacheShaderAlloc( name, size ) R_CacheShaderAllocExt( name, size, __FILE__, __LINE__ )
#define R_CacheShaderFree( name, ptr ) R_CacheShaderFreeExt( name, ptr, __FILE__, __LINE__ )

shader_t *R_FindCachedShader( const char *name, int lightmapIndex, int hash );
void R_BackupShaders( void );
void R_PurgeShaders( int count );
void R_PurgeLightmapShaders( void );
void R_LoadCacheShaders( void );
// done.

//------------------------------------------------------------------------------
// Ridah, mesh compression
#define NUMMDCVERTEXNORMALS  256

extern float r_anormals[NUMMDCVERTEXNORMALS][3];

// NOTE: MDC_MAX_ERROR is effectively the compression level. the lower this value, the higher
// the accuracy, but with lower compression ratios.
#define MDC_MAX_ERROR       0.1     // if any compressed vert is off by more than this from the
									// actual vert, make this a baseframe

#define MDC_DIST_SCALE      0.05    // lower for more accuracy, but less range

// note: we are locked in at 8 or less bits since changing to byte-encoded normals
#define MDC_BITS_PER_AXIS   8
#define MDC_MAX_OFS         127.0   // to be safe

#define MDC_MAX_DIST        ( MDC_MAX_OFS * MDC_DIST_SCALE )

#if 0
void R_MDC_DecodeXyzCompressed( mdcXyzCompressed_t *xyzComp, vec3_t out, vec3_t normal );
#else   // optimized version
#define R_MDC_DecodeXyzCompressed( ofsVec, out, normal ) \
	( out )[0] = ( (float)( ( ofsVec ) & 255 ) - MDC_MAX_OFS ) * MDC_DIST_SCALE; \
	( out )[1] = ( (float)( ( ofsVec >> 8 ) & 255 ) - MDC_MAX_OFS ) * MDC_DIST_SCALE; \
	( out )[2] = ( (float)( ( ofsVec >> 16 ) & 255 ) - MDC_MAX_OFS ) * MDC_DIST_SCALE; \
	VectorCopy( ( r_anormals )[( ofsVec >> 24 )], normal );
#endif

void R_AddMDCSurfaces( trRefEntity_t *ent );
// done.
//------------------------------------------------------------------------------

void R_LatLongToNormal( vec3_t outNormal, short latLong );


/*
============================================================

GL FOG

============================================================
*/

//extern glfog_t		glfogCurrent;
extern glfog_t glfogsettings[NUM_FOGS];         // [0] never used (FOG_NONE)
extern glfogType_t glfogNum;                    // fog type to use (from the fog_t enum list)

extern qboolean fogIsOn;

extern void         R_FogOff( void );
extern void         R_FogOn( void );

extern void R_SetFog( int fogvar, int var1, int var2, float r, float g, float b, float density );

extern int skyboxportal;


// Ridah, virtual memory
void *R_Hunk_Begin( void );
void R_Hunk_End( void );
void R_FreeImageBuffer( void );

qboolean R_inPVS( const vec3_t p1, const vec3_t p2 );

qboolean R_HaveExtension( const char *ext );

#define GLE( ret, name, ... ) extern ret ( APIENTRY * q##name )( __VA_ARGS__ );
	QGL_Core_PROCS;
	QGL_Ext_PROCS;
	QGL_ARB_PROGRAM_PROCS;
	QGL_VBO_PROCS;
	QGL_FBO_PROCS;
	QGL_FBO_OPT_PROCS;
#undef GLE

// VBO functions

extern void RB_StageIteratorVBO( void );
extern void R_BuildWorldVBO( msurface_t *surf, int surfCount );

extern void VBO_PushData( int itemIndex, shaderCommands_t *input );
extern void VBO_UnBind( void );
extern int VBO_Active( void );

extern void VBO_Cleanup( void );
extern void VBO_QueueItem( int itemIndex );
extern void VBO_ClearQueue( void );
extern void VBO_Flush( void );

// ARB shaders definitions

typedef enum {
	Vertex,
	Fragment
} programType;

typedef enum {
	DEFAULT_VERTEX,
	DEFAULT_FRAGMENT,

	PROGRAM_BASE,

	DUMMY_VERTEX = PROGRAM_BASE,

	// locate all fog programs in predefined order (sequentially after non-fogged ones)
	// so we can easy switch/adjust them without many if() statements
#ifdef USE_PMLIGHT
	DLIGHT_VERTEX,
	DLIGHT_VERTEX_FOG,
	DLIGHT_VERTEX_FOG_LEVEL,

	DLIGHT_FRAGMENT,
	DLIGHT_FRAGMENT_FOG,

	DLIGHT_ABS_FRAGMENT,
	DLIGHT_ABS_FRAGMENT_FOG,

	DLIGHT_LINEAR_FRAGMENT,
	DLIGHT_LINEAR_FRAGMENT_FOG,

	DLIGHT_LINEAR_ABS_FRAGMENT,
	DLIGHT_LINEAR_ABS_FRAGMENT_FOG,

	DLIGHT_DIRECTIONAL_FRAGMENT,
	DLIGHT_DIRECTIONAL_FRAGMENT_FOG,

	DLIGHT_DIRECTIONAL_ABS_FRAGMENT,
	DLIGHT_DIRECTIONAL_ABS_FRAGMENT_FOG,
#endif
	SPRITE_FRAGMENT,
	GAMMA_FRAGMENT,
	BLOOM_EXTRACT_FRAGMENT,
	BLUR_FRAGMENT,
	BLUR2_FRAGMENT,
	BLENDX_FRAGMENT,
	BLEND2_FRAGMENT,
	BLEND2_GAMMA_FRAGMENT,

	PROGRAM_COUNT

} programNum;

extern const char *fogVPCode;
extern const char *fogLevelVPCode;

qboolean ARB_CompileProgram( programType ptype, const char *text, GLuint program );
void ARB_ProgramEnableExt( GLuint vertexProgram, GLuint fragmentProgram );

void QGL_SetRenderScale( qboolean verbose );

#endif //TR_LOCAL_H
