/****************************************************************************

    PROGRAM: telnet.c

    PURPOSE: Windows networking kernel - Telnet

    FUNCTIONS:

    WinMain() - calls initialization function, processes message loop
    InitApplication() - initializes window data and registers window
    InitInstance() - saves instance handle and creates main window
    MainWndProc() - processes messages
    About() - processes messages for "About" dialog box

    COMMENTS:

        Windows can have several copies of your application running at the
        same time.  The variable hInst keeps track of which instance this
        application is so that processing will be to the correct window.

****************************************************************************/

#include <windows.h>            /* required for all Windows applications */
#include <string.h>
#include "telnet.h"            /* specific to this program          */
#include "auth.h"

HANDLE hInst;   /* current instance      */
HWND hWnd;      /* Main window handle.   */
CONFIG *tmpConfig;
GLOBALHANDLE hGlobalMem,hTitleMem;
CONNECTION *con=NULL;
char hostdata[MAXGETHOSTSTRUCT];
HGLOBAL ghCon;
SCREEN *fpScr;
int debug=1;
char strTmp[512];
BOOL bAutoConnection=FALSE; 
char szAutoHostName[64];
char szUserName[64];
char szHostName[64];

/****************************************************************************

    FUNCTION: WinMain(HANDLE, HANDLE, LPSTR, int)

    PURPOSE: calls initialization function, processes message loop

    COMMENTS:

        Windows recognizes this function by name as the initial entry point 
        for the program.  This function calls the application initialization 
        routine, if no other instance of the program is running, and always 
        calls the instance initialization routine.  It then executes a message 
        retrieval and dispatch loop that is the top-level control structure 
        for the remainder of execution.  The loop is terminated when a WM_QUIT 
        message is received, at which time this function exits the application 
        instance by returning the value passed by PostQuitMessage(). 

        If this function must abort before entering the message loop, it 
        returns the conventional value NULL.  

****************************************************************************/

int PASCAL WinMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow)
HANDLE hInstance;                /* current instance             */
HANDLE hPrevInstance;            /* previous instance            */
LPSTR lpCmdLine;                 /* command line                 */
int nCmdShow;                    /* show-window type (open/icon) */
{
    MSG msg;                     /* message                      */

    if (!hPrevInstance)
    	if (!InitApplication(hInstance)) /* Initialize shared things */
        	return (FALSE);      /* Exits if unable to initialize    */

    /* Perform initializations that apply to a specific instance */

	if (lpCmdLine[0]) {
		bAutoConnection=TRUE;
		lstrcpy((char *)szAutoHostName,lpCmdLine);
	}
	else bAutoConnection=FALSE;
	
    if (!InitInstance(hInstance, nCmdShow))
        return (FALSE);

    /* Acquire and dispatch messages until a WM_QUIT message is received. */

    while (GetMessage(&msg, NULL, NULL, NULL)) {
        TranslateMessage(&msg); /* Translates virtual key codes  */
        DispatchMessage(&msg);  /* Dispatches message to window  */
    }
    return (msg.wParam);    /* Returns the value from PostQuitMessage */
}


/****************************************************************************

    FUNCTION: InitApplication(HANDLE)

    PURPOSE: Initializes window data and registers window class

    COMMENTS:

        This function is called at initialization time only if no other 
        instances of the application are running.  This function performs 
        initialization tasks that can be done once for any number of running 
        instances.  

        In this case, we initialize a window class by filling out a data 
        structure of type WNDCLASS and calling the Windows RegisterClass() 
        function.  Since all instances of this application use the same window 
        class, we only need to do this when the first instance is initialized.  


****************************************************************************/

