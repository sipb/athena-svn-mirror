main()
{
     printf("You %s define MALLOC_0_RETURNS_NULL.\n",
	    malloc(0) ? "should NOT" : "SHOULD");
     exit(0);
}
