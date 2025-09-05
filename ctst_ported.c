/*
 * Modified ctst.c - Core game file with platform abstraction
 * This shows how to adapt the original 3DO code to use our platform layer
 */

// Replace 3DO includes with our compatibility layer
#include "threedo_compat.h"

// Keep essential game includes
#include "castle.h"
#include "objects.h"
// Temporarily exclude these still-conflicting headers:
// #include "imgfile.h" 
// #include "loaf.h"
// #include "sound.h"
// #include "flow.h"
// #include "app_proto.h"

// Global variables (unchanged from original)
MapEntry levelmap[WORLDSIZ][WORLDSIZ];
JoyData jd;

// Global rendering data structures  
VisOb visobs[MAXVISOBS];
VisOb* curviso;
Vertex obverts[NOBVERTS], xfobverts[NOBVERTS];
Vertex* curobv;
int32 nobverts;
int32 nvisv, nviso;
uint32 vertsused[(NGRIDPOINTS + 31) >> 5];

// 3D transformation data
Vector plusx, plusz, minusx, minusz;
Vertex xformsquare[NUNITVERTS];
Vector xformvects[4];
Matrix camera;

// Original unit square data (unchanged)
Vertex unitsquare[NUNITVERTS] = {
    { 0,        0,        0 },
    { HALF_F16, 0,        0 },
    { ONE_F16,  0,        0 },
    { 0,        0,        HALF_F16 },
    { HALF_F16, 0,        HALF_F16 },
    { ONE_F16,  0,        HALF_F16 },
    { 0,        0,        ONE_F16 },
    { HALF_F16, 0,        ONE_F16 },
    { ONE_F16,  0,        ONE_F16 },
    { 0,        -ONE_F16, 0 },
    { HALF_F16, -ONE_F16, 0 },
    { ONE_F16,  -ONE_F16, 0 },
    { 0,        -ONE_F16, HALF_F16 },
    { HALF_F16, -ONE_F16, HALF_F16 },
    { ONE_F16,  -ONE_F16, HALF_F16 },
    { 0,        -ONE_F16, ONE_F16 },
    { HALF_F16, -ONE_F16, ONE_F16 },
    { ONE_F16,  -ONE_F16, ONE_F16 }
};

Vector unitvects[4] = {
    { ONE_F16,  0,        0 },
    { 0,        0,        ONE_F16 },
    { -ONE_F16, 0,        0 },
    { 0,        0,        -ONE_F16 }
};

// Game state variables (unchanged from original)
Vector plusx, plusz, minusx, minusz;
Vertex xformsquare[NUNITVERTS];
Vector xformvects[4];
Matrix camera;

Vertex xfverts[MAXWALLVERTS], projverts[MAXWALLVERTS];
int16 grididxs[NGRIDPOINTS];

VisOb visobs[MAXVISOBS];
VisOb* curviso;
uint32 vertsused[(NGRIDPOINTS + 31) >> 5];
Vertex obverts[NOBVERTS], xfobverts[NOBVERTS];
Vertex* curobv;
int32 nobverts;

int32 nvisv, nviso;

Vertex playerpos, campos;
frac16 playerdir;
int32 playerhealth = 100;
int32 playerlives = 3;
int32 gunpower = 100;
int32 nkeys = 0;
int32 score = 0;
int32 xlifethresh = 50000;
int32 xlifeincr = 500000;

ImageEnv* walliev;
ImageEntry* wallimgs;
CelArray* ca_zombie, *ca_gun, *ca_ray;
CCB* guncel;
uint32 ccbextra;

int32 floorcolor = 0x001100;
int32 ceilingcolor = 0x000066;
frac16 damagefade = 0;

frac16 scale = 0x24000; // DEFAULT_SCALE
int32 throttleshift = 4;
int32 cy = CY;

int8 skiptitle = FALSE;
int8 laytest = FALSE;
int8 practice = FALSE;
int8 domusic = TRUE;
int8 dosfx = TRUE;
int8 exitedlevel = FALSE;
int8 gottalisman = FALSE;

Item joythread = 0;
Item vblIO = 0, sportIO = 0;

// File paths
char* sequencefile = "levelseq";
char* wallimagefile = "walls.loaf";
char* spoolmusicfile = "$progdir/music/ambient1.aifc";
char* levelname = "level1";

char* levelseqbuf = NULL;
char** seqnames = NULL;
int32 nseq = 0;
int32 level = 1;
int32 screenpages = 2;

Item sgrpitem = 0;
// Platform-managed rastports (declared in platform/sdl/graphics_sdl.c)
extern RastPort* platform_get_rastport(int index);
RastPort* rpvis = NULL;
RastPort* rprend = NULL;

// Compatibility implementations
void die(const char* message)
{
    printf("FATAL ERROR: %s\n", message);
    platform_error(message);
    exit(1);
}

