#define STACK_PUSH 	0
#define STACK_POP	1
#define EMPTY_STACK	2

#define push(data, size)	dostack((caddr_t) data, STACK_PUSH, size)
#define pop(data, size)		dostack((caddr_t) data, STACK_POP, size)
#define popall()		dostack((caddr_t) NULL, EMPTY_STACK, 0)
     
