/*
 * Copyright (c) 1992-2000 MagniComp 
 * This software may only be used in accordance with the license which is 
 * available as http://www.magnicomp.com/sysinfo/4.0/sysinfo-eu-license.shtml
 */

/*
 * $Revision: 1.1.1.1 $
 */

#ifndef __os_irix_h__
#define __os_irix_h__

#include <unistd.h>
#include <fcntl.h>
#include <sys/param.h>
#include <stdlib.h>
#include <nlist.h>
#if	OSVER >= 62
#include <sys/sema.h>		/* for <netinet/in_var.h> in netif.c */
#include <sys/hashing.h>	/* for <netinet/in_var.h> in netif.c */
#endif

#define HAVE_DEVICE_SUPPORT
#if	OSVER >= 62
#	define HAVE_SOFTINFO_SUPPORT
#endif	/* OSVER >= 62 */
#define KMEMFILE		"/dev/kmem"
#define NAMELIST		"/unix"
#define HAVE_PHYSMEM
#define PHYSMEM_SYM 		"physmem"

/*
 * Don't try finding duplicate device names by their type.
 */
#define NO_DUP_DEV_BY_TYPE	1

#if	defined(irix64)
/* 
 * Use 64-bit values
 */
#define NLIST_TYPE		struct nlist64
#define NLIST_FUNC		nlist64
#define KVMADDR_T		off64_t
#else	/* !irix64 */
/* 
 * Don't use 64-bit 
 */
#define NLIST_TYPE		struct nlist
#define KVMADDR_T		off_t
#endif	/* irix64 */

/*
 * Pathnames
 */
#define _PATH_DEV_DSK		"/dev/dsk"
#define _PATH_DEV_RDSK		"/dev/rdsk"
#define _PATH_DEV_GRAPHICS	"/dev/graphics"
#define _PATH_DEV_SCSI		"/dev/scsi"

/*
 * Max number of disk device Logical Unit Numbers (LUN) to check for.
 */
#define SI_MAX_LUN		8

/*
 * Compatibility
 */
#define GETPAGESIZE()		sysconf(_SC_PAGESIZE)

#if OSVER <= 62
/*
 * Need to declare vararg functions, otherwise %f formats won't work in
 * 64-bit IRIX.
 */
extern int			snprintf(char *, size_t, const char *, ...);
#endif	/* OSVER <= 62 */

/*
 * Graphics
 */
#if	defined(HAVE_SYS_GFX_H)
#define HAVE_GRAPHICS_HDRS
#include <sys/gfx.h>
#endif	/* HAVE_SYS_GFX_H */

/*
 * Starting with IRIX 6.5, all the hw graphics specific .h files where
 * removed.  We include the parts we need below.
 */

#if	defined(HAVE_SYS_GFX_H)

#if	defined(HAVE_SYS_GR1NEW_H)
#include <sys/gr1new.h>
#else	/* HAVE_SYS_GR1NEW_H */
/*
 * From <sys/gr1new.h>
 */
#define GR1_TYPE_GR1	0		/* standard Personal Iris GR1 */
#define GR1_TYPE_VGR_PB	1		/* 9U-sized on private bus */
#define GR1_TYPE_UNK	0		/* ie, not initialized */
#define GR1_TYPE_PGR	3		/* 6U-sized (on private bus) */
struct gr1_info {
	struct gfx_info gfx_info; 	/* device independent information */
	unsigned char BoardType;	/* see GR1 variants below */
	unsigned char REversion; 	/* 1 or 2 */
	unsigned char Bitplanes; 	/* 8 or 24 */
 	unsigned char Auxplanes;	/* 2 or 4 */
	unsigned char Widplanes;	/* 2 or 4 */
	unsigned char Zplanes;		/* 24 if installed */
	unsigned char Turbo;		/* true if turbo option installed */
	unsigned char SmallMonitor;	/* true if small monitor attached */
	unsigned char Cursorplanes; 	/* 1 or 2 */
	unsigned char VRAM1Meg;		/* 1 if have 1 megabit VRAMs */
	signed char picrev;
};
#endif	/* HAVE_SYS_GR1NEW_H */

#if	defined(HAVE_SYS_GR2_H)
#include <sys/gr2.h>
#else	/* HAVE_SYS_GR2_H */
/*
 * From <sys/gr2.h>
 */
struct gr2_info {
	struct gfx_info gfx_info; 	/* device independent information */

	unsigned char BoardType;	/* 0 for a GR2, 1 for HI1 */
	unsigned char Bitplanes; 	/* 8 or 24 */
 	unsigned char Auxplanes;	/* 2 or 4 */
	unsigned char Wids;		/* 2 or 4 */
	unsigned char Zbuffer;		/* true if zbuffer option installed */
	unsigned char MonitorType;	/* monitor id */
	unsigned char GfxBoardRev;
	unsigned char PICRev;
	unsigned char HQ2Rev;
	unsigned char GE7Rev;
	unsigned char RE3Rev;
	unsigned char VC1Rev;
	unsigned char VidBckEndRev; 
	unsigned char Xabnormalexit;	/* X killed without closing file
					   descriptor. */
	unsigned char GEs;		
	unsigned char MonTiming;	/* monitor timinig */
	unsigned char VidBrdRev;	/* video board revision */
	unsigned char PanelType;	/* flat panel type */
};
#endif	/* HAVE_SYS_GR2_H */

#if	defined(HAVE_SYS_LG1_H)
#include <sys/lg1.h>
#else	/* HAVE_SYS_LG1_H */
/*
 * From <sys/lg1.h>
 */
struct lg1_info {
			/* device independent information */
	struct gfx_info gfx_info; 
			/* lg1 specific information */
	unsigned char boardrev;
	unsigned char rexrev;
	unsigned char vc1rev;
		/* next 2 only supported for boardrev >= 1 */
	unsigned char monitortype;
	unsigned int videoinstalled;
		/* only valid for IP12 */
	unsigned char picrev;
};
#endif	/* HAVE_SYS_LG1_H */

#if	defined(HAVE_SYS_NG1_H)
#include <sys/ng1.h>
#else	/* HAVE_SYS_NG1_H */
/*
 * From <sys/ng1.h>
 */
typedef struct {
    	int flags;
        short w, h;		/* monitor resolution */
        short fields_sec;	/* fields/sec */
} ng1_vof_info_t;
struct ng1_info {
			/* device independent information */
	struct gfx_info gfx_info; 
			/* ng1 specific information */
	unsigned char boardrev;
	unsigned char rex3rev;
	unsigned char vc2rev;
	unsigned char monitortype;
	unsigned char videoinstalled;
	unsigned char mcrev;
	unsigned char bitplanes;	/* 8 or 24 */
	unsigned char xmap9rev;
	unsigned char cmaprev;
	ng1_vof_info_t ng1_vof_info;
	unsigned char bt445rev;
	unsigned char paneltype;
};
#endif	/* HAVE_SYS_NG1_H */

#if	defined(HAVE_SYS_VENICE_H)
#include <sys/venice.h>
#else	/* HAVE_SYS_VENICE_H */
/*
 * From <sys/venice.h>
 */
#define VENICE_VS2_MAX_HEADS	6	/* heads available on VS2 */
#define VENICE_DG2_MAX_FIELDS	8	/* Number of fields/frame */
typedef struct {

    /* Size of active area for width and height */
    int vof_width;	/* the size of DG2 memory used */
    int vof_height;	/* must be <= the display surface size */

    int cursor_fudge_x;
    int cursor_fudge_y;

    int flags;		/* see #defines above */

    short unused;	/* 16 bits spare (used to be half of xfer_bandwidth) */
    /*
     * fields_per_frame contains the number of fields each frame must scan out.
     *
     * For standard interlaced formats, this number will be two.  It may be
     * more than that for 'special' formats (three for sequential rgb, four
     * for funky interlaced formats, two for old-style stereo, etc...).
     *
     * field_with_uppermost_line contains the obvious, but vofgen embodies some
     * not-so-obvious rules to deduce this numerical index.
     */
    unsigned char fields_per_frame;
    unsigned char field_with_uppermost_line;
    int hwalk_length;	/* number of operations available in hblank */
    int vof_framerate;	/* VS2 vof loader will need this too */

    int	monitor_type;	    /* Monitor type.  See getmonitor(3G). */

    int vof_total_width;      /* Total number of pix, including blanking */
    int vof_total_height;     /* Total number of lines, including blanking */
    int encoder_x_offset;     /* Must be hand-tuned per vof currently. */
    int encoder_y_offset;     /* has been zero up until now, but ??? */

    int pix_density[3][3]; /* boolean table of vof vs hw config capabilities (RM count) */
    int lines_in_field[VENICE_DG2_MAX_FIELDS];	/* Per-field listing */

} venice_vof_file_info_t;
typedef struct {

    int active;		/* whether this channel is being used or not */
    int cursor_enable;	/* whether to put a cursor on this channel or not */

    int pan_x;		/* offset between visible output and display surface */
    int pan_y;

    venice_vof_file_info_t vof_file_info;

} venice_vof_head_info_t;
/*
 * Structure returned by ioctl GETBOARDINFO when executed on a VENICE.
 */
struct venice_info {

	/* the device-dependant structure contains two numbers, xpmax
	 * and ypmax. These are the user's desired display surface area
	 * (which may be different from his viewable screen size).
	 */

	struct gfx_info gfx_info; 	/* device independent information */

	/*
	 * VENICE specific information.
	 * Note: If we ever implement a multi-head single-pipe X server/kernel,
	 * some of this must be rearranged.
	 */

	int	ge_rev;			/* GE revision (indicative of count) */
	int	rm_count;		/* number of RM boards in the system */
	int	pixel_depth;		/* small, medium, or large */
	int	tiles_per_line;		/* xpmax / (80 * #RMs) */
	int     baseTileAddr;           /* Starting tile number of frame buffer */
	int     ilOffset;               /* Initial Line Offset (pixel/10 offset within tile) */
	int	pixel_density;		/* 10bitRGBA, 10bitRGB, or 8bitRGB */
	int	hwalk_length;		/* # ops available in hblank */
	int	tex_memory_size;	/* # of 16 bit words in one texture memory bank RM4: 1024*1024; RM5: 2048*2048 */

	/*
	 * For VS2, there are six independent channels; hence the head_info[6].
	 * If not using a VS2, then only entry 0 will be used.
	 */

	int	video_format;		/* identifier for video format */
	int	going_to_vs2;		/* we're feeding a VS2 */
	unsigned int vs2_vme_phys_addr;/* physical vme-bus address of VS2 */

	venice_vof_head_info_t vof_head_info[VENICE_VS2_MAX_HEADS];

	int	ge_count;

	/*
	 * Store the VME adapter number to be used for this gfx pipe; 
	 * EVEREST can support up to three separate VME buses, each of which
	 * is uniquely identified by an adapter number (ranging from 0 to 63).
	 *
	 * MP based SkyWriter systems may have two VS2 boards sharing the
	 * same VME bus, and will differentiate themselves by address.

	 * The virtual base address is determined by the kernel during edtinit,
	 * and is the address to be supplied to mmap; the device to open
	 * will be either "/dev/mmem" (for MP) or "/dev/vme/vme<adapter #>a24n"
	 * for EVEREST based systems.  Everest user level programs will need
	 * to supply the physical VME address from above as their address for
	 * mmap(); MP based systems use the virtual address from below as
	 * their address for mmap().
	 */
	unsigned int vs2_vme_adapter_number;
	caddr_t vs2_vme_virtual_base_addr;

	/*
	 * Maintain a copy of the DG2 vof's display screen table, so that we 
	 * can dynamically patch it on the VLIST (to effect channel to channel
	 * cursor switches in MCO mode).
	 */
	unsigned short DG2_display_screen_table[8];
	int rm_rev;
};
#endif	/* HAVE_SYS_VENICE_H */

#endif	/* HAVE_SYS_GFX_H */

#endif	/* __os_irix_h__ */
