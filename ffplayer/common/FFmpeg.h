#pragma once

#if !defined(__STDC_LIMIT_MACROS)
	#define __STDC_LIMIT_MACROS
#endif

#if !defined(__STDC_CONSTANT_MACROS)
	#define __STDC_CONSTANT_MACROS
#endif

#if !defined(__STDC_FORMAT_MACROS)
	#define __STDC_FORMAT_MACROS
#endif

extern "C"
{
	#include <libavformat/avformat.h>
	#include <libavcodec/avcodec.h>
	#include <libavdevice/avdevice.h>
	#include <libavutil/channel_layout.h>
	#include <libavutil/frame.h>
	#include <libswscale/swscale.h>
	#include <libswresample/swresample.h>
	#include <libavutil/avassert.h>
	#include <libavutil/avstring.h>
	#include <libavutil/imgutils.h>
	#include <libavutil/timestamp.h>
	#include <libavutil/time.h>
	#include <libavutil/error.h>
	#include <libavutil/pixdesc.h>
	#include <libavutil/avutil.h>

	#include <libavfilter/avfiltergraph.h>
	#include <libavfilter/buffersink.h>
	#include <libavfilter/buffersrc.h>
}

typedef struct _FilterContext {
	AVFilterContext *buffersink_ctx;
	AVFilterContext *buffersrc_ctx;
	AVFilterGraph *filter_graph;
} FilterContext;

#define MAX_THREAD_COUNT	8
#define LOG_MAX		256