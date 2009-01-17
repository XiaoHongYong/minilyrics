/* in_flac - Winamp2 FLAC input plugin
 * Copyright (C) 2000,2001,2002,2003,2004,2005  Josh Coalson
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "../../base/mlstdafx.h"
#include "../../base/base.h"

#include "playback.h"
#include "MDFlac.h"

void AudioInfoFromStreamInfo(AUDIO_INFO &audioInfo, const FLAC__StreamMetadata_StreamInfo *streaminfo);

// libFLAC callbacks
static FLAC__SeekableStreamDecoderReadStatus ReadCallback(const FLAC__SeekableStreamDecoder *decoder, FLAC__byte buffer[], unsigned *bytes, void *client_data)
{
	IMediaInput		*pInput = ((CMDFlac *)client_data)->m_pInput;

	*bytes = pInput->Read(buffer, *bytes);
	return FLAC__SEEKABLE_STREAM_DECODER_READ_STATUS_OK;
}

static FLAC__SeekableStreamDecoderSeekStatus SeekCallback(const FLAC__SeekableStreamDecoder *decoder, FLAC__uint64 absolute_byte_offset, void *client_data)
{
	IMediaInput		*pInput = ((CMDFlac *)client_data)->m_pInput;

	if (pInput->Seek(absolute_byte_offset, SEEK_SET) == ERR_OK)
		return FLAC__SEEKABLE_STREAM_DECODER_SEEK_STATUS_OK;
	else
		return FLAC__SEEKABLE_STREAM_DECODER_SEEK_STATUS_ERROR;
}

static FLAC__SeekableStreamDecoderTellStatus TellCallback(const FLAC__SeekableStreamDecoder *decoder, FLAC__uint64 *absolute_byte_offset, void *client_data)
{
	IMediaInput		*pInput = ((CMDFlac *)client_data)->m_pInput;

	*absolute_byte_offset = pInput->GetPos();
	return FLAC__SEEKABLE_STREAM_DECODER_TELL_STATUS_OK;
}

static FLAC__SeekableStreamDecoderLengthStatus LengthCallback(const FLAC__SeekableStreamDecoder *decoder, FLAC__uint64 *stream_length, void *client_data)
{
	IMediaInput		*pInput = ((CMDFlac *)client_data)->m_pInput;
	uint32			nSize;

	if (pInput->GetSize(nSize) == ERR_OK)
	{
		*stream_length = nSize;
		return FLAC__SEEKABLE_STREAM_DECODER_LENGTH_STATUS_OK;
	}
	else
		return FLAC__SEEKABLE_STREAM_DECODER_LENGTH_STATUS_ERROR;
}

static FLAC__bool EofCallback(const FLAC__SeekableStreamDecoder *decoder, void *client_data)
{
	IMediaInput		*pInput = ((CMDFlac *)client_data)->m_pInput;

	return pInput->IsEOF();
}

static FLAC__StreamDecoderWriteStatus WriteCallback(const FLAC__SeekableStreamDecoder *decoder, const FLAC__Frame *frame, const FLAC__int32 *const buffer[], void *client_data)
{
	CMDFlac *pMDFlac = (CMDFlac*)client_data;
	int				nSize = frame->header.blocksize * pMDFlac->m_nBytesPerSample;
	IFBuffer		*pBuf = pMDFlac->m_pMemAllocator->AllocFBuffer(nSize);
	ASSERT(pBuf);
	if (!pBuf)
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	int16			*pBuffer = (int16*)pBuf->data();
	int				nSamples = frame->header.blocksize;
	int				i;

	if (pMDFlac->m_bKillThread)
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;

	if (frame->header.channels == 1)
	{
		for (i = 0; i < nSamples; i++) 
			pBuffer[i] = (int16)buffer[0][i];
	}
	else
	{
		// Interleave the channels
		unsigned c;
		for (i = 0; i < nSamples; i++)
			for (c = 0; c < frame->header.channels; c++)
				*(pBuffer++) = (int16)buffer[c][i];
	}
	pBuf->resize(nSize);

	if (!pMDFlac->OutputWrite(pBuf, frame->header.bits_per_sample,
		frame->header.channels, frame->header.sample_rate))
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;

	return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;
}

static void MetadataCallback(const FLAC__SeekableStreamDecoder *decoder, const FLAC__StreamMetadata *metadata, void *client_data)
{
	CMDFlac *pMDFlac = (CMDFlac*)client_data;
	AUDIO_INFO	&m_audioInfo = pMDFlac->m_audioInfo;

	if (metadata->type == FLAC__METADATA_TYPE_STREAMINFO)
	{
		FLAC__ASSERT(metadata->data.stream_info.total_samples < 0x100000000); /* this plugin can only handle < 4 gigasamples */

		m_audioInfo.nMediaFileSize = 0;
		pMDFlac->m_pInput->GetSize(m_audioInfo.nMediaFileSize);

		AudioInfoFromStreamInfo(m_audioInfo, &(metadata->data.stream_info));

		pMDFlac->m_nBytesPerSample = m_audioInfo.nChannels * m_audioInfo.nBps / 8;

		if (m_audioInfo.nBps != 8 && m_audioInfo.nBps != 16 && m_audioInfo.nBps != 24)
		{
			// FLAC_plugin__show_error("This plugin can only handle 8/16/24-bit samples.");
			pMDFlac->m_bKillThread = true;
		}
	}
