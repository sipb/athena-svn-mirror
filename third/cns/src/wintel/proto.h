int PASCAL WinMain(HANDLE, HANDLE, LPSTR, int);
BOOL InitApplication(HANDLE);
BOOL InitInstance(HANDLE, int);
long FAR PASCAL MainWndProc(HWND, UINT, WPARAM, LPARAM);
BOOL FAR PASCAL About(HWND, WORD, WORD, LONG);
BOOL FAR PASCAL OpenTelnetDlg(HWND, WORD, WORD, LONG);
int TelnetSend(kstream, char *, int, int);
BOOL FAR PASCAL ConfigSessionDlg(HWND, WORD, WORD, LONG);

int OpenTelnetConnection(void);
int NEAR DoDialog(char *szDialog, FARPROC lpfnDlgProc);
HCONNECTION FindConnectionFromPortNum(int ID);
HCONNECTION FindConnectionFromTelstate(int telstate);
HCONNECTION FindConnectionFromScreen(HSCREEN hScreen);
CONNECTION * GetNewConnection(void);
void start_negotiation(kstream ks);
/* somewhere... */
struct machinfo * FAR PASCAL Shostlook(char *hname);
