#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <X11/Intrinsic.h>

typedef  struct _XmPictureRec*       XmPicture;
typedef  struct _XmPictureStateRec*  XmPictureState;

XmPicture      XmParsePicture           (char*);
XmPictureState XmGetNewPictureState     (XmPicture);
char*          XmPictureProcessCharacter(XmPictureState, char, Boolean*);
void           XmPictureDelete          (XmPicture);
void           XmPictureDeleteState     (XmPictureState);
char*          XmPictureGetCurrentString(XmPictureState);
char*          XmPictureDoAutoFill      (XmPictureState);
