/**********************************************************************
Copyright (c) 1991 MPEG/audio software simulation group, All Rights Reserved
musicin.c
**********************************************************************/
/**********************************************************************
 * MPEG/audio coding/decoding software, work in progress              *
 *   NOT for public distribution until verified and approved by the   *
 *   MPEG/audio committee.  For further information, please contact   *
 *   Davis Pan, 508-493-2241, e-mail: pan@3d.enet.dec.com             *
 *                                                                    *
 * VERSION 4.0                                                        *
 *   changes made since last update:                                  *
 *   date   programmers         comment                               *
 * 3/01/91  Douglas Wong,       start of version 1.1 records          *
 *          Davis Pan                                                 *
 * 3/06/91  Douglas Wong,       rename: setup.h to endef.h            *
 *                              removed extraneous variables          *
 * 3/21/91  J.Georges Fritsch   introduction of the bit-stream        *
 *                              package. This package allows you      *
 *                              to generate the bit-stream in a       *
 *                              binary or ascii format                *
 * 3/31/91  Bill Aspromonte     replaced the read of the SB matrix    *
 *                              by an "code generated" one            *
 * 5/10/91  W. Joseph Carter    Ported to Macintosh and Unix.         *
 *                              Incorporated Jean-Georges Fritsch's   *
 *                              "bitstream.c" package.                *
 *                              Modified to strictly adhere to        *
 *                              encoded bitstream specs, including    *
 *                              "Berlin changes".                     *
 *                              Modified user interface dialog & code *
 *                              to accept any input & output          *
 *                              filenames desired.  Also added        *
 *                              de-emphasis prompt and final bail-out *
 *                              opportunity before encoding.          *
 *                              Added AIFF PCM sound file reading     *
 *                              capability.                           *
 *                              Modified PCM sound file handling to   *
 *                              process all incoming samples and fill *
 *                              out last encoded frame with zeros     *
 *                              (silence) if needed.                  *
 *                              Located and fixed numerous software   *
 *                              bugs and table data errors.           *
 * 27jun91  dpwe (Aware Inc)    Used new frame_params struct.         *
 *                              Clear all automatic arrays.           *
 *                              Changed some variable names,          *
 *                              simplified some code.                 *
 *                              Track number of bits actually sent.   *
 *                              Fixed padding slot, stereo bitrate    *
 *                              Added joint-stereo : scales L+R.      *
 * 6/12/91  Earle Jennings      added fix for MS_DOS in obtain_param  *
 * 6/13/91  Earle Jennings      added stack length adjustment before  *
 *                              main for MS_DOS                       *
 * 7/10/91  Earle Jennings      conversion of all float to FLOAT      *
 *                              port to MsDos from MacIntosh completed*
 * 8/ 8/91  Jens Spille         Change for MS-C6.00                   *
 * 8/22/91  Jens Spille         new obtain_parameters()               *
 *10/ 1/91  S.I. Sudharsanan,   Ported to IBM AIX platform.           *
 *          Don H. Lee,                                               *
 *          Peter W. Farrett                                          *
 *10/ 3/91  Don H. Lee          implemented CRC-16 error protection   *
 *                              newly introduced functions are        *
 *                              I_CRC_calc, II_CRC_calc and encode_CRC*
 *                              Additions and revisions are marked    *
 *                              with "dhl" for clarity                *
 *11/11/91 Katherine Wang       Documentation of code.                *
 *                                (variables in documentation are     *
 *                                surround by the # symbol, and an '*'*
 *                                denotes layer I or II versions)     *
 * 2/11/92  W. Joseph Carter    Ported new code to Macintosh.  Most   *
 *                              important fixes involved changing     *
 *                              16-bit ints to long or unsigned in    *
 *                              bit alloc routines for quant of 65535 *
 *                              and passing proper function args.     *
 *                              Removed "Other Joint Stereo" option   *
 *                              and made bitrate be total channel     *
 *                              bitrate, irrespective of the mode.    *
 *                              Fixed many small bugs & reorganized.  *
 * 2/25/92  Masahiro Iwadare    made code cleaner and more consistent *
 * 8/07/92  Mike Coleman        make exit() codes return error status *
 *                              made slight changes for portability   *
 *19 aug 92 Soren H. Nielsen    Changed MS-DOS file name extensions.  *
 * 8/25/92  Shaun Astarabadi    Replaced rint() function with explicit*
 *                              rounding for portability with MSDOS.  *
 * 9/22/92  jddevine@aware.com  Fixed _scale_factor_calc() calls.     *
 *10/19/92  Masahiro Iwadare    added info->mode and info->mode_ext   *
 *                              updates for AIFF format files         *
 * 3/10/93  Kevin Peterson      In parse_args, only set non default   *
 *                              bit rate if specified in arg list.    *
 *                              Use return value from aiff_read_hdrs  *
 *                              to fseek to start of sound data       *
 * 7/26/93  Davis Pan           fixed bug in printing info->mode_ext  *
 *                              value for joint stereo condition      *
 * 8/27/93 Seymour Shlien,      Fixes in Unix and MSDOS ports,        *
 *         Daniel Lauzon, and                                         *
 *         Bill Truerniet                                             *
 **********************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef MS_DOS
#include <dos.h>
#endif
#include <stdlib.h>
#include "musicin.h"


#define TRUE    1
#define FALSE   0

#define MIN(A, B)       ((A) < (B) ? (A) : (B))
#define MAX(A, B)       ((A) > (B) ? (A) : (B))

/* Global variable definitions for "musicin.c" */

