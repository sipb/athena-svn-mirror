main()
{
     char *ptr1, *ptr2;

     ptr1 = (char *) malloc(0);
     ptr2 = (char *) realloc(ptr1, 0);

     printf("You %s define MALLOC_0_RETURNS_NULL.\n",
	    (ptr1 && ptr2) ? "should NOT" : "SHOULD");
     exit(0);
}
