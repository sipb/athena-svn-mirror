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
io.c

This is the IO/ motion frame stuff.  It is derived from the JPEG io.c
package, so it is somewhat unsuited for fixed-frame ratio fixed
component number MPEG work.

************************************************************
*/

/*LABEL io.c */

#include <stdio.h>
#include <stdlib.h>
#include "globals.h"

static void Get4Ptr();
static void Get2Ptr();

/*PRIVATE*/
#define BlockWidth BLOCKWIDTH
#define BlockHeight BLOCKHEIGHT


/*START*/

/*BFUNC

MakeFS() constructs an IO structure and assorted book-keeping
instructions for all components of the frame.

EFUNC*/

void MakeFS(vid_stream,flag)
     mpeg1encoder_VidStream *vid_stream;
     int flag;
{
  BEGIN("MakeFS");
  int i;

  vid_stream->CFStore = MakeStructure(FSTORE);
  vid_stream->CFStore->NumberComponents=vid_stream->CFrame->NumberComponents;
  for(i=0;i<vid_stream->CFStore->NumberComponents;i++)
    {
      if (!(vid_stream->CFStore->Iob[i]=MakeStructure(IOBUF)))
	{
	  WHEREAMI();
	  printf("Cannot make IO structure\n");
	  exit(ERROR_MEMORY);
	}
      vid_stream->CFStore->Iob[i]->flag = flag;
      vid_stream->CFStore->Iob[i]->hpos = 0;
      vid_stream->CFStore->Iob[i]->vpos = 0;
      vid_stream->CFStore->Iob[i]->hor = vid_stream->CFrame->hf[i];
      vid_stream->CFStore->Iob[i]->ver = vid_stream->CFrame->vf[i];
      vid_stream->CFStore->Iob[i]->width = vid_stream->CFrame->Width[i];
      vid_stream->CFStore->Iob[i]->height = vid_stream->CFrame->Height[i];
      vid_stream->CFStore->Iob[i]->mem = MakeMem(vid_stream->CFrame->Width[i],
				    vid_stream->CFrame->Height[i]);
    }      
}

/*BFUNC

SuperSubCompensate(arrays,)
     ) subtracts off the compensation from three arrays; subtracts off the compensation from three arrays,
forward compensation from the first, backward from the second,
interpolated from the third. This is done with a corresponding portion
of the memory in the forward and backward IO buffers.

EFUNC*/

void SuperSubCompensate(vid_stream,fmcmatrix,bmcmatrix,imcmatrix,XIob,YIob)
     mpeg1encoder_VidStream *vid_stream;
     int *fmcmatrix;
     int *bmcmatrix;
     	int *imcmatrix;
     IOBUF *XIob;
     IOBUF *YIob;
{
  /*BEGIN("SuperSubCompensate");*/
  int i,/*a,b,*/val;
  int *mask,*nask;

  MakeMask(vid_stream->MX, vid_stream->MY, vid_stream->Mask, XIob);
  MakeMask(vid_stream->NX, vid_stream->NY, vid_stream->Nask, YIob);

  /* Old stuff pre-SantaClara */
/*
  a = (16*(FrameInterval - FrameDistance) + FrameInterval/2)/FrameInterval;
  b = 16 - a;
*/

  for(mask=vid_stream->Mask,nask=vid_stream->Nask,i=0;i<64;i++)
    {
      *fmcmatrix++ -= *mask;
      *bmcmatrix++ -= *nask;

      /* Old stuff pre-SantaClara */
      /* 
      val = a*(*mask++) + b *(*nask++);
      if (val > 0) {val = (val+8)/16;}
      else {val = (val-8)/16;}
      */
      /* Should always be positive */
      val = ((*mask++)+(*nask++)+1)>>1;
      *imcmatrix++ -= val;
    }
}

/*BFUNC

Sub2Compensate() does a subtraction of the prediction from the current
matrix with a corresponding portion of the memory in the forward and
backward IO buffers.

EFUNC*/

