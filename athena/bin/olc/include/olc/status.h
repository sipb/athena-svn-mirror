
#define OFF             0          /* registered in OLC */
#define ON              1<<1       /* ??? */
#define FIRST           1<<2       /* first crack at questions in specialty */
#define DUTY            1<<3       /* general olc duty */
#define SECOND          1<<4       /* backseat at questions in specialty */
#define URGENT          1<<5       /* backseat general duty */
#define SIGNED_ON       (ON | FIRST | DUTY | SECOND | URGENT)

#define LOGGED_OUT      1<<8       /* dearly departed */
#define MACHINE_DOWN    1<<9
#define UNKNOWN_STATUS  1<<10
#define NOT_HERE        1<<11
#define ACTIVE          1<<12

#define BUSY            1<<14      /* answering a question */
#define CACHED          1<<15      /* structure is in a cache */

/* question status flags */

#define PENDING         1<<16      /* question previously forwarded */
#define NOT_SEEN        1<<17      /* yet to get help */
#define DONE            1<<18      /* resolved the question */
#define CANCEL          1<<19      /* cancelled the question */
#define SERVICED        1<<20      /* was user helped ?*/
#define QUESTION_STATUS (PENDING | NOT_SEEN | DONE | CANCEL | SERVICED)
#define PICKUP          1<<22      /*waiting for user */
#define REFERRED        1<<23      /* waiting for someone else */

/* user status flags */