void kprintf(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

// Modified openstuff() function - platform initialization
void openstuff(void)
{
    // Graphics initialization (already done in main_sdl.c)
    printf("Graphics already initialized by platform layer\n");

    // Create screen group using platform abstraction
    Item screen_items[NSCREENS];
    if (CreateScreenGroup(screen_items, NULL) < 0) {
        die("Can't create screen group.\n");
    }
    sgrpitem = screen_items[0];

    // Setup rastports (use platform-managed rastports)
    rpvis = platform_get_rastport(0);
    rprend = platform_get_rastport(1);
    
    if (!rpvis || !rprend) {
        die("Failed to get platform rastports.\n");
    }
    
    printf("Using platform rastports: rpvis->rp_BitmapItem=%d, rprend->rp_BitmapItem=%d\n", 
           rpvis->rp_BitmapItem, rprend->rp_BitmapItem);

    // Math folio (no-op on modern systems)
    if (OpenMathFolio() < 0) {
        printf("Warning: Math folio not available (not needed on modern systems)\n");
    }

    // Get VBL IO (implemented by platform layer)
    if ((vblIO = GetVBLIOReq()) < 0) {
        die("Can't get VBL I/O request.\n");
    }

    // Audio initialization (already done in main_sdl.c)
    printf("Audio already initialized by platform layer\n");

    // Initialize other game systems
    opentimer();
    
    // Load level sequence
    loadlevelsequence();
}

void closestuff(void)
{
    // Cleanup level stuff
    if (levelseqbuf) {
        free(levelseqbuf);
        levelseqbuf = NULL;
    }
    if (seqnames) {
        FreeMem(seqnames, nseq * sizeof(char*));
        seqnames = NULL;
    }

    // Close screen group
    if (sgrpitem) {
        Item screen_items[NSCREENS] = {sgrpitem, sgrpitem + 1};
        DeleteScreenGroup(screen_items);
        sgrpitem = 0;
    }

    // Platform shutdown handled in main_sdl.c
    printf("Game systems closed\n");
}

// Modified main game loop - now returns instead of running forever
int dogame(void)
{
    Matrix tmpmat;
    Vector trans;
    RastPort* tmp;
    int32 framecount;
    int retval = 0;
    bool preva = FALSE, prevb = FALSE, prevc = FALSE, prevs = FALSE;
    
    // Frame rate limiting variables
    static uint32_t last_frame_time = 0;
    uint32_t current_time;
    uint32_t frame_duration = 16; // ~60fps (1000/60 = 16.67ms)

    // Initialize game
    openlevelstuff();
    opengamestuff();
    fullstop();

    printf("Starting game loop...\n");

    // Main game loop - modified to handle modern event system
    int loop_count = 0;
    while (retval == 0) {
        loop_count++;
        
        // Handle platform events
        int poll_result = platform_poll_events();
        if (poll_result != 0) {
            printf("Platform poll events returned %d, exiting loop (iteration %d)\n", poll_result, loop_count);
            retval = FC_COMPLETED; // Quit requested
            break;
        }

        // Update input
        platform_update_input_state();
        platform_update_joydata_from_input(&jd);

        // Check for level completion
        if (exitedlevel) {
            printf("Level exited flag set, exiting loop (iteration %d)\n", loop_count);
            retval = FC_COMPLETED;
            break;
        }

        // Player movement (original logic)
        moveplayer(&playerpos, &playerdir);

        // Handle pause
        if (!prevs && jd.jd_StartDown) {
            if (laytest) {
                retval = FC_BUCKY_NEXTLEVEL;
                break;
            } else {
                // Handle pause logic
                printf("Game paused\n");
                jd.jd_StartDown = TRUE; // Force interlock
            }
        }
        prevs = jd.jd_StartDown;

        // Handle options
        if (jd.jd_XDown) {
            // Would call options menu
            printf("Options pressed\n");
        }

        // Handle actions
        if (!preva && jd.jd_ADown) {
            shoot();
        }
        preva = jd.jd_ADown;

        if (!prevb && jd.jd_BDown) {
            probe();
        }
        prevb = jd.jd_BDown;

        if (!prevc && jd.jd_CDown) {
            // Would call map
            printf("Map pressed\n");
            jd.jd_CDown = TRUE; // Force interlock
        }
        prevc = jd.jd_CDown;

        // Update game state
        framecount = jd.jd_FrameCount;
        resetjoydata();
        moveobjects(framecount);
        cyclewalls(framecount);

        // Extra lives
        if (score >= xlifethresh) {
            playerlives++;
            playsound(SFX_GRABLIFE);
            xlifethresh += xlifeincr;
        }

        // Damage fade
        if (damagefade) {
            if ((damagefade -= (ONE_F16 >> 6) * framecount) < 0) {
                damagefade = 0;
            }
            if (!damagefade) {
                fadetolevel(rpvis, ONE_F16 - damagefade);
            }
            fadetolevel(rprend, ONE_F16 - damagefade);
        }

        // Swap render targets
        tmp = rpvis;
        rpvis = rprend;
        rprend = tmp;
        
        // Add frame rate limiting before displaying screen to prevent rapid flashing
        current_time = platform_get_ticks();
        
        if (current_time - last_frame_time >= frame_duration) {
            DisplayScreen(rpvis->rp_ScreenItem, 0);
            last_frame_time = current_time;
        }

        // Transform unit vectors
        newmat(&tmpmat);
        applyyaw(&tmpmat, &camera, playerdir);
        MulManyVec3Mat33_F16((VECTCAST)xformvects,
                           (VECTCAST)unitvects,
                           (MATCAST)&camera, 4);

        // Camera positioning
        applyyaw(&tmpmat, &camera, -playerdir);
        campos.X = playerpos.X + (xformvects[3].X >> 1);
        campos.Y = playerpos.Y + (xformvects[3].Y >> 1);
        campos.Z = playerpos.Z + (xformvects[3].Z >> 1);

        trans.X = -campos.X;
        trans.Y = -campos.Y;
        trans.Z = -campos.Z;

        copyverts(unitsquare, xformsquare, NUNITVERTS);
        translatemany(&trans, xformsquare, NUNITVERTS);
        MulManyVec3Mat33_F16((VECTCAST)xformsquare,
                           (VECTCAST)xformsquare,
                           (MATCAST)&camera, NUNITVERTS);

        // Update direction vectors
        minusx.X = -(plusx.X = xformsquare[2].X - xformsquare[0].X);
        minusx.Y = plusx.Y = 0;
        minusx.Z = -(plusx.Z = xformsquare[2].Z - xformsquare[0].Z);

        minusz.X = -(plusz.X = xformsquare[6].X - xformsquare[0].X);
        minusz.Y = plusz.Y = 0;
        minusz.Z = -(plusz.Z = xformsquare[6].Z - xformsquare[0].Z);

        // Extract and render
        extractcels(campos.X, campos.Z, playerdir);

        // Limit frame rate
        WaitVBL(vblIO, 1);
    }

    closelevelstuff();
    return retval;
}

// Stub implementations for missing functions
void opentimer(void)
{
    printf("Timer initialized (no-op on modern systems)\n");
}

void* allocloadfile(const char* filename, int32 flags, int32* size_out)
{
    // Simple file loading implementation
    char* full_path = (char*)platform_get_resource_path(filename);
    FILE* file = fopen(full_path, "rb");
    if (!file) {
        printf("Failed to load file: %s\n", full_path);
        if (size_out) *size_out = 0;
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    void* buffer = malloc(size + 1); // +1 for null terminator
    if (!buffer) {
        fclose(file);
        if (size_out) *size_out = 0;
        return NULL;
    }

    fread(buffer, 1, size, file);
    ((char*)buffer)[size] = '\0'; // Null terminate
    fclose(file);

    if (size_out) *size_out = size;
    printf("Loaded file: %s (%ld bytes)\n", filename, size);
    return buffer;
}

// Timing implementation
struct timeval {
    long tv_sec;
    long tv_usec;
};

void gettime(struct timeval* tv)
{
    if (tv) {
        uint32 ticks = platform_get_ticks();
        tv->tv_sec = ticks / 1000;
        tv->tv_usec = (ticks % 1000) * 1000;
    }
}

int32 subtime(struct timeval* end, struct timeval* start)
{
    if (!end || !start) return 0;
    
    long sec_diff = end->tv_sec - start->tv_sec;
    long usec_diff = end->tv_usec - start->tv_usec;
    
    return (int32)(sec_diff * 1000 + usec_diff / 1000);
}

// Math folio
int OpenMathFolio(void)
{
    return 0; // No-op on modern systems
}

// Kernel base simulation  
ThreeDOKernelBase g_kernel_base_storage = {0};
ThreeDOKernelBase* g_ThreeDOKernelBase = &g_kernel_base_storage;

Item AllocSignal(int signal)
{
    return signal; // Simplified
}

void PrintfSysErr(int32 error)
{
    printf("System error: %d\n", error);
}

// Additional stubs
void MemorySize(void) { }
void MemoryMove(void) { }

Item NewMsgPort(void* name)
{
    return 1; // Dummy item
}

Item CreateMsgItem(Item port)
{
    return port + 100; // Dummy item
}

void drawnumxy(Item bitmap, int32 number, int32 x, int32 y)
{
    printf("Draw number %d at (%d,%d) (not implemented)\n", number, x, y);
}

void* FreeMemToMemLists(void* ptr)
{
    free(ptr);
    return NULL;
}

void* AllocMemFromMemLists(int32 size, uint32 flags)
{
    return AllocMem(size, flags);
}

int InitFileFolioGlue(void)
{
    return 0; // No-op
}

// Keep all original game functions unchanged...
// (The rest of the original ctst.c functions would go here)
// For brevity, I'm not including all of them, but they would remain largely unchanged