void Sub2Compensate(vid_stream,matrix,XIob,YIob)
     mpeg1encoder_VidStream *vid_stream;
     int *matrix;
     IOBUF *XIob;
     IOBUF *YIob;
{
  /*BEGIN("Sub2Compensate");*/
  int i,/*a,b,*/val;
  int *mask,*nask;

  MakeMask(vid_stream->MX, vid_stream->MY, vid_stream->Mask, XIob);
  MakeMask(vid_stream->NX, vid_stream->NY, vid_stream->Nask, YIob);

  /* Old stuff pre-SantaClara */
  /*
  a = (16*(FrameInterval - FrameDistance) + FrameInterval/2)/FrameInterval;
  b = 16 - a;
  */

  for(mask=vid_stream->Mask,nask=vid_stream->Nask,i=0;i<64;i++)
    {
      /* Old stuff pre-SantaClara */
      /* 
      val = a*(*mask++) + b *(*nask++);
      if (val > 0) {val = (val+8)/16;}
      else {val = (val-8)/16;}
      */
      val = ((*mask++)+(*nask++)+1)>>1;
      *matrix++ -= val;
    }
}


/*BFUNC

Add2Compensate() does an addition of the prediction from the current
matrix with a corresponding portion of the memory in the forward and
backward IO buffers.

EFUNC*/


void Add2Compensate(vid_stream,matrix,XIob,YIob)
     mpeg1encoder_VidStream *vid_stream;
     int *matrix;
     IOBUF *XIob;
     IOBUF *YIob;
{
  /*BEGIN("Add2Compensate");*/
  int i,/*a,b,*/val;
  int *mask,*nask;
  
  MakeMask(vid_stream->MX, vid_stream->MY, vid_stream->Mask, XIob);
  MakeMask(vid_stream->NX, vid_stream->NY, vid_stream->Nask, YIob);

  /* Old stuff pre-SantaClara */
  /*
  a = (16*(FrameInterval - FrameDistance) + FrameInterval/2)/FrameInterval;
  b = 16 - a;
  */
  
  for(mask=vid_stream->Mask,nask=vid_stream->Nask,i=0;i<64;i++)
    {
      /* Old stuff pre-SantaClara */
      /*
      val = a*(*mask++) + b *(*nask++);
      if (val > 0) {val = (val+8)/16;}
      else {val = (val-8)/16;}
      */
      val = ((*mask++)+(*nask++)+1)>>1;
      *matrix += val;
      if (*matrix>255) {*matrix=255;}
      else if (*matrix<0){*matrix=0;}
      matrix++;
    }
}

/*BFUNC

SubCompensate() does a subtraction of the prediction from the current
matrix with a corresponding portion of the memory in the target IO
buffer.

EFUNC*/

void SubCompensate(vid_stream,matrix,XIob)
     mpeg1encoder_VidStream *vid_stream;
     int *matrix;
     IOBUF *XIob;
{
  /*BEGIN("SubCompensate");*/
  int i;
  int *mask;

  MakeMask(vid_stream->MX, vid_stream->MY, vid_stream->Mask, XIob);

  for(mask=vid_stream->Mask,i=0;i<64;i++)
    *matrix++ -= *mask++;
}


/*BFUNC

AddCompensate() does an addition of the prediction from the current
matrix with a corresponding portion of the memory in the target IO
buffer.

EFUNC*/

void AddCompensate(vid_stream,matrix,XIob)
     mpeg1encoder_VidStream *vid_stream;
     int *matrix;
     IOBUF *XIob;
{
  /*BEGIN("AddCompensate");*/
  int i;
  int *mask;

  MakeMask(vid_stream->MX, vid_stream->MY, vid_stream->Mask, XIob);
  for(mask=vid_stream->Mask,i=0;i<64;i++)
    {
      *matrix += *mask++;
      if (*matrix>255) {*matrix=255;}
      else if (*matrix<0){*matrix=0;}
      matrix++;
    }
}

void MakeMask(x,y,mask,XIob)
     int x;
     int y;
     int *mask;
     IOBUF *XIob;
{
  /*BEGIN("MakeMask");*/
  int i,j,rx,ry,dx,dy;
  unsigned char *aptr,*bptr,*cptr,*dptr;

  rx = x>>1;  ry = y>>1;
  dx = x&1;  dy = y&1;
  aptr = (((XIob->vpos *  BlockHeight) + ry)*XIob->width)
    + (XIob->hpos * BlockWidth) + rx
      + XIob->mem->data;
  if (dx)
    {
      bptr = aptr + dx;
      if (dy)
	{
	  cptr = aptr + XIob->width;
	  dptr = cptr + dx;
	  Get4Ptr(XIob->width,mask,aptr,bptr,cptr,dptr);
	}
      else
	{
	  Get2Ptr(XIob->width,mask,aptr,bptr);
	}
    }
  else if (dy)
    {
      cptr = aptr + XIob->width;
      Get2Ptr(XIob->width,mask,aptr,cptr);
    }
  else
    {
      for(i=0;i<BlockHeight;i++)
	{
	  for(j=0;j<BlockWidth;j++)
	    *(mask++) = *aptr++;
	  aptr = aptr-BlockWidth+XIob->width;
	}
    }
}