// 	else if (metadata->type == FLAC__METADATA_TYPE_VORBIS_COMMENT) 
// 	{
// 		const FLAC__StreamMetadata_VorbisComment *comments = &metadata->data.vorbis_comment;
// 		tchar_t	s[256];
//     
// 		if (p->Format.Comment.Node) 
// 		{
// 			uint32_t No;
// 			for (No = 0; No < comments->num_comments; ++No) 
// 			{
// 				UTF8ToTcs(s, TSIZEOF(s), (const char *)comments->comments[No].entry);
// 				p->Format.Comment.Node->Set(p->Format.Comment.Node, p->Format.Comment.No, s, sizeof(s));
// 			}
// 		}    
// 	}
}

static void ErrorCallback(const FLAC__SeekableStreamDecoder *decoder, FLAC__StreamDecoderErrorStatus status, void *client_data)
{
	CMDFlac *pMDFlac = (CMDFlac*)client_data;

	if (status!=FLAC__STREAM_DECODER_ERROR_STATUS_LOST_SYNC)
		pMDFlac->m_bKillThread = true;
}

int FP_Init(CMDFlac *pMDFlac)
{
	FLAC__SeekableStreamDecoderState state;

	FLAC__seekable_stream_decoder_set_client_data(pMDFlac->m_pFlacDecoder, pMDFlac);
	FLAC__seekable_stream_decoder_set_read_callback(pMDFlac->m_pFlacDecoder, ReadCallback);
	FLAC__seekable_stream_decoder_set_seek_callback(pMDFlac->m_pFlacDecoder, SeekCallback);
	FLAC__seekable_stream_decoder_set_length_callback(pMDFlac->m_pFlacDecoder, LengthCallback);
	FLAC__seekable_stream_decoder_set_tell_callback(pMDFlac->m_pFlacDecoder, TellCallback);
	FLAC__seekable_stream_decoder_set_eof_callback(pMDFlac->m_pFlacDecoder, EofCallback);
	FLAC__seekable_stream_decoder_set_write_callback(pMDFlac->m_pFlacDecoder, WriteCallback);
	FLAC__seekable_stream_decoder_set_metadata_callback(pMDFlac->m_pFlacDecoder, MetadataCallback);
	FLAC__seekable_stream_decoder_set_error_callback(pMDFlac->m_pFlacDecoder, ErrorCallback);
	FLAC__seekable_stream_decoder_set_metadata_respond_all(pMDFlac->m_pFlacDecoder);

	state = FLAC__seekable_stream_decoder_init(pMDFlac->m_pFlacDecoder);
	if (state != FLAC__FILE_DECODER_OK)
	{
		FLAC__seekable_stream_decoder_finish(pMDFlac->m_pFlacDecoder);
		return ERR_PLAYER_INVALID_FILE;
	}

	if (!FLAC__seekable_stream_decoder_process_until_end_of_metadata(pMDFlac->m_pFlacDecoder))
	{
		FLAC__seekable_stream_decoder_finish(pMDFlac->m_pFlacDecoder);
		return ERR_PLAYER_INVALID_FILE;
	}

	return ERR_OK;
}


