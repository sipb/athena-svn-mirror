/* check-install: looks at installation (in tree or installed) and checks
   permissions, ownerships. Would be an easy perl script, but for now
   we need it in C. */

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#ifndef S_ISDIR
#ifdef S_IFDIR
#define S_ISDIR(i) (((i) & S_IFMT) == S_IFDIR)
#else /* ! defined (S_IFDIR) */
#define S_ISDIR(i) (((i) & 0170000) == 040000)
#endif /* ! defined (S_IFDIR) */
#endif /* ! defined (S_ISDIR) */

int any_fatal;

check_file(path, modes)
     char *path;
     int modes;
{
  struct stat sbuf;
  int file_mode;

  if(stat(path,&sbuf) == -1) {
    switch(errno) {
    case EACCES:
      printf("FATAL: bad permissions above %s\n", path);
      any_fatal++;
      break;
    case ENOENT:
      printf("FATAL: file %s MISSING\n", path);
      any_fatal++;
      break;
    default:
      perror(path);
    }
    return;
  }

  file_mode = sbuf.st_mode;
  if (S_ISDIR (file_mode))
    file_mode &= 0777;
  else
    file_mode &= 07777;
  if (file_mode != modes) {
    printf("WARNING: permissions on %s 0%o; expected 0%o\n",
	   path, file_mode, modes);
  }
  return;
}

check_owner(path, uid, gid)
     char *path;
     int uid, gid;
{
  struct stat sbuf;
  int ret;

  if(stat(path,&sbuf) == -1) {
    switch(errno) {
    case EACCES:
      printf("FATAL: bad permissions above %s\n", path);
      any_fatal++;
      break;
    case ENOENT:
      printf("FATAL: file %s MISSING\n", path);
      any_fatal++;
      break;
    default:
      perror(path);
    }
    return;
  }
  
  if(uid != -1 && sbuf.st_uid != uid) {
    printf("FATAL: file %s incorrect uid %d expected %d\n",
	   path, sbuf.st_uid, uid);
    any_fatal++;
  }

  if(gid != -1 && sbuf.st_gid != gid) {
    printf("FATAL: file %s incorrect gid %d expected %d\n",
	   path, sbuf.st_gid, gid);
    any_fatal++;
  }

  return;
}

#include <netdb.h>
#include <netinet/in.h>

check_service(name, proto, exval)
     char *name;
     char *proto;
     int exval;
{
  struct servent *sv;
  sv = getservbyname(name, proto);
  if(!sv) {
    printf("FATAL: service %s protocol %s not in /etc/services\n",
	   name, proto);
    any_fatal++;
    return;
  }
  if(sv->s_port != htons(exval)) {
    printf("WARNING: service %s protocol %s value %d != expected %d\n",
	   name, proto, ntohs(sv->s_port), exval);
  }
  return;
}


usage(name) 
     char *name;
{
  fprintf(stderr, "Usage: %s pathname (value of DESTDIR)\n", name);
}

