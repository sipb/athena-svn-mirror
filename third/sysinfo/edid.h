/*
 * Copyright (c) 1992-1998 Michael A. Cooper.
 * This software may be freely used and distributed provided it is not
 * sold for profit or used in part or in whole for commercial gain
 * without prior written agreement, and the author is credited
 * appropriately.
 */

/*
 * $Revision: 1.1.1.1 $
 */

/*
 * This file contains VESA EDID definetions common to SysInfo.
 *
 * EDID info from:
 *	Extended Display Identification Data (EDID) Standard
 *		Version 3, Revision Date: 11/13/97
 */

#ifndef __edid_h__
#define __edid_h__ 

/*
 * Standard EDID header should contain EDID_HEADER
 */
#define EDID_HEADER	{ 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00 }

/*
 * Monitor Data Tags identifying type of Descriptor Block (DataType)
 */
#define MDD_SERIAL	0xFF		/* Monitor Serial Number (ASCII) */
#define MDD_STRING	0xFE		/* General ASCII String */
#define MDD_RANGE	0xFD		/* Monitor Range Limits */
#define MDD_NAME	0xFC		/* Monitor Name (ASCII) */
#define MDD_COLORPT	0xFB		/* Color Point Data */
#define MDD_STDTIMING	0xFA		/* Add. Std Timing Identifications */
/* DataTypes of range 0x00 - 0x0F are reserved for Manufacturer use */

/*
 * Detailed Timing Descriptor
 */
typedef struct {
    u_short	PixelClock;		/* Hz */
    u_char	HorActive1;		/* # Pixels */
    u_char	HorBlanking1;		/* # Pixels */
    u_char	HorActive2 : 4,
		HorBlanking2 : 4;
    u_char	VerActive2;		/* # Lines */
    u_char	VerBlanking2;		/* # Lines */
    u_char	VerActive3 : 4,
		VerBlanking3 : 4;
    u_char	HorSyncOffSet1;		/* # Pixels */
    u_char	HorSyncPulseWidth;	/* # Pixels */
    u_char	VertSyncOffset1 : 4,	/* # lines */
		VertSyncPulseWidth1 : 4;/* # lines */
    u_char	HorSyncOffSet2 : 2,
		HorSyncPulseWidth2 : 2,
		VerSyncOffSet2 : 2,
		VerSyncPulseWidth2 : 2;
    u_char	HorImageSize1;		/* Horizontal Image Size (mm) */
    u_char	VerImageSize1;		/* Vertical Image Size (mm) */
    u_char	HorImageSize2 : 4,
		VerImageSize2 : 4;
    u_char	HorBorder;		/* # Pixels */
    u_char	VerBorder;		/* # Lines */
    u_char	Interlaced : 1,		/* 0=Non-Interlaced 1=Interlaced */
		Flags : 7;
} DetailTiming_t;

/*
 * Monitor Range Limits
 * Used when MonDescriptor_t.DataType == MDD_RANGE
 */
typedef struct {
    u_char	MinVertical;		/* Min. Vertical Rate (Hz) */
    u_char	MaxVertical;		/* Max. Vertical Rate (Hz) */
    u_char	MinHorizontal;		/* Min. Horizontal rate (KHz) */
    u_char	MaxHorizontal;		/* Max. Horizontal rate (KHz) */
    u_char	MaxPixelClock;		/* Max. Support. Pixel Clock (MHz/10)*/
    u_char	GTFreserved[8];		/* Reserved for GTF standard */
} MonRange_t;

/*
 * Monitor Descriptor Block
 * This is a general buffer that is interpretted based on DataType.
 */
typedef struct {
    u_short	Flag1;			/* Should be == 0x0000 if descriptor */
    u_char	Reserved;		/* Reserved */
    u_char	DataType;		/* Type of data (see MDD_*) */
    u_char	Flag2;			/* Should be == 0x00 if descriptor */
    u_char	Data[13];		/* The data itself */
} Detail_t;

/*
 * Standard Timings consistant of 2-byte values
 */
typedef struct {
    u_char	HorActivePixels;	/* Horizontal Active Pixels */
    u_char	Aspect : 2,		/* Aspect Ratio */
		RefreshRate : 6;	/* Refesh Rate */
} StdTiming_t;

/*
 * EDID Version 1
 */