/*Bit_stream_struc   bs; */

/* Implementations */

/************************************************************************
 *
 * obtain_parameters
 *
 * PURPOSE:  Prompts for and reads user input for encoding parameters
 *
 * SEMANTICS:  The parameters read are:
 * - input and output filenames
 * - sampling frequency (if AIFF file, will read from the AIFF file header)
 * - layer number
 * - mode (stereo, joint stereo, dual channel or mono)
 * - psychoacoustic model (I or II)
 * - total bitrate, irrespective of the mode
 * - de-emphasis, error protection, copyright and original or copy flags
 *
 ************************************************************************/

/************************************************************************
 *
 * print_config
 *
 * PURPOSE:  Prints the encoding parameters used
 *
 ************************************************************************/

void
print_config (fr_ps, psy, num_samples, inPath, outPath)
     frame_params *fr_ps;
     int *psy;
     unsigned long *num_samples;
     char inPath[MAX_NAME_SIZE];
     char outPath[MAX_NAME_SIZE];
{
  layer *info = fr_ps->header;

  printf ("mpegaudio: Encoding configuration:\n");
  if (info->mode != MPG_MD_JOINT_STEREO)
    printf ("mpegaudio: Layer=%s   mode=%s   extn=%d   psy model=%d\n",
        mpegaudio_layer_names[info->lay - 1], mpegaudio_mode_names[info->mode],
        info->mode_ext, *psy);
  else
    printf
        ("mpegaudio: Layer=%s   mode=%s   extn=data dependant   psy model=%d\n",
        mpegaudio_layer_names[info->lay - 1], mpegaudio_mode_names[info->mode],
        *psy);
  printf ("mpegaudio: samp frq=%.1f kHz   total bitrate=%d kbps\n",
      mpegaudio_s_freq[info->sampling_frequency],
      mpegaudio_bitrate[info->lay - 1][info->bitrate_index]);
  printf ("mpegaudio: de-emph=%d   c/right=%d   orig=%d   errprot=%d\n",
      info->emphasis, info->copyright, info->original, info->error_protection);
}

