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
prototypes.h

This file contains the functional prototypes for type checking, if
required.

************************************************************
*/

/* mpeg.c */

mpeg1encoder_VidStream *mpeg1encoder_new_encoder();
int mpeg1encoder_new_picture(mpeg1encoder_VidStream *vid_stream, unsigned char *data, unsigned long size, int state);
void MpegDecodeSequence(mpeg1encoder_VidStream *vid_stream);
void MpegEncodeIPBDFrame(mpeg1encoder_VidStream *vid_stream);
void MpegDecodeIPBDFrame(mpeg1encoder_VidStream *vid_stream);
void PrintImage(mpeg1encoder_VidStream *vid_stream);
void PrintFrame(mpeg1encoder_VidStream *vid_stream);
void MakeImage(mpeg1encoder_VidStream *vid_stream);
void MakeFrame(mpeg1encoder_VidStream *vid_stream);
void MakeFGroup(mpeg1encoder_VidStream *vid_stream);
void LoadFGroup(mpeg1encoder_VidStream *vid_stream, int index);
void MakeFStore(mpeg1encoder_VidStream *vid_stream);
void MakeStat(mpeg1encoder_VidStream *vid_stream);
void SetCCITT(mpeg1encoder_VidStream *vid_stream);
void CreateFrameSizes(mpeg1encoder_VidStream *vid_stream);
void Help(mpeg1encoder_VidStream *vid_stream);
void MakeFileNames(mpeg1encoder_VidStream *vid_stream);
void VerifyFiles(mpeg1encoder_VidStream *vid_stream);
int Integer2TimeCode(mpeg1encoder_VidStream *vid_stream, int fnum);
int TimeCode2Integer(mpeg1encoder_VidStream *vid_stream, int tc);

/* codec.c */
void EncodeAC(mpeg1encoder_VidStream *vid_stream, int index, int *matrix);
void CBPEncodeAC(mpeg1encoder_VidStream *vid_stream, int index, int *matrix);
void DecodeAC(mpeg1encoder_VidStream *vid_stream, int index, int *matrix);
void CBPDecodeAC(mpeg1encoder_VidStream *vid_stream, int index, int *matrix);
int DecodeDC(mpeg1encoder_VidStream *vid_stream, DHUFF *LocalDHuff);
void EncodeDC(mpeg1encoder_VidStream *vid_stream, int coef, EHUFF *LocalEHuff);

/* huffman.c */
void inithuff(mpeg1encoder_VidStream *vid_stream);
int Encode(mpeg1encoder_VidStream *vid_stream, int val, EHUFF *huff);
int Decode(mpeg1encoder_VidStream *vid_stream, DHUFF *huff);
void PrintDhuff(DHUFF *huff);
void PrintEhuff(EHUFF *huff);
void PrintTable(int *table);

/* io.c */
void MakeFS(mpeg1encoder_VidStream *vid_stream, int flag);
void SuperSubCompensate(mpeg1encoder_VidStream *vid_stream,
	          int *fmcmatrix, int *bmcmatrix, int *imcmatrix,
		  IOBUF *XIob, IOBUF *YIob);
void Sub2Compensate(mpeg1encoder_VidStream *vid_stream,
	          int *matrix, IOBUF *XIob, IOBUF *YIob);
void Add2Compensate(mpeg1encoder_VidStream *vid_stream,
	          int *matrix, IOBUF *XIob, IOBUF *YIob);
void SubCompensate(mpeg1encoder_VidStream *vid_stream,
	          int *matrix, IOBUF *XIob);
void AddCompensate(mpeg1encoder_VidStream *vid_stream,
	          int *matrix, IOBUF *XIob);

void MakeMask(int x, int y, int *mask, IOBUF *XIob);
void CopyCFS2FS(mpeg1encoder_VidStream *vid_stream, FSTORE *fs);
void ClearFS(mpeg1encoder_VidStream *vid_stream);
void InitFS(mpeg1encoder_VidStream *vid_stream);
void ReadFS(mpeg1encoder_VidStream *vid_stream);
void InstallIob(mpeg1encoder_VidStream *vid_stream, int index);
void InstallFSIob(mpeg1encoder_VidStream *vid_stream, FSTORE *fs, int index);
void WriteFS(mpeg1encoder_VidStream *vid_stream);
void MoveTo(mpeg1encoder_VidStream *vid_stream,
	          int hp, int vp, int h, int v);
int Bpos(mpeg1encoder_VidStream *vid_stream,
	          int hp, int vp, int h, int v);
void ReadBlock(mpeg1encoder_VidStream *vid_stream, int *store);
void WriteBlock(mpeg1encoder_VidStream *vid_stream, int *store);
void PrintIob(mpeg1encoder_VidStream *vid_stream);

/* chendct.c */
void ChenDct(int *x, int *y);
void ChenIDct(int *x, int *y);

/* lexer.c */
void initparser();
void parser();

/* marker.c */
void WriteStuff(mpeg1encoder_VidStream *vid_stream);
void ByteAlign(mpeg1encoder_VidStream *vid_stream);
void WriteVEHeader(mpeg1encoder_VidStream *vid_stream);
void WriteVSHeader(mpeg1encoder_VidStream *vid_stream);
int ReadVSHeader(mpeg1encoder_VidStream *vid_stream);
void WriteGOPHeader(mpeg1encoder_VidStream *vid_stream);
void ReadGOPHeader(mpeg1encoder_VidStream *vid_stream);
void WritePictureHeader(mpeg1encoder_VidStream *vid_stream);
void ReadPictureHeader(mpeg1encoder_VidStream *vid_stream);
void WriteMBSHeader(mpeg1encoder_VidStream *vid_stream);
void ReadMBSHeader(mpeg1encoder_VidStream *vid_stream);
void ReadHeaderTrailer(mpeg1encoder_VidStream *vid_stream);
int ReadHeaderHeader(mpeg1encoder_VidStream *vid_stream);
int ClearToHeader(mpeg1encoder_VidStream *vid_stream);
void WriteMBHeader(mpeg1encoder_VidStream *vid_stream);
int ReadMBHeader(mpeg1encoder_VidStream *vid_stream);

/* me.c */
void initme(mpeg1encoder_VidStream *vid_stream);
void HPFastBME(mpeg1encoder_VidStream *vid_stream, int rx, int ry, MEM *rm,
	     int cx, int cy, MEM *cm, int ox, int oy);
void BruteMotionEstimation(mpeg1encoder_VidStream *vid_stream,
		     MEM *pmem, MEM *fmem);
void InterpolativeBME(mpeg1encoder_VidStream *vid_stream);

/* mem.c */
void CopyMem(MEM *m1,MEM *m2);
void ClearMem(MEM *m1);
void SetMem(int value, MEM *m1);
MEM *MakeMem(int width, int height);
void FreeMem(MEM *mem);
MEM *LoadMem(char *filename, int width, int height, MEM *omem);
MEM *LoadPartialMem(char *filename, int pwidth, int pheight, int width, int height, MEM *omem);
MEM *SaveMem(char *filename, MEM *mem);
MEM *SavePartialMem(char *filename, int pwidth, int pheight, MEM *mem);

/* stat.c */
void Statistics(mpeg1encoder_VidStream *vid_stream, FSTORE * RefFS, FSTORE * NewFS);

/* stream.c */
void readalign(mpeg1encoder_VidStream *vid_stream);
void mropen(mpeg1encoder_VidStream *vid_stream, char *filename);
void mrclose(mpeg1encoder_VidStream *vid_stream);
void mwopen(mpeg1encoder_VidStream *vid_stream, char *filename);
void mwclose(mpeg1encoder_VidStream *vid_stream);
void zeroflush(mpeg1encoder_VidStream *vid_stream);
void mputb(mpeg1encoder_VidStream *vid_stream, int b);
int mgetb(mpeg1encoder_VidStream *vid_stream);
void mputv(mpeg1encoder_VidStream *vid_stream, int n, int b);
int mgetv(mpeg1encoder_VidStream *vid_stream, int n);
long mwtell(mpeg1encoder_VidStream *vid_stream);
long mrtell(mpeg1encoder_VidStream *vid_stream);
void mwseek(mpeg1encoder_VidStream *vid_stream, long distance);
void mrseek(mpeg1encoder_VidStream *vid_stream, long distance);
int seof(mpeg1encoder_VidStream *vid_stream);

/* transform.c */
void ReferenceDct(int *matrix, int *newmatrix);
void ReferenceIDct(int *matrix, int *newmatrix);
void TransposeMatrix(int *matrix, int *newmatrix);
void MPEGIntraQuantize(int *matrix, int *qptr, int qfact);
void MPEGIntraIQuantize(int *matrix, int *qptr, int qfact);
void MPEGNonIntraQuantize(int *matrix, int *qptr, int qfact);
void MPEGNonIntraIQuantize(int *matrix, int *qptr, int qfact);
void BoundIntegerMatrix(int *matrix);
void BoundQuantizeMatrix(int *matrix);
void BoundIQuantizeMatrix(int *matrix);
void ZigzagMatrix(int *imatrix, int *omatrix);
void IZigzagMatrix(int *imatrix, int *omatrix);
void PrintMatrix(int *matrix);
void ClearMatrix(int *matrix);