BOOL InitApplication(hInstance)
HANDLE hInstance;                  /* current instance       */
{
    WNDCLASS  wc;

	ScreenInit(hInstance);

    /* Fill in window class structure with parameters that describe the       */
    /* main window.                                                           */

    wc.style = CS_HREDRAW|CS_VREDRAW;                    /* Class style(s).                    */
    wc.lpfnWndProc = MainWndProc;       /* Function to retrieve messages for  */
                                        /* windows of this class.             */
    wc.cbClsExtra = 0;                  /* No per-class extra data.           */
    wc.cbWndExtra = 0;                  /* No per-window extra data.          */
    wc.hInstance = hInstance;           /* Application that owns the class.   */
    wc.hIcon = NULL; //LoadIcon(hInstance, "NCSA");
    wc.hCursor = NULL; //Cursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL; //GetStockObject(WHITE_BRUSH); 
    wc.lpszMenuName =  NULL;   /* Name of menu resource in .RC file. */
    wc.lpszClassName = "telnetWClass"; /* Name used in call to CreateWindow. */


    /* Register the window class and return success/failure code. */
    return (RegisterClass(&wc));
}


/****************************************************************************

    FUNCTION:  InitInstance(HANDLE, int)

    PURPOSE:  Saves instance handle and creates main window

    COMMENTS:

        This function is called at initialization time for every instance of 
        this application.  This function performs initialization tasks that 
        cannot be shared by multiple instances.  

        In this case, we save the instance handle in a static variable and 
        create and display the main program window.  
        
****************************************************************************/
BOOL InitInstance(hInstance, nCmdShow)
    HANDLE          hInstance;          /* Current instance identifier.       */
    int             nCmdShow;           /* Param for first ShowWindow() call. */
{
    int xScreen=0, yScreen=0;

    /* Save the instance handle in static variable, which will be used in  */
    /* many subsequence calls from this application to Windows.            */

    hInst = hInstance;

    /* Create a main window for this application instance.  */
    hWnd = CreateWindow(
        "telnetWClass",                 /* See RegisterClass() call.          */
        "TCPWin",                       /* Text for window title bar.         */
        WS_SYSMENU,                     /* Window style.                      */
        xScreen/3,                      /* Default horizontal position.       */
        yScreen/3,                      /* Default vertical position.         */
        xScreen/3,                      /* Default width.                     */
        yScreen/3,                      /* Default height.                    */
        NULL,                           /* Overlapped windows have no parent. */
        NULL,                           /* Use the window class menu.         */
        hInstance,                      /* This instance owns this window.    */
        NULL                            /* Pointer not needed.                */
    );
    if (!hWnd)
        return (FALSE);
        
//    ShowWindow(hWnd, SW_SHOW);  /* Show the window   */
//    UpdateWindow(hWnd);          /* Sends WM_PAINT message      */        

    {
        WSADATA wsaData;
        if (WSAStartup(0x0101,&wsaData)!=0) {   /* Initialize the network */
	        MessageBox(NULL,"Couldn't initialize Winsock!",NULL,MB_OK|MB_ICONEXCLAMATION);
	        return FALSE;
		}        	
    }

	if (!OpenTelnetConnection()) {
		WSACleanup();
		return(FALSE);
	}		

    return (TRUE);
}

/****************************************************************************

    FUNCTION: MainWndProc(HWND, UINT, WPARAM, LPARAM)

    PURPOSE:  Processes messages

    MESSAGES:

    WM_COMMAND    - application menu (About dialog box)
    WM_DESTROY    - destroy window

****************************************************************************/

