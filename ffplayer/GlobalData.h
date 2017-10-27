#pragma once
const int MAX_VOLUME_METER = 512;
const int MIN_VOLUME_METER = 1;
const int AVCODEC_MAX_AUDIO_FRAME_SIZE = 192000;


#include <deque>
using namespace std;

struct AVRenderPacket {
	double pts;
	double duration;

	int    got_picture;
	void  *data;
	int    size;
	int	   decode_size;

	enum AVMediaType codec_type;

#pragma pack(push, 1)
	union source {
		struct video {
			int width;
			int height;
			AVPixelFormat pix_fmt;
		} v;

		struct audio {
			int channels;
			int sample_rate;
			int nb_samples;
			AVSampleFormat sample_fmt;			
		} a;
	} o;
#pragma pack(pop)

	AVPacket pkt;
};

typedef deque<AVRenderPacket*> RenderPacketSet;