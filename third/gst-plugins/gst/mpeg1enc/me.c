/*************************************************************
Copyright (C) 1990, 1991, 1993 Andy C. Hung, all rights reserved.
PUBLIC DOMAIN LICENSE: Stanford University Portable Video Research
Group. If you use this software, you agree to the following: This
program package is purely experimental, and is licensed "as is".
Permission is granted to use, modify, and distribute this program
without charge for any purpose, provided this license/ disclaimer
notice appears in the copies.  No warranty or maintenance is given,
either expressed or implied.  In no event shall the author(s) be
liable to you or a third party for any special, incidental,
consequential, or other damages, arising out of the use or inability
to use the program for any purpose (or the loss of data), even if we
have been advised of such possibilities.  Any public reference or
advertisement of this source code should refer to it as the Portable
Video Research Group (PVRG) code, and not by any author(s) (or
Stanford University) name.
*************************************************************/
/*
************************************************************
me.c

This file does much of the motion estimation and compensation.

************************************************************
*/

/*LABEL me.c */

#include <stdlib.h>
#include "globals.h"

static int Do4Check(unsigned char *aptr, unsigned char *bptr, unsigned char *cptr,
		  unsigned char *dptr, unsigned char *eptr, int width, int lim);

static int Do2Check(unsigned char *aptr, unsigned char *bptr, unsigned char *cptr,
		  int width, int lim);

#define COMPARISON >=  /* This is to compare for short-circuit exit */

/*START*/

/*BFUNC

initme() initializes the motion estimation to the proper number of
estimated frames, by FrameInterval.

EFUNC*/

void initme(mpeg1encoder_VidStream *vid_stream)
{
  /* BEGIN("initme"); */
  int i;

  vid_stream->FMX = (int **) calloc(vid_stream->FrameInterval+1,sizeof(int *));
  vid_stream->BMX = (int **) calloc(vid_stream->FrameInterval+1,sizeof(int *));
  vid_stream->FMY = (int **) calloc(vid_stream->FrameInterval+1,sizeof(int *));
  vid_stream->BMY = (int **) calloc(vid_stream->FrameInterval+1,sizeof(int *));

  for(i=0;i<vid_stream->FrameInterval+1;i++)
    {
      vid_stream->FMX[i] = (int *) calloc(8192,sizeof(int));
      vid_stream->BMX[i] = (int *) calloc(8192,sizeof(int));
      vid_stream->FMY[i] = (int *) calloc(8192,sizeof(int));
      vid_stream->BMY[i] = (int *) calloc(8192,sizeof(int));
     }
}