/************************************************************************
 *
 * main
 *
 * PURPOSE:  MPEG I Encoder supporting layers 1 and 2, and
 * psychoacoustic models 1 (MUSICAM) and 2 (AT&T)
 *
 * SEMANTICS:  One overlapping frame of audio of up to 2 channels are
 * processed at a time in the following order:
 * (associated routines are in parentheses)
 *
 * 1.  Filter sliding window of data to get 32 subband
 * samples per channel.
 * (window_subband,filter_subband)
 *
 * 2.  If joint stereo mode, combine left and right channels
 * for subbands above #jsbound#.
 * (*_combine_LR)
 *
 * 3.  Calculate scalefactors for the frame, and if layer 2,
 * also calculate scalefactor select information.
 * (*_scale_factor_calc)
 *
 * 4.  Calculate psychoacoustic masking levels using selected
 * psychoacoustic model.
 * (*_Psycho_One, psycho_anal)
 *
 * 5.  Perform iterative bit allocation for subbands with low
 * mask_to_noise ratios using masking levels from step 4.
 * (*_main_bit_allocation)
 *
 * 6.  If error protection flag is active, add redundancy for
 * error protection.
 * (*_CRC_calc)
 *
 * 7.  Pack bit allocation, scalefactors, and scalefactor select
 * information (layer 2) onto bitstream.
 * (*_encode_bit_alloc,*_encode_scale,II_transmission_pattern)
 *
 * 8.  Quantize subbands and pack them into bitstream
 * (*_subband_quantization, *_sample_encoding)
 *
 ************************************************************************/

struct mpegaudio_encoder *
mpegaudio_init_encoder ()
{
  struct mpegaudio_encoder *new = malloc (sizeof (struct mpegaudio_encoder));
  layer *info;

  new->extra_slot = 0;
  new->frameNum = 0;
  new->sentBits = 0;

  /* Most large variables are declared dynamically to ensure
     compatibility with smaller machines */

  new->sb_sample = (SBS FAR *) mpegaudio_mem_alloc (sizeof (SBS), "sb_sample");
  new->j_sample = (JSBS FAR *) mpegaudio_mem_alloc (sizeof (JSBS), "j_sample");
  new->win_que = (IN FAR *) mpegaudio_mem_alloc (sizeof (IN), "Win_que");
  new->subband = (SUB FAR *) mpegaudio_mem_alloc (sizeof (SUB), "subband");
  new->win_buf =
      (short FAR **) mpegaudio_mem_alloc (sizeof (short *) * 2, "win_buf");

  /* clear buffers */
  memset ((char *) new->buffer, 0, sizeof (new->buffer));
  memset ((char *) new->bit_alloc, 0, sizeof (new->bit_alloc));
  memset ((char *) new->scalar, 0, sizeof (new->scalar));
  memset ((char *) new->j_scale, 0, sizeof (new->j_scale));
  memset ((char *) new->scfsi, 0, sizeof (new->scfsi));
  memset ((char *) new->ltmin, 0, sizeof (new->ltmin));
  memset ((char *) new->lgmin, 0, sizeof (new->lgmin));
  memset ((char *) new->max_sc, 0, sizeof (new->max_sc));
  memset ((char *) new->snr32, 0, sizeof (new->snr32));
  memset ((char *) new->sam, 0, sizeof (new->sam));

  new->fr_ps.header = &new->info;
  new->fr_ps.tab_num = -1;      /* no table loaded */
  new->fr_ps.alloc = NULL;
  new->info.version = MPEG_AUDIO_ID;

  info = new->fr_ps.header;

  info->lay = DFLT_LAY;
  switch (DFLT_MOD) {
    case 's':
      info->mode = MPG_MD_STEREO;
      info->mode_ext = 0;
      break;
    case 'd':
      info->mode = MPG_MD_DUAL_CHANNEL;
      info->mode_ext = 0;
      break;
    case 'j':
      info->mode = MPG_MD_JOINT_STEREO;
      break;
    case 'm':
      info->mode = MPG_MD_MONO;
      info->mode_ext = 0;
      break;
    default:
      fprintf (stderr, "Bad mode dflt %c\n", DFLT_MOD);
      abort ();
  }
  new->model = DFLT_PSY;
  if ((info->sampling_frequency =
          mpegaudio_SmpFrqIndex ((long) (1000 * DFLT_SFQ))) < 0) {
    fprintf (stderr, "bad sfrq default %.2f\n", DFLT_SFQ);
    abort ();
  }

  new->bitrate = DFLT_BRT;

  if ((info->bitrate_index = mpegaudio_BitrateIndex (info->lay, DFLT_BRT)) < 0) {
    fprintf (stderr, "bad default bitrate %u\n", DFLT_BRT);
    abort ();
  }
  switch (DFLT_EMP) {
    case 'n':
      info->emphasis = 0;
      break;
    case '5':
      info->emphasis = 1;
      break;
    case 'c':
      info->emphasis = 3;
      break;
    default:
      fprintf (stderr, "Bad emph dflt %c\n", DFLT_EMP);
      abort ();
  }
  info->copyright = 0;
  info->original = 0;
  info->error_protection = FALSE;
  new->num_samples = MAX_U_32_NUM;

  return new;
}


