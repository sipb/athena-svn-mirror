/*
 * afs_AdminVosErrors.h:
 * This file is automatically generated; please do not edit it.
 */
#define ADMVOSSERVERNULL                         (22016L)
#define ADMVOSCELLHANDLENULL                     (22017L)
#define ADMVOSCELLHANDLEBADMAGIC                 (22018L)
#define ADMVOSCELLHANDLEINVALID                  (22019L)
#define ADMVOSCELLHANDLEINVALIDVOS               (22020L)
#define ADMVOSCELLHANDLENOAFSTOKENS              (22021L)
#define ADMVOSMUSTBERWVOL                        (22022L)
#define ADMVOSBACKUPVOLWRONGSERVER               (22023L)
#define ADMVOSSERVERHANDLENULL                   (22024L)
#define ADMVOSSERVERHANDLEINVALID                (22025L)
#define ADMVOSSERVERHANDLEBADMAGIC               (22026L)
#define ADMVOSSERVERNOCONNECTION                 (22027L)
#define ADMVOSPARTITIONPNULL                     (22028L)
#define ADMVOSPARTITIONTOOLARGE                  (22029L)
#define ADMVOSVOLUMENAMETOOLONG                  (22030L)
#define ADMVOSVOLUMENAMENULL                     (22031L)
#define ADMVOSVOLUMENAMEINVALID                  (22032L)
#define ADMVOSVOLUMEID                           (22033L)
#define ADMVOSVOLUMEIDTOOBIG                     (22034L)
#define ADMVOSVOLUMENOEXIST                      (22035L)
#define ADMVOSVOLUMENAMEDUP                      (22036L)
#define ADMVOSPARTITIONNAMENULL                  (22037L)
#define ADMVOSPARTITIONNAMEINVALID               (22038L)
#define ADMVOSPARTITIONNAMETOOLONG               (22039L)
#define ADMVOSPARTITIONNAMETOOSHORT              (22040L)
#define ADMVOSPARTITIONNAMENOTLOWER              (22041L)
#define ADMVOSPARTITIONIDNULL                    (22042L)
#define ADMVOSPARTITIONIDTOOLARGE                (22043L)
#define ADMVOSEXCLUDEREQUIRESPREFIX              (22044L)
#define ADMVOSSERVERADDRESSPNULL                 (22045L)
#define ADMVOSSERVERENTRYPNULL                   (22046L)
#define ADMVOSSERVERTRANSACTIONSTATUSPNULL       (22047L)
#define ADMVOSVLDBENTRYNULL                      (22048L)
#define ADMVOSVLDBDELETEALLNULL                  (22049L)
#define ADMVOSNEWVOLUMENAMENULL                  (22050L)
#define ADMVOSSERVERANDPARTITION                 (22051L)
#define ADMVOSDUMPFILENULL                       (22052L)
#define ADMVOSDUMPFILEWRITEFAIL                  (22053L)
#define ADMVOSDUMPFILEOPENFAIL                   (22054L)
#define ADMVOSRESTOREVOLEXIST                    (22055L)
#define ADMVOSRESTOREFILEOPENFAIL                (22056L)
#define ADMVOSRESTOREFILECLOSEFAIL               (22057L)
#define ADMVOSRESTOREFILEREADFAIL                (22058L)
#define ADMVOSRESTOREFILEWRITEFAIL               (22059L)
#define ADMVOSRESTOREVOLUMENAMETOOBIG            (22060L)
#define ADMVOSVOLUMENAMEANDVOLUMEIDNULL          (22061L)
#define ADMVOSVOLUMEPNULL                        (22062L)
#define ADMVOSVOLUMEMOVERWONLY                   (22063L)
#define ADMVOSVOLUMERELEASERWONLY                (22064L)
#define ADMVOSVOLUMENOREPLICAS                   (22065L)
#define ADMVOSVOLUMENOREADWRITE                  (22066L)
#define ADMVOSVLDBBADENTRY                       (22067L)
#define ADMVOSVLDBBADSERVER                      (22068L)
#define ADMVOSVLDBDIFFERENTADDR                  (22069L)
#define ADMVOSVLDBNOENTRIES                      (22070L)
extern void initialize_av_error_table ();
#define ERROR_TABLE_BASE_av (22016L)

/* for compatibility with older versions... */
#define init_av_err_tbl initialize_av_error_table
#define av_err_base ERROR_TABLE_BASE_av
