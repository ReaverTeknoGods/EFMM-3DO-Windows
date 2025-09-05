/*
 * 3DO compatibility layer - includes all the headers the original code expects
 * This file should be included instead of the original 3DO headers
 */

#ifndef THREEDO_COMPAT_H
#define THREEDO_COMPAT_H

// Standard C headers that work everywhere
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>

// Our platform abstraction layer
#include "platform/platform_types.h"
#include "platform/platform_graphics.h"
#include "platform/platform_audio.h"
#include "platform/platform_input.h"

// 3DO compatibility types and constants
#define TRUE 1
#define FALSE 0

// 3DO memory types (already defined in platform_types.h)
// MEMTYPE_NORMAL, MEMTYPE_VRAM, etc.

// 3DO math constants (already defined in platform_types.h)
// ONE_F16, HALF_F16, etc.

// 3DO system constants
#define NSCREENS 2

// Debug macros
#ifdef DEBUG
    #define DBUG(x) printf x
    #define FULLDBUG(x) printf x
#else
    #define DBUG(x)
    #define FULLDBUG(x)
#endif

// Utility macros
#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

// Function that the original code expects to exist
extern void die(const char* message);
extern void kprintf(const char* format, ...);

// Forward declarations for structures the original code uses
struct timeval;

// Timing functions
void gettime(struct timeval* tv);
int32 subtime(struct timeval* end, struct timeval* start);

// These functions need to be implemented or stubbed
void opentimer(void);
void* allocloadfile(const char* filename, int32 flags, int32* size_out);

// Portfolio/kernel compatibility stubs - rename to avoid Windows conflicts
typedef struct ThreeDOKernelBase {
    struct {
        struct {
            Item n_Item;
        } t;
    }* kb_CurrentTask;
} ThreeDOKernelBase;

extern ThreeDOKernelBase* g_ThreeDOKernelBase;
#define KernelBase g_ThreeDOKernelBase

Item AllocSignal(int signal);
void PrintfSysErr(int32 error);

// Math folio compatibility
int OpenMathFolio(void);

// Various 3DO API functions that need stubs or implementations
void MemorySize(void);
void MemoryMove(void);
Item NewMsgPort(void* name);
Item CreateMsgItem(Item port);

// Graphics utility functions
void drawnumxy(Item bitmap, int32 number, int32 x, int32 y);

// Memory management compatibility
void* FreeMemToMemLists(void* ptr);
void* AllocMemFromMemLists(int32 size, uint32 flags);

// File system compatibility
int InitFileFolioGlue(void);

// Function pointer types for object system
typedef int32 (*ObjFunc)(void* object, int operation, void* data);

// Error checking macros from 3DO
#define CHECKRESULT(val, name) \
    if ((val) < 0) { \
        printf("Error in %s: %d\n", name, (int)(val)); \
        goto error; \
    }

#define CHECKPTR(val, name) \
    if ((val) == NULL) { \
        printf("NULL pointer in %s\n", name); \
        goto error; \
    }

// Forward declarations for types that would come from excluded headers
typedef struct ImageEnv {
    void* dummy;
} ImageEnv;

typedef struct ImageEntry {
    void* dummy;
} ImageEntry;

// Some constants from the original 3DO code
#define ANGSTEP         ONE_F16
#define QUARTER_F16     (ONE_F16 >> 2)
#define BIGSTEP         (0x7FFFFFF - (128 << 16))
#define DEFAULT_SCALE   0x24000
#define DAMAGEFADEDECAY (ONE_F16 >> 6)
#define SETBIT(a,n)     ((a)[(n) >> 5] |= (1 << ((n) & 31)))

// Game geometry constants  
#define CELSIZ          6
#define XOFF            (320 / 2)
#define YOFF            50
#define NUNITVERTS      18

#define ZCLIP (ONE_F16 >> 6)
#define CY 120
#define WORLDSIZ 128
#define WORLDSIZ_F16 (WORLDSIZ << 16)
#define MAXWALLVERTS 2048
// NGRIDPOINTS is defined in castle.h
#define MAXVISOBS 512
#define NOBVERTS 1024
#define GRIDCUTOFF 16
#define WALLIMGBASE 0