void
mpegaudio_sync_parms (struct mpegaudio_encoder *new)
{
  /*print_config(&new->fr_ps, &new->model, &new->num_samples, */
  /*             new->original_file_name, new->encoded_file_name); */

  mpegaudio_hdr_to_frps (&new->fr_ps);
  new->stereo = new->fr_ps.stereo;
  new->error_protection = new->info.error_protection;

  if ((new->info.bitrate_index =
          mpegaudio_BitrateIndex (new->info.lay, new->bitrate)) < 0) {
    fprintf (stderr, "bad bitrate %u\n", new->bitrate);
    return;
  }
  if ((new->info.sampling_frequency =
          mpegaudio_SmpFrqIndex (new->frequency)) < 0) {
    fprintf (stderr, "bad sfrq %d\n", new->frequency);
    abort ();
  }

  if (new->info.lay == 1) {
    new->bitsPerSlot = 32;
    new->samplesPerFrame = 384;
  } else {
    new->bitsPerSlot = 8;
    new->samplesPerFrame = 1152;
  }
  /* Figure average number of 'slots' per frame. */
  /* Bitrate means TOTAL for both channels, not per side. */
  new->avg_slots_per_frame = ((double) new->samplesPerFrame /
      mpegaudio_s_freq[new->info.sampling_frequency]) *
      ((double) mpegaudio_bitrate[new->info.lay - 1][new->info.bitrate_index] /
      (double) new->bitsPerSlot);
  new->whole_SpF = (int) new->avg_slots_per_frame;
  /*printf("mpegaudio: slots/frame = %d\n",new->whole_SpF); */
  new->frac_SpF = new->avg_slots_per_frame - (double) new->whole_SpF;
  new->slot_lag = -new->frac_SpF;
  /*printf("mpegaudio: frac SpF=%.3f, tot bitrate=%d kbps, s freq=%.1f kHz\n", */
  /*       new->frac_SpF, bitrate[new->info.lay-1][new->info.bitrate_index], */
  /*       s_freq[new->info.sampling_frequency]); */

  /*if (new->frac_SpF != 0) */
  /*printf("mpegaudio: Fractional number of slots, padding required\n"); */
  /*else */
  new->info.padding = 0;
}

