#include <X11/StringDefs.h>
#include <stdio.h>
#include <string.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>
#include <X11/IntrinsicP.h>
#include <X11/Xresource.h>
#include <ctype.h>

#define MONO 0
#define GRAY 1
#define COLOR 2


/*
 * a few things taken from Xt/Converters.c, with a few mods...
 */
#ifdef __STDC__
#define Const const
#else
#define Const /**/
#endif

static Const String XtNwrongParameters = "wrongParameters";

#define	done(type, value) \
	{							\
	    if (toVal->addr != NULL) {				\
		if (toVal->size < sizeof(type)) {		\
		    toVal->size = sizeof(type);			\
		    return False;				\
		}						\
		*(type*)(toVal->addr) = (value);		\
	    }							\
	    else {						\
		static type static_val;				\
		static_val = (value);				\
		toVal->addr = (XtPointer)&static_val;		\
	    }							\
	    toVal->size = sizeof(type);				\
	    return True;					\
	}

static XtConvertArgRec multiColorConvertArgs[] = {
    {XtWidgetBaseOffset,
       (XtPointer)XtOffset(Widget, core.screen),
       sizeof(Screen *)},
    {XtWidgetBaseOffset,
       (XtPointer)XtOffset(Widget, core.colormap),
       sizeof(Colormap)}
};



/*
 *  CvtStringToPixel taken from Xt/Converters.c, and modified to suit
 *  our needs...
 */