#define PIXELS2UNITS(p) ((p) << (16 - 7))

// Flow control constants
#define FC_COMPLETED 1
#define FC_NEWGAME 2
#define FC_LOADGAME 3
#define FC_RESTART 4
#define FC_BUCKY_NEXTLEVEL 5
#define FC_DIED 6
#define FC_BUCKY_QUIT 7

// Collision detection types
#define PLAYERAD (3 * ONE_F16 / 8)

// Forward type declarations (proper types are in original headers)
struct MapEntry;
struct VisOb;
struct Object;

// Cell extraction data structure (from original 3DO code)
typedef struct ExtDat {
    frac16 x, z,
           angl, angr,
           sinl, cosl,
           sinr, cosr,
           liml, limr,
           stepl, stepr;
    ubyte minx, maxx,
          minz, maxz;
    ubyte xcnt, zcnt;
    ubyte chopcone;
} ExtDat;

// Global arrays for visible geometry (will be defined in main game file)
// extern struct VisOb visobs[MAXVISOBS];
// extern struct VisOb* curviso;

// Forward declarations for collision functions
int genpathbox(BBox* path, BBox* start, BBox* end);
int checkcontact(PathBox* pb, BBox* bb, int block);
void blockpath(PathBox* pb, BBox* bb);

// Forward declarations for rendering functions  
void clearvertsused(void);
void extract_north(ExtDat* ed);
void extract_west(ExtDat* ed);
void extract_south(ExtDat* ed);
void extract_east(ExtDat* ed);
void processgrid(void);
void processvisobs(void);
void rendercels(void);
void platform_wait_vbl(int frames);
void platform_clear_screen(void);

// Function declarations for movement
int moveposition(Vertex* ppos, Vector* trans, struct Object* ignoreob, int checkobs, int isplayer);

// Additional platform functions
void platform_delay(uint32 ms);
void platform_clear_framebuffer(uint32 color);
void platform_present_framebuffer(void);

// Level loading functions
void loadlevelsequence(void);
void loadlevelmap(const char* levelname);

// Sound effect IDs
#define SFX_GUNZAP 1

// Function declarations for game functions implemented as stubs
void loadlevelsequence(void);
void openlevelstuff(void);
void opengamestuff(void);
void fullstop(void);
void moveplayer(Vertex* pos, frac16* dir);
void shoot(void);
void probe(void);
void resetjoydata(void);
void moveobjects(int32 dt);
void cyclewalls(int32 dt);
void playsound(int sound_id);
void fadetolevel(RastPort* rp, frac16 level);
void newmat(Matrix* mat);
void applyyaw(Matrix* dest, Matrix* src, frac16 yaw);
void copyverts(Vertex* dest, Vertex* src, int32 count);
void translatemany(Vector* offset, Vertex* verts, int32 count);
void extractcels(frac16 x, frac16 y, frac16 z);
void closelevelstuff(void);
void MulManyVec3Mat33_F16(void* result, void* vectors, void* matrix, int32 count);

// Platform function declarations
void platform_error(const char* message);
const char* platform_get_resource_path(const char* filename);
uint32 platform_get_ticks(void);
#define SFX_GUNDRAINED 2
#define SFX_GUNEMPTY 3
#define SFX_DOORLOCKED 4
#define SFX_OPENDOOR 5
#define SFX_CLOSEDOOR 6
#define SFX_FLOORTRIGGER 7
#define SFX_GRABKEY 8
#define SFX_GRABHEALTH 9
#define SFX_GRABAMMO 10
#define SFX_GRABMONEY 11
#define SFX_GRABLIFE 12
#define SFX_GRABTALISMAN 13
#define SFX_CANTGRAB 14
#define SFX_OUCHSMALL 15
#define SFX_OUCHMED 16
#define SFX_OUCHBIG 17

// Default file paths
extern char* sequencefile;
extern char* wallimagefile;
extern char* spoolmusicfile;
extern char* levelname;

#endif // THREEDO_COMPAT_H