int
mpegaudio_encode_frame (struct mpegaudio_encoder *enc, unsigned char *inbuf,
    unsigned char *outbuf, unsigned long *outlen)
{
  int i, j, k, adb, ret;

  ret =
      mpegaudio_get_audio (inbuf, enc->buffer, enc->num_samples, enc->stereo,
      enc->info.lay);

  gst_putbits_init (&enc->pb);
  gst_putbits_new_buffer (&enc->pb, outbuf, *outlen);

  enc->sentBits = 0;

  enc->win_buf[0] = &enc->buffer[0][0];
  enc->win_buf[1] = &enc->buffer[1][0];
  if (enc->frac_SpF != 0) {
    if (enc->slot_lag > (enc->frac_SpF - 1.0)) {
      enc->slot_lag -= enc->frac_SpF;
      enc->extra_slot = 0;
      enc->info.padding = 0;
      /*  printf("No padding for this frame\n"); */
    } else {
      enc->extra_slot = 1;
      enc->info.padding = 1;
      enc->slot_lag += (1 - enc->frac_SpF);
      /*  printf("Padding for this frame\n");    */
    }
  }
  adb = (enc->whole_SpF + enc->extra_slot) * enc->bitsPerSlot;

  switch (enc->info.lay) {

/***************************** Layer I **********************************/

    case 1:
      for (j = 0; j < SCALE_BLOCK; j++)
        for (k = 0; k < enc->stereo; k++) {
          mpegaudio_window_subband (&enc->win_buf[k], &(*enc->win_que)[k][0],
              k);
          mpegaudio_filter_subband (&(*enc->win_que)[k][0],
              &(*enc->sb_sample)[k][0][j][0]);
        }

      mpegaudio_I_scale_factor_calc (*enc->sb_sample, enc->scalar, enc->stereo);
      if (enc->fr_ps.actual_mode == MPG_MD_JOINT_STEREO) {
        mpegaudio_I_combine_LR (*enc->sb_sample, *enc->j_sample);
        mpegaudio_I_scale_factor_calc (enc->j_sample, &enc->j_scale, 1);
      }

      mpegaudio_put_scale (enc->scalar, &enc->fr_ps, enc->max_sc);

      if (enc->model == 1)
        mpegaudio_I_Psycho_One (enc->buffer, enc->max_sc, enc->ltmin,
            &enc->fr_ps);
      else {
        for (k = 0; k < enc->stereo; k++) {
          mpegaudio_psycho_anal (&enc->buffer[k][0], &enc->sam[k][0], k,
              enc->info.lay, enc->snr32,
              (FLOAT) mpegaudio_s_freq[enc->info.sampling_frequency] * 1000);
          for (i = 0; i < SBLIMIT; i++)
            enc->ltmin[k][i] = (double) enc->snr32[i];
        }
      }

      mpegaudio_I_main_bit_allocation (enc->ltmin, enc->bit_alloc, &adb,
          &enc->fr_ps);

      if (enc->error_protection)
        mpegaudio_I_CRC_calc (&enc->fr_ps, enc->bit_alloc, &enc->crc);

      mpegaudio_encode_info (&enc->fr_ps, &enc->pb);

      if (enc->error_protection)
        mpegaudio_encode_CRC (enc->crc, &enc->pb);

      mpegaudio_I_encode_bit_alloc (enc->bit_alloc, &enc->fr_ps, &enc->pb);
      mpegaudio_I_encode_scale (enc->scalar, enc->bit_alloc, &enc->fr_ps,
          &enc->pb);
      mpegaudio_I_subband_quantization (enc->scalar, *enc->sb_sample,
          enc->j_scale, *enc->j_sample, enc->bit_alloc, *enc->subband,
          &enc->fr_ps);
      mpegaudio_I_sample_encoding (*enc->subband, enc->bit_alloc, &enc->fr_ps,
          &enc->pb);
      for (i = 0; i < adb; i++)
        gst_putbits1 (&enc->pb, 0);
      break;

/***************************** Layer 2 **********************************/

    case 2:
      for (i = 0; i < 3; i++)
        for (j = 0; j < SCALE_BLOCK; j++)
          for (k = 0; k < enc->stereo; k++) {
            mpegaudio_window_subband (&enc->win_buf[k], &(*enc->win_que)[k][0],
                k);
            mpegaudio_filter_subband (&(*enc->win_que)[k][0],
                &(*enc->sb_sample)[k][i][j][0]);
          }

      mpegaudio_II_scale_factor_calc (*enc->sb_sample, enc->scalar, enc->stereo,
          enc->fr_ps.sblimit);
      mpegaudio_pick_scale (enc->scalar, &enc->fr_ps, enc->max_sc);
      if (enc->fr_ps.actual_mode == MPG_MD_JOINT_STEREO) {
        mpegaudio_II_combine_LR (*enc->sb_sample, *enc->j_sample,
            enc->fr_ps.sblimit);
        mpegaudio_II_scale_factor_calc (enc->j_sample, &enc->j_scale, 1,
            enc->fr_ps.sblimit);
      }

      /* this way we calculate more mono than we need */
      /* but it is cheap */
      if (enc->model == 1)
        mpegaudio_II_Psycho_One (enc->buffer, enc->max_sc, enc->ltmin,
            &enc->fr_ps);
      else {
        for (k = 0; k < enc->stereo; k++) {
          mpegaudio_psycho_anal (&enc->buffer[k][0], &enc->sam[k][0], k,
              enc->info.lay, enc->snr32,
              (FLOAT) mpegaudio_s_freq[enc->info.sampling_frequency] * 1000);
          for (i = 0; i < SBLIMIT; i++)
            enc->ltmin[k][i] = (double) enc->snr32[i];
        }
      }

      mpegaudio_II_transmission_pattern (enc->scalar, enc->scfsi, &enc->fr_ps);
      mpegaudio_II_main_bit_allocation (enc->ltmin, enc->scfsi, enc->bit_alloc,
          &adb, &enc->fr_ps);

      if (enc->error_protection)
        mpegaudio_II_CRC_calc (&enc->fr_ps, enc->bit_alloc, enc->scfsi,
            &enc->crc);

      mpegaudio_encode_info (&enc->fr_ps, &enc->pb);

      if (enc->error_protection)
        mpegaudio_encode_CRC (enc->crc, &enc->pb);

      mpegaudio_II_encode_bit_alloc (enc->bit_alloc, &enc->fr_ps, &enc->pb);
      mpegaudio_II_encode_scale (enc->bit_alloc, enc->scfsi, enc->scalar,
          &enc->fr_ps, &enc->pb);
      mpegaudio_II_subband_quantization (enc->scalar, *enc->sb_sample,
          enc->j_scale, *enc->j_sample, enc->bit_alloc, *enc->subband,
          &enc->fr_ps);
      mpegaudio_II_sample_encoding (*enc->subband, enc->bit_alloc, &enc->fr_ps,
          &enc->pb);
      for (i = 0; i < adb; i++)
        gst_putbits1 (&enc->pb, 0);
      break;

/***************************** Layer 3 **********************************/

    case 3:
      break;

  }

  enc->frameBits = gst_putbits_bitcount (&enc->pb) - enc->sentBits;
  if (enc->frameBits % enc->bitsPerSlot)        /* a program failure */
    fprintf (stderr, "Sent %ld bits = %ld slots plus %ld %d\n",
        enc->frameBits, enc->frameBits / enc->bitsPerSlot,
        enc->frameBits % enc->bitsPerSlot, gst_putbits_bitcount (&enc->pb));

  *outlen = enc->frameBits / 8;

  return ret;

  /* } */
}