static void Get4Ptr(width,matrix,aptr,bptr,cptr,dptr)
     int width;
     int *matrix;
     unsigned char *aptr;
     unsigned char *bptr;
     unsigned char *cptr;
     unsigned char *dptr;
{
  int i,j;

  for(i=0;i<BlockHeight;i++) /* should be unrolled */
    {
      for(j=0;j<BlockWidth;j++)
	{
	  *(matrix++) = ((((int)*aptr++) + ((int)*bptr++) +
			  ((int)*cptr++) + ((int)*dptr++) + 2) >> 2);
	}
      aptr = aptr-BlockWidth+width;
      bptr = bptr-BlockWidth+width;
      cptr = cptr-BlockWidth+width;
      dptr = dptr-BlockWidth+width;
    }
}


static void Get2Ptr(width,matrix,aptr,bptr)
     int width;
     int *matrix;
     unsigned char *aptr;
     unsigned char *bptr;
{
  int i,j;

  for(i=0;i<BlockHeight;i++)  /* should be unrolled */
    {
      for(j=0;j<BlockWidth;j++)
	{
	  *(matrix++) = ((((int) *aptr++) + ((int)*bptr++) + 1) >> 1);
	}
      aptr = aptr-BlockWidth+width;
      bptr = bptr-BlockWidth+width;
    }
}

/*BFUNC

CopyCFS2FS() copies all of the CFrame Iob's to a given frame store.

EFUNC*/

void CopyCFS2FS(vid_stream,fs)
     mpeg1encoder_VidStream *vid_stream;
     FSTORE *fs;
{
  /*BEGIN("CopyIob2FS");*/
  int i;

  for(i=0;i<vid_stream->CFStore->NumberComponents;i++)
    CopyMem(vid_stream->CFStore->Iob[i]->mem,fs->Iob[i]->mem);
}

/*BFUNC

ClearFS() clears the entire frame store passed into it.

EFUNC*/

void ClearFS(vid_stream)
     mpeg1encoder_VidStream *vid_stream;
{
  /*BEGIN("ClearFS");*/
  int i;

  for(i=0;i<vid_stream->CFStore->NumberComponents;i++)
    ClearMem(vid_stream->CFStore->Iob[i]->mem);
}

/*BFUNC

InitFS() initializes a frame store that is passed into it. It creates
the IO structures and the memory structures.

EFUNC*/

void InitFS(vid_stream)
     mpeg1encoder_VidStream *vid_stream;
{
  BEGIN("InitFS");
  int i;

  for(i=0;i<vid_stream->CFStore->NumberComponents;i++)
    {
      if (!(vid_stream->CFStore->Iob[i]=MakeStructure(IOBUF)))
	{
	  WHEREAMI();
	  printf("Cannot create IO structure.\n");
	  exit(ERROR_MEMORY);
	}
      vid_stream->CFStore->Iob[i]->flag = 0;
      vid_stream->CFStore->Iob[i]->hpos = 0;
      vid_stream->CFStore->Iob[i]->vpos = 0;
      vid_stream->CFStore->Iob[i]->hor = vid_stream->CFrame->hf[i];
      vid_stream->CFStore->Iob[i]->ver = vid_stream->CFrame->vf[i];
      vid_stream->CFStore->Iob[i]->width = vid_stream->CFrame->Width[i];
      vid_stream->CFStore->Iob[i]->height = vid_stream->CFrame->Height[i];
      vid_stream->CFStore->Iob[i]->mem = MakeMem(vid_stream->CFrame->Width[i],
				vid_stream->CFrame->Height[i]);
    }
}

/*BFUNC

ReadFS() loads the memory images from the filenames designated in the
CFrame structure.

EFUNC*/

