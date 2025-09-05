/*
	File:		SCelSubscriber.h

	Contains:	Definitions for SCel Subscriber's data-stream chunk and run-time
				data structures.

	Written by:	3DO Software Attic
				Darren Gibbs (Proto Subscriber)
				Chris McFall (SCel Subscriber)

	Copyright:	© 1994 by The 3DO Company. All rights reserved.
				This material constitutes confidential and proprietary
				information of the 3DO Company and shall not be used by
				any Person or for any purpose except as expressly
				authorized in writing by the 3DO Company.

	Change History (most recent first):

				10/ 6/94	crm		Added SetSCelChannelFlags function to provide a way to
									enable a channel.  Renamed GetSegmentCount and
									GetFramesRemaining to avoid name collision with other
									subscribers.
				10/ 6/94	crm		Replace per-channel semaphore with single subscriber-context
									semaphore.
				 9/28/94	crm		Added field to detect stream time loop-back.
				 9/22/94	crm		Added memory functions to support new stream
				 					format and allow copying of arriving data chunks.
				 9/20/94	crm		added presentation message pointer to channel struct
				 9/07/94	ec		got rid of "typedef" in enum SCelControlOpcode 
				 4/23/94	crm		Created SCel subscriber from Proto subscriber base.
				 3/23/94	rdg		New today.

	To Do:
*/

#ifndef __SCELSUBSCRIBER_H__
#define __SCELSUBSCRIBER_H__

#include "graphics.h"
#include "DataStream.h"
#include "SubscriberUtils.h"
#include "SCelErrors.h"
#include "SCelFileFormat.h"


/*************/
/* Constants */
/*************/

/* Max number of logical channels per subscription */
#define	SCEL_MAX_CHANNELS		8		

/* NewPtr and FreePtr functions used by the SCel subscriber whenever
 * it has to allocate or free memory dynamically.
 */
typedef void *(*SCelNewPtrFcn) (uint32, uint32, uint32, int32, int32);
typedef void *(*SCelFreePtrFcn) (void *, uint32, int32, int32);


/*************************************************************/
/* Subscriber logical channel, SCEL_MAX_CHANNELS per context */
/*************************************************************/

typedef struct SCelChannel {
	ulong				status;				/* state bits */
	SubsQueue			dataMsgQueue;		/* queued data chunks */
	SubscriberMsgPtr	presentMsg;			/* message containing data chunk currently being presented */
	SCelDataChunkPtr	presentChunk;		/* pointer to chunk part of message or copied chunk */

	long				version;			/* version number of current segment */
	long				duration;			/* duration of current segment */
	long				numFrames;			/* number of frames in current segment */
	long				headerCount;		/* count of headers that have flowed through */
	long				framesRemaining;	/* count of frames (data chunks) remaining in this segment */
	} SCelChannel, *SCelChannelPtr;


/**************************************/
/* Subscriber context, one per stream */
/**************************************/

typedef struct SCelContext {
	Item			creatorTask;				/* who to signal when done initializing */
	ulong			creatorSignal;				/* signal used to synchronize initialization */
	long			creatorStatus;				/* result code for initialization */

	Item			threadItem;					/* subscriber thread item */
	void*			threadStackBlock;			/* address of thread's stack memory block */

	Item			requestPort;				/* message port item for subscriber requests */
	ulong			requestPortSignal;			/* signal to detect request port message arrival */

	DSStreamCBPtr	streamCBPtr;				/* stream to which this subscriber belongs */

	uint32			status;						/* flags word for subscriber status information */
	SCelNewPtrFcn	newPtrFcn;					/* procedure to call to allocate memory */
	SCelFreePtrFcn	freePtrFcn;					/* procedure to call to free memory */

	uint32			prevStreamTime;				/* keep track of stream clock so as to detect loop-back */
	Item			lock;						/* protects this subscriber context */

	SCelChannel		channel[SCEL_MAX_CHANNELS];	/* array of channels (N.B., max channels is fixed) */

	} SCelContext, *SCelContextPtr;



/*****************************/
/* Public routine prototypes */
/*****************************/
#ifdef __cplusplus 
extern "C" {
#endif

long	NewSCelSubscriber( SCelContextPtr *pCtx, DSStreamCBPtr streamCBPtr, 
						   long deltaPriority );
long	DisposeSCelSubscriber( SCelContextPtr ctx );

long	InitSCelSubscriber( void );
long	CloseSCelSubscriber( void );

CCB*	GetSCelList ( SCelContextPtr ctx, long channelNumber );
CCB*	GetSCelListAndType ( SCelContextPtr ctx, long channelNumber, long* celListType );

long	GetSCelSegmentCount ( SCelContextPtr ctx, long channelNumber );
long	GetSCelFramesRemaining ( SCelContextPtr ctx, long channelNumber );

long	SetSCelChannelFlags ( SCelContextPtr ctx, long channelNumber, long mask, long flagValues );

long	SetSCelMemoryFcns ( SCelContextPtr ctx, SCelNewPtrFcn newPtrFcn, SCelFreePtrFcn freePtrFcn );

#ifdef __cplusplus
}
#endif

#endif	/* __SCELSUBSCRIBER_H__ */