void
mpegaudio_end (struct mpegaudio_encoder *enc)
{

  printf ("Avg slots/frame = %.3f; b/smp = %.2f; br = %.3f kbps\n",
      (FLOAT) enc->sentBits / (enc->frameNum * enc->bitsPerSlot),
      (FLOAT) enc->sentBits / (enc->frameNum * enc->samplesPerFrame),
      (FLOAT) enc->sentBits / (enc->frameNum * enc->samplesPerFrame) *
      mpegaudio_s_freq[enc->info.sampling_frequency]);

  /*if (fclose(musicin) != 0){ */
  /*   printf("Could not close \"%s\".\n", enc->original_file_name); */
  /*   exit(2); */
  /* } */

#ifdef  MACINTOSH
  mpegaudio_set_mac_file_attr (encoded_file_name, VOL_REF_NUM, CREATOR_ENCODE,
      FILETYPE_ENCODE);
#endif

  printf ("Encoding of \"%s\" with psychoacoustic model %d is finished\n",
      enc->original_file_name, enc->model);
  printf ("The MPEG encoded output file name is \"%s\"\n",
      enc->encoded_file_name);
  exit (0);
}

unsigned long
mpegaudio_get_number_of_input_bytes (struct mpegaudio_encoder *enc)
{
  if (enc->info.lay == 1) {
    if (enc->stereo == 2) {     /* layer 1, stereo */
      return 768 * sizeof (short);
    } else {                    /* layer 1, mono */
      return 384 * sizeof (short);
    }
  } else {
    if (enc->stereo == 2) {     /* layer 2 (or 3), stereo */
      return 2304 * sizeof (short);
    } else {                    /* layer 2 (or 3), mono */
      return 1152 * sizeof (short);
    }
  }
}