static Boolean CvtMultiStrToPxl(dpy, args, num_args, fromVal, toVal, closure_ret)
    Display*	dpy;
    XrmValuePtr args;
    Cardinal    *num_args;
    XrmValuePtr	fromVal;
    XrmValuePtr	toVal;
    XtPointer	*closure_ret;
{
    String	    str = (String)fromVal->addr;
    XColor	    screenColor;
    XColor	    exactColor;
    Screen	    *screen;
    /*
     *  XtPerDisplay    pd = _XtGetPerDisplay(dpy);
     *  we don't want to use private functions, however, that means we
     *  can't deal with reverseVideo.  See below...
     */
    Colormap	    colormap;
    Status	    status;
    String          params[1];
    Cardinal	    num_params=1;

    static int	    init=0;
    static int	    type;
    char	    sep = ',';
    String	    copy, address2, address3, color;
    XtAppContext    appContext = XtDisplayToApplicationContext(dpy);
    char	    *ptr;


    if (*num_args != 2)
      XtAppErrorMsg(appContext, XtNwrongParameters, "CvtMultiStrToPxl",
		    "XtToolkitError",
	"String to pixel conversion needs screen and colormap arguments",
        (String *)NULL, (Cardinal *)NULL);

    screen = *((Screen **) args[0].addr);
    colormap = *((Colormap *) args[1].addr);



    copy = XtNewString(str);

    /*
     * figure out the display type -- MONO, GRAY or COLOR
     */
    if (!init)
      {
	init = 1;
	if (DisplayPlanes(dpy, XScreenNumberOfScreen(screen)) > 1)
	  {
	    Visual *visual;
	    visual =
	      DefaultVisualOfScreen(screen);
	    switch(visual->class)
	      {
	      case StaticGray:
	      case GrayScale:
		type = GRAY;
		break;
	      case StaticColor:
	      case PseudoColor:
	      case TrueColor:
	      case DirectColor:
		type = COLOR;
		break;
	      default:
		type = MONO;
		break;
	      }
	  }
      }

    /*
     * parse the resource -- if there are 2 or 3 "words", then
     * assign them to the other addresses.
     */
    if ((ptr = address2 = strchr(copy, sep))  !=  NULL)
      {
	ptr--;				/* strip trailing spaces off word */
	while (isspace(*ptr))		/*   before comma */
	  ptr--;
	*++ptr = '\0';			/* terminate string */

	address2++;			/* strip leading spaces off word */
	while (isspace(*address2))	/*   after comma */
	  address2++;

	if ((ptr = address3 = strchr(address2, sep))  !=  NULL)
	  {
	    ptr--;			/* strip trailing spaces off word */
	    while (isspace(*ptr))	/*   before comma */
	      ptr--;
	    *++ptr = '\0';		/* terminate string */

	    address3++;			/* strip leading spaces off word */
	    while (isspace(*address3))	/*   after comma */
	      address3++;

	    ptr = address3 + strlen(address3) - 1;	/* strip trailing */
	    while (isspace(*ptr))	/*   spaces off last word... */
	      ptr--;
	    *++ptr = '\0';		/* terminate string */
	  }
	else
	  {				/* if there are only 2 words, then */
	    address3 = address2;	/* set the "color address" to the */
	    address2 = copy;		/* 2nd one, and the "grayscale */
	  }				/* address" to the 1st one... */
      }
    else				/* if there is only 1 word, then */
      address3 = address2 = copy;	/* set all addresses to it... */

    /*
     * figure out which address to use based on the display type.
     */
    switch(type)
      {
      case COLOR:
	color = address3;
	break;
      case GRAY:
	color = address2;
	break;
      default:
	color = copy;
	break;
      }

    if (strcasecmp(color, XtDefaultBackground) == 0) {
	*closure_ret = False;
	XtFree(copy);
	/* if (pd->rv) done(Pixel, BlackPixelOfScreen(screen)) */
	/* else */
/*
 *  Since we don't want to have to depend on header files in the Xt
 *  sources, we can't deal with reverseVideo...  oh well...
 */
	done(Pixel, WhitePixelOfScreen(screen));
    }
    if (strcasecmp(color, XtDefaultForeground) == 0) {
	*closure_ret = False;
	XtFree(copy);
	/* if (pd->rv) done(Pixel, WhitePixelOfScreen(screen)) */
        /* else	*/
/*
 *  Since we don't want to have to depend on header files in the Xt
 *  sources, we can't deal with reverseVideo...  oh well...
 */
	done(Pixel, BlackPixelOfScreen(screen));
    }

/*
 *  From here on down, this is almost identical to the old
 *  CvtStringToPixel, except that "str" was replaced by "color", and we
 *  also have to free "copy" before returning, to avoid memory leaks...
 */

    if (*color == '#') {  /* some color rgb definition */

        status = XParseColor(DisplayOfScreen(screen), colormap,
			     (char*)color, &screenColor);

        if (status == 0) {
	    params[0] = color;
	    XtAppWarningMsg(appContext, "badFormat", "CvtMultiStrToPxl",
			    "XtToolkitError",
		  "RGB color specification \"%s\" has invalid format",
		  params, &num_params);
	    *closure_ret = False;
	    XtFree(copy);
	    return False;
	}
	else
           status = XAllocColor(DisplayOfScreen(screen), colormap,
                                &screenColor);
    } else  /* some color name */

        status = XAllocNamedColor(DisplayOfScreen(screen), colormap,
                                  (char*)color, &screenColor, &exactColor);
    if (status == 0) {
	String msg, type;
	params[0] = color;
	/* Server returns a specific error code but Xlib discards it.  Ugh */
	if (*color == '#' ||
	    XLookupColor(DisplayOfScreen(screen), colormap, (char*)color,
			 &exactColor, &screenColor)) {
	    type = "noColormap";
	    msg = "Cannot allocate colormap entry for \"%s\"";
	}
	else {
	    type = "badValue";
	    msg = "Color name \"%s\" is not defined in server database";
	}

	XtAppWarningMsg(appContext, type, "CvtMultiStrToPxl",
			"XtToolkitError", msg, params, &num_params);
	*closure_ret = False;
	XtFree(copy);
	return False;
    } else {
	*closure_ret = (char*)True;
	XtFree(copy);
        done(Pixel, screenColor.pixel);
    }
}



void add_converter ()
{
  XtSetTypeConverter(XtRString, XtRPixel, CvtMultiStrToPxl,
		     multiColorConvertArgs,
		     XtNumber(multiColorConvertArgs),
		     XtCacheNone, NULL);
}