typedef struct {
    /* EDID Header */
    u_char		Header[8];	/* EDID Header */
    /* Vendor / Product Identification */
 				/* EISA Manufacturer ID code (2-bytes) */
    u_short		ManIDpad : 1,		/* Padding */
			ManIDc1 : 5,		/* Character 1 */
			ManIDc2 : 5,		/* Character 2 */
			ManIDc3 : 5;		/* Character 3 */
    u_char		ProductIdCode[2]; /* Product ID code */
    u_char		Serial[4];	/* Serial number */
    u_char		ManWeek;	/* Week # manufactured */
    u_char		ManYear;	/* Year manufactured */
    /* EDID Version / Revision */
    u_char		EDIDversion;	/* EDID Version # */
    u_char		EDIDrevision;	/* EDID Revision # */
    /* Basic Display Parameters / Features */
    u_char		SigType : 1,	/* 0=Analog 1=Digital */
			SigInfo : 7;
    u_char		HorImageSize;	/* Max Horizontal Image Size (cm) */
    u_char		VerImageSize;	/* Max Vertical Image Size (cm) */
    u_char		Gamma;		/* Display Transfer Characteristic */
    u_char		DpmsStandBy : 1,/* Supports DPMS Stand-By mode */
			DpmsSuspend : 1,/* Supports DPMS Suspend mode */
			DpmsActiveOff : 1, /* Supports DPMS Active-Off */
			DisplayType : 2,/* mono, color, etc */
			StdColorSpace : 1,/* Uses std default color space */
			HasPrefTiming : 1,/* Preferred Timing in 1st Detail 
					     Timing Block */
			HasGtf : 1; 	/* Supports GTF standard timings */
    /* Color Characteristics */
    u_char		ColorChars[10];
    /* Established Timings */
    u_char		EstTimings[3];
    /* Standard Timings */
    StdTiming_t		StdTimings[8];
    /* Detailed Timing Descriptor Blocks */
    Detail_t		Details[4];
    /* Misc */
    u_char		ExtFlag;	/* Extension Flag: # of following
					   128-byte EDID extension blocks */
    u_char		CheckSum;	/* 1-byte sum of all 128-bytes in
					   this EDID should == 0 */
} EDIDv1_t;

#define EDID_V1_SIZE		128	/* This better be 128-bytes */

/*
 * EDID Version 2
 */
