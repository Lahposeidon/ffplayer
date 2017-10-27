#include "StdAfx.h"
#include "AVDecode.h"

#pragma warning(disable: 4996)

CAVDecode::CAVDecode(const char * filename, AVInputFormat *iformat) :
	m_nVideo_Stream_Idx(-1), m_nAudio_Stream_Idx(-1),
	m_nVideo_Frame_Count(0), m_nAudio_Frame_Count(0), m_ifmt_ctx(nullptr)
{
	m_vfilter_ctx.filter_graph = nullptr;
	m_vfilter_ctx.buffersrc_ctx = nullptr;
	m_vfilter_ctx.buffersink_ctx = nullptr;

	m_afilter_ctx.filter_graph = nullptr;
	m_afilter_ctx.buffersrc_ctx = nullptr;
	m_afilter_ctx.buffersink_ctx = nullptr;

	if (OpenInputFile(filename, iformat) < 0)
	{
		FreeResource();
		throw std::runtime_error("av_open_input_file");
	}		
}

CAVDecode::~CAVDecode(void)
{
	FreeResource();
	DebugAndLogPrint(_T("Release Source"));
}

int CAVDecode::OpenInputFile( const char *filename, AVInputFormat *iformat)
{
	CStringA strFileName = filename;
	int ret = 0;
	m_ifmt_ctx = avformat_alloc_context();
	AVDictionary* dictOptions = NULL;

	OpenCodecContext(&m_nVideo_Stream_Idx, m_ifmt_ctx, AVMEDIA_TYPE_VIDEO);
	OpenCodecContext(&m_nAudio_Stream_Idx, m_ifmt_ctx, AVMEDIA_TYPE_AUDIO);

	/* dump input information to stderr */
	av_dump_format(m_ifmt_ctx, 0, this->m_strFileName.GetBuffer(), 0);

	if (m_nVideo_Stream_Idx < 0 || m_nAudio_Stream_Idx < 0) {
		DebugAndLogPrint(_T("Could not find audio or video stream in the input, aborting"));
		ret = -1;
	}
	else
	{
		ret = init_video_filter();
		ret = init_audio_filter();
	}

	av_dict_free(&dictOptions);

	return ret;
}

int CAVDecode::OpenCodecContext(int *stream_idx, AVFormatContext *fmt_ctx, enum AVMediaType type)
{
	int ret;
	AVStream *st;
	AVCodec *dec = NULL;
	ret = av_find_best_stream(fmt_ctx, type, -1, -1, &dec, 0);

	if (ret < 0)
		return -1;

	*stream_idx = ret;
	st = fmt_ctx->streams[*stream_idx];
	/* find decoder for the stream */

	AVCodecContext *dec_ctx = st->codec;

	if (!dec) {
		//fprintf(stderr, "Failed to find %s codec\n", av_get_media_type_string(type));
		return AVERROR(EINVAL);
	}
	/* Init the decoders, with or without reference counting */
	if ((ret = avcodec_open2(dec_ctx, dec, NULL)) < 0) {
		//fprintf(stderr, "Failed to open %s codec\n", av_get_media_type_string(type));
		return ret;
	}

	return 0;
}

