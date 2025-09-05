/***************************************************************/
//
//      Writed by Altmer (mail: max220291@list.ru, altmer@arts-union.ru)
//      август 2005 года
//
/***************************************************************/
#include <io.h>
#include <fcntl.h>


#define CEL_FLAG_PACKED(x) ((x>>9)&1)
#define CEL_FLAG_CCBPRE(x) ((x>>22)&1)
#define CEL_PRE0_BPP(x) (x&7)
#define CEL_PRE0_UNCODED(x) ((x>>4)&1)

typedef struct{
  unsigned int flags;
  unsigned int nextptr;
  unsigned int sourceptr;
  unsigned int plutptr;
  unsigned int xpos;
  unsigned int ypos;
  unsigned int hdx;
  unsigned int hdy;
  unsigned int vdx;
  unsigned int vdy;
  unsigned int hddx;
  unsigned int hddy;
  unsigned int pixc;
  unsigned int pre0;
  unsigned int pre1;
} CCB_3DO;

typedef struct{
 int x_res;
 int y_res;
 int pdat_size;
 CCB_3DO ccb_chank;
 unsigned short plut_data[32];
 unsigned char *pdat_buf;
 unsigned int *image_buf;
} CelImage_3DO;

typedef struct{
 int flag;
 int hand;
 int fpos;
 int curr_pos;
 int file_size;
 int ctype;
 int frames;
 int cntfrm;
 int playf;
 int delay;
} _3DOStream_inf;

int OpenCel(char *file);
int OpenIMAGE(char *file);

int get_frame_from_Stream();
int getStream(char *file);
void initStream();
void closeStream();

int CelToRaw();
void CelStrClear();
extern CelImage_3DO CelIm;
extern _3DOStream_inf _3dosinfo;
extern const int bits_per_pixel[];
