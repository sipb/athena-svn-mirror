/* Additions to texmfmp.h for pdfTeX */

#define pdfischarused(f, c) ((boolean)(pdfcharused[f][c/8] & (1<<(c%8))))
#define pdfsetcharused(f, c) pdfcharused[f][c/8] |= (1<<(c%8))

/* writepdf() always writes by fwrite() */
#define       writepdf(a, b) \
  (void) fwrite ((char *) &pdfbuf[a], sizeof (pdfbuf[a]), \
                 (int) ((b) - (a) + 1), pdffile)

/* web2c input/output routines */
#define vfbopenin(f) \
    open_input (&(f), kpse_vf_format, FOPEN_RBIN_MODE)
#define typeonebopenin(f) \
    open_input (&(f), kpse_type1_format, FOPEN_RBIN_MODE)
#define truetypebopenin(f) \
    open_input (&(f), kpse_truetype_format, FOPEN_RBIN_MODE)
#define texpsheaderbopenin(f) \
    open_input (&(f), kpse_tex_ps_header_format, FOPEN_RBIN_MODE)

void writezip(boolean, integer);
extern void writeimg(integer, integer);
extern void dopdffont(integer, integer);
extern void readconfig();
extern void adjustcfgdimens(scaled);
extern integer readimg();
extern void addimageref(integer);
extern void deleteimageref(integer);
extern void libpdffinish();
extern integer imagewidth(integer);
extern integer imageheight(integer);
extern integer imagexres(integer);
extern integer imageyres(integer);
extern boolean ispdfimage(integer);
extern integer epdforigx(integer);
extern integer epdforigy(integer);
extern integer cfgoutput();
extern integer cfgcompresslevel();
extern integer cfgdecimaldigits();
extern integer cfgpkresolution();
extern integer cfgimageresolution();
extern integer cfgincludeformresources();
extern integer cfgpagewidth();
extern integer cfgpageheight();
extern integer cfghorigin();
extern integer cfgvorigin();
extern integer newvfpacket(internalfontnumber);
extern eightbits packetbyte();
extern void pushpacketstate();
extern void poppacketstate();
extern void startpacket(internalfontnumber, integer);
extern integer fmlookup();
extern void storepacket(integer, integer, integer);
extern void printextraxobjects();
extern void printextrafonts();
extern void printotherresources();
extern void appendresourcesname(integer, integer);
extern void deleteresourcesnames();
