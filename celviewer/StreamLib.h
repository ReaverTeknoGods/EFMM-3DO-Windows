#ifndef STREAMLIB_3DO
#define STREAMLIB_3DO

#define	DS_HDR_MAX_PRELOADINST	16
#define	DS_HDR_MAX_SUBSCRIBER	16

#define	HEADER_CHUNK_TYPE		(0x53484452)	/* 'SHDR' */

#define	DS_STREAM_VERSION		2		/* Stream header version number */


typedef struct DSHeaderSubs {
	long		subscriberType;			/* data type for subscriber */
	long		deltaPriority;			/* its delta priority */
	} DSHeaderSubs, *DSHeaderSubsPtr;


typedef struct DSHeaderChunk {
	//SUBS_CHUNK_COMMON;			/* from SubscriberUtils.h */
        long		chunkType;		/* chunk type */					\
	long		chunkSize;		/* chunk size including header */	\
	long		time;			/* position in stream time */		\
	long		channel;		/* logical channel number */		\
	long		subChunkType;	        /* data sub-type */

	long		headerVersion;			/* version of header data */

	long		streamBlockSize;		/* size of stream buffers in this stream */
	long		streamBuffers;			/* suggested number of stream buffers to use */
	long		streamerDeltaPri;		/* delta priority for streamer thread */
	long		dataAcqDeltaPri;		/* delta priority for data acquisition thread */
	long		numSubsMsgs;			/* number of subscriber messages to allocate */

	long		audioClockChan;			/* logical channel number of audio clock channel */
	long		enableAudioChan;		/* mask of audio channels to enable */
	
	long		preloadInstList[DS_HDR_MAX_PRELOADINST];
										/* NULL terminated preloaded instrument list */

	DSHeaderSubs	subscriberList[DS_HDR_MAX_SUBSCRIBER];
										/* NULL terminated list of subscriber tags */

	} DSHeaderChunk,*DSHeaderChunkPtr;

typedef	struct CinePakHeader {
	//SUBS_CHUNK_COMMON;
        long		chunkType;		/* chunk type */					\
	long		chunkSize;		/* chunk size including header */	\
	long		time;			/* position in stream time */		\
	long		channel;		/* logical channel number */		\
	long		subChunkType;	        /* data sub-type */

	long		version;		/*	0 for this version			*/
	long		cType;			/*	video compression type		*/
	long		height;			/*	Height of each frame		*/
	long		width;			/*	Width of each frame			*/
	long		scale;			/*	Timescale of Film			*/
	long		count;			/*	Number of frames			*/
} CinePakHeader, *CinePakHeaderPtr;


typedef	struct	CinePakFrame {
	//SUBS_CHUNK_COMMON;
        long		chunkType;		/* chunk type */					\
	long		chunkSize;		/* chunk size including header */	\
	long		time;			/* position in stream time */		\
	long		channel;		/* logical channel number */		\
	long		subChunkType;	        /* data sub-type */

	long		duration;		/*	Duration of this sample		*/
	long		frameSize;		/*	Number of bytes in frame	*/
	char		frameData[4];	/*	compressed frame data...	*/
} CinePakFrame, *CinePakFramePtr;


#endif