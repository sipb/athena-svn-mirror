#define DELETEPREFIX ".#"
#define DELETEREPREFIX "\\.#"
#define ERROR_MASK 1
#define NO_DELETE_MASK 2

typedef struct {
     char *user_name;
     char *real_name;
} listrec;

listrec *sort_files();
listrec *unique();
