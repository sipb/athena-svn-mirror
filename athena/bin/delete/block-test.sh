cd ${TMPDIR-/tmp}

cat >test$$.c <<PROGRAM_IS_DONE
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <stdio.h>

main()
{
     struct stat statbuf;
     char buf[4096];
     int testfile;
     char filename[20];

     sprintf(filename, "test%d.out", getppid());

     testfile = open(filename, O_CREAT | O_RDWR | O_TRUNC, 0644);
     if (testfile < 0) {
	  fprintf(stderr, "Could not open temporary file \"%s\".\n", filename);
	  exit(1);
     }

     if (write(testfile, buf, sizeof(buf)) != sizeof(buf)) {
	  fprintf(stderr, "Could not write all data to file \"%s\".\n",
		  filename);
	  exit(1);
     }

     if (close(testfile) < 0) {
	  fprintf(stderr, "Error closing \"%s\".\n", filename);
	  exit(1);
     }

     if (stat(filename, &statbuf) < 0) {
	  fprintf(stderr, "Could not stat file \"%s\".\n", filename);
	  exit(1);
     }

     if (statbuf.st_blocks == sizeof(buf) / 512) {
	  printf("You SHOULD define USE_BLOCKS.\n");
     }
     else {
	  printf("You SHOULD NOT define USE_BLOCKS.\n");
	  printf("However, you have an interesting machine that delete might be made to work\nbetter with.  Please contact the author (see the README file for an address)\nand tell him what kind of machine you have and what operating system it is\nrunning.\n");
     }
     exit(0);
}
PROGRAM_IS_DONE


if ${CC-cc} -o test$$ test$$.c 2>&1 >/dev/null; then
	if ./test$$; then : ;
	else
		echo "Test program did not succeed."
		echo "This means you probably shouldn't define USE_BLOCKS."
	fi
else
	echo "Could not compile test program."
	echo "This means you probably shouldn't define USE_BLOCKS."
fi


rm test$$*
exit 0
