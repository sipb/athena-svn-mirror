/*
 * kconfig.c
 */
extern int main(void);
extern void mainEvent(void);
extern int HandleMouseDown(EventRecord *event);
extern int HandleMenu(long which, short modifiers);
extern int doupdate(WindowPtr window);
extern int doactivate(WindowPtr window, int mod);
extern void dogrow(WindowPtr window, Point p);
extern int handapple(int accitem);
extern void about(void);
extern pascal void pictdrawproc(short depth, short flags, GDHandle device, DialogPtr dialog);
extern void drawpict(DialogPtr dialog, int id);
extern pascal void dopict(DialogPtr dialog, short itemNo);
extern pascal void dooutline(DialogPtr dialog, short itemNo);
extern void updatedisplay(void);
extern void setText(DialogPtr dialog, int item, char *text);
extern void buildmain(void);
extern void setdcellstring(unsigned char *string, domaintype *dp);
extern void setscellstring(unsigned char *string, servertype *sp);
extern void setrcellstring(unsigned char *string, credentialstype *rp);
extern pascal void drawRealm(DialogPtr dialog, short item);
extern pascal void dolist(DialogPtr dialog, short itemNo);
extern void mainhit(EventRecord *event, DialogPtr dlg, int item);
extern void klist_dialog(void);
extern pascal Boolean klistFilter(DialogPtr dialog, EventRecord *event, short *itemHit);
extern Boolean editlist(int dlog, char *e1, char *e2, int *admin);
extern pascal Boolean okFilter(DialogPtr dialog, EventRecord *event, short *itemHit);
extern int popRealms(Rect *rect, char *retstring);
extern Boolean newdp(domaintype *dp, char *e1, char *e2);
extern Boolean newsp(servertype *sp, char *e1, char *e2, int admin);
extern void bzero(void *dst, long n);
extern void bcopy(void *src, void *dst, int n);
extern Ptr getmem(size_t size);
extern int getout(int exit);
extern void doalert(char *format, ...);
extern int strcasecmp(char *a, char *b);
extern int fatal(char *string);
extern char *copystring(char *src);
extern short isPressed(unsigned short k);
extern void doLogin(void);
extern void doLogout(void);
extern void getRealmMaps(void);
extern void getServerMaps(void);
extern void getCredentialsList(void);
extern void killCredentialsList(void);
extern void addRealmMap(char *host, char *realm);
extern void deleteRealmMap(char *host);
extern void deleteCredentials(credentialstype *rp);
extern void addServerMap(char *host, char *realm, int admin);
extern void deleteServerMap(char *host, char *realm);
extern void kerror(char *text, int error);
extern int lowcall(int cscode);
extern int hicall(int cscode);
extern void qlink(void **flist, void *fentry);
extern void *qunlink(void **flist, void *fentry);
extern void fixmenuwidth(MenuHandle themenu, int minwidth);
extern int doshadow(Rect *rect);
extern void dotriangle(Rect *rect);
extern void trimstring(char *cp);
extern void kpass_dialog(void);
extern pascal Boolean internalBufferFilter(DialogPtr dlog, EventRecord *event, short *itemHit);
extern void DeleteRange(unsigned char *buffer, short start, short end);
extern void InsertChar(unsigned char *buffer, short pos, char c);
extern void hidestring(unsigned char *cp);
extern void setctltxt(DialogPtr dialog, int ctl, unsigned char *text);
extern void readprefs(void);
extern void writeprefs(void);
extern int openprefres(int create);
extern Boolean trapAvailable(int theTrap);