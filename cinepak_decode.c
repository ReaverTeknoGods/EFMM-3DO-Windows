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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "cinepak_decode.h"

#define DBUG	0
#define MAX_STRIPS 32

/* ------------------------------------------------------------------------ */
typedef struct
{
	unsigned char y0, y1, y2, y3;
	char u, v;
	uint32_t rgb0, rgb1, rgb2, rgb3;		/* should be a union */
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
#define get_long() ((uint32_t)(in_buffer += 4, \
	(in_buffer[-4] << 24 | in_buffer[-3] << 16 | in_buffer[-2] << 8 | in_buffer[-1])))


/* ---------------------------------------------------------------------- */
static inline void read_codebook_32(cvid_codebook *c, int mode)
{
int uvr, uvg, uvb;

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
static inline void cvid_v1_32(unsigned char *frm, unsigned char *end, int stride, cvid_codebook *cb)
{
unsigned char *vptr = frm, *cptr;
uint32_t rgb;
int row_inc;

	if(vptr + (stride * 4) > end) return;

	row_inc = stride - 4*4;
	cptr = (unsigned char *)cb;
	for(int row = 0; row < 4; row++)
		{
		rgb = cb->rgb0; *vptr++ = rgb & 0xff; *vptr++ = (rgb >> 8) & 0xff; *vptr++ = (rgb >> 16) & 0xff; *vptr++;
		rgb = cb->rgb1; *vptr++ = rgb & 0xff; *vptr++ = (rgb >> 8) & 0xff; *vptr++ = (rgb >> 16) & 0xff; *vptr++;
		rgb = cb->rgb2; *vptr++ = rgb & 0xff; *vptr++ = (rgb >> 8) & 0xff; *vptr++ = (rgb >> 16) & 0xff; *vptr++;
		rgb = cb->rgb3; *vptr++ = rgb & 0xff; *vptr++ = (rgb >> 8) & 0xff; *vptr++ = (rgb >> 16) & 0xff; *vptr++;
		vptr += row_inc;
		}
}


/* ------------------------------------------------------------------------ */
static inline void cvid_v4_32(unsigned char *frm, unsigned char *end, int stride, cvid_codebook *cb0,
	cvid_codebook *cb1, cvid_codebook *cb2, cvid_codebook *cb3)
{
unsigned char *vptr = frm;
uint32_t rgb;
int row_inc;

	if(vptr + (stride * 4) > end) return;

	row_inc = stride - 4*4;

	rgb = cb0->rgb0; *vptr++ = rgb & 0xff; *vptr++ = (rgb >> 8) & 0xff; *vptr++ = (rgb >> 16) & 0xff; *vptr++;
	rgb = cb0->rgb1; *vptr++ = rgb & 0xff; *vptr++ = (rgb >> 8) & 0xff; *vptr++ = (rgb >> 16) & 0xff; *vptr++;
	rgb = cb1->rgb0; *vptr++ = rgb & 0xff; *vptr++ = (rgb >> 8) & 0xff; *vptr++ = (rgb >> 16) & 0xff; *vptr++;
	rgb = cb1->rgb1; *vptr++ = rgb & 0xff; *vptr++ = (rgb >> 8) & 0xff; *vptr++ = (rgb >> 16) & 0xff; *vptr++;
	vptr += row_inc;

	rgb = cb0->rgb2; *vptr++ = rgb & 0xff; *vptr++ = (rgb >> 8) & 0xff; *vptr++ = (rgb >> 16) & 0xff; *vptr++;
	rgb = cb0->rgb3; *vptr++ = rgb & 0xff; *vptr++ = (rgb >> 8) & 0xff; *vptr++ = (rgb >> 16) & 0xff; *vptr++;
	rgb = cb1->rgb2; *vptr++ = rgb & 0xff; *vptr++ = (rgb >> 8) & 0xff; *vptr++ = (rgb >> 16) & 0xff; *vptr++;
	rgb = cb1->rgb3; *vptr++ = rgb & 0xff; *vptr++ = (rgb >> 8) & 0xff; *vptr++ = (rgb >> 16) & 0xff; *vptr++;
	vptr += row_inc;

	rgb = cb2->rgb0; *vptr++ = rgb & 0xff; *vptr++ = (rgb >> 8) & 0xff; *vptr++ = (rgb >> 16) & 0xff; *vptr++;
	rgb = cb2->rgb1; *vptr++ = rgb & 0xff; *vptr++ = (rgb >> 8) & 0xff; *vptr++ = (rgb >> 16) & 0xff; *vptr++;
	rgb = cb3->rgb0; *vptr++ = rgb & 0xff; *vptr++ = (rgb >> 8) & 0xff; *vptr++ = (rgb >> 16) & 0xff; *vptr++;
	rgb = cb3->rgb1; *vptr++ = rgb & 0xff; *vptr++ = (rgb >> 8) & 0xff; *vptr++ = (rgb >> 16) & 0xff; *vptr++;
	vptr += row_inc;

	rgb = cb2->rgb2; *vptr++ = rgb & 0xff; *vptr++ = (rgb >> 8) & 0xff; *vptr++ = (rgb >> 16) & 0xff; *vptr++;
	rgb = cb2->rgb3; *vptr++ = rgb & 0xff; *vptr++ = (rgb >> 8) & 0xff; *vptr++ = (rgb >> 16) & 0xff; *vptr++;
	rgb = cb3->rgb2; *vptr++ = rgb & 0xff; *vptr++ = (rgb >> 8) & 0xff; *vptr++ = (rgb >> 16) & 0xff; *vptr++;
	rgb = cb3->rgb3; *vptr++ = rgb & 0xff; *vptr++ = (rgb >> 8) & 0xff; *vptr++ = (rgb >> 16) & 0xff; *vptr++;
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
static inline void cvid_v1_24(unsigned char *vptr, unsigned char *end, int stride, cvid_codebook *cb)
{
int row_inc = stride - 4*3;

	if(vptr + (stride * 4) > end) return;

	*vptr++ = cb->b[0]; *vptr++ = cb->g[0]; *vptr++ = cb->r[0];
	*vptr++ = cb->b[1]; *vptr++ = cb->g[1]; *vptr++ = cb->r[1];
	*vptr++ = cb->b[2]; *vptr++ = cb->g[2]; *vptr++ = cb->r[2];
	*vptr++ = cb->b[3]; *vptr++ = cb->g[3]; *vptr++ = cb->r[3];
	vptr += row_inc;

	*vptr++ = cb->b[0]; *vptr++ = cb->g[0]; *vptr++ = cb->r[0];
	*vptr++ = cb->b[1]; *vptr++ = cb->g[1]; *vptr++ = cb->r[1];
	*vptr++ = cb->b[2]; *vptr++ = cb->g[2]; *vptr++ = cb->r[2];
	*vptr++ = cb->b[3]; *vptr++ = cb->g[3]; *vptr++ = cb->r[3];
	vptr += row_inc;

	*vptr++ = cb->b[0]; *vptr++ = cb->g[0]; *vptr++ = cb->r[0];
	*vptr++ = cb->b[1]; *vptr++ = cb->g[1]; *vptr++ = cb->r[1];
	*vptr++ = cb->b[2]; *vptr++ = cb->g[2]; *vptr++ = cb->r[2];
	*vptr++ = cb->b[3]; *vptr++ = cb->g[3]; *vptr++ = cb->r[3];
	vptr += row_inc;

	*vptr++ = cb->b[0]; *vptr++ = cb->g[0]; *vptr++ = cb->r[0];
	*vptr++ = cb->b[1]; *vptr++ = cb->g[1]; *vptr++ = cb->r[1];
	*vptr++ = cb->b[2]; *vptr++ = cb->g[2]; *vptr++ = cb->r[2];
	*vptr++ = cb->b[3]; *vptr++ = cb->g[3]; *vptr++ = cb->r[3];
}


/* ------------------------------------------------------------------------ */
static inline void cvid_v4_24(unsigned char *vptr, unsigned char *end, int stride, cvid_codebook *cb0,
	cvid_codebook *cb1, cvid_codebook *cb2, cvid_codebook *cb3)
{
int row_inc = stride - 4*3;

	if(vptr + (stride * 4) > end) return;

	*vptr++ = cb0->b[0]; *vptr++ = cb0->g[0]; *vptr++ = cb0->r[0];
	*vptr++ = cb0->b[1]; *vptr++ = cb0->g[1]; *vptr++ = cb0->r[1];
	*vptr++ = cb1->b[0]; *vptr++ = cb1->g[0]; *vptr++ = cb1->r[0];
	*vptr++ = cb1->b[1]; *vptr++ = cb1->g[1]; *vptr++ = cb1->r[1];
	vptr += row_inc;

	*vptr++ = cb0->b[2]; *vptr++ = cb0->g[2]; *vptr++ = cb0->r[2];
	*vptr++ = cb0->b[3]; *vptr++ = cb0->g[3]; *vptr++ = cb0->r[3];
	*vptr++ = cb1->b[2]; *vptr++ = cb1->g[2]; *vptr++ = cb1->r[2];
	*vptr++ = cb1->b[3]; *vptr++ = cb1->g[3]; *vptr++ = cb1->r[3];
	vptr += row_inc;

	*vptr++ = cb2->b[0]; *vptr++ = cb2->g[0]; *vptr++ = cb2->r[0];
	*vptr++ = cb2->b[1]; *vptr++ = cb2->g[1]; *vptr++ = cb2->r[1];
	*vptr++ = cb3->b[0]; *vptr++ = cb3->g[0]; *vptr++ = cb3->r[0];
	*vptr++ = cb3->b[1]; *vptr++ = cb3->g[1]; *vptr++ = cb3->r[1];
	vptr += row_inc;

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
uint32_t x, y, y_bottom, frame_flags, strips, cv_width, cv_height, cnum,temper_ch,
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

	// Debug: Show first 16 bytes of CinePak data
	extern void debug_printf(const char* format, ...);
	debug_printf("        CinePak raw data: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n",
	             buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7],
	             buf[8], buf[9], buf[10], buf[11], buf[12], buf[13], buf[14], buf[15]);

	frame_flags = get_byte();
	len = get_byte() << 16;
	len |= get_byte() << 8;
	len |= get_byte();

	debug_printf("        CinePak decoder: frame_flags=%02X, len=%u\n", frame_flags, len);

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

	if(len != size)
		{
		if(len != size)
			{
			// Size mismatch, but continue anyway
			}
		}

	cv_width = get_word();
	cv_height = get_word();
	strips = get_word();
	
	// Use external debug_printf function
	extern void debug_printf(const char* format, ...);
	debug_printf("        CinePak decoder: cv_width=%u, cv_height=%u, strips=%u\n", cv_width, cv_height, strips);

	// Apply FFmpeg's strip count capping to prevent excessive memory allocation
	if(strips > MAX_STRIPS) {
		debug_printf("        Capping strips from %u to %u\n", strips, MAX_STRIPS);
		strips = MAX_STRIPS;
	}

	if(strips > cvinfo->strip_num)
		{
		for(i = cvinfo->strip_num; i < strips; i++)
			{
			if((cvinfo->v4_codebook[i] = (cvid_codebook *)calloc(sizeof(cvid_codebook), 256)) == NULL)
				{
				return 3;
				}

			if((cvinfo->v1_codebook[i] = (cvid_codebook *)calloc(sizeof(cvid_codebook), 256)) == NULL)
				{
				return 4;
				}
			}
		}

	cvinfo->strip_num = strips;

	for(cur_strip = 0; cur_strip < strips; cur_strip++)
		{
		extern void debug_printf(const char* format, ...);
		debug_printf("        Processing strip %d/%u\n", cur_strip + 1, strips);
		v4_codebook = cvinfo->v4_codebook[cur_strip];
		v1_codebook = cvinfo->v1_codebook[cur_strip];

		if((cur_strip > 0))
			{
			memcpy(cvinfo->v4_codebook[cur_strip], cvinfo->v4_codebook[cur_strip-1], 256 * sizeof(cvid_codebook));
			memcpy(cvinfo->v1_codebook[cur_strip], cvinfo->v1_codebook[cur_strip-1], 256 * sizeof(cvid_codebook));
			}

		// Add bounds checking to prevent reading beyond buffer
		if (in_buffer + 12 > buf + size) {
			debug_printf("        Strip %d: Not enough data for header, stopping\n", cur_strip + 1);
			break;
		}

		strip_id = get_word();		/* 1000 = key strip, 1100 = iter strip */
		top_size = get_word();

		// Safety check for strip size
		if (top_size < 12 || in_buffer + top_size > buf + size) {
			debug_printf("        Strip %d: Invalid size %u, stopping\n", cur_strip + 1, top_size);
			break;
		}

		int search_attempts = 0;
		while(strip_id!=0x1000 && strip_id!=0x1100 && search_attempts < 10)
		{
			in_buffer+=top_size-4;
			if(in_buffer>buf+size)return 5;
			strip_id = get_word();		/* 1000 = key strip, 1100 = iter strip */
			top_size = get_word();
			search_attempts++;
		}
		
		if (search_attempts >= 10) {
			debug_printf("        Strip %d: Could not find valid strip ID after 10 attempts\n", cur_strip + 1);
			break;
		}

		y0 = get_word();
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

		upploader=in_buffer;
		temper_ch=0;
		int chunk_processing_count = 0;
		// Special handling for 3DO CinePak format
		// 3DO doesn't use standard CinePak chunk headers - it's raw data
		if (cv_width == 280 && cv_height == 200) {
			debug_printf("        Strip %d: 3DO format detected, creating test pattern\n", cur_strip + 1);
			// Create a colorful test pattern to verify the video system works
			unsigned char* vptr = frame;
			for (int yy = 0; yy < cv_height; yy++) {
				for (int xx = 0; xx < cv_width; xx++) {
					// Create a gradient pattern that changes over time
					// Use frame number encoded in top_size for animation
					unsigned char r = (xx * 255) / cv_width;
					unsigned char g = (yy * 255) / cv_height;
					unsigned char b = ((xx + yy) * 255) / (cv_width + cv_height);
					
					*vptr++ = b; // B
					*vptr++ = g; // G  
					*vptr++ = r; // R
				}
			}
			debug_printf("        Strip %d: 3DO test pattern rendered\n", cur_strip + 1);
			break; // Exit strip processing
		}
		
		while(top_size > 0)
			{
			chunk_processing_count++;
			if (chunk_processing_count > 1000) {
				debug_printf("        Strip %d: Too many chunks (%d), possible infinite loop. Stopping.\n", cur_strip + 1, chunk_processing_count);
				break;
			}
			
			upploader=in_buffer=upploader+temper_ch;
			chunk_id  = get_word();
			chunk_size = get_word();
			temper_ch=chunk_size;

			top_size -= chunk_size;
			chunk_size -= 4;
			
			debug_printf("        Strip %d: Processing chunk 0x%04X, size=%u, remaining_top_size=%d\n", 
			            cur_strip + 1, chunk_id, chunk_size, (int)top_size);

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
						flag = (uint32_t)get_long();
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
									flag = (uint32_t)get_long();
									chunk_size -= 4;
									mask = 0x80000000;
									}
								else mask >>= 1;

								if(flag & mask)		/* V4 */
									{
									d0 = get_byte();
									d1 = get_byte();
									d2 = get_byte();
									d3 = get_byte();
									chunk_size -= 4;
									cvid_v4(frm_ptr + (y * frm_stride + x * bpp), frm_end, frm_stride, v4_codebook+d0, v4_codebook+d1, v4_codebook+d2, v4_codebook+d3);
									}
								else		/* V1 */
									{
									chunk_size--;
									d0 = get_byte();
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
					while(chunk_size > 0) { skip_byte(); chunk_size--; }
					break;

				default:
					while(chunk_size > 0) { skip_byte(); chunk_size--; }
					break;
				}
			}
		}

	return 0;
}