void ReadFS(vid_stream)
     mpeg1encoder_VidStream *vid_stream;
{
  /*BEGIN("ReadFS");*/
  int i;
  
  /*
  for(i=0;i<vid_stream->CFrame->NumberComponents;i++)
    {
      printf("load frame %s\n", vid_stream->CFrame->ComponentFileName[i]);
      if (vid_stream->CImage->PartialFrame)
	vid_stream->CFStore->Iob[i]->mem = LoadPartialMem(vid_stream->CFrame->ComponentFileName[i],
					      vid_stream->CFrame->PWidth[i],
					      vid_stream->CFrame->PHeight[i],
					      vid_stream->CFrame->Width[i],
					      vid_stream->CFrame->Height[i],
					      vid_stream->CFStore->Iob[i]->mem);
      else
	vid_stream->CFStore->Iob[i]->mem = LoadMem(vid_stream->CFrame->ComponentFileName[i],
				       vid_stream->CFrame->Width[i],
				       vid_stream->CFrame->Height[i],
				       vid_stream->CFStore->Iob[i]->mem);
    }
    */

  printf("load frame %d\n", vid_stream->CurrentFrame - vid_stream->BaseFrame);
  i = vid_stream->CurrentFrame - vid_stream->BaseFrame;
  printf("load frame %s %p %p %p\n", vid_stream->CFrame->ComponentFileName[0], vid_stream->CFStore->Iob[0]->mem->data,
  				vid_stream->CFStore->Iob[1]->mem->data, vid_stream->CFStore->Iob[2]->mem->data);

  printf("load frame %d %d\n",vid_stream->CFrame->Width[0], vid_stream->CFrame->Height[0]);
  memcpy(vid_stream->CFStore->Iob[0]->mem->data, vid_stream->frame_buffer[i], vid_stream->CFrame->Width[0]*
				       vid_stream->CFrame->Height[0]);
  
  printf("load frame %d %d\n",vid_stream->CFrame->Width[1], vid_stream->CFrame->Height[1]);
  memcpy(vid_stream->CFStore->Iob[2]->mem->data, vid_stream->frame_buffer[i] + 
		  vid_stream->CFrame->Width[0]*vid_stream->CFrame->Height[0],
		  vid_stream->CFrame->Width[2]*vid_stream->CFrame->Height[2]);
  
  printf("load frame %d %d\n",vid_stream->CFrame->Width[2], vid_stream->CFrame->Height[2]);
  memcpy(vid_stream->CFStore->Iob[1]->mem->data, vid_stream->frame_buffer[i] +
		  vid_stream->CFrame->Width[0]*vid_stream->CFrame->Height[0]+
		  vid_stream->CFrame->Width[2]*vid_stream->CFrame->Height[2],
		  vid_stream->CFrame->Width[1]*vid_stream->CFrame->Height[1]);
		  
   
}

/*BFUNC

InstallIob() installs a particular CFrame Iob as the target Iob.

EFUNC*/

void InstallIob(vid_stream,index)
     mpeg1encoder_VidStream *vid_stream;
     int index;
{
  /*BEGIN("InstallIob");*/

  vid_stream->Iob = vid_stream->CFStore->Iob[index];
}

void InstallFSIob(vid_stream,fs,index)
     mpeg1encoder_VidStream *vid_stream;
     FSTORE *fs;
     int index;
{
  vid_stream->Iob = fs->Iob[index];
}


/*BFUNC

WriteFS() writes the frame store out.

EFUNC*/

void WriteFS(vid_stream)
     mpeg1encoder_VidStream *vid_stream;
{
  /*BEGIN("WriteIob");*/
  int i;

  for(i=0;i<vid_stream->CFrame->NumberComponents;i++)
    {
      if (vid_stream->CImage->PartialFrame)
	SavePartialMem(vid_stream->CFrame->ComponentFileName[i],
		       vid_stream->CFrame->PWidth[i],
		       vid_stream->CFrame->PHeight[i],
		       vid_stream->CFStore->Iob[i]->mem);
      else
	SaveMem(vid_stream->CFrame->ComponentFileName[i],vid_stream->CFStore->Iob[i]->mem);
    }  
}

/*BFUNC

MoveTo() moves the installed Iob to a given location designated by the
horizontal and vertical offsets.

EFUNC*/