int CAVDecode::init_video_filter()
{
	AVStream* stream = m_ifmt_ctx->streams[m_nVideo_Stream_Idx];
	AVFilterInOut *inputs, *outputs;
	char args[512];

	m_vfilter_ctx.filter_graph = avfilter_graph_alloc();
	if (m_vfilter_ctx.filter_graph == nullptr)
		return AVERROR(ENOMEM);

	if (avfilter_graph_parse2(m_vfilter_ctx.filter_graph, "null", &inputs, &outputs) < 0)
	{
		DebugAndLogPrint(_T("Failed to parse video filtergraph"));
		return -2;
	}

	//input 필터 생성-----------------------------------------------------------------------------------
	snprintf(args, sizeof(args), "time_base=%d/%d:video_size=%dx%d:pix_fmt=%d:pixel_aspect=%d/%d:sws_param=flags=%d",
		stream->time_base.num, stream->time_base.den,
		stream->codecpar->width, stream->codecpar->height,
		(AVPixelFormat)stream->codecpar->format,
		stream->codecpar->sample_aspect_ratio.num, stream->codecpar->sample_aspect_ratio.den,
		SWS_BILINEAR);

	//buffer source 필터 생성
	if (avfilter_graph_create_filter(&m_vfilter_ctx.buffersrc_ctx, avfilter_get_by_name("buffer"),
		"in", args, NULL, m_vfilter_ctx.filter_graph) < 0)
	{
		DebugAndLogPrint(_T("Failed to create video buffer source"));
		return -3;
	}

	//buffer source 필터를 필터그래프 input으로 연결
	if (avfilter_link(m_vfilter_ctx.buffersrc_ctx, 0, inputs->filter_ctx, 0) < 0)
	{
		DebugAndLogPrint(_T("Failed to link video buffer source"));
		return -4;
	}

	//Output 필터 생성-------------------------------------------------------------------------------------
	//Buffer Sink 필터 생성

	enum AVPixelFormat fmts[] = { AV_PIX_FMT_RGB24, AV_PIX_FMT_NONE };
	if (avfilter_graph_create_filter(&m_vfilter_ctx.buffersink_ctx, avfilter_get_by_name("buffersink"),
		"out", NULL, NULL, m_vfilter_ctx.filter_graph) < 0)
	{
		DebugAndLogPrint(_T("Failed to create video buffer sink"));
		return -3;
	}

	if(av_opt_set_int_list(m_vfilter_ctx.buffersink_ctx, "pix_fmts", fmts, AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN) < 0)
	{ 
		DebugAndLogPrint(_T("Failed to set output pixel format"));
		return -3;
	}
	
	//비디오 프레임 해상도 변경을 위한 리스케일 필터 생성
	AVFilterContext* rescale_filter;
	CString strResolution = _T("1280x720");
	int nPos = strResolution.Find(_T("x"));
	CString strWidth = strResolution.Mid(0, nPos);
	CString strHeight = strResolution.Mid(nPos+1);
	snprintf(args, sizeof(args), "%s:%s", ((CStringA)strWidth).GetBuffer(), ((CStringA)strHeight).GetBuffer());
	if (avfilter_graph_create_filter(&rescale_filter, avfilter_get_by_name("scale"),
		"scale", args, NULL, m_vfilter_ctx.filter_graph) < 0)
	{
		DebugAndLogPrint(_T("Failed to create video scale filter"));
		return -4;
	}

	//필터그래프의 output을 aformat 필터로 연결
	if(avfilter_link(outputs->filter_ctx, 0, rescale_filter, 0) < 0)
	{
		DebugAndLogPrint(_T("Failed to link video format filter"));
		return -4;
	}

	//aformat필터를 Buffer Sink와 연결
	if (avfilter_link(rescale_filter, 0, m_vfilter_ctx.buffersink_ctx, 0) < 0)
	{
		DebugAndLogPrint(_T("Failed to link video filter"));
		return -4;
	}

	// 필터 연결
	if (avfilter_graph_config(m_vfilter_ctx.filter_graph, NULL) < 0)
	{
		DebugAndLogPrint(_T("Failed to configure video filter context"));
		return -5;
	}

	avfilter_inout_free(&inputs);
	avfilter_inout_free(&outputs);

	return 0;
}

int CAVDecode::init_audio_filter()
{
	AVStream* stream = m_ifmt_ctx->streams[m_nAudio_Stream_Idx];
	AVFilterContext* resample_filter;
	AVFilterInOut *inputs, *outputs;
	char args[512];

	m_afilter_ctx.filter_graph = avfilter_graph_alloc();
	if (m_afilter_ctx.filter_graph == nullptr)
		return -1;

	if (avfilter_graph_parse2(m_afilter_ctx.filter_graph, "anull", &inputs, &outputs) < 0)
	{
		DebugAndLogPrint(_T("Failed to parse audio filtergraph"));
		return -2;
	}

	uint64_t channel_layout = stream->codecpar->channel_layout;
	if(stream->codecpar->channel_layout == 0)
		channel_layout = (uint64_t)av_get_default_channel_layout(stream->codecpar->channels);

	//input 필터 생성----------------------------------------------------------------------------------------
	snprintf(args, sizeof(args), "time_base=%d/%d:sample_rate=%d:sample_fmt=%s:channel_layout=%lli",
		stream->time_base.num, stream->time_base.den,
		stream->codecpar->sample_rate, av_get_sample_fmt_name((AVSampleFormat)stream->codecpar->format),
		channel_layout);

	//buffer source 필터 생성
	if (avfilter_graph_create_filter(&m_afilter_ctx.buffersrc_ctx, avfilter_get_by_name("abuffer"),
		"in", args, NULL, m_afilter_ctx.filter_graph) < 0)
	{
		DebugAndLogPrint(_T("Failed to create audio buffer source"));
		return -3;
	}

	//buffer source 필터를 필터그래프 input으로 연결
	if (avfilter_link(m_afilter_ctx.buffersrc_ctx, 0, inputs->filter_ctx, 0) < 0)
	{
		DebugAndLogPrint(_T("Failed to link audio buffer source"));
		return -4;
	}

	//Output 필터 생성---------------------------------------------------------------------------------------
	//Buffer Sink 필터 생성
	if (avfilter_graph_create_filter(&m_afilter_ctx.buffersink_ctx, avfilter_get_by_name("abuffersink"),
		"out", NULL, NULL, m_afilter_ctx.filter_graph) < 0)
	{
		DebugAndLogPrint(_T("Failed to create audio buffer sink"));
		return -3;
	}

	//오디오 프레임 포맷 변경을 위한 aformat 필터 생성
	CString strChannleLayout = _T("stereo");
	snprintf(args, sizeof(args), "sample_fmts=%s:sample_rates=%d:channel_layouts=%lli",
		av_get_sample_fmt_name(AV_SAMPLE_FMT_S16), stream->codecpar->sample_rate,
		av_get_channel_layout(((CStringA)strChannleLayout).GetBuffer()));
	if (avfilter_graph_create_filter(&resample_filter, avfilter_get_by_name("aformat"),
		"aformat", args, NULL, m_afilter_ctx.filter_graph) < 0)
	{
		DebugAndLogPrint(_T("Failed to create audio format filter"));
		return -4;
	}

	//필터그래프의 output을 aformat 필터로 연결
	if (avfilter_link(outputs->filter_ctx, 0, resample_filter, 0) < 0)
	{
		DebugAndLogPrint(_T("Failed to link audio format filter"));
		return -4;
	}

	//aformat필터를 Buffer Sink와 연결
	if (avfilter_link(resample_filter, 0, m_afilter_ctx.buffersink_ctx, 0) < 0)
	{
		DebugAndLogPrint(_T("Failed to link audio format filter"));
		return -4;
	}

	// 필터 연결
	if (avfilter_graph_config(m_afilter_ctx.filter_graph, NULL) < 0)
	{
		DebugAndLogPrint(_T("Failed to configure audio filter context"));
		return -5;
	}

	av_buffersink_set_frame_size(m_afilter_ctx.buffersink_ctx, stream->codecpar->frame_size);

	avfilter_inout_free(&inputs);
	avfilter_inout_free(&outputs);
	return 0;
}

void CAVDecode::AnalyzeParamSetting()
{
	int maxAnalyzeDuration = 0;
	int probeSize = 0;

	if (maxAnalyzeDuration > 0)
		m_ifmt_ctx->max_analyze_duration = maxAnalyzeDuration * (AV_TIME_BASE / 1000);
	if (probeSize > 0)
		m_ifmt_ctx->probesize = probeSize;

	DebugAndLogPrintA("[%s] SOURCE: OPEN - Max analyze duration [%d]", __FUNCTION__, maxAnalyzeDuration);
	DebugAndLogPrintA("[%s] SOURCE: OPEN - Probesize [%d]", __FUNCTION__, probeSize);
}