long FAR PASCAL MainWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    HSCREEN hgScr;
    SCREEN *fpScr;
    HGLOBAL hBuffer;
    LPSTR lpBuffer; 
    char c;
    int iEvent,namelen,cnt,ret;
    char buf[1024];
    struct sockaddr_in name;
	struct sockaddr_in remote_addr;
   	struct hostent *remote_host;
	char *tmpCommaLoc;
    
    switch (message) {

	case WM_MYSCREENCHANGEBKSP:
	    if (!con) break;
	    con->backspace=wParam;
	    if (con->backspace==VK_BACK) {
	        con->ctrl_backspace=0x7f;
			WritePrivateProfileString(INI_TELNET, INI_BACKSPACE, INI_BACKSPACE_BS, TELNET_INI);
		}
	    else {
	        con->ctrl_backspace=VK_BACK;
			WritePrivateProfileString(INI_TELNET, INI_BACKSPACE, INI_BACKSPACE_DEL, TELNET_INI);
		}
       	GetPrivateProfileString(INI_HOSTS, INI_HOST "0", "", buf, 128, TELNET_INI);
       	tmpCommaLoc=strchr(buf,',');
       	if (tmpCommaLoc) {
			tmpCommaLoc++;
			if (con->backspace==VK_BACK)
				strcpy(tmpCommaLoc, INI_HOST_BS);
			else
				strcpy(tmpCommaLoc, INI_HOST_DEL);
		}
		WritePrivateProfileString(INI_HOSTS, INI_HOST "0", buf, TELNET_INI);
	    break;

	case WM_MYSCREENCHAR:
	    if (!con) break;
	    if (wParam==VK_BACK)
	        wParam=con->backspace;
	        else
	        if (wParam==0x7f)
	            wParam=con->ctrl_backspace;
	    TelnetSend(con->ks, (char *)&wParam,1,NULL);
	    break;
	
	case WM_MYSYSCHAR:
	    if (!con) break;
	    c= (char) wParam;
	    switch (c) {
	        case 'f':
	            getsockname(con->socket,(struct sockaddr *)&name,(int *)&namelen);
	            wsprintf(buf,"ftp %d.%d.%d.%d\n",
	                name.sin_addr.S_un.S_un_b.s_b1,
	                name.sin_addr.S_un.S_un_b.s_b2,
	                name.sin_addr.S_un.S_un_b.s_b3,
	                name.sin_addr.S_un.S_un_b.s_b4);
	            TelnetSend(con->ks,buf,lstrlen(buf),NULL);
	            break;                
	        case 'x':
	            hgScr=con->hScreen;
	            fpScr=(SCREEN *)GlobalLock(hgScr);
	            if (fpScr == NULL) break;
	            PostMessage(fpScr->hWnd,WM_CLOSE,NULL,NULL);
	            break;
	                            
	    }                
	    break;
	        
	case WM_MYSCREENBLOCK:
	    if (!con) break;
	    hBuffer= (HGLOBAL) wParam;
	    lpBuffer= GlobalLock(hBuffer);
	    TelnetSend(con->ks,lpBuffer,lstrlen(lpBuffer),NULL);
	    GlobalUnlock(hBuffer);
	    break;

	case WM_MYSCREENCLOSE:
	    if (!con) break;
	    kstream_destroy(con->ks);
		DestroyWindow(hWnd);
	    break;        

	case WM_QUERYOPEN:
	    return(0);
	    break;

	case WM_DESTROY:          /* message: window being destroyed */
	    kstream_destroy(con->ks);
	    GlobalUnlock(ghCon);
	    GlobalFree(ghCon); 
	    WSACleanup();
	    PostQuitMessage(0);
	    break;

	case WM_NETWORKEVENT: 
		iEvent=WSAGETSELECTEVENT(lParam);

		switch (iEvent) {    	

		case FD_READ:
	        cnt=recv(con->socket,buf,1024,NULL);
			/*
			The following line has been removed until kstream supports
			non-blocking IO or larger size reads (jrivlin@fusion.com).
			*/
			/* cnt=kstream_read(con->ks, buf, 1024); */
	        buf[cnt]=0;
	        parse((CONNECTION *)con,(unsigned char *)buf,cnt);
	        ScreenEm(buf,cnt,con->hScreen);
	        break;

	    case FD_CLOSE:
			kstream_destroy(con->ks);
			GlobalUnlock(ghCon);
			GlobalFree(ghCon); 
			WSACleanup();
			PostQuitMessage(0);
			break;

		case FD_CONNECT:
			ret = WSAGETSELECTERROR(lParam);
			if (ret) {
				wsprintf(buf,"Error %d on Connect",ret);
		        MessageBox(NULL,buf,NULL,MB_OK|MB_ICONEXCLAMATION);
				kstream_destroy(con->ks);
				GlobalUnlock(ghCon);
				GlobalFree(ghCon); 
				WSACleanup();
				PostQuitMessage(0);
				break;
		    }
			start_negotiation(con->ks);
			break;		
  	    }
  	    break;  	        

	case WM_HOSTNAMEFOUND:
		ret = WSAGETASYNCERROR(lParam);
		if (ret) {
			wsprintf(buf,"Error %d on GetHostbyName",ret);
        	MessageBox(NULL,buf,NULL,MB_OK|MB_ICONEXCLAMATION);
		    kstream_destroy(con->ks);
		    GlobalUnlock(ghCon);
		    GlobalFree(ghCon); 
		    WSACleanup();
		    PostQuitMessage(0);
        	break;
        }
        remote_host= (struct hostent *) hostdata;
		remote_addr.sin_family= AF_INET;    
		remote_addr.sin_addr.S_un.S_un_b.s_b1= remote_host->h_addr[0];
		remote_addr.sin_addr.S_un.S_un_b.s_b2= remote_host->h_addr[1];
		remote_addr.sin_addr.S_un.S_un_b.s_b3= remote_host->h_addr[2];
		remote_addr.sin_addr.S_un.S_un_b.s_b4= remote_host->h_addr[3];
		remote_addr.sin_port= htons(23);
		
		connect(con->socket,(struct sockaddr*) &remote_addr, 
		    	sizeof(struct sockaddr));    
		break;

	case WM_MYSCREENSIZE:
		con->width = LOWORD(lParam);			/* width in characters */
		con->height = HIWORD(lParam);		/* height in characters */
		if (con->bResizeable)
			send_naws(con);
		wsprintf(buf, "%d", con->height);
		WritePrivateProfileString(INI_TELNET, INI_HEIGHT, buf, TELNET_INI);
		wsprintf(buf, "%d", con->width);
		WritePrivateProfileString(INI_TELNET, INI_WIDTH, buf, TELNET_INI);
	    break;
		
	default:              /* Passes it on if unproccessed    */
	    return(DefWindowProc(hWnd, message, wParam, lParam));
    }
    return (NULL);
}