main(argc, argv)
     int argc;
     char **argv;
{
  char path[1024];
  char *path_add;

  any_fatal = 0;

  if (argc != 2) {
    usage(argv[0]);
    return 1;
  }
  strcpy(path,argv[1]);
  path_add = path+strlen(path);

#define CHK(p,m) { strcpy(path_add, p); check_file(path,m); }

  CHK("/usr/kerberos",0755);
  if (any_fatal) {
    printf("%s not found -- please try again.\n", path);
    return 2;
  }
  path_add = path+strlen(path);

  CHK("/bin",0755);
  CHK("/etc",0755);
  CHK("/include",0755);
  CHK("/install",0755);
  CHK("/lib",0755);
  CHK("/man",0755);
  CHK("/database",0700);
  check_owner(path,0,-1);

  CHK("/bin/build_pwfile", 0555);
  CHK("/bin/compile_et", 0555);
  CHK("/bin/kadmin", 0555);
  CHK("/bin/kdestroy", 0555);
  CHK("/bin/kinit", 0555);
  CHK("/bin/klist", 0555);
  CHK("/bin/kpasswd", 0555);
  CHK("/bin/ksrvtgt", 0555);
  CHK("/bin/ksrvutil", 0555);
  CHK("/bin/ksu", 04555);
  check_owner(path,0,-1);
  CHK("/bin/mk_cmds", 0555);
  CHK("/bin/movemail", 0555);
  CHK("/bin/pfrom", 0555);
  CHK("/bin/telnet", 0555);
  CHK("/bin/ftp", 0555);
  CHK("/bin/rcp", 04555);
  check_owner(path,0,-1);
  CHK("/bin/rlogin", 0555);
  CHK("/bin/cygin", 0555);
  CHK("/bin/rsh", 0555);
  CHK("/bin/tcom", 0555);
  CHK("/bin/tftp", 0555);
  CHK("/etc/ext_srvtab", 0555);
  CHK("/etc/kadmind", 0555);
  CHK("/etc/kdb_destroy", 0555);
  CHK("/etc/kdb_edit", 0555);
  CHK("/etc/kdb_init", 0555);
  CHK("/etc/kdb_util", 0555);
  CHK("/etc/kerberos", 0555);
  CHK("/etc/klogind", 0555);
  CHK("/etc/kprop", 0555);
  CHK("/etc/kpropd", 0555);
  CHK("/etc/start-kpropd", 0555);
  CHK("/etc/push-kprop", 0555);
  CHK("/etc/kshd", 0555);
  CHK("/etc/kstash", 0555);
  CHK("/etc/login.krb", 0555);
  CHK("/etc/popper", 0555);
  CHK("/etc/tftpd", 0555);

  CHK("/include/com_err.h", 0444);
  CHK("/include/des.h", 0444);
  CHK("/include/kadm.h", 0444);
  CHK("/include/kadm_err.h", 0444);
  CHK("/include/kdc.h", 0444);
  CHK("/include/klog.h", 0444);
  CHK("/include/krb.h", 0444);
  CHK("/include/kerberos.h", 0444);
  CHK("/include/krb_db.h", 0444);
  CHK("/include/krb_err.h", 0444);
  CHK("/include/kstream.h", 0444);
  CHK("/include/mit-copyright.h", 0444);
  CHK("/include/mit-sipb-copyright.h", 0444);

#if 0
  /* these aren't installed yet when check-install is run. */
  CHK("/lib/krb.conf", 0644);
  CHK("/lib/krb.realms", 0644);
#endif

  CHK("/lib/libacl.a", 0444);
  CHK("/lib/libcom_err.a", 0444);
#ifndef NOENCRYPTION
  CHK("/lib/libdes.a", 0444);
#endif
  CHK("/lib/libkadm.a", 0444);
  CHK("/lib/libkdb.a", 0444);
  CHK("/lib/libkrb.a", 0444);
  CHK("/lib/libkstream.a", 0444);
  CHK("/lib/libss.a", 0444);

  CHK("/include/ss/copyright.h", 0444);
  CHK("/include/ss/mit-sipb-copyright.h", 0444);
  CHK("/include/ss/ss.h", 0444);
  CHK("/include/ss/ss_err.h", 0444);
  CHK("/include/ss/ss_internal.h", 0444);
  CHK("/man/man1/compile_et.1", 0444);
  CHK("/man/man1/ftp.1", 0444);
  CHK("/man/man1/kdestroy.1", 0444);
  CHK("/man/man1/kerberos.1", 0444);
  CHK("/man/man1/kinit.1", 0444);
  CHK("/man/man1/klist.1", 0444);
  CHK("/man/man1/kpasswd.1", 0444);
  CHK("/man/man1/ksrvtgt.1", 0444);
  CHK("/man/man1/ksu.1", 0444);
  CHK("/man/man1/pfrom.1", 0444);
  CHK("/man/man1/rcp.1", 0444);
  CHK("/man/man1/rlogin.1", 0444);
  CHK("/man/man1/rsh.1", 0444);
  CHK("/man/man1/telnet.1", 0444);
  CHK("/man/man1/tftp.1", 0444);
  CHK("/man/man3/acl_check.3", 0444);
  CHK("/man/man3/com_err.3", 0444);
  CHK("/man/man3/des_cbc_cksum.3", 0444);
  CHK("/man/man3/des_cbc_encrypt.3", 0444);
  CHK("/man/man3/des_crypt.3", 0444);
  CHK("/man/man3/des_ecb_encrypt.3", 0444);
  CHK("/man/man3/des_pcbc_encrypt.3", 0444);
  CHK("/man/man3/des_random_key.3", 0444);
  CHK("/man/man3/des_read_password.3", 0444);
  CHK("/man/man3/des_set_key.3", 0444);
  CHK("/man/man3/des_string_to_key.3", 0444);
  CHK("/man/man3/kerberos.3", 0444);
  CHK("/man/man3/krb_ck_repl.3", 0444);
  CHK("/man/man3/krb_get_admhst.3", 0444);
  CHK("/man/man3/krb_get_cred.3", 0444);
  CHK("/man/man3/krb_get_krbhst.3", 0444);
  CHK("/man/man3/krb_get_lrealm.3", 0444);
  CHK("/man/man3/krb_get_phost.3", 0444);
  CHK("/man/man3/krb_kntoln.3", 0444);
  CHK("/man/man3/krb_mk_err.3", 0444);
  CHK("/man/man3/krb_mk_priv.3", 0444);
  CHK("/man/man3/krb_mk_req.3", 0444);
  CHK("/man/man3/krb_mk_safe.3", 0444);
  CHK("/man/man3/krb_net_read.3", 0444);
  CHK("/man/man3/krb_net_write.3", 0444);
  CHK("/man/man3/krb_rd_err.3", 0444);
  CHK("/man/man3/krb_rd_priv.3", 0444);
  CHK("/man/man3/krb_rd_req.3", 0444);
  CHK("/man/man3/krb_rd_safe.3", 0444);
  CHK("/man/man3/krb_realmofhost.3", 0444);
  CHK("/man/man3/krb_recvauth.3", 0444);
  CHK("/man/man3/krb_sendauth.3", 0444);
  CHK("/man/man3/krb_set_key.3", 0444);
  CHK("/man/man3/krb_set_tkt_string.3", 0444);
  CHK("/man/man3/kuserok.3", 0444);
  CHK("/man/man3/quad_cksum.3", 0444);
  CHK("/man/man3/tf_close.3", 0444);
  CHK("/man/man3/tf_get_cred.3", 0444);
  CHK("/man/man3/tf_get_pinst.3", 0444);
  CHK("/man/man3/tf_get_pname.3", 0444);
  CHK("/man/man3/tf_init.3", 0444);
  CHK("/man/man3/tf_util.3", 0444);
  CHK("/man/man5/krb.conf.5", 0444);
  CHK("/man/man5/krb.realms.5", 0444);
  CHK("/man/man8/ftpd.8", 0444);
  CHK("/man/man8/kadmin.8", 0444);
  CHK("/man/man8/kadmind.8", 0444);
  CHK("/man/man8/kdb_destroy.8", 0444);
  CHK("/man/man8/kdb_edit.8", 0444);
  CHK("/man/man8/kdb_init.8", 0444);
  CHK("/man/man8/kdb_util.8", 0444);
  CHK("/man/man8/klogind.8", 0444);
  CHK("/man/man8/kshd.8", 0444);
  CHK("/man/man8/ksrvutil.8", 0444);
  CHK("/man/man8/kstash.8", 0444);
  CHK("/man/man8/popper.8", 0444);
  CHK("/man/man8/tcom.8", 0444);
  CHK("/man/man8/telnetd.8", 0444);
  CHK("/man/man8/tftpd.8", 0444);

  check_service("klogin", "tcp", 543);
  check_service("kerberos", "udp", 750);
  check_service("kerberos", "tcp", 750);
  check_service("kerberos_master", "udp", 751);
  check_service("kerberos_master", "tcp", 751);
  check_service("passwd_server", "udp", 752);
  check_service("kpop", "tcp", 1109);
  check_service("kshell", "tcp", 544);
  check_service("eklogin", "tcp", 2105);
  check_service("krb_prop", "tcp", 754);


  if(any_fatal) printf("%d fatal errors.\n", any_fatal);
  else printf("No fatal errors.\n");
  return any_fatal != 0;
}
