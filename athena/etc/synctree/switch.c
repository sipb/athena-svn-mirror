/*
 * Copyright (C) 1988  Tim Shepard		All rights reserved.
 * Copyright (C) 1989  MIT/Project Athena	All rights reserved.
 */

#if defined(_AIX) && defined(i386)
#define unlink(a) (rmslink(a) && unlink(a))
#endif

#define noupdate(problem) printf("Not %sing %s because %s\n",action,targname,problem)
#define update_error(problem) printf("Error %sing %s: %s: %s\n",action,targname,problem,strerror(errno))

        switch (typeofaction) {
	    int status;
#define action "copy"
	case ACTION_COPY:
	case ACTION_LOCAL:
	    /* We want to ignore "virtual" files */
	    if (srctype == TYPE_V)
		break;
	    
	    if (exists) {
		if ((verbosef || nflag) && (typeofaction == ACTION_LOCAL)) {
		    noupdate("the target is a local version.");
		    break;
		}
		if (srctype != targtype && (srctype & TYPE_V) == 0) {
		    if (!fflag && (targtype != TYPE_L)) {
			noupdate("the file types diff.  Use -f to override.");
			break;
		    }
		    if (verbosef || nflag)
			printf("Removing %s (file types differ).\n", targname);
		    if (!nflag) {
			if (targtype == TYPE_D) {
			    if (recursive_rmdir(targname)) {
				update_error("recursive rmdir failed");
				break;
			    }
			} else			    
			if (unlink(targname)) {
			    update_error("unlink failed");
			    break;
			}
		    }
		    exists = FALSE;
		}
	    }
	    switch (srctype) {
	    case TYPE_R:
		if (!exists) {
		    if (nflag) {
			printf("Copying %s (mode is %5.5o).\n",
			       srcname,targname,srcmode);
			break;
		    }
		    if (copyfile(srcname,targname,srcmode))
			update_error("copy failed");
		    setdates();
		    if (pflag &&
			(chown(targname, -1, srcgid) ||
			 chown(targname, srcuid, -1)))
			update_error("chown failed");
		    break;
		}
		switch(status = filecheck3()) {
		case UPTODATE:
		    break;
		case NEWERDATE:
		    if (!fflag) {
			noupdate("the target is a more recent copy.");
			break;
		    }
		    /* fall through */
		case OUTOFDATE:
		    if ((verbosef || nflag) && (typeofaction == ACTION_LOCAL)){
			noupdate("the target is a local version (the target is old).");
			break;
		    }
		    if (verbosef || nflag)
			printf("Updating out of date %s .\n", targname);
		    if (!nflag) {
			if (unlink(targname)) {
			    update_error("unlink failed");
			    break;
			} else if (copyfile(srcname,targname,srcmode)) {
			    update_error("copy failed");
			    break;
			}
			setdates();
		    }
		    /* fall through */
		case FIXOWNANDMODE:
		case FIXMODE:
		    if ((verbosef || nflag) && (typeofaction == ACTION_LOCAL)){
			noupdate("the target is a local version (the ownership is different).");
			break;
		    }
		    if ((verbosef || nflag) && (status == FIXOWNANDMODE))
			printf("Fixing ownership and modes of %s .\n",
			       targname);
		    if ((verbosef || nflag) && (status == FIXMODE))
			printf("Fixing modes of %s .\n", targname);

		    if (!nflag && chmod(targname, srcmode))
			update_error("chmod() failed");
		    if (!nflag && pflag && (status != FIXMODE) &&
			(chown(targname, -1, srcgid) ||
			 chown(targname, srcuid, -1)))
			update_error("chown failed");
		    break;		    
		default:
		    noupdate("of an internal error: filecheck3() returned an unknown value.");
		}
		break;
		  
	    case TYPE_L:
		if (exists) {
		    char buf1[BUFSIZ], buf2[BUFSIZ];
		    int buf1_len, buf2_len;
		    
		    if (((buf1_len = readlink(srcname, buf1, BUFSIZ)) ==
			 (buf2_len = readlink(targname, buf2, BUFSIZ))) &&
			!strncmp(buf1,buf2,buf1_len))
			break;
		    if ((verbosef || nflag) && (typeofaction == ACTION_LOCAL)){
			noupdate("the target is a local link (the links are different).");
			break;
		    }
		    if (verbosef || nflag)
			printf("Updating symbolic link %s .\n", targname);
		    if (!nflag && unlink(targname))
			update_error("unlink failed");
		} else
		    if (verbosef || nflag)
			printf("Copying symbolic link %s .\n",srcname);
		if (nflag)
		    break;
		if (copylink(srcname,targname,srcmode))
		    update_error("copy of symbolic link failed.");
#if !defined(_AIX)
		else if (pflag && chown(targname,srcuid,srcgid))
		    update_error("chown() failed.");
#endif
		break;
	      
	    case TYPE_D:
		if (!exists) {
		    if (verbosef || nflag)
			printf("Creating directory %s .\n", targname);
		    if (nflag)
			break;
		    if (mkdir(targname,0777)) {
			update_error("mkdir() failed");
			break;		/* No point continuing... */
		    }
		    else if (chmod(targname, srcmode))
			update_error("chmod() failed");
		    else if (pflag && chown(targname, srcuid, srcgid))
			update_error("chown() failed");
		} else {
		    switch(status = dircheck()) {
		    case FIXMODE:
		    case FIXOWNANDMODE:
			if ((verbosef || nflag) &&
			    (typeofaction == ACTION_LOCAL)) {
			    noupdate("the target is a local version");
			    break;
			}
			if ((verbosef || nflag) && (status == FIXMODE))
			    printf("Fixing modes of %s .\n", targname);
			else if (verbosef || nflag)
			    printf("Fixing ownership and modes of %s .\n",
				   targname);
			if (!nflag && chmod(targname, srcmode))
			    update_error("chmod() failed");
			if (!nflag && pflag &&
			    (chown(targname, -1, srcgid) ||
			     chown(targname, srcuid, -1))) {
			    update_error("chown() failed");
			    break;
			}
			break;		    
		    }
		}
		if (dodir(srcname, targname, part))
		    printf("The update of %s from %s aborted prematurely.\n",
			   targname, srcname);
		break;

	    case TYPE_C:
	    case TYPE_B:
		noupdate("copying of devices is not yet supported.");
		break;
	    }
	    break;
	    
#undef  action
#define action "link"
        case ACTION_LINK:
	    if (exists) {
		char linkbuf[MAXPATHLEN];
		int linklen;
		
		switch(targtype) {
		case TYPE_V:
		    /* This is a virtual file... so we should ignore it */
		    break;
		case TYPE_L:
		    linklen = readlink(targname, linkbuf, MAXPATHLEN);
		    if ((!strncmp(linkbuf, srcname, linklen)) &&
			(*((char *)srcname + linklen) == 0))
			break;
		    /* fall through */
		default:
		    if (!fflag && targtype != TYPE_L) {
			noupdate("the target was not a symlink.  Use -f to override.\n");
			break;
		    }
		    if (verbosef || nflag) printf("Removing %s .\n",targname);
		    if (!nflag)
			if (targtype == TYPE_D) {
			    if (recursive_rmdir(targname)) {
				update_error("recursive rmdir failed");
				break;
			    }
			} else			    
			if (unlink(targname)) {
			    update_error("unlink failed");
			    break;
			}
		    exists = FALSE;
		    break;
		}
	    }
	    if (exists) break;
	    if (verbosef || nflag)
		printf("Creating symbolic link from %s to %s .\n", targname, srcname);
	    if (!nflag && symlink(srcname,targname))
		update_error("symlink failed");
	    break;

/*
 * The action words are intentionally mispelled to
 * get the correct spelling during message displays
 */
#undef	action
#define	action "ignor"
	case ACTION_IGNORE:
	    break;

#undef update_error
#define update_error(problem) printf("Error %sing %s: %s: %s\n",action,targname,problem,strerror(errno))

#undef action
#define action "delet"
	case ACTION_DELETE:
	    if (!exists)
		break;
	    if (verbosef || nflag)
		printf("Removing %s.\n", targname);
	    if (!nflag) {
		if (targtype & TYPE_D) {
		    if (recursive_rmdir(targname))
			update_error("recursive rmdir failed");
		}
		else if (unlink(targname))
		    update_error("unlink failed");
	    }
	    break;
	}