/****************************************************************************

    FUNCTION: SaveHostName(hostname)

    PURPOSE: Saves the currently selected host name in the KERBEROS.INI file
		and returns the preferred backspace setting if one exists for
		that host.

	RETURNS: VK_BACK or 0x7f depending on the desired backspace setting.

****************************************************************************/

int SaveHostName(char *host)
{
    char buf[128];
	char hostName[10][128];
	char *tmpCommaLoc;
	char tmpName[128];
	char tmpBuf[80];
	int nhosts;
	int n;
    int iHostNum;
	int bs;

	nhosts=0;
    for (iHostNum = 0; iHostNum < 10; iHostNum++) {
        wsprintf(tmpBuf,INI_HOST "%d",iHostNum);
        GetPrivateProfileString(INI_HOSTS,tmpBuf,"",hostName[iHostNum],128,TELNET_INI);
		strcpy(tmpName,hostName[iHostNum]);
        tmpCommaLoc=strchr(tmpName,',');  
        if (tmpCommaLoc) {
			*tmpCommaLoc='\0';
			while (tmpCommaLoc[1]==' ')
				tmpCommaLoc++;
		}
		if (!hostName[iHostNum][0])
			break;
		nhosts++;
        if (strcmp(tmpName,host) == 0)
			break;
    }

	for (iHostNum++; iHostNum < 10; iHostNum++) {
        wsprintf(tmpBuf,INI_HOST "%d",iHostNum);
        GetPrivateProfileString(INI_HOSTS,tmpBuf,"",hostName[iHostNum],128,TELNET_INI);
		if (!hostName[iHostNum][0])
			break;
		nhosts++;
	}

    if (tmpCommaLoc)
		tmpCommaLoc++;

    if (tmpName[0] && tmpCommaLoc) {
		if (_stricmp(tmpCommaLoc,INI_HOST_DEL) == 0)
			bs=0x7f;
		else if (_stricmp(tmpCommaLoc,INI_HOST_BS) == 0)
			bs=VK_BACK;
	}
    else {
   	    GetPrivateProfileString(INI_TELNET,INI_BACKSPACE,INI_BACKSPACE_BS,
			tmpBuf,sizeof(tmpBuf),TELNET_INI);
		if (_stricmp(tmpBuf,INI_BACKSPACE_DEL) == 0)
			bs=0x7f;
		else
			bs=VK_BACK;
	}

	strcpy(buf,tmpConfig->title);
	strcat(buf,",");
	if (bs==VK_BACK)
		strcat(buf,INI_BACKSPACE_BS);
	else
		strcat(buf,INI_BACKSPACE_DEL);

	WritePrivateProfileString(INI_HOSTS,INI_HOST "0",buf,TELNET_INI);
	n=1;
    for (iHostNum = 0; iHostNum < nhosts; iHostNum++) {
		strcpy(tmpName, hostName[iHostNum]);
        tmpCommaLoc=strchr(tmpName,',');  
        if (tmpCommaLoc) {
			*tmpCommaLoc='\0';
			while (tmpCommaLoc[1]==' ')
				tmpCommaLoc++;
		}
        if (strcmp(tmpName,host) != 0) {
	        wsprintf(tmpBuf,INI_HOST "%d",n++);
			WritePrivateProfileString(INI_HOSTS,tmpBuf,hostName[iHostNum],TELNET_INI);
		}
    }
	return (bs);
}