static int Do4Check(aptr,bptr,cptr,dptr,eptr,width,lim)
     unsigned char *aptr;
     unsigned char *bptr;
     unsigned char *cptr;
     unsigned char *dptr;
     unsigned char *eptr;
     int width;
     int lim;
{
  /* BEGIN("Do4Check"); */
  int val,i,data;
  for(val=0,i=0;i<16;i++)
    {
      data=(*(aptr++) - ((*bptr++ + *cptr++ + *dptr++ + *eptr++ + 2) >> 2));
      if (data<0) {val-=data;} else {val+=data;}
      data=(*(aptr++) - ((*bptr++ + *cptr++ + *dptr++ + *eptr++ + 2) >> 2));
      if (data<0) {val-=data;} else {val+=data;}
      data=(*(aptr++) - ((*bptr++ + *cptr++ + *dptr++ + *eptr++ + 2) >> 2));
      if (data<0) {val-=data;} else {val+=data;}
      data=(*(aptr++) - ((*bptr++ + *cptr++ + *dptr++ + *eptr++ + 2) >> 2));
      if (data<0) {val-=data;} else {val+=data;}
      data=(*(aptr++) - ((*bptr++ + *cptr++ + *dptr++ + *eptr++ + 2) >> 2));
      if (data<0) {val-=data;} else {val+=data;}
      data=(*(aptr++) - ((*bptr++ + *cptr++ + *dptr++ + *eptr++ + 2) >> 2));
      if (data<0) {val-=data;} else {val+=data;}
      data=(*(aptr++) - ((*bptr++ + *cptr++ + *dptr++ + *eptr++ + 2) >> 2));
      if (data<0) {val-=data;} else {val+=data;}
      data=(*(aptr++) - ((*bptr++ + *cptr++ + *dptr++ + *eptr++ + 2) >> 2));
      if (data<0) {val-=data;} else {val+=data;}
      data=(*(aptr++) - ((*bptr++ + *cptr++ + *dptr++ + *eptr++ + 2) >> 2));
      if (data<0) {val-=data;} else {val+=data;}
      data=(*(aptr++) - ((*bptr++ + *cptr++ + *dptr++ + *eptr++ + 2) >> 2));
      if (data<0) {val-=data;} else {val+=data;}
      data=(*(aptr++) - ((*bptr++ + *cptr++ + *dptr++ + *eptr++ + 2) >> 2));
      if (data<0) {val-=data;} else {val+=data;}
      data=(*(aptr++) - ((*bptr++ + *cptr++ + *dptr++ + *eptr++ + 2) >> 2));
      if (data<0) {val-=data;} else {val+=data;}
      data=(*(aptr++) - ((*bptr++ + *cptr++ + *dptr++ + *eptr++ + 2) >> 2));
      if (data<0) {val-=data;} else {val+=data;}
      data=(*(aptr++) - ((*bptr++ + *cptr++ + *dptr++ + *eptr++ + 2) >> 2));
      if (data<0) {val-=data;} else {val+=data;}
      data=(*(aptr++) - ((*bptr++ + *cptr++ + *dptr++ + *eptr++ + 2) >> 2));
      if (data<0) {val-=data;} else {val+=data;}
      data=(*(aptr++) - ((*bptr++ + *cptr++ + *dptr++ + *eptr++ + 2) >> 2));
      if (data<0) {val-=data;} else {val+=data;}
      if (val COMPARISON lim) return(val+1);
      aptr += (width - 16);
      bptr += (width - 16);
      cptr += (width - 16);
      dptr += (width - 16);
      eptr += (width - 16);
    }
  return(val);
}

static int Do2Check(aptr,bptr,cptr,width,lim)
     unsigned char *aptr;
     unsigned char *bptr;
     unsigned char *cptr;
     int width;
     int lim;
{
  /* BEGIN("Do2Check"); */
  int val,i,data;

  for(val=0,i=0;i<16;i++)
    {
      data=(*(aptr++) - ((*bptr++ + *cptr++ + 1) >> 1));
      if (data<0) {val-=data;} else {val+=data;}
      data=(*(aptr++) - ((*bptr++ + *cptr++ + 1) >> 1));
      if (data<0) {val-=data;} else {val+=data;}
      data=(*(aptr++) - ((*bptr++ + *cptr++ + 1) >> 1));
      if (data<0) {val-=data;} else {val+=data;}
      data=(*(aptr++) - ((*bptr++ + *cptr++ + 1) >> 1));
      if (data<0) {val-=data;} else {val+=data;}
      data=(*(aptr++) - ((*bptr++ + *cptr++ + 1) >> 1));
      if (data<0) {val-=data;} else {val+=data;}
      data=(*(aptr++) - ((*bptr++ + *cptr++ + 1) >> 1));
      if (data<0) {val-=data;} else {val+=data;}
      data=(*(aptr++) - ((*bptr++ + *cptr++ + 1) >> 1));
      if (data<0) {val-=data;} else {val+=data;}
      data=(*(aptr++) - ((*bptr++ + *cptr++ + 1) >> 1));
      if (data<0) {val-=data;} else {val+=data;}
      data=(*(aptr++) - ((*bptr++ + *cptr++ + 1) >> 1));
      if (data<0) {val-=data;} else {val+=data;}
      data=(*(aptr++) - ((*bptr++ + *cptr++ + 1) >> 1));
      if (data<0) {val-=data;} else {val+=data;}
      data=(*(aptr++) - ((*bptr++ + *cptr++ + 1) >> 1));
      if (data<0) {val-=data;} else {val+=data;}
      data=(*(aptr++) - ((*bptr++ + *cptr++ + 1) >> 1));
      if (data<0) {val-=data;} else {val+=data;}
      data=(*(aptr++) - ((*bptr++ + *cptr++ + 1) >> 1));
      if (data<0) {val-=data;} else {val+=data;}
      data=(*(aptr++) - ((*bptr++ + *cptr++ + 1) >> 1));
      if (data<0) {val-=data;} else {val+=data;}
      data=(*(aptr++) - ((*bptr++ + *cptr++ + 1) >> 1));
      if (data<0) {val-=data;} else {val+=data;}
      data=(*(aptr++) - ((*bptr++ + *cptr++ + 1) >> 1));
      if (data<0) {val-=data;} else {val+=data;}
      if (val COMPARISON lim) return(val+1);
      aptr += (width - 16);
      bptr += (width - 16);
      cptr += (width - 16);
    }
  return(val);
}