void MoveTo(vid_stream,hp,vp,h,v)
     mpeg1encoder_VidStream *vid_stream;
     int hp;
     int vp;
     int h;
     int v;
{
  /*BEGIN("MoveTo");*/

  vid_stream->Iob->hpos = hp*vid_stream->Iob->hor + h;
  vid_stream->Iob->vpos = vp*vid_stream->Iob->ver + v;
}

/*BFUNC

Bpos() returns the designated MDU number inside of the frame of the
installed Iob given by the input gob, mdu, horizontal and vertical
offset. It returns 0 on error.

EFUNC*/

int Bpos(vid_stream,hp,vp,h,v)
     mpeg1encoder_VidStream *vid_stream;
     int hp;
     int vp;
     int h;
     int v;
{
  /*BEGIN("Bpos");*/

  return((vp*vid_stream->Iob->ver + v)*(vid_stream->Iob->width/BlockWidth) + 
	 (hp * vid_stream->Iob->hor + h));
}


/*BFUNC

ReadBlock() reads a block from the currently installed Iob into a
designated matrix.

EFUNC*/

void ReadBlock(vid_stream,store)
     mpeg1encoder_VidStream *vid_stream;
     int *store;
{
  /*BEGIN("ReadBlock");*/
  int i,j;
  unsigned char *loc;

  loc = vid_stream->Iob->vpos*vid_stream->Iob->width*BlockHeight
    + vid_stream->Iob->hpos*BlockWidth+vid_stream->Iob->mem->data;
  for(i=0;i<BlockHeight;i++)
    {
      for(j=0;j<BlockWidth;j++) {*(store++) = *(loc++);}
      loc += vid_stream->Iob->width - BlockWidth;
    }
  if ((++vid_stream->Iob->hpos % vid_stream->Iob->hor)==0)
    {
      if ((++vid_stream->Iob->vpos % vid_stream->Iob->ver) == 0)
	{
	  if (vid_stream->Iob->hpos < 
	      ((vid_stream->Iob->width - 1)/(BlockWidth*vid_stream->Iob->hor))*vid_stream->Iob->hor + 1)
	    {
	      vid_stream->Iob->vpos -= vid_stream->Iob->ver;
	    }
	  else {vid_stream->Iob->hpos = 0;}
	}
      else {vid_stream->Iob->hpos -= vid_stream->Iob->hor;}
    }
}

/*BFUNC

WriteBlock() writes a input matrix as a block into the currently
designated IOB structure.

EFUNC*/

void WriteBlock(vid_stream,store)
     mpeg1encoder_VidStream *vid_stream;
     int *store;
{
  int i,j;
  unsigned char *loc;

  loc = vid_stream->Iob->vpos*vid_stream->Iob->width*BlockHeight +
    vid_stream->Iob->hpos*BlockWidth+vid_stream->Iob->mem->data;
  for(i=0;i<BlockHeight;i++)
    {
      for(j=0;j<BlockWidth;j++)	{*(loc++) =  *(store++);}
      loc += vid_stream->Iob->width - BlockWidth;
    }
  if ((++vid_stream->Iob->hpos % vid_stream->Iob->hor)==0)
    {
      if ((++vid_stream->Iob->vpos % vid_stream->Iob->ver) == 0)
	{
	  if (vid_stream->Iob->hpos < 
	      ((vid_stream->Iob->width - 1)/(BlockWidth*vid_stream->Iob->hor))*vid_stream->Iob->hor + 1)
	    {
	      vid_stream->Iob->vpos -= vid_stream->Iob->ver;
	    }
	  else {vid_stream->Iob->hpos = 0;}
	}
      else {vid_stream->Iob->hpos -= vid_stream->Iob->hor;}
    }
}

/*BFUNC

PrintIob() prints out the current Iob structure to the standard output
device.

EFUNC*/

void PrintIob(vid_stream)
      mpeg1encoder_VidStream *vid_stream;
{
  printf("IOB: %p\n",vid_stream->Iob);
  if (vid_stream->Iob)
    {
      printf("hor: %d  ver: %d  width: %d  height: %d\n",
	     vid_stream->Iob->hor,vid_stream->Iob->ver,vid_stream->Iob->width,vid_stream->Iob->height);
      printf("flag: %d  Memory Structure: %p\n",vid_stream->Iob->flag,vid_stream->Iob->mem);
    }
}

/*END*/