int OpenTelnetConnection(void) {
    int nReturn,ret;
    struct sockaddr_in         sockaddr;
	char *p;
	static struct kstream_crypt_ctl_block ctl;
    char buf[128];

    hGlobalMem=GlobalAlloc(GPTR,sizeof(CONFIG));
    tmpConfig=(CONFIG *) GlobalLock(hGlobalMem);
    
    if (bAutoConnection) {
		hTitleMem= GlobalAlloc(GPTR,lstrlen(szAutoHostName));
   		tmpConfig->title= (char *) GlobalLock(hTitleMem);            
		lstrcpy(tmpConfig->title,(char *)szAutoHostName);
    }
	else {
    	nReturn=DoDialog("OPENTELNETDLG",OpenTelnetDlg);
    	if (nReturn==FALSE) return(FALSE);
	}
                     	
    con=(CONNECTION *)GetNewConnection();
    if (con==NULL) return(0);

	tmpConfig->width=GetPrivateProfileInt(INI_TELNET,INI_WIDTH,DEF_WIDTH,TELNET_INI);
	tmpConfig->height=GetPrivateProfileInt(INI_TELNET,INI_HEIGHT,DEF_HEIGHT,TELNET_INI);
	con->width = tmpConfig->width;
	con->height = tmpConfig->height;

	con->backspace = SaveHostName(tmpConfig->title);

	if (con->backspace==VK_BACK) {
        tmpConfig->backspace=TRUE;
        con->ctrl_backspace=0x7f;
		}
	else {
        tmpConfig->backspace=FALSE;
        con->ctrl_backspace=0x08;
	}

    tmpConfig->hwndTel=hWnd;
    con->hScreen=(HSCREEN)InitNewScreen((CONFIG *)tmpConfig);
    if (!(con->hScreen)) {
    	OutputDebugString("Failed to initialize new screen! \r\n");
        GlobalUnlock(con->hScreen);
        GlobalUnlock(ghCon);
        GlobalFree(ghCon);
        GlobalUnlock(hGlobalMem); 
        GlobalFree(hGlobalMem);
        return(-1);
    }
    ret=(SOCKET)socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    
    if (ret==SOCKET_ERROR) {
        wsprintf(buf,"Socket error on socket = %d!",WSAGetLastError());
        MessageBox(NULL,buf,NULL,MB_OK|MB_ICONEXCLAMATION);
        fpScr= (SCREEN *) GlobalLock(con->hScreen);
        if (fpScr!=NULL)
        	DestroyWindow(fpScr->hWnd);
        GlobalUnlock(con->hScreen);
        GlobalUnlock(ghCon);
        GlobalFree(ghCon);
        GlobalUnlock(hGlobalMem);
        GlobalFree(hGlobalMem);
        return(-1);
    } 
    
    con->socket=ret;
    
    sockaddr.sin_family = AF_INET ;  
    sockaddr.sin_addr.s_addr = htonl( INADDR_ANY );
    sockaddr.sin_port = htons( 0 ) ;
    
    ret=bind(con->socket,(struct sockaddr *)&sockaddr,(int)sizeof(struct sockaddr_in));
    
    if (ret==SOCKET_ERROR) {
        wsprintf(buf,"Socket error on bind!");
        MessageBox(NULL,buf,NULL,MB_OK|MB_ICONEXCLAMATION);
        fpScr= (SCREEN *) GlobalLock(con->hScreen);
        if (fpScr!=NULL)
        	DestroyWindow(fpScr->hWnd);
        GlobalUnlock(con->hScreen);
        GlobalUnlock(ghCon);
        GlobalFree(ghCon);
        GlobalUnlock(hGlobalMem);
        GlobalFree(hGlobalMem);
        return(-1);
    }
    WSAAsyncSelect(con->socket,hWnd,WM_NETWORKEVENT,FD_READ|FD_CLOSE|FD_CONNECT);

  	lstrcpy(szHostName,tmpConfig->title);
	p = strchr(szHostName, '@');
	if (p != NULL) {
		*p = 0;
		strcpy (szUserName,szHostName);
		strcpy(szHostName,++p);
	}

   	WSAAsyncGetHostByName(hWnd,WM_HOSTNAMEFOUND,szHostName,hostdata,MAXGETHOSTSTRUCT); 
    GlobalUnlock(con->hScreen);
//    GlobalUnlock(ghCon);
//    GlobalFree(ghCon);
    GlobalUnlock(hGlobalMem);
    GlobalFree(hGlobalMem);

	ctl.encrypt = auth_encrypt;
	ctl.decrypt = auth_decrypt;
	ctl.init = auth_init;
	ctl.destroy = auth_destroy;

	con->ks = kstream_create_from_fd(con->socket,&ctl,NULL);

	kstream_set_buffer_mode(con->ks, 0);

	if (con->ks == NULL)
		return(-1);

    return(1);
}