/*BFUNC

HPFastBME() does a fast brute-force motion estimation with two indexes
into two memory structures. The motion estimation has a short-circuit
abort to speed up calculation.

EFUNC*/

void HPFastBME(vid_stream,rx,ry,rm,cx,cy,cm,ox,oy)
     mpeg1encoder_VidStream *vid_stream;
     int rx;
     int ry;
     MEM *rm;
     int cx;
     int cy;
     MEM *cm;
     int ox;
     int oy;
{
  BEGIN("HPFastBME");
  int dx,dy,lx,ly,px,py,incr,xdir,ydir;
  register int i,j,data,val;
  register unsigned char *bptr,*cptr;
  unsigned char *baseptr;

  if ((ox < vid_stream->MinX)||(ox > vid_stream->MaxX))
    {
      WHEREAMI();
      printf("X coord out of bounds [%d,%d]: %d\n",vid_stream->MinX,vid_stream->MaxX,ox);
    }
  if ((oy < vid_stream->MinY)||(oy > vid_stream->MaxY))
    {
      WHEREAMI();
      printf("Y coord out of bounds [%d,%d]: %d\n",vid_stream->MinY,vid_stream->MaxY,oy);
    }
  vid_stream->MX=px=ox; vid_stream->MY=py=oy;                       /* Start search point at offset */
  vid_stream->MV=0;
  bptr=rm->data + (rx+ox) + ((ry+oy) * rm->width);
  baseptr=cm->data + cx + (cy * cm->width);
  cptr=baseptr;
  for(i=0;i<16;i++)                    /* Calculate [ox,oy] compensation */
    {
      for(j=0;j<16;j++)
	{
	  data=(*(bptr++)-*(cptr++));
	  if (data<0) {vid_stream->MV-=data;} else {vid_stream->MV+=data;}
	}
      bptr += (rm->width - 16);
      cptr += (cm->width - 16);
    }
  xdir=ydir=0;
  for(incr=1;incr<vid_stream->CircleLimit;incr++)
    {
      if ((py > (vid_stream->MaxY))||(py < vid_stream->MinY))
	{
	  if (xdir) px += incr;
	  else px -= incr;
	}
      else
	{
	  for(dx=0;dx<incr;dx++)
	    {
	      if (xdir) {px++;} else {px--;}         /* Move search point */
	      if ((px > (vid_stream->MaxX)) || (px < (vid_stream->MinX)))
		continue;                            /* check logical bds */
	      lx = px+rx; ly = py+ry;
	      if (((lx >= 0) && (lx < rm->width-16)) &&  /* check phys. bds */
		  ((ly >= 0) && (ly < rm->height-16)))
		{
		  bptr = rm->data + lx + (ly * rm->width);
		  cptr = baseptr;
		  for(val=i=0;i<16;i++)
		    {
		      data=(*(bptr++)-*(cptr++));
		      if (data<0) {val-=data;} else	{val+=data;}
		      data=(*(bptr++)-*(cptr++));
		      if (data<0) {val-=data;} else	{val+=data;}
		      data=(*(bptr++)-*(cptr++));
		      if (data<0) {val-=data;} else	{val+=data;}
		      data=(*(bptr++)-*(cptr++));
		      if (data<0) {val-=data;} else	{val+=data;}
		      data=(*(bptr++)-*(cptr++));
		      if (data<0) {val-=data;} else	{val+=data;}
		      data=(*(bptr++)-*(cptr++));
		      if (data<0) {val-=data;} else	{val+=data;}
		      data=(*(bptr++)-*(cptr++));
		      if (data<0) {val-=data;} else	{val+=data;}
		      data=(*(bptr++)-*(cptr++));
		      if (data<0) {val-=data;} else	{val+=data;}
		      data=(*(bptr++)-*(cptr++));
		      if (data<0) {val-=data;} else	{val+=data;}
		      data=(*(bptr++)-*(cptr++));
		      if (data<0) {val-=data;} else	{val+=data;}
		      data=(*(bptr++)-*(cptr++));
		      if (data<0) {val-=data;} else	{val+=data;}
		      data=(*(bptr++)-*(cptr++));
		      if (data<0) {val-=data;} else	{val+=data;}
		      data=(*(bptr++)-*(cptr++));
		      if (data<0) {val-=data;} else	{val+=data;}
		      data=(*(bptr++)-*(cptr++));
		      if (data<0) {val-=data;} else	{val+=data;}
		      data=(*(bptr++)-*(cptr++));
		      if (data<0) {val-=data;} else	{val+=data;}
		      data=(*(bptr++)-*(cptr++));
		      if (data<0) {val-=data;} else	{val+=data;}
		      if (val COMPARISON vid_stream->MV) break;
		      bptr += (rm->width - 16);
		      cptr += (cm->width - 16);
		    }
		  if (val < vid_stream->MV)
		    {
		      vid_stream->MV = val; 
		      vid_stream->MX = px;
		      vid_stream->MY = py;
		    }
		}
	    }
	}
      xdir = 1-xdir;
      if ((px > (vid_stream->MaxX))||(px < vid_stream->MinX))
	{
	  if (ydir) py += incr;
	  else py -= incr;
	}
      else
	{
	  for(dy=0;dy<incr;dy++)
	    {
	      if (ydir) {py++;} else {py--;}  /* Move search point */
	      if ((py > (vid_stream->MaxY)) || (py < (vid_stream->MinY))) continue;
	      lx = px+rx; ly = py+ry;
	      if (((lx >= 0) && (lx <= rm->width-16)) &&
		  ((ly >= 0) && (ly <= rm->height-16)))
		{
		  bptr = rm->data + lx + (ly * rm->width);
		  cptr = baseptr;
		  for(val=i=0;i<16;i++)
		    {
		      data=(*(bptr++)-*(cptr++));
		      if (data<0) {val-=data;} else	{val+=data;}
		      data=(*(bptr++)-*(cptr++));
		      if (data<0) {val-=data;} else	{val+=data;}
		      data=(*(bptr++)-*(cptr++));
		      if (data<0) {val-=data;} else	{val+=data;}
		      data=(*(bptr++)-*(cptr++));
		      if (data<0) {val-=data;} else	{val+=data;}
		      data=(*(bptr++)-*(cptr++));
		      if (data<0) {val-=data;} else	{val+=data;}
		      data=(*(bptr++)-*(cptr++));
		      if (data<0) {val-=data;} else	{val+=data;}
		      data=(*(bptr++)-*(cptr++));
		      if (data<0) {val-=data;} else	{val+=data;}
		      data=(*(bptr++)-*(cptr++));
		      if (data<0) {val-=data;} else	{val+=data;}
		      data=(*(bptr++)-*(cptr++));
		      if (data<0) {val-=data;} else	{val+=data;}
		      data=(*(bptr++)-*(cptr++));
		      if (data<0) {val-=data;} else	{val+=data;}
		      data=(*(bptr++)-*(cptr++));
		      if (data<0) {val-=data;} else	{val+=data;}
		      data=(*(bptr++)-*(cptr++));
		      if (data<0) {val-=data;} else	{val+=data;}
		      data=(*(bptr++)-*(cptr++));
		      if (data<0) {val-=data;} else	{val+=data;}
		      data=(*(bptr++)-*(cptr++));
		      if (data<0) {val-=data;} else	{val+=data;}
		      data=(*(bptr++)-*(cptr++));
		      if (data<0) {val-=data;} else	{val+=data;}
		      data=(*(bptr++)-*(cptr++));
		      if (data<0) {val-=data;} else	{val+=data;}
		      if (val COMPARISON vid_stream->MV) break;
		      bptr += (rm->width - 16);
		      cptr += (cm->width - 16);
		    }
		  if (val < vid_stream->MV)
		    {
		      vid_stream->MV = val; 
		      vid_stream->MX = px;
		      vid_stream->MY = py;
		    }
		}
	    }
	}
      ydir = 1-ydir;
    }
  /* At this point, MX and MY contain the integer mc vectors. */
  /* Look at nearest neighboring pels; dx and dy hold the offset. */

  dx=dy=0;
  vid_stream->aptr = baseptr;
  bptr = rm->data + (vid_stream->MX + rx) + ((vid_stream->MY + ry) * rm->width);
  if (((2*vid_stream->MX-1)>=vid_stream->BMinX)&&((2*vid_stream->MY-1)>=vid_stream->BMinY)&&
      ((vid_stream->MX+rx)>0) && ((vid_stream->MY+ry)>0))
    {
      cptr = bptr-1;
      vid_stream->dptr = bptr-rm->width;
      vid_stream->eptr = vid_stream->dptr-1;
      val = Do4Check(vid_stream->aptr,bptr,cptr,vid_stream->dptr,vid_stream->eptr,rm->width,vid_stream->MV);
      if (val < vid_stream->MV)
	{
	  vid_stream->MV = val; 
	  dx = dy = -1;
	}
    }
  if (((2*vid_stream->MY-1)>=vid_stream->BMinY)&&((vid_stream->MY+ry)>0))
    {
      cptr = bptr-rm->width;
      val = Do2Check(vid_stream->aptr,bptr,cptr,rm->width,vid_stream->MV);
      if (val < vid_stream->MV)
	{
	  vid_stream->MV = val; 
	  dx = 0; dy = -1;
	}
    }
  if (((2*vid_stream->MX+1)<= vid_stream->BMaxX)&&((2*vid_stream->MY-1)>=vid_stream->BMinY)&&
      ((vid_stream->MX+rx+16)<rm->width) && ((vid_stream->MY+ry)>0))
    {
      cptr = bptr+1;
      vid_stream->dptr = bptr-rm->width;
      vid_stream->eptr = vid_stream->dptr+1;
      val = Do4Check(vid_stream->aptr,bptr,cptr,vid_stream->dptr,vid_stream->eptr,rm->width,vid_stream->MV);
      if (val < vid_stream->MV)
	{
	  vid_stream->MV = val; 
	  dx = 1; dy = -1;
	}
    }
  if (((2*vid_stream->MX-1)>=vid_stream->BMinX)&&((vid_stream->MX+rx) > 0))
    {
      cptr = bptr-1;
      val = Do2Check(vid_stream->aptr,bptr,cptr,rm->width,vid_stream->MV);
      if (val < vid_stream->MV)
	{
	  vid_stream->MV = val; 
	  dx = -1; dy = 0;
	}
    }
  if (((2*vid_stream->MX+1)<=vid_stream->BMaxX)&&((vid_stream->MX+rx+16)<rm->width))
    {
      cptr = bptr+1;
      val = Do2Check(vid_stream->aptr,bptr,cptr,rm->width,vid_stream->MV);
      if (val < vid_stream->MV)
	{
	  vid_stream->MV = val; 
	  dx = 1; dy = 0;
	}
    }
  if (((2*vid_stream->MX-1)>=vid_stream->BMinX)&& ((2*vid_stream->MY+1)<=vid_stream->BMaxY)&&
      ((vid_stream->MX+rx)>0) && ((vid_stream->MY+ry+16)<rm->height))
    {
      cptr = bptr-1;
      vid_stream->dptr = bptr+rm->width;
      vid_stream->eptr = vid_stream->dptr-1;
      val = Do4Check(vid_stream->aptr,bptr,cptr,vid_stream->dptr,vid_stream->eptr,rm->width,vid_stream->MV);
      if (val < vid_stream->MV)
	{
	  vid_stream->MV = val; 
	  dx = -1; dy = 1;
	}
    }
  if (((2*vid_stream->MY+1)<=vid_stream->BMaxY)&&((vid_stream->MY+ry+16)<rm->height))
    {
      cptr = bptr+rm->width;
      val = Do2Check(vid_stream->aptr,bptr,cptr,rm->width,vid_stream->MV);
      if (val < vid_stream->MV)
	{
	  vid_stream->MV = val; 
	  dx = 0; dy = 1;
	}
    }
  if (((2*vid_stream->MY+1)<=vid_stream->BMaxY)&&((2*vid_stream->MX+1)<=vid_stream->BMaxX)&&
      ((vid_stream->MX+rx+16)<rm->width) && ((vid_stream->MY+ry+16) < rm->height))
    {
      cptr = bptr+1;
      vid_stream->dptr = bptr+rm->width;
      vid_stream->eptr = vid_stream->dptr+1;
      val = Do4Check(vid_stream->aptr,bptr,cptr,vid_stream->dptr,vid_stream->eptr,rm->width,vid_stream->MV);
      if (val < vid_stream->MV)
	{
	  vid_stream->MV = val; 
	  dx = dy = 1;
	}
    }
  vid_stream->MX = vid_stream->MX*2 + dx;
  vid_stream->MY = vid_stream->MY*2 + dy;
}

