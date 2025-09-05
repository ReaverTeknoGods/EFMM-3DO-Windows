/*  :ts=8 bk=0
 *
 * castle.h:	Various definitions - Ported for Cross-platform compatibility.
 *
 * Leo L. Schwab					9305.18
 * Ported to SDL2/Cross-platform                        2024
 */
#ifndef	_CASTLE_H
#define	_CASTLE_H

#include "threedo_compat.h"


/***************************************************************************
 * Math structures - using platform abstraction types
 */
// Vertex, Vector, Matrix, and math types are defined in threedo_compat.h

/*
 * Cast macros for compatibility with original code
 */
#define	VECTCAST	frac16 (*)[3]
#define	MATCAST		frac16 (*)[3]

/*
 * Useful constants.
 */
#define	ONE_F16		(1 << 16)
#define	HALF_F16	(1 << 15)
#define QUARTER_F16     (ONE_F16 >> 2)

#define	F_INT(x)	((x) & ~0xFFFF)
#define	F_FRAC(x)	((x) & 0xFFFF)

#define	PIXELS2UNITS(p)	((p) << (16 - 7))	/*  1.0 == 128 pixels	*/


/***************************************************************************
 * Object definitions (monsters, bullets, tables, chairs, amphibious landing
 * crafts, etc.).
 **
 * ewhac 9307.01:  Removed to objects.h.  Go look there.
 */

/***************************************************************************
 * Cell info.
 */
typedef struct	MapEntry {
	struct Object	*me_Obs;
	ubyte		me_NSImage,	// Indicies into ImageEntries.
			me_EWImage;
	ubyte		me_Flags;
	ubyte		me_VisFlags;
} MapEntry;

/*
 * Flags defining which faces of the block have art mapped on them.
 */
#define	VISF_NORTH	1
#define	VISF_WEST	(1<<1)
#define	VISF_SOUTH	(1<<2)
#define	VISF_EAST	(1<<3)

#define	VISF_ALLDIRS	(VISF_NORTH | VISF_WEST | VISF_SOUTH | VISF_EAST)

/*
 * Flags for the automapper; they are the faces we have actually looked at.
 */
#define	MAPF_NORTH	(1<<4)
#define	MAPF_WEST	(1<<5)
#define	MAPF_SOUTH	(1<<6)
#define	MAPF_EAST	(1<<7)

#define	MAPF_ALLDIRS	(MAPF_NORTH | MAPF_WEST | MAPF_SOUTH | MAPF_EAST)


/*
 * Flags defining wall characteristics.
 */
#define	MEF_OPAQUE	1	/*  Can't see through it.		*/
#define	MEF_WALKSOLID	(1<<1)	/*  Can't walk through it.		*/
#define	MEF_SHOTSOLID	(1<<2)	/*  Can't shoot through it.		*/
#define	MEF_ARTWORK	(1<<3)	/*  There's something to draw.		*/


/***************************************************************************
 * Visible cell entry.
 */
typedef struct VisOb {
	int32		vo_LIdx,
			vo_RIdx;
	ubyte		vo_Type;	/*  I don't appear to use this... */
	ubyte		vo_ImgIdx;
	ubyte		vo_VisFlags;
	ubyte		vo_MEFlags;
	struct MapEntry	*vo_ME;
} VisOb;

#define	VOF_THINGISIMG	1		/*  vo_Thing points to ImageEntry. */

#define	VOTYP_WALL	0		/*  These mightn't get used.	*/
#define	VOTYP_OBJECT	1


/***************************************************************************
 * Joypad data - using platform definition from platform_input.h
 */
// JoyData is already defined in platform_input.h

/***************************************************************************
 * Bounding box structures - using platform definitions from platform_types.h  
 */
// BBox and PathBox are already defined in platform_types.h


/***************************************************************************
 * Game-related constants.
 */
#define	WORLDSIZ	128	/*  Increased for final game  */
#define	WORLDSIZ_F16	(WORLDSIZ << 16)

#define	GRIDSIZ		(WORLDSIZ + 1)

#define	MAXNCELS	(WORLDSIZ * WORLDSIZ)
#define	NGRIDPOINTS	(GRIDSIZ * GRIDSIZ)

#define	GRIDCUTOFF	16  /* Reduced for performance */
#define	MAXWALLVERTS	2048

#define	NOBVERTS	1024

#define	MAXVISOBS	512

#define	CX		160
#define	CY		120

#define	MAGIC		320

#define	ZCLIP		(ONE_F16 >> 6)

#define	ONE_HD		(1 << 20)
#define ONE_VD		(1 << 16)

#define	REALCELDIM(x)	((x) <= 0  ?  1 << -(x)  :  (x))

// Number of unit vertices in our basic square
#define NUNITVERTS      18


/***************************************************************************
 * Rendering environment - using platform abstraction
 */
// RastPort is defined in threedo_compat.h

/***************************************************************************
 * Global variables (extern declarations)
 */
extern MapEntry levelmap[WORLDSIZ][WORLDSIZ];
extern Vertex unitsquare[NUNITVERTS];
extern Vector unitvects[4];
extern Vector plusx, plusz, minusx, minusz;
extern Vertex xformsquare[NUNITVERTS];
extern Vector xformvects[4];
extern Matrix camera;

extern Vertex xfverts[MAXWALLVERTS], projverts[MAXWALLVERTS];
extern int16 grididxs[NGRIDPOINTS];
extern VisOb visobs[MAXVISOBS];
extern VisOb* curviso;
extern uint32 vertsused[(NGRIDPOINTS + 31) >> 5];
extern Vertex obverts[NOBVERTS], xfobverts[NOBVERTS];
extern Vertex* curobv;
extern int32 nobverts;
extern int32 nvisv, nviso;

extern Vertex playerpos, campos;
extern frac16 playerdir;
extern int32 playerhealth;
extern int32 playerlives;
extern int32 gunpower;
extern int32 nkeys;
extern int32 score;
extern int32 xlifethresh;
extern int32 xlifeincr;

extern int32 floorcolor, ceilingcolor;
extern frac16 damagefade;
extern frac16 scale;
extern int32 throttleshift;
extern int32 cy;

extern int8 skiptitle;
extern int8 laytest;
extern int8 practice;
extern int8 domusic;
extern int8 dosfx;
extern int8 exitedlevel;
extern int8 gottalisman;

extern Item joythread;
extern Item vblIO, sportIO;
extern Item sgrpitem;
extern RastPort rports[NSCREENS];
extern RastPort* rpvis;
extern RastPort* rprend;

extern char* levelseqbuf;
extern char** seqnames;
extern int32 nseq;
extern int32 level;
extern int32 screenpages;


#endif	/* _CASTLE_H */