typedef struct {
    /* EDID Info */
    u_char		EDIDversion : 4,	/* EDID Version */
			EDIDrevision : 4;	/* EDID Revision */
    /* Manufacturer / Product Info */
    				/* EISA Manufacturer ID code (2-bytes) */
    u_short		ManIDpad : 1,		/* Padding */
			ManIDc1 : 5,		/* Character 1 */
			ManIDc2 : 5,		/* Character 2 */
			ManIDc3 : 5;		/* Character 3 */
    u_char		ProductIdCode[2]; /* Product ID code */
    u_char		ManWeek;	/* Week # manufactured */
    u_short		ManYear;	/* Year manufactured */
    u_char		ManProduct[32];	/* Manufacturer + Product names */
    u_char		Serial[16];	/* Serial number */
    /* Basic Display Parameters/Features */
    u_char		PriPhysInt : 4,	/* Primary physical interface */
			SecPhysInt : 4;	/* Secondary physical interface */
    u_char		PriVidInt : 4,	/* Primary Video interface */
			SecVidInt : 4;	/* Secondary Video interface */
    	/* Analog Interface */
    u_char		AnalogInfo;	/* Analog interface info */
    u_char	       	PixelClock : 1,	/* Has Pixel Clock */
			Reserved0 : 7;
    u_char		Reserved1[2];
    	/* Digital Interface */
    u_char		DisplayEnableParity : 1, /* Display enabled */
			EdgeShiftClock : 1,	/* falling edge shift clock */
			RecvrUnits : 2,	/* # of data receiver units */
			ChSpeed : 4;	/* Channel Speed Exponent */
    u_char		MinChSpeed;	/* Min. channel speed */
    u_char		MaxChSpeed;	/* Max. channel speed */
    u_char		DigitalFormat;	/* Digital interface data format */
    u_char		ColorEncoding : 4,	/* Color encoding */
			SecColorEncoding : 4;	/* Secondary color encoding */
    u_char		DepthSub0 : 4,	/* Bit-Depth sub-channel 0 */
			DepthSub1 : 4;	/* Bit-Depth sub-channel 1 */
    u_char		DepthSub2 : 4,	/* Bit-Depth sub-channel 2 */
			DepthSub3 : 4;	/* Bit-Depth sub-channel 3 */
    	/* Not sure why this is repeated in the std */
    u_char		OthDepthSub0 : 4,/* Bit-Depth sub-channel 0 */
			OthDepthSub1 : 4;/* Bit-Depth sub-channel 1 */
    u_char		OthDepthSub2 : 4,/* Bit-Depth sub-channel 2 */
			OthDepthSub3 : 4;/* Bit-Depth sub-channel 3 */
 	/* Display Device Description */
    u_char		DispTechType : 4,	/* Display Technology Type */
			DispTypeSubType : 4;	/*    " subtype */
    u_char		DisplayType : 1,	/* Mono, Color */
			SelChromat : 1,	/* Selectable Display chromaticity */
			CondUpdate : 1,	/* Conditional Update */
			ScanOrient : 2,	/* Scan Orientation */
			DispBg : 1,	/* Display Background */
			PhysImp : 2;	/* Physical Implementation */
    u_char		DpmsStandBy : 1,/* Supports DPMS Stand-By mode */
			DpmsSuspend : 1,/* Supports DPMS Suspend mode */
			DpmsActiveOff : 1, /* Supports DPMS Active-Off */
			DpmsOff : 1,	/* Supports DPMS Off */
			Stereo : 3,	/* Stereo Support info */
			Reserved2 : 1;
    u_char		AudioInType : 1, 	/* Audio Input Type */
			AudioInInt : 2,		/* Audio Input Interface */
			AudioOutType : 1,	/* Audio Output Type */
			AudioOutInt : 2,	/* Audio Output Interface */
			VideoInType : 2;	/* Video Input Type */
    u_char		TouchScreen : 1,	/* Contains touch screen */
			LightPen : 1,		/* Supports light pen */
			LumProbe : 1,		/* Supports Luminance Probe */
			Colorimeter : 1,	/* Supports colorimeter */
			AdjOrient : 1,		/* Adjustable orientation */
			Reserved3 : 3;
    	/* Display Response Time */
    u_char		RiseExp : 4,	/* Rise Time Exponenet */
			RiseSecs : 4;	/* Rise Time in seconds */
    u_char		FallExp : 4,	/* Fall Time Exponent */
			FallSecs : 4;	/* Fall Time in seconds */
 	/* Color/Luminance */
    u_char		GammaWhite;	/* White Gamma */
    u_char		GammaColor0;	/* Color 0 (red) Gamma */
    u_char		GammaColor1;	/* Color 1 (green) Gamma */
    u_char		GammaColor2;	/* Color 2 (blue) Gamma */
    u_short		MaxLum;		/* Max Luminance */
    u_char		StdRGB : 1,	/* Standard RGB model */
			AdjGamma : 1,	/* Adjustable Gamma */
			Reserved4 : 6;
    u_char		LumOffSet;	/* Luminance Offset */
    	/* Chromaticity */
    u_char		Chromaticity[10];
    u_char		WhitePoints[10];
    	/* Display Spatial Description */
    u_short		HorImageSize;	/* Max Horizontal Image Size (mm) */
    u_short		VerImageSize;	/* Max Vertical Image Size (mm) */
    u_short		MaxHorAddr;	/* Max. Hor. Addressibility (pixels) */
    u_short		MaxVerAddr;	/* Max. Ver. Addressibility (pixels) */
    u_char		HorPitch;	/* Max Hor. Pixel Pitch */
    u_char		VerPitch;	/* Max Ver. Pixel Pitch */
    	/* GTF */
    u_char		GTF;
    	/* Timing codes */
    u_char		ExtFlag : 1,	/* 1 more EDID block follows */
			HasPrefTiming : 1,	/* Preferred Timing in 1st 
						   Detail Timing Block */
			HasLumTab : 1,		/* Luminance Table provided */
			NumFreqRanges : 3,	/* # of Frequency Ranges */
			NumDetailRange : 2;	/* # of Detail Range Limits */
    u_char		NumTimingCodes : 5,	/* # of Timing Codes listed */
			NumDetailTimings : 3;	/* # of De3tailed Timings */
    	/* Tables */
    u_char		Tables[127];
    u_char		CheckSum;	/* 1-byte sum of all == 0 */
} EDIDv2_t;
#define EDID_V2_SIZE		256	/* Better be 256 */

#endif	/* __edid_h__ */
