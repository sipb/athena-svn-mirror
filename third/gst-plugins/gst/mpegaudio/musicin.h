/**********************************************************************
Copyright (c) 1991 MPEG/audio software simulation group, All Rights Reserved
musicin.c
**********************************************************************/

#ifndef __MUSICIN_H__
#define __MUSICIN_H__

#include <gst/putbits/putbits.h>
#include "common.h"
#include "encoder.h"

typedef double SBS[2][3][SCALE_BLOCK][SBLIMIT];
typedef double JSBS[3][SCALE_BLOCK][SBLIMIT];
typedef double IN[2][HAN_SIZE];
typedef unsigned int SUB[2][3][SCALE_BLOCK][SBLIMIT];

struct mpegaudio_encoder 
{
  SBS  FAR        *sb_sample;
  JSBS FAR        *j_sample;
  IN   FAR        *win_que;
  SUB  FAR        *subband;

  frame_params fr_ps;
  layer info;
  char original_file_name[MAX_NAME_SIZE];
  char encoded_file_name[MAX_NAME_SIZE];
  short FAR **win_buf;
  short FAR buffer[2][1152];
  unsigned int bit_alloc[2][SBLIMIT], scfsi[2][SBLIMIT];
  unsigned int scalar[2][3][SBLIMIT], j_scale[3][SBLIMIT];
  double FAR ltmin[2][SBLIMIT], lgmin[2][SBLIMIT], max_sc[2][SBLIMIT];
  FLOAT snr32[32];
  short sam[2][1056];
  int whole_SpF, extra_slot;
  double avg_slots_per_frame, frac_SpF, slot_lag;
  int model, stereo, error_protection;
  unsigned int crc;
  unsigned long bitsPerSlot, samplesPerFrame, frameNum;
  unsigned long frameBits, sentBits;
  unsigned long num_samples;
  gst_putbits_t pb;
  int bitrate;
  int frequency;
};

struct mpegaudio_encoder *mpegaudio_init_encoder();
void mpegaudio_sync_parms (struct mpegaudio_encoder *enc);
int mpegaudio_encode_frame(struct mpegaudio_encoder *enc, unsigned char *inbuf, unsigned char *outbuf, unsigned long *outlen);
unsigned long mpegaudio_get_number_of_input_bytes(struct mpegaudio_encoder *enc);
#endif /* __MUSICIN_H__ */
