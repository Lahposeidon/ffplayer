#pragma once
#include "FFmpeg.h"
#include "GlobalData.h"

class CAVDecode
{
public:
	CAVDecode(const char * filename, AVInputFormat *iformat = nullptr);
	virtual ~CAVDecode(void);

private:
	AVFormatContext *m_ifmt_ctx;
	int m_nVideo_Stream_Idx;
	int m_nAudio_Stream_Idx;
	int m_nVideo_Frame_Count;
	int m_nAudio_Frame_Count;
	
	int m_dst_width;
	int m_dst_height;
	int m_dst_ch_layout;
	int m_dst_sample_rate;

	FilterContext m_vfilter_ctx;
	FilterContext m_afilter_ctx;

	int OpenCodecContext(int *stream_idx, AVFormatContext *fmt_ctx, enum AVMediaType type);
	int init_video_filter();
	int init_audio_filter();
	CStringA m_strFileName;
	void AnalyzeParamSetting();
	void FreeResource();

public:
	int OpenInputFile(const char * filename, AVInputFormat *iformat);
	AVFormatContext* getFormatContext(){return m_ifmt_ctx;}
	//int DecodePacket(AVCodecContext* codec_ctx, AVPacket *pkt);
	int DecodePacket(AVCodecContext * codec_ctx, AVPacket * pkt, AVFrame * avFrame);
	int getVideoStreamIndex() { return m_nVideo_Stream_Idx; }
	int getAudioStreamIndex() { return m_nAudio_Stream_Idx; }
	AVCodecParameters* getCodecParam(int stream_idx);

	static CAVDecode* getSource(const char * filename, AVInputFormat *iformat = nullptr);
	void release();
	FilterContext* getFilterContext(int Index);
};