CONNECTION *GetNewConnection(void) {
    CONNECTION *fpCon;

    ghCon = GlobalAlloc(GHND,sizeof(CONNECTION));
    if (ghCon==NULL) return NULL;
    fpCon = (CONNECTION *)GlobalLock(ghCon);
    if (fpCon==NULL) return NULL;
    fpCon->backspace=TRUE;
	fpCon->bResizeable=TRUE;
    return(fpCon);
}
   
/****************************************************************************/
int NEAR DoDialog(char *szDialog, FARPROC lpfnDlgProc) {
    int nReturn;
    
    lpfnDlgProc= MakeProcInstance(lpfnDlgProc,hInst);
    if (lpfnDlgProc==NULL)
        MessageBox(hWnd,"Couldn't make procedure instance",NULL,MB_OK);    
    
    nReturn= DialogBox(hInst,szDialog,hWnd,lpfnDlgProc);
    FreeProcInstance(lpfnDlgProc);
    return (nReturn);
}    

/****************************************************************************

    FUNCTION: OpenTelnetDlg(HWND, unsigned, WORD, LONG)

    PURPOSE:  Processes messages for "Open New Telnet Connection" dialog box

    MESSAGES:

    WM_INITDIALOG - initialize dialog box
    WM_COMMAND    - Input received

****************************************************************************/