/*BFUNC

BruteMotionEstimation() does a brute-force motion estimation on all
aligned 16x16 blocks in two memory structures. It is presented as a
compatibility-check routine.

EFUNC*/

void BruteMotionEstimation(vid_stream,pmem,fmem)
     mpeg1encoder_VidStream *vid_stream;
     MEM *pmem;
     MEM *fmem;
{
  /* BEGIN("BruteMotionEstimation"); */
  int x,y;

  vid_stream->CircleLimit=vid_stream->SearchLimit;
  for(vid_stream->MeN=0,y=0;y<fmem->height;y+=16)
    {
      for(x=0;x<fmem->width;x+=16)
	{
	  HPFastBME(vid_stream,x,y,pmem,x,y,fmem,0,0);
	  vid_stream->MeVAR[vid_stream->MeN] = vid_stream->VAR;
	  vid_stream->MeVAROR[vid_stream->MeN] = vid_stream->VAROR;
	  vid_stream->MeMWOR[vid_stream->MeN] = vid_stream->MWOR;
	  vid_stream->MeX[vid_stream->MeN] = vid_stream->MX;
	  vid_stream->MeY[vid_stream->MeN] = vid_stream->MY;
	  vid_stream->MeVal[vid_stream->MeN] = vid_stream->MV;
	  vid_stream->MeN++;
	}
    }
}

/*BFUNC

InterpolativeBME() does the interpolative block motion estimation for
an entire frame interval at once.  Although motion estimation can be
done sequentially with considerable success, the temporal and spatial
locality of doing it all at once is probably better.

EFUNC*/

void InterpolativeBME(vid_stream)
     mpeg1encoder_VidStream *vid_stream;
{
  /* BEGIN("InterpolativeBME"); */
  int i,dx,dy,rx,ry,x,y,n;

  /* Do first forward predictive frame */
  if (vid_stream->FrameInterval)
    {
      vid_stream->MaxX = vid_stream->MaxY = 7;
      vid_stream->MinX = vid_stream->MinY = -8;
      vid_stream->BMaxX = vid_stream->BMaxY = 15;
      vid_stream->BMinX = vid_stream->BMinY = -16;
      printf("Doing Forward: 1\n");
      for(n=y=0;y<vid_stream->FFS[0]->height;y+=16)
	{
	  /* printf("Y:%d",y); */
	  for(rx=ry=dx=dy=x=0;x<vid_stream->FFS[0]->width;x+=16)
	    {
	      vid_stream->CircleLimit = vid_stream->SearchLimit + MAX(rx,ry)+1;
	      HPFastBME(vid_stream,x,y,vid_stream->FFS[0],x,y,vid_stream->FFS[1],dx/2,dy/2);
	      vid_stream->FMX[1][n] = vid_stream->MX;
	      vid_stream->FMY[1][n] = vid_stream->MY;
	      /* printf("[%d:%d]",MX,MY);*/
	      if (vid_stream->MVPrediction)
		{
		  dx = vid_stream->MX; dy = vid_stream->MY;
		  if (dx < vid_stream->BMinX) dx = vid_stream->BMinX;
		  else if (dx > vid_stream->MaxX) dx = vid_stream->BMaxX;
		  if (dy < vid_stream->BMinY) dy = vid_stream->BMinY;
		  else if (dy > vid_stream->MaxY) dy = vid_stream->BMaxY;
		  rx = abs(dx); ry = abs(dy);
		}
	      n++;
	    }
	  /* printf("\n");*/
	}
      for(i=2;i<=vid_stream->FrameInterval;i++)
	{
	  vid_stream->MaxX = vid_stream->MaxY = (i<<3)-1;
	  vid_stream->MinX = vid_stream->MinY = -(i<<3);
	  vid_stream->BMaxX = vid_stream->BMaxY = (i<<4)-1;
	  vid_stream->BMinX = vid_stream->BMinY =  -(i<<4);
	  printf("Doing Forward: %d\n",i);
	  for(n=0,y=0;y<vid_stream->FFS[0]->height;y+=16)
	    {
	      /* printf("Y:%d",y);*/
	      for(rx=ry=dx=dy=x=0;x<vid_stream->FFS[0]->width;x+=16)
		{
		  vid_stream->CircleLimit = vid_stream->SearchLimit + MAX(rx,ry)+1;
		  HPFastBME(vid_stream,x,y,vid_stream->FFS[0],x,y,vid_stream->FFS[i],dx/2,dy/2);
		  vid_stream->FMX[i][n] = vid_stream->MX;
		  vid_stream->FMY[i][n] = vid_stream->MY;
		  /* printf("[%d:%d]",MX,MY);*/
		  if (vid_stream->MVPrediction)
		    {
		      dx = vid_stream->MX-vid_stream->FMX[i-1][n]+vid_stream->FMX[i-1][n+1];  /* Next pos */
		      dy = vid_stream->MY-vid_stream->FMY[i-1][n]+vid_stream->FMY[i-1][n+1];  /* next pos */
		      if (dx < vid_stream->BMinX) dx = vid_stream->BMinX;
		      else if (dx > vid_stream->MaxX) dx = vid_stream->BMaxX;
		      if (dy < vid_stream->BMinY) dy = vid_stream->BMinY;
		      else if (dy > vid_stream->MaxY) dy = vid_stream->BMaxY;
		      rx = abs(dx - vid_stream->FMX[i-1][n+1]);  /* Distance from 0pred */
		      ry = abs(dy - vid_stream->FMY[i-1][n+1]);
		    }
		  else if (vid_stream->MVTelescope)
		    {
		      dx = vid_stream->FMX[i-1][n+1];  /* Next pos */
		      dy = vid_stream->FMY[i-1][n+1];  /* next pos */
		    }
		  n++;
		}
	    }
	}
    }
  if (vid_stream->FrameInterval>1)
    {
      /* Do first backward predictive frame */
      vid_stream->MaxX = vid_stream->MaxY = 7;
      vid_stream->MinX = vid_stream->MinY = -8;
      vid_stream->BMaxX = vid_stream->BMaxY = 15;
      vid_stream->BMinX = vid_stream->BMinY = -16;
      printf("Doing Backward: %d\n",vid_stream->FrameInterval - 1);
      for(n=0,y=0;y<vid_stream->FFS[vid_stream->FrameInterval]->height;y+=16)
	{
	  for(rx=ry=dx=dy=x=0;x<vid_stream->FFS[vid_stream->FrameInterval]->width;x+=16)
	    {
	      vid_stream->CircleLimit = vid_stream->SearchLimit + MAX(rx,ry)+1;
	      HPFastBME(vid_stream,x,y,vid_stream->FFS[vid_stream->FrameInterval],
			x,y,vid_stream->FFS[vid_stream->FrameInterval-1],dx/2,dy/2);
	      vid_stream->BMX[vid_stream->FrameInterval-1][n] = vid_stream->MX;
	      vid_stream->BMY[vid_stream->FrameInterval-1][n] = vid_stream->MY;
	      if (vid_stream->MVPrediction)
		{
		  dx = vid_stream->MX; dy = vid_stream->MY;
		  if (dx < vid_stream->BMinX) dx = vid_stream->BMinX;
		  else if (dx > vid_stream->MaxX) dx = vid_stream->BMaxX;
		  if (dy < vid_stream->BMinY) dy = vid_stream->BMinY;
		  else if (dy > vid_stream->MaxY) dy = vid_stream->BMaxY;
		  rx = abs(dx); ry = abs(dy);
		}
	      n++;
	    }
	}
      for(i=vid_stream->FrameInterval-2;i>0;i--)
	{
	  vid_stream->MaxX = vid_stream->MaxY = ((vid_stream->FrameInterval-i)<<3)-1;
	  vid_stream->MinX = vid_stream->MinY = - ((vid_stream->FrameInterval-i)<<3);
	  vid_stream->BMaxX = vid_stream->BMaxY = ((vid_stream->FrameInterval-i)<<4)-1;
	  vid_stream->BMinX = vid_stream->BMinY = - ((vid_stream->FrameInterval-i)<<4);
	  printf("Doing Backward: %d\n",i);
	  for(n=0,y=0;y<vid_stream->FFS[vid_stream->FrameInterval]->height;y+=16)
	    {
	      for(rx=ry=dx=dy=x=0;x<vid_stream->FFS[vid_stream->FrameInterval]->width;x+=16)
		{
		  vid_stream->CircleLimit = vid_stream->SearchLimit + MAX(rx,ry)+1;
		  HPFastBME(vid_stream,x,y,vid_stream->FFS[vid_stream->FrameInterval],x,y,vid_stream->FFS[i],dx/2,dy/2);
		  vid_stream->BMX[i][n] = vid_stream->MX;
		  vid_stream->BMY[i][n] = vid_stream->MY;
		  if (vid_stream->MVPrediction)
		    {
		      dx = vid_stream->MX-vid_stream->BMX[i+1][n]+vid_stream->BMX[i+1][n+1];
		      dy = vid_stream->MY-vid_stream->BMY[i+1][n]+vid_stream->BMY[i+1][n+1];
		      if (dx < vid_stream->BMinX) dx = vid_stream->BMinX;
		      else if (dx > vid_stream->MaxX) dx = vid_stream->BMaxX;
		      if (dy < vid_stream->BMinY) dy = vid_stream->BMinY;
		      else if (dy > vid_stream->MaxY) dy = vid_stream->BMaxY;
		      rx = abs(dx - vid_stream->BMX[i+1][n+1]);
		      ry = abs(dy - vid_stream->BMY[i+1][n+1]);
		    }
		  else if (vid_stream->MVTelescope)
		    {
		      dx = vid_stream->BMX[i+1][n+1];
		      dy = vid_stream->BMY[i+1][n+1];
		    }
		  n++;
		}
	    }
	}
    }
}

/*END*/

