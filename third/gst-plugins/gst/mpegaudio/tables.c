#include <stdio.h>
#include "common.h"
#include "encoder.h"

/***********************************************************************
 *
 * Read one of the data files ("alloc_*") specifying the bit allocation 
 * quatization parameters for each subband in layer II encoding
 *
 **********************************************************************/

#include "table_alloc.h"

int mpegaudio_read_bit_alloc(table, alloc)        /* read in table, return # subbands */
int table;
al_table *alloc;
{
   int i, j, n;

   if( table>3 || table<0 ) table = 0;

   for(n=0;n<alloc_len[table];n++)
   {
      i = alloc_tab[table][n].i;
      j = alloc_tab[table][n].j;
      (*alloc)[i][j].steps = alloc_tab[table][n].steps;
      (*alloc)[i][j].bits  = alloc_tab[table][n].bits;
      (*alloc)[i][j].group = alloc_tab[table][n].group;
      (*alloc)[i][j].quant = alloc_tab[table][n].quant;
   }
   return alloc_sblim[table];
}
 
/************************************************************************
 *
 * read_ana_window()
 *
 * PURPOSE:  Reads encoder window file "enwindow" into array #ana_win#
 *
 ************************************************************************/
 
#include "table_enwindow.h"

void mpegaudio_read_ana_window(ana_win)
double FAR ana_win[HAN_SIZE];
{
    int i;

    for(i=0;i<512;i++) ana_win[i] = mpegaudio_enwindow_tab[i];
}

/******************************************************************************
routine to read in absthr table from a file.
******************************************************************************/

#include "table_absthr.h"

void mpegaudio_read_absthr(absthr, table)
FLOAT *absthr;
int table;
{
   int i;
   for(i=0; i<HBLKSIZE; i++) absthr[i] = mpegaudio_absthr_tab[table][i];
}

int mpegaudio_crit_band;
int FAR *mpegaudio_cbound;
int mpegaudio_sub_size;

#include "table_cb.h"

void mpegaudio_read_cbound(lay,freq)  /* this function reads in critical */
int lay, freq;              /* band boundaries                 */
{
 int i,n;

 n = (lay-1)*3 + freq;

 mpegaudio_crit_band = mpegaudio_cb_len[n];
 mpegaudio_cbound = (int FAR *) mpegaudio_mem_alloc(sizeof(int) * mpegaudio_crit_band, "cbound");
 for(i=0;i<mpegaudio_crit_band;i++) mpegaudio_cbound[i] = mpegaudio_cb_tab[n][i];

}        

#include "table_th.h"

void mpegaudio_read_freq_band(ltg,lay,freq)  /* this function reads in   */
int lay, freq;                     /* frequency bands and bark */
g_ptr FAR *ltg;                /* values                   */
{
 int i,n;

 n = (lay-1)*3 + freq;

 mpegaudio_sub_size = mpegaudio_th_len[n];
 *ltg = (g_ptr FAR ) mpegaudio_mem_alloc(sizeof(g_thres) * mpegaudio_sub_size, "ltg");
 (*ltg)[0].line = 0;          /* initialize global masking threshold */
 (*ltg)[0].bark = 0;
 (*ltg)[0].hear = 0;
 for(i=1;i<mpegaudio_sub_size;i++){
    (*ltg)[i].line = mpegaudio_th_tab[n][i-1].line;
    (*ltg)[i].bark = mpegaudio_th_tab[n][i-1].bark;
    (*ltg)[i].hear = mpegaudio_th_tab[n][i-1].hear;
 }
}