BOOL FAR PASCAL OpenTelnetDlg(HWND hDlg, WORD message, WORD wParam, LONG lParam) {
    char szConnectName[256];
    HDC  hDC;
    int  xExt,yExt;
    DWORD Ext;
    HWND hEdit;
    int  nLen;
    int iHostNum=0;
    char tmpName[128],tmpBuf[80];
    char *tmpCommaLoc;
    
    switch (message) {
        case WM_INITDIALOG:        /* message: initialize dialog box */
            hDC=GetDC(hDlg);
            Ext=GetDialogBaseUnits();
            xExt=(190 *LOWORD(Ext)) /4 ;
            yExt=(72 * HIWORD(Ext)) /8 ;
            GetPrivateProfileString(INI_HOSTS,INI_HOST "0","",tmpName,128,TELNET_INI);
            if (tmpName[0]) {
            	tmpCommaLoc = strchr(tmpName,',');
				if (tmpCommaLoc)
					*tmpCommaLoc = '\0';
            	SetDlgItemText(hDlg,TEL_CONNECT_NAME,tmpName);
            }	
            hEdit=GetWindow(GetDlgItem(hDlg,TEL_CONNECT_NAME),GW_CHILD);
            while (TRUE) {
                wsprintf(tmpBuf,INI_HOST "%d",iHostNum++);
                GetPrivateProfileString(INI_HOSTS,tmpBuf,"",tmpName,128,TELNET_INI);
                tmpCommaLoc=strchr(tmpName,',');  
                if (tmpCommaLoc)
					*tmpCommaLoc='\0';
                if (tmpName[0])
                    SendDlgItemMessage(hDlg,TEL_CONNECT_NAME,CB_ADDSTRING,0,(LPARAM)((LPSTR)tmpName));
                else
					break;
            }
            SetWindowPos(hDlg,NULL,(GetSystemMetrics(SM_CXSCREEN)/2)-(xExt/2),
                (GetSystemMetrics(SM_CYSCREEN)/2)-(yExt/2),0,0,
                SWP_NOSIZE|SWP_NOZORDER|SWP_SHOWWINDOW);
            ReleaseDC(hDlg,hDC);      
            SendMessage(hEdit,WM_USER+1,NULL,NULL);
            SendMessage(hDlg,WM_SETFOCUS,NULL,NULL);
            return (TRUE);

        case WM_COMMAND:              /* message: received a command */
            switch (wParam) {
                case TEL_CANCEL:
                     EndDialog(hDlg, FALSE);        /* Exits the dialog box        */
                     break;
                 
                case TEL_OK:
                     GetDlgItemText(hDlg,TEL_CONNECT_NAME,szConnectName,256);
                     nLen= lstrlen(szConnectName);
                     if (nLen==0) { 
                        MessageBox(hDlg,"You must enter a session name!",NULL,MB_OK);
                        break;
                        }
                     hTitleMem= GlobalAlloc(GPTR,nLen);
                     tmpConfig->title= (char *) GlobalLock(hTitleMem);
                     lstrcpy(tmpConfig->title,szConnectName);
                     EndDialog(hDlg,TRUE);
                     break;
            }                
            return (FALSE);                    
    }
    return (FALSE);               /* Didn't process a message    */
}

/****************************************************************************

    FUNCTION: TelnetSend(kstream ks, char *buf, int len, int flags)

    PURPOSE:	This is a replacement for the WinSock send() function, to
		send a buffer of characters to an output socket.  It differs
		by retrying endlessly if sending the bytes would cause
		the send() to block.  <gnu@cygnus.com> observed EWOULDBLOCK
		errors when running using TCP Software's PC/TCP 3.0 stack,
		even when writing as little as 109 bytes into a socket
		that had no more than 9 bytes queued for output.  Note also that
		a kstream is used during output rather than a socket to facilitate
		encryption.

		Eventually, for cleanliness and responsiveness, this
		routine should not loop; instead, if the send doesn't
		send all the bytes, it should put them into a buffer
		and return.  Message handling code would send out the
		buffer whenever it gets an FD_WRITE message.

****************************************************************************/

int TelnetSend(kstream ks, char *buf, int len, int flags) {

	int writelen;
	int origlen = len;

	while (TRUE) {
		writelen = kstream_write(ks, buf, len);

		if (writelen == len)	   /* Success, first or Nth time */
			return (origlen);

		if (writelen == SOCKET_ERROR) {
			if (WSAGetLastError() != WSAEWOULDBLOCK)
				return (SOCKET_ERROR);	/* Some error */
			/* For WOULDBLOCK, immediately repeat the send. */
		}
		else {
			/* Partial write; update the pointers and retry. */
			len -= writelen;
			buf += writelen;
		}
	}
}