/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*
*  This source code is provided to you by Adobe on a non-exclusive,
*  royalty-free basis to facilitate your development of PostScript
*  language programs.  You may incorporate it into your software as is
*  or modified, provided that you include the following copyright
*  notice with every copy of your software containing any portion of
*  this source code.
*
* Copyright 1991 Adobe Systems Incorporated.  All Rights Reserved.
*
* Adobe does not warrant or guarantee that this source code will
* perform in any manner.  You alone assume any risks and
* responsibilities associated with implementing, using or
* incorporating this source code into your software.
*
* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

extern char *PSResFontOutline, *PSResFontPrebuilt, *PSResFontAFM,
	*PSResFontBDF, *PSResFontFamily, *PSResFontBDFSizes,
	 *PSResForm, *PSResPattern, *PSResEncoding, *PSResProcSet;

typedef enum {PSSaveReturnValues, PSSaveByType, PSSaveEverything}
	PSResourceSavePolicy;

#define _NO_PROTO

#ifdef _NO_PROTO

extern int ListPSResourceFiles();
extern int ListPSResourceTypes();
extern void FreePSResourceStorage();
extern void SetPSResourcePolicy();
typedef int (*PSResourceEnumerator)();
extern void EnumeratePSResourceFiles();
extern int CheckPSResourceTime();
typedef char *(*PSResMallocProc)();
typedef char *(*PSResReallocProc)();
typedef void (*PSResFreeProc)();
typedef void (*PSResFileWarningHandlerProc)();

#else /* _NO_PROTO */

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern int ListPSResourceFiles(char *psResourcePathOverride,
			       char *defaultPath,
			       char *resourceType,
			       char *resourceName,
			       char ***resourceNamesReturn,
			       char ***resourceFilesReturn);

extern int ListPSResourceTypes(char *psResourcePathOverride,
			       char *defaultPath,
			       char ***resourceTypeReturn);

extern void FreePSResourceStorage(int everything);

extern void SetPSResourcePolicy(PSResourceSavePolicy policy,
				int willList,
				char **resourceTypes);

typedef int (*PSResourceEnumerator)(char *resourceType,
				    char *resourceName,
				    char *resourceFile,
				    char *private);

extern void EnumeratePSResourceFiles(char *psResourcePathOverride,
				     char *defaultPath,
				     char *resourceType,
				     char *resourceName,
				     PSResourceEnumerator enumerator,
				     char *private);

extern int CheckPSResourceTime(char *psResourcePathOverride,
			       char *defaultPath);

typedef char *(*PSResMallocProc)(int size);

typedef char *(*PSResReallocProc)(char *ptr,
				  int size);

typedef void (*PSResFreeProc)(char *ptr);

typedef void (*PSResFileWarningHandlerProc)(char *fileNamem, char *extraInfo);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif /* _NO_PROTO */

extern PSResMallocProc PSResMalloc;

extern PSResReallocProc PSResRealloc;

extern PSResFreeProc PSResFree;

extern PSResFileWarningHandlerProc PSResFileWarningHandler;
