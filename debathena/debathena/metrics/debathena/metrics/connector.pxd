cdef extern from "errno.h":
    int errno

cdef extern from "stdlib.h":
    ctypedef long size_t

cdef extern from "sys/types.h":
    ctypedef long pid_t

cdef extern from "sys/socket.h":
    ctypedef long socklen_t
    struct sockaddr:
        pass

    int socket(int, int, int)
    int bind(int, sockaddr *, socklen_t)
    size_t send(int, void *, size_t, int)
    size_t recvfrom(int, void *, size_t, int, sockaddr *, socklen_t *)

    enum:
        SOCK_DGRAM
        PF_NETLINK
        AF_NETLINK

cdef extern from "linux/netlink.h":
    enum:
        NETLINK_CONNECTOR
        NLMSG_DONE
    struct sockaddr_nl:
        unsigned long nl_family
        unsigned long nl_pad
        unsigned long nl_pid
        unsigned long nl_groups
    struct nlmsghdr:
        long nlmsg_len
        long nlmsg_type
        long nlmsg_flags
        long nlmsg_seq
        long nlmsg_pid
    long NLMSG_LENGTH(long)
    void * NLMSG_DATA(void *)

cdef extern from "linux/connector.h":
    enum:
        CN_IDX_PROC
        CN_VAL_PROC
    struct cb_id:
        long idx
        long val
    struct cn_msg:
        cb_id id
        long seq
        long ack
        long len
        long flags
        char data[0]

cdef extern from "linux/cn_proc.h":
    enum proc_cn_mcast_op:
        PROC_CN_MCAST_LISTEN
        PROC_CN_MCAST_IGNORE

    enum what:
        C_PROC_EVENT_NONE "PROC_EVENT_NONE"
        C_PROC_EVENT_FORK "PROC_EVENT_FORK"
        C_PROC_EVENT_EXEC "PROC_EVENT_EXEC"
        C_PROC_EVENT_UID "PROC_EVENT_UID"
        C_PROC_EVENT_GID "PROC_EVENT_GID"
        C_PROC_EVENT_EXIT "PROC_EVENT_EXIT"

    struct ack_proc_event:
        long err
    struct fork_proc_event:
        pid_t parent_pid
        pid_t parent_tgid
        pid_t child_pid
        pid_t child_tgid
    struct exec_proc_event:
        pid_t process_pid
        pid_t process_tgid
    union id_proc_event_r:
        long ruid
        long rgid
    union id_proc_event_e:
        long euid
        long egid
    struct id_proc_event:
        pid_t process_pid
        pid_t process_tgid
        id_proc_event_r r
        id_proc_event_e e
    struct exit_proc_event:
        pid_t process_pid
        pid_t process_tgid
        long exit_code
        long exit_signal

    union event_data:
        ack_proc_event ack
        fork_proc_event fork
        exec_proc_event exec_ "exec"
        id_proc_event id_ "id"
        exit_proc_event exit_ "exit"

    struct proc_event:
        what what
        long cpu
        long timestamp_ns
        event_data event_data

cdef extern from "string.h":
    char * strerror(int)

cdef extern from "unistd.h":
    pid_t getpid()
    int close(int)