void CAVDecode::FreeResource()
{
	if (this->m_vfilter_ctx.filter_graph != NULL)
	{
		avfilter_graph_free(&(this->m_vfilter_ctx.filter_graph));
	}

	if (this->m_afilter_ctx.filter_graph != NULL)
	{
		avfilter_graph_free(&(this->m_afilter_ctx.filter_graph));
	}

	if (this->m_ifmt_ctx)
	{
		for (UINT i = 0; i < this->m_ifmt_ctx->nb_streams; i++)
		{
			AVCodecContext* codec_ctx = this->m_ifmt_ctx->streams[i]->codec;
			if (codec_ctx != nullptr)
			{
				avcodec_flush_buffers(codec_ctx);
				avcodec_close(codec_ctx);
			}
		}
		/* Close the output file. */
		if(this->m_ifmt_ctx->pb)
			avio_close(this->m_ifmt_ctx->pb);
		/* Close the input_context */
		if (m_strFileName.Find("DeckLink") >= 0)
			avformat_close_input(&(this->m_ifmt_ctx));
		else
			avformat_free_context(this->m_ifmt_ctx);			
	}
}

int CAVDecode::DecodePacket(AVCodecContext* codec_ctx, AVPacket *pkt, AVFrame* avFrame)
{
	int ret = 0;
	ret = avcodec_send_packet(codec_ctx, pkt);

	if (ret < 0) {
		char buf[256];
		DebugAndLogPrintA("Error decoding packet : %s(value - %d, index - %d)", buf, ret, pkt->stream_index);
		return ret;
	}

	avcodec_receive_frame(codec_ctx, avFrame);

	if (ret < 0) {
		char buf[256];
		DebugAndLogPrintA("Error decoding frame : %s(value - %d, index - %d)", buf, ret, pkt->stream_index);
		return ret;
	}

	avFrame->pts = av_frame_get_best_effort_timestamp(avFrame);

	if (codec_ctx->codec_type == AVMEDIA_TYPE_VIDEO) {
		/* decode video frame */
#ifdef _DEBUG
		char buf[AV_TS_MAX_STRING_SIZE];
		ZeroMemory(buf, AV_TS_MAX_STRING_SIZE);

		DebugAndLogPrintA("video_frame :%d coded_n:%d pts:%s",
			m_nVideo_Frame_Count++, avFrame->coded_picture_number,
			av_ts_make_time_string(buf, avFrame->pts, &codec_ctx->time_base));
#endif
	}
	else if (codec_ctx->codec_type == AVMEDIA_TYPE_AUDIO) {
#ifdef _DEBUG
		char buf[AV_TS_MAX_STRING_SIZE];
		ZeroMemory(buf, AV_TS_MAX_STRING_SIZE);

		DebugAndLogPrintA("audio_frame :%d nb_samples:%d pts:%s",
			m_nAudio_Frame_Count++, avFrame->nb_samples,
			av_ts_make_time_string(buf, avFrame->pts, &codec_ctx->time_base));
#endif
	}

	return avFrame->pkt_size;
}

AVCodecParameters* CAVDecode::getCodecParam(int stream_idx)
{
	AVStream* avStream = this->m_ifmt_ctx->streams[stream_idx];
	return avStream->codecpar;
}

CAVDecode* CAVDecode::getSource(const char * filename, AVInputFormat *iformat)
{
	CAVDecode *avSource = nullptr;

	try {
		avSource = new CAVDecode(filename, iformat);
	}
	catch (...) {
		DebugAndLogPrintA("source[%s] create fail.", filename);
		avSource = nullptr;
	}	

	return avSource;
}

void CAVDecode::release()
{
	delete this;
}

FilterContext* CAVDecode::getFilterContext(int Index)
{
	if (Index == m_nVideo_Stream_Idx)
		return &m_vfilter_ctx;
	else
		return &m_afilter_ctx;
}
