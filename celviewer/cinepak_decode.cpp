/* ------------------------------------------------------------------------
 * Radius Cinepak Video Decoder
 *
 * Dr. Tim Ferguson, 2001.
 * For more details on the algorithm:
 *         http://www.csse.monash.edu.au/~timf/videocodec.html
 *
 * This is basically a vector quantiser with adaptive vector density.  The
 * frame is segmented into 4x4 pixel blocks, and each block is coded using
 * either 1 or 4 vectors.
 *
 * There are still some issues with this code yet to be resolved.  In
 * particular with decoding in the strip boundaries.  However, I have not
 * yet found a sequence it doesn't work on.  Ill keep trying :)
 *
 * You may freely use this source code.  I only ask that you reference its
 * source in your projects documentation:
 *       Tim Ferguson: http://www.csse.monash.edu.au/~timf/
 * ------------------------------------------------------------------------ */
//#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
//#include <unistd.h>
#include <math.h>


#define DBUG	0
#define MAX_STRIPS 4800

/* ------------------------------------------------------------------------ */
typedef struct
{
   //     int flag;
	unsigned char y0, y1, y2, y3;
	char u, v;
	unsigned long rgb0, rgb1, rgb2, rgb3;		/* should be a union */
	unsigned char r[4], g[4], b[4];
} cvid_codebook;

typedef struct {
	cvid_codebook *v4_codebook[MAX_STRIPS];
	cvid_codebook *v1_codebook[MAX_STRIPS];
	int strip_num;
} cinepak_info;


/* ------------------------------------------------------------------------ */
static unsigned char *in_buffer, uiclip[1024], *uiclp = NULL;

#define get_byte() *(in_buffer++)
#define skip_byte() in_buffer++
#define get_word() ((unsigned short)(in_buffer += 2, \
	(in_buffer[-2] << 8 | in_buffer[-1])))
#define get_long() ((unsigned long)(in_buffer += 4, \
	(in_buffer[-4] << 24 | in_buffer[-3] << 16 | in_buffer[-2] << 8 | in_buffer[-1])))


/* ---------------------------------------------------------------------- */
static inline void read_codebook_32(cvid_codebook *c, int mode)
{
int uvr, uvg, uvb;

  //      c->flag=-1;
	if(mode)		/* black and white */
		{
	     	c->y0 = get_byte();
	     	c->y1 = get_byte();
	     	c->y2 = get_byte();
	     	c->y3 = get_byte();
	     	c->u = c->v = 0;

	     	c->rgb0 = (c->y0 << 16) | (c->y0 << 8) | c->y0;
	     	c->rgb1 = (c->y1 << 16) | (c->y1 << 8) | c->y1;
	     	c->rgb2 = (c->y2 << 16) | (c->y2 << 8) | c->y2;
	     	c->rgb3 = (c->y3 << 16) | (c->y3 << 8) | c->y3;
		}
	else			/* colour */
		{
		c->y0 = get_byte();  /* luma */
		c->y1 = get_byte();
		c->y2 = get_byte();
		c->y3 = get_byte();
		c->u = get_byte(); /* chroma */
		c->v = get_byte();

		uvr = c->v << 1;
		uvg = -((c->u+1) >> 1) - c->v;
		uvb = c->u << 1;

		c->rgb0 = (uiclp[c->y0 + uvr] << 16) | (uiclp[c->y0 + uvg] << 8) | uiclp[c->y0 + uvb];
		c->rgb1 = (uiclp[c->y1 + uvr] << 16) | (uiclp[c->y1 + uvg] << 8) | uiclp[c->y1 + uvb];
		c->rgb2 = (uiclp[c->y2 + uvr] << 16) | (uiclp[c->y2 + uvg] << 8) | uiclp[c->y2 + uvb];
		c->rgb3 = (uiclp[c->y3 + uvr] << 16) | (uiclp[c->y3 + uvg] << 8) | uiclp[c->y3 + uvb];
		}
}


/* ------------------------------------------------------------------------ */
inline void cvid_v1_32(unsigned char *frm, unsigned char *end, int stride, cvid_codebook *cb)
{
unsigned long *vptr = (unsigned long *)frm, rgb;
int row_inc = stride/4;
  //      if (cb->flag!=-1)return;
	vptr[0] = rgb = cb->rgb0; vptr[1] = rgb;
	vptr[2] = rgb = cb->rgb1; vptr[3] = rgb;
	vptr += row_inc; if(vptr > (unsigned long *)end) return;
	vptr[0] = rgb = cb->rgb0; vptr[1] = rgb;
	vptr[2] = rgb = cb->rgb1; vptr[3] = rgb;
	vptr += row_inc; if(vptr > (unsigned long *)end) return;
	vptr[0] = rgb = cb->rgb2; vptr[1] = rgb;
	vptr[2] = rgb = cb->rgb3; vptr[3] = rgb;
	vptr += row_inc; if(vptr > (unsigned long *)end) return;
	vptr[0] = rgb = cb->rgb2; vptr[1] = rgb;
	vptr[2] = rgb = cb->rgb3; vptr[3] = rgb;
}


/* ------------------------------------------------------------------------ */
inline void cvid_v4_32(unsigned char *frm, unsigned char *end, int stride, cvid_codebook *cb0,
	cvid_codebook *cb1, cvid_codebook *cb2, cvid_codebook *cb3)
{
unsigned long *vptr = (unsigned long *)frm;
int row_inc = stride/4;
     //   if (cb0->flag!=-1)goto m1;

	vptr[0] = cb0->rgb0;
	vptr[1] = cb0->rgb1;
m1:
     //   if (cb1->flag!=-1)goto m2;
	vptr[2] = cb1->rgb0;
	vptr[3] = cb1->rgb1;
m2:
	vptr += row_inc; if(vptr > (unsigned long *)end) return;
     //   if (cb0->flag!=-1)goto m3;
	vptr[0] = cb0->rgb2;
	vptr[1] = cb0->rgb3;
m3:
     //   if (cb1->flag!=-1)goto m4;
	vptr[2] = cb1->rgb2;
	vptr[3] = cb1->rgb3;
m4:
	vptr += row_inc; if(vptr > (unsigned long *)end) return;
    //    if (cb2->flag!=-1)goto m5;
	vptr[0] = cb2->rgb0;
	vptr[1] = cb2->rgb1;
m5:
    //    if (cb3->flag!=-1)goto m6;
	vptr[2] = cb3->rgb0;
	vptr[3] = cb3->rgb1;
m6:
	vptr += row_inc; if(vptr > (unsigned long *)end) return;
   //     if (cb2->flag!=-1)goto m7;
	vptr[0] = cb2->rgb2;
	vptr[1] = cb2->rgb3;
m7:
 //       if (cb3->flag!=-1)goto m8;
	vptr[2] = cb3->rgb2;
	vptr[3] = cb3->rgb3;
m8: ;
}


/* ---------------------------------------------------------------------- */
static inline void read_codebook_24(cvid_codebook *c, int mode)
{
int uvr, uvg, uvb;

	if(mode)		/* black and white */
		{
		c->y0 = get_byte();
		c->y1 = get_byte();
		c->y2 = get_byte();
		c->y3 = get_byte();
		c->u = c->v = 0;

		c->r[0] = c->g[0] = c->b[0] = c->y0;
		c->r[1] = c->g[1] = c->b[1] = c->y1;
		c->r[2] = c->g[2] = c->b[2] = c->y2;
		c->r[3] = c->g[3] = c->b[3] = c->y3;
		}
	else			/* colour */
		{
		c->y0 = get_byte();  /* luma */
		c->y1 = get_byte();
		c->y2 = get_byte();
		c->y3 = get_byte();
		c->u = get_byte(); /* chroma */
		c->v = get_byte();

		uvr = c->v << 1;
		uvg = -((c->u+1) >> 1) - c->v;
		uvb = c->u << 1;

		c->r[0] = uiclp[c->y0 + uvr]; c->g[0] = uiclp[c->y0 + uvg]; c->b[0] = uiclp[c->y0 + uvb];
		c->r[1] = uiclp[c->y1 + uvr]; c->g[1] = uiclp[c->y1 + uvg]; c->b[1] = uiclp[c->y1 + uvb];
		c->r[2] = uiclp[c->y2 + uvr]; c->g[2] = uiclp[c->y2 + uvg]; c->b[2] = uiclp[c->y2 + uvb];
		c->r[3] = uiclp[c->y3 + uvr]; c->g[3] = uiclp[c->y3 + uvg]; c->b[3] = uiclp[c->y3 + uvb];
		}
}


/* ------------------------------------------------------------------------ */
void cvid_v1_24(unsigned char *vptr, unsigned char *end, int stride, cvid_codebook *cb)
{
unsigned char r, g, b;
int row_inc = stride-4*3;

	*vptr++ = b = cb->b[0]; *vptr++ = g = cb->g[0]; *vptr++ = r = cb->r[0];
	*vptr++ = b; *vptr++ = g; *vptr++ = r;
	*vptr++ = b = cb->b[1]; *vptr++ = g = cb->g[1]; *vptr++ = r = cb->r[1];
	*vptr++ = b; *vptr++ = g; *vptr++ = r;
	vptr += row_inc; if(vptr > end) return;
	*vptr++ = b = cb->b[0]; *vptr++ = g = cb->g[0]; *vptr++ = r = cb->r[0];
	*vptr++ = b; *vptr++ = g; *vptr++ = r;
	*vptr++ = b = cb->b[1]; *vptr++ = g = cb->g[1]; *vptr++ = r = cb->r[1];
	*vptr++ = b; *vptr++ = g; *vptr++ = r;
	vptr += row_inc; if(vptr > end) return;
	*vptr++ = b = cb->b[2]; *vptr++ = g = cb->g[2]; *vptr++ = r = cb->r[2];
	*vptr++ = b; *vptr++ = g; *vptr++ = r;
	*vptr++ = b = cb->b[3]; *vptr++ = g = cb->g[3]; *vptr++ = r = cb->r[3];
	*vptr++ = b; *vptr++ = g; *vptr++ = r;
	vptr += row_inc; if(vptr > end) return;
	*vptr++ = b = cb->b[2]; *vptr++ = g = cb->g[2]; *vptr++ = r = cb->r[2];
	*vptr++ = b; *vptr++ = g; *vptr++ = r;
	*vptr++ = b = cb->b[3]; *vptr++ = g = cb->g[3]; *vptr++ = r = cb->r[3];
	*vptr++ = b; *vptr++ = g; *vptr++ = r;
}


/* ------------------------------------------------------------------------ */
void cvid_v4_24(unsigned char *vptr, unsigned char *end, int stride, cvid_codebook *cb0,
	cvid_codebook *cb1, cvid_codebook *cb2, cvid_codebook *cb3)
{
int row_inc = stride-4*3;

	*vptr++ = cb0->b[0]; *vptr++ = cb0->g[0]; *vptr++ = cb0->r[0];
	*vptr++ = cb0->b[1]; *vptr++ = cb0->g[1]; *vptr++ = cb0->r[1];
	*vptr++ = cb1->b[0]; *vptr++ = cb1->g[0]; *vptr++ = cb1->r[0];
	*vptr++ = cb1->b[1]; *vptr++ = cb1->g[1]; *vptr++ = cb1->r[1];
	vptr += row_inc; if(vptr > end) return;
	*vptr++ = cb0->b[2]; *vptr++ = cb0->g[2]; *vptr++ = cb0->r[2];
	*vptr++ = cb0->b[3]; *vptr++ = cb0->g[3]; *vptr++ = cb0->r[3];
	*vptr++ = cb1->b[2]; *vptr++ = cb1->g[2]; *vptr++ = cb1->r[2];
	*vptr++ = cb1->b[3]; *vptr++ = cb1->g[3]; *vptr++ = cb1->r[3];
	vptr += row_inc; if(vptr > end) return;
	*vptr++ = cb2->b[0]; *vptr++ = cb2->g[0]; *vptr++ = cb2->r[0];
	*vptr++ = cb2->b[1]; *vptr++ = cb2->g[1]; *vptr++ = cb2->r[1];
	*vptr++ = cb3->b[0]; *vptr++ = cb3->g[0]; *vptr++ = cb3->r[0];
	*vptr++ = cb3->b[1]; *vptr++ = cb3->g[1]; *vptr++ = cb3->r[1];
	vptr += row_inc; if(vptr > end) return;
	*vptr++ = cb2->b[2]; *vptr++ = cb2->g[2]; *vptr++ = cb2->r[2];
	*vptr++ = cb2->b[3]; *vptr++ = cb2->g[3]; *vptr++ = cb2->r[3];
	*vptr++ = cb3->b[2]; *vptr++ = cb3->g[2]; *vptr++ = cb3->r[2];
	*vptr++ = cb3->b[3]; *vptr++ = cb3->g[3]; *vptr++ = cb3->r[3];
}


/* ------------------------------------------------------------------------
 * Call this function once at the start of the sequence and save the
 * returned context for calls to decode_cinepak().
 */
void *decode_cinepak_init(void)
{
cinepak_info *cvinfo;
int i;
        //uiclp=NULL;
	if((cvinfo =(cinepak_info*) calloc(sizeof(cinepak_info), 1)) == NULL) return NULL;
	cvinfo->strip_num = 0;

	if(uiclp == NULL)
		{
		uiclp = uiclip+512;
		for(i = -512; i < 512; i++)
			uiclp[i] = (i < 0 ? 0 : (i > 255 ? 255 : i));
		}

        for(i = 0; i < MAX_STRIPS; i++)
			{
			 cvinfo->v4_codebook[i] = NULL;
                         cvinfo->v1_codebook[i] = NULL;
			}


	return (void *)cvinfo;
}

void decode_cinepak_free(void* context)
{
  int i;
  cinepak_info *cvinfo = (cinepak_info *)context;
  uiclp=NULL;
  for(i = 0; i < MAX_STRIPS; i++)
			{
			 if(cvinfo->v4_codebook[i]!= NULL) free(cvinfo->v4_codebook[i]);
                         cvinfo->v4_codebook[i] = NULL;
                         if(cvinfo->v1_codebook[i]!= NULL) free(cvinfo->v1_codebook[i]);
                         cvinfo->v1_codebook[i] = NULL;
			}

  free(cvinfo);
}
/* ------------------------------------------------------------------------
 * This function decodes a buffer containing a Cinepak encoded frame.
 *
 * context - the context created by decode_cinepak_init().
 * buf - the input buffer to be decoded
 * size - the size of the input buffer
 * frame - the output frame buffer (24 or 32 bit per pixel)
 * width - the width of the output frame
 * height - the height of the output frame
 * bit_per_pixel - the number of bits per pixel allocated to the output
 *   frame (only 24 or 32 bpp are supported)
 */
int decode_cinepak(void *context, unsigned char *buf, int size, unsigned char *frame, int width, int height, int bit_per_pixel)
{
cinepak_info *cvinfo = (cinepak_info *)context;
cvid_codebook *v4_codebook, *v1_codebook, *codebook = NULL;
unsigned long x, y, y_bottom, frame_flags, strips, cv_width, cv_height, cnum,temper_ch,
	strip_id, chunk_id, x0, y0, x1, y1, ci, flag, mask;
long len, top_size, chunk_size;
unsigned char *frm_ptr, *frm_end, *upploader;
int i, cur_strip, d0, d1, d2, d3, frm_stride, bpp = 3;
void (*read_codebook)(cvid_codebook *c, int mode) = read_codebook_24;
void (*cvid_v1)(unsigned char *frm, unsigned char *end, int stride, cvid_codebook *cb) = cvid_v1_24;
void (*cvid_v4)(unsigned char *frm, unsigned char *end, int stride, cvid_codebook *cb0,
	cvid_codebook *cb1, cvid_codebook *cb2, cvid_codebook *cb3) = cvid_v4_24;

	x = y = 0;
	y_bottom = 0;
	in_buffer = buf;

	frame_flags = get_byte();
	len = get_byte() << 16;
	len |= get_byte() << 8;
	len |= get_byte();

	switch(bit_per_pixel)
		{
		case 24:
			bpp = 3;
			read_codebook = read_codebook_24;
			cvid_v1 = cvid_v1_24;
			cvid_v4 = cvid_v4_24;
			break;
		case 32:
			bpp = 4;
			read_codebook = read_codebook_32;
			cvid_v1 = cvid_v1_32;
			cvid_v4 = cvid_v4_32;
			break;
		}

	frm_stride = width * bpp;
	frm_ptr = frame;
	frm_end = frm_ptr + width * height * bpp;
        //if(size>len)
        //size=len;
	if(len != size)
		{
		//if(len & 0x01) {len+=3;len&=0xfffffffc;} /* AVIs tend to have a size mismatch */
		if(len != size)
			{
			//printf("CVID: corruption %d (QT/AVI) != %ld (CV)\n", size, len);
			//// return;
	//	      	return 1;
			}
		}

	cv_width = get_word();
	cv_height = get_word();
	strips = get_word();

	if(strips > cvinfo->strip_num)
		{
                if(strips >= MAX_STRIPS)
			{
			//printf("CVID: strip overflow (more than %d)\n", MAX_STRIPS);
			return 2;
			}

		for(i = cvinfo->strip_num; i < strips; i++)
			{
			if((cvinfo->v4_codebook[i] = (cvid_codebook *)calloc(sizeof(cvid_codebook), 256)) == NULL)
				{
				//printf("CVID: codebook v4 alloc err\n");
				return 3;
				}

			if((cvinfo->v1_codebook[i] = (cvid_codebook *)calloc(sizeof(cvid_codebook), 256)) == NULL)
				{
				//printf("CVID: codebook v1 alloc err\n");
				return 4;
				}
			}
		}
        //cur_strip= cvinfo->strip_num;
	cvinfo->strip_num = strips;

#if DBUG
	printf("CVID: <%ld,%ld> strips %ld\n", cv_width, cv_height, strips);
#endif

	for(cur_strip = 0; cur_strip < strips; cur_strip++)
		{
		v4_codebook = cvinfo->v4_codebook[cur_strip];
		v1_codebook = cvinfo->v1_codebook[cur_strip];

		if((cur_strip > 0) /*&& (!(frame_flags & 0x01))*/)
			{
			memcpy(cvinfo->v4_codebook[cur_strip], cvinfo->v4_codebook[cur_strip-1], 256 * sizeof(cvid_codebook));
			memcpy(cvinfo->v1_codebook[cur_strip], cvinfo->v1_codebook[cur_strip-1], 256 * sizeof(cvid_codebook));
			}

		strip_id = get_word();		/* 1000 = key strip, 1100 = iter strip */
		top_size = get_word();

		while(strip_id!=0x1000 && strip_id!=0x1100)
		{
		  in_buffer+=top_size-4;
		  if(in_buffer>buf+size)return 5;
		  strip_id = get_word();		/* 1000 = key strip, 1100 = iter strip */
		  top_size = get_word();
		}

		y0 = get_word();		/* FIXME: most of these are ignored at the moment */
		x0 = get_word();
		y1 = get_word();
		x1 = get_word();

                if(y0!=0 || x0!=0 || x1!=width || y1!=height)
                {
                 return 8;
                }

		y_bottom += y1;
		top_size -= 12;
		x = 0;
		//if(x1 != width)
		//	printf("CVID: Warning x1 (%ld) != width (%d)\n", x1, width);

#if DBUG
	printf("   %d) %04lx %04ld <%ld,%ld> <%ld,%ld> yt %ld\n",
		cur_strip, strip_id, top_size, x0, y0, x1, y1, y_bottom);
#endif
                upploader=in_buffer;
                temper_ch=0;
		while(top_size > 0)
			{
                        upploader=in_buffer=upploader+temper_ch;
			chunk_id  = get_word();
			chunk_size = get_word();
                        temper_ch=chunk_size;


#if DBUG
	printf("        %04lx %04ld\n", chunk_id, chunk_size);
#endif
			top_size -= chunk_size;
			chunk_size -= 4;

			switch(chunk_id)
				{
					/* -------------------- Codebook Entries -------------------- */
				case 0x2000:
				case 0x2200:
					codebook = (chunk_id == 0x2200 ? v1_codebook : v4_codebook);
					cnum = chunk_size/6;
					for(i = 0; i < cnum; i++) read_codebook(codebook+i, 0);
					break;

				case 0x2400:
				case 0x2600:		/* 8 bit per pixel */
					codebook = (chunk_id == 0x2600 ? v1_codebook : v4_codebook);
					cnum = chunk_size/4;  
					for(i = 0; i < cnum; i++) read_codebook(codebook+i, 1);
					break;

				case 0x2100:
				case 0x2300:
					codebook = (chunk_id == 0x2300 ? v1_codebook : v4_codebook);

					ci = 0;
					while(chunk_size >3)
						{

						flag = get_long();
						chunk_size -= 4;

						for(i = 0; i < 32; i++)
							{
                                                        if(chunk_size<6)break;
							if(flag & 0x80000000)
								{
								chunk_size -= 6;
								read_codebook(codebook+ci, 0);
								}

							ci++;
							flag <<= 1;
							}
						}
					while(chunk_size > 0) { skip_byte(); chunk_size--; }
					break;

				case 0x2500:
				case 0x2700:		/* 8 bit per pixel */
					codebook = (chunk_id == 0x2700 ? v1_codebook : v4_codebook);

					ci = 0;
					while(chunk_size > 3)
						{
						flag = get_long();
						chunk_size -= 4;

						for(i = 0; i < 32; i++)
							{
                                                        if(chunk_size<4)break;
							if(flag & 0x80000000)
								{
								chunk_size -= 4;
								read_codebook(codebook+ci, 1);
								}

							ci++;
							flag <<= 1;
							}
						}
					while(chunk_size > 0) { skip_byte(); chunk_size--; }
					break;

					/* -------------------- Frame -------------------- */
				case 0x3000: 
					while((chunk_size > 0) && (y < y_bottom))
						{
                                                if(y+4==y_bottom && x>=width)break;
						flag = get_long();
						chunk_size -= 4;

						for(i = 0; i < 32; i++)
							{
							if(y >= y_bottom) break;
                                                        if(y+4==y_bottom && x>=width)break;
							if(flag & 0x80000000)	/* 4 bytes per block */
								{
								d0 = get_byte();
								d1 = get_byte();
								d2 = get_byte();
								d3 = get_byte();
								chunk_size -= 4;
								cvid_v4(frm_ptr + (y * frm_stride + x * bpp), frm_end, frm_stride, v4_codebook+d0, v4_codebook+d1, v4_codebook+d2, v4_codebook+d3);
								}
							else		/* 1 byte per block */
								{
								cvid_v1(frm_ptr + (y * frm_stride + x * bpp), frm_end, frm_stride, v1_codebook + get_byte());
								chunk_size--;
								}

							x += 4;
							if(x >= width)
								{
								x = 0;
								y += 4;
								}
							flag <<= 1;
							}

						}
					while(chunk_size > 0) { skip_byte(); chunk_size--; }
					break;

				case 0x3100:
					while((chunk_size > 3) && (y < y_bottom))
						{
                                                if(y+4==y_bottom && x>=width)break;
							/* ---- flag bits: 0 = SKIP, 10 = V1, 11 = V4 ---- */
						flag = (unsigned long)get_long();
						chunk_size -= 4;
						mask = 0x80000000;

						while((mask) && (y < y_bottom))
							{
                                                        if(y+4==y_bottom && x>=width)break;
							if(flag & mask)
								{
								if(mask == 1)
									{
									if(chunk_size < 0) break;
									flag = (unsigned long)get_long();
									chunk_size -= 4;
									mask = 0x80000000;
									}
							   	else mask >>= 1;

								if(flag & mask)		/* V4 */
									{
									d0 = get_byte();
                                                                        //if(d0>=0x80)d0=0;
									d1 = get_byte();
                                                                        //if(d1>=0x80)d1=0;
									d2 = get_byte();
                                                                        //if(d2>=0x80)d2=0;
									d3 = get_byte();
                                                                        //if(d3>=0x80)d3=0;
									chunk_size -= 4;
								 	cvid_v4(frm_ptr + (y * frm_stride + x * bpp), frm_end, frm_stride, v4_codebook+d0, v4_codebook+d1, v4_codebook+d2, v4_codebook+d3);
									}
								else		/* V1 */
									{
									chunk_size--;
                                                                        d0 = get_byte();
                                                                       // if(d0>=0xc0)d0=0;
							      		cvid_v1(frm_ptr + (y * frm_stride + x * bpp), frm_end, frm_stride, v1_codebook + d0);
									}
								}		/* else SKIP */

							mask >>= 1;
							x += 4;
							if(x >= width)
								{
								x = 0;
								y += 4;
								}
							}
						}

					while(chunk_size > 0) { skip_byte(); chunk_size--; }
					break;

				case 0x3200:		/* each byte is a V1 codebook */
					while((chunk_size > 0) && (y < y_bottom))
						{
                                                if(y+4==y_bottom && x>=width)break;
						cvid_v1(frm_ptr + (y * frm_stride + x * bpp), frm_end, frm_stride, v1_codebook + get_byte());
						chunk_size--;
						x += 4;
						if(x >= width)
							{
							x = 0;
							y += 4;
							}
						}
finlandia:
					while(chunk_size > 0) { skip_byte(); chunk_size--; }
					break;

				default:
                                        //skip_byte();
					//printf("CVID: unknown chunk_id %08lx\n", chunk_id);
					while(chunk_size > 0) { skip_byte(); chunk_size--; }
					break;
				}
			}
		}

 return 0;
}

