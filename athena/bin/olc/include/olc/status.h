
#define OFF             0       /* registered in OLC */
#define ON              1       /* ??? */
#define FIRST           2       /* first crack at questions in specialty */
#define DUTY            4       /* general olc duty */
#define SECOND          8       /* backseat at questions in specialty */
#define URGENT          16      /* backseat general duty */
#define SIGNED_ON       31      /* signed on at all? */
#define BUSY            64      /* answering a question */
#define CACHED          128     /* structure is in a cache */

/* question status flags */

#define PENDING         512       /* question previously forwarded */
#define NOT_SEEN        1024      /* yet to get help */
#define DONE            2048      /* resolved the question */
#define CANCEL          4096      /* cancelled the question */
#define SERVICED        8192      /* was user helped ?*/
#define QUESTION_STATUS 15872     /* the above 5 bits */

/* user status flags */

#define LOGGED_OUT      1         /* dearly departed */
#define MACHINE_DOWN    2
#define UNKNOWN_STATUS  4
#define NOT_HERE        7
#define ACTIVE          8