/*
 *  decode
 */

void FP_seek(CMDFlac *pMDFlac)
{
	AUDIO_INFO	&m_audioInfo = pMDFlac->m_audioInfo;
	FLAC__uint64 target_sample =
		(FLAC__uint64)m_audioInfo.nTotalSamples * pMDFlac->m_nSeekPos / m_audioInfo.nMediaLength;

	FLAC__seekable_stream_decoder_seek_absolute(pMDFlac->m_pFlacDecoder, target_sample);
}

// int FP_Decode(CMDFlac *pMDFlac, IFBuffer *pBuf)
// {
// 	AUDIO_INFO	&m_audioInfo = pMDFlac->m_audioInfo;
// 
// 	/* fill reservoir */
// 	while (pMDFlac->m_BufferFilled < SAMPLES_PER_WRITE)
// 	{
// 		if (FLAC__seekable_stream_decoder_get_state(pMDFlac->m_pFlacDecoder) == FLAC__FILE_DECODER_END_OF_FILE)
// 		{
// 			break;
// 		}
// 		else if (!FLAC__seekable_stream_decoder_process_single(pMDFlac->m_pFlacDecoder))
// 		{
// //			FLAC_plugin__show_error("Error while processing frame (%s).", FLAC__FileDecoderStateString[FLAC__file_decoder_get_state(decoder)]);
// 			break;
// 		}
// // 		if (!FLAC__seekable_stream_decoder_get_decode_position(decoder, &decode_position))
// // 			decode_position = 0;
// 	}
// 
// 	/* output samples */
// 	if (pMDFlac->m_BufferFilled > 0)
// 	{
// 		const unsigned n = min(wide_samples_in_reservoir_, SAMPLES_PER_WRITE);
// 		const unsigned channels = m_audioInfo.nChannels;
// 		unsigned i;
// 		int bytes;
// 
// 		// if (cfg.replaygain.enable && m_audioInfo.has_replaygain)
// // 		if (m_audioInfo.has_replaygain && cfg.replaygain.enable && cfg.resolution.replaygain.dither)
// // 		{
// // 			bytes = FLAC__replaygain_synthesis__apply_gain(
// // 				sample_buffer,
// // 				true, /* little_endian_data_out */
// // 				m_audioInfo.nBpsOutput == 8, /* unsigned_data_out */
// // 				reservoir__,
// // 				n,
// // 				channels,
// // 				m_audioInfo.nBps,
// // 				m_audioInfo.nBpsOutput,
// // 				m_audioInfo.replay_scale,
// // 				cfg.replaygain.hard_limit,
// // 				cfg.resolution.replaygain.dither,
// // 				&m_audioInfo.dither_context
// // 			);
// // 		}
// // 		else
// 		{
// 			bytes = FLAC__plugin_common__pack_pcm_signed_little_endian(
// 				sample_buffer,
// 				reservoir__,
// 				n,
// 				channels,
// 				m_audioInfo.nBps,
// 				m_audioInfo.nBpsOutput
// 			);
// 		}
// 
// 		wide_samples_in_reservoir_ -= n;
// 		for (i = 0; i < channels; i++)
// 			memmove(&reservoir_[i][0], &reservoir_[i][n], sizeof(reservoir_[0][0]) * wide_samples_in_reservoir_);
// 
// 		return bytes;
// 	}
// 	else
// 	{
// 		return 0;
// 	}
// }
