/*
	File:		SCelFileFormat.h

	Contains:	Data structures that define the chunks found in a SCEL file.

	Written by:	3DO Software Attic
				Chris McFall

	Copyright:	© 1994 by The 3DO Company. All rights reserved.
				This material constitutes confidential and proprietary
				information of the 3DO Company and shall not be used by
				any Person or for any purpose except as expressly
				authorized in writing by the 3DO Company.

	Change History (most recent first):

				 9/22/94	crm		Added support new stream format to allow copying
				 					of arriving data chunks.
				 9/20/94	crm		New today.  Extracted from SCelSubscriber.h

	To Do:
*/
 
#ifndef __SCELFILEFORMAT_H__
#define __SCELFILEFORMAT_H__

#include "types.h"
#include "graphics.h"
#include "DataStream.h"


/* Stream version supported by this subscriber.  Used for sanity 
 * checking when a header chunk is received.
 * Versions:
 *				0	original
 *				1	flags field added to data chunks
 */
#define SCEL_VERSION_CURRENT		1	
#define SCEL_VERSION_DATAFLAGS		1



/* Subscriber data type.  
 */
#define	SCEL_CHUNK_TYPE				CHAR4LITERAL('S','C','E','L')	

/* Header subchunk type.
 * Each header chunk in the stream defines the start of a
 * segment of scel data chunks (i.e., frames).
 */
#define	SCEL_HEADER_CHUNK_TYPE		CHAR4LITERAL('H','E','A','D')	

/* Data subchunk type.
 * Each scel data chunk is a self-contained linked-list
 * of CCB structures and associated PLUTs and source pixel data.
 */
#define	SCEL_DATA_CHUNK_TYPE		CHAR4LITERAL('C','L','S','T')	

/* Settings subchunk type.
 * Each scel settings chunk describes a set of updates to the
 * current CCB list.
 */
#define	SCEL_SETTINGS_CHUNK_TYPE	CHAR4LITERAL('S','E','T','S')

/* Values for SCel data chunk flags */
#define	SCEL_FLAG_COPYDATA		(1 << 16)		/* copy the data chunk so stream buffer is free */



/***************************************/
/* Tags describe modifiable CCB fields */
/***************************************/

enum {
	SET_END		= 0,
	SET_X,
	SET_Y,
	SET_HDX,
	SET_HDY,
	SET_VDX,
	SET_VDY,
	SET_HDDX,
	SET_HDDY,
	SET_PIXC,
	SET_PLUT,
	SET_SKIP
};



/********************************/
/* Subscriber Chunk definitions */
/********************************/


/*
 * Header Chunk
 * Describes a segment of the data stream that is a
 * sequence of SCEL frames.
 */
typedef	struct SCelHeaderChunk {
	SUBS_CHUNK_COMMON;
	long		version;						/* version control for stream data */
	long		numFrames;						/* count of frames in this segment */
	long		totalDuration;					/* total duration of this segment (in audio ticks) */
	long		reserved;
} SCelHeaderChunk, *SCelHeaderChunkPtr;


/*
 * Data Chunk
 * A single frame of the sequence, consisting of a
 * cel list and type information.  The format allows
 * data chunks to carry user-defined data that has been
 * inserted by the data-prep tool.
 */
typedef struct SCelDataChunk {
	SUBS_CHUNK_COMMON;
	long		duration;						/* duration of this frame (in audio ticks) */
	long		offset;							/* offset to first CCB from top of SCelDataChunk */
	long		celListType;					/* sub-type of CLST chunk */
	/* ----------------------------------------- */
	long		flags;							/* flags field added to version 1 streams */
	/* ----------------------------------------- */
	/* encapsulated cel list is placed here */
} SCelDataChunk, *SCelDataChunkPtr;


/*
 * Settings struct indicates a change to a CCB field
 * in the current frame's cel list.
 */
typedef struct SCelSetting {
	int16		tag;							/* indicates which CCB field to update */
	int16		ccbIndex;						/* which CCB in list to update; zero is first CCB */
	uint32		value;							/* update value or relative pointer to update data */
} SCelSetting, *SCelSettingPtr;


/*
 * Settings Chunk
 * Describes one or more modifications to the CCBs in
 * the current frame's cel list.
 */
typedef struct SCelSettingsChunk {
	SUBS_CHUNK_COMMON;
	uint32		reserved;
	SCelSetting	settings[1];					/* variable-length array of updates to CCB list */
} SCelSettingsChunk, *SCelSettingsChunkPtr;


#endif	/* __SCELFILEFORMAT_H__ */
