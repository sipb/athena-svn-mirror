#include <stdio.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <strings.h>
#include <sys/stat.h>

#define DELETEPREFIX ".#"
#define DELETEREPREFIX "\\.#"

typedef struct {
     char *realname;		/* The real filesystem name, with */
				/* deletion characters */
     char *username;		/* The name the user enters to */
				/* undelete it */
} filerec;

static filerec cwd = {"./", "./"};
static filerec root = {"/", "/"};

extern char *malloc(), *realloc();

char *whoami;
char *lastpart(), *add_char(), *parse_pattern(), *firstpart(), *append();
filerec *match_pattern();

int interactive, recursive, verbose, directoriesonly;

/* ARGSUSED */
main(argc, argv)
int argc;
char *argv[];
{
     whoami = lastpart(argv[0]);
     interactive = recursive = verbose = directoriesonly = 0;

     
}




/*
 * parse_pattern returns an area of memory allocated by malloc when it
 * is successful.  Therefore, other procedures calling parse_pattern
 * should use free() to free the region of memory when they are done
 * with it.
 */
char *parse_pattern(file_pattern)
char *file_pattern;
{
     char *re_pattern, *cur_ptr, *re_ptr;
     int guess_length;
     
     guess_length = strlen(file_pattern) + 5;
     re_ptr = re_pattern = malloc(guess_length);
     cur_ptr = file_pattern;
     
     if (! re_pattern) {
	  perror(sprintf(error_buf, "%s: parse_pattern", whoami));
	  return ((char *) NULL);
     }

     for (cur_ptr = file_pattern, re_ptr = re_pattern; *cur_ptr != NULL;
	  cur_ptr++) {
	  if (*cur_ptr == '\\') {
	       if (! cur_ptr[1]) {
		    fprintf(stderr,
			    "%s: parse_pattern: incomplete expression\n",
			    whoami);
		    return((char *) NULL);
	       }
	       if (! add_char(&re_pattern, &re_ptr, &guess_length, '\\'))
		    return ((char *) NULL);
	       cur_ptr++;
	       if (! add_char(&re_pattern, &re_ptr, &guess_length, *cur_ptr))
		    return ((char *) NULL);
	       continue;
	  }
	  else if (*cur_ptr == '*') {
	       if (! add_char(&re_pattern, &re_ptr, &guess_length, '.'))
		    return ((char *) NULL);
	       if (! add_char(&re_pattern, &re_ptr, &guess_length, '*'))
		    return ((char *) NULL);
	       continue;
	  }
	  else if (*cur_ptr == '?') {
	       if (! add_char(&re_pattern, &re_ptr, &guess_length, '.'))
		    return ((char *) NULL);
	       continue;
	  }
	  else {
	       if (! add_char(&re_pattern, &re_ptr, &guess_length, *cur_ptr))
		    return ((char *) NULL);
	  }
     }
     if (! add_char(&re_pattern, &re_ptr, &guess_length, '\0'))
	  return ((char *) NULL);
     return (re_pattern);
}




char *add_char(start, finish, length, chr)
char **start, **finish;
int *length;
char chr;
{
     if (*finish - *start == *length) {
	  *start = realloc(*start, *length + 5);
	  if (! *start)
	       return ((char *) NULL);
	  *finish = *start + *length - 1;
	  *length += 5;
     }
     **finish = chr;
     (*finish)++;
     return(*start);
}

	  

     
filerec *add_arrays(array1, num1, array2, num2)
filerec *array1, *array2;
int *num1, *num2;
{
     int counter;
     
     array1 = (filerec *) realloc(array1, sizeof(filerec) * (*num1 + *num2));
     if (! array1) {
	  fprintf(stderr, "%s: Error in realloc in add_arrays!\n", whoami);
	  exit(1);
     }
     for (counter = *num1; counter < *num1 + *num2; counter++)
	  array1[counter] = array2[counter - *num1];
     free(array2);
     *num1 += *num2;
     return(array1);
}




filerec *match_pattern(base, file_pattern, num_returned)
filerec base;
char *file_pattern;
int *num_returned;
{
     char search_string[MAXNAMLEN], deleted_search_string[MAXNAMLEN];
     char rest_of_string[MAXPATHLEN];
     filerec *rec_list;
     int num_rec_list;
     DIR *dirp;
     struct direct *dp;
     struct stat stat_buf;
     char realname[MAXPATHLEN], username[MAXPATHLEN];
     
     rec_list = (filerec *) malloc(0); /* so it can be realloc'd later */
     num_rec_list = 0;

     strcpy(search_string, firstpart(file_pattern, rest_of_string));
     strcpy(deleted_search_string, DELETEREPREFIX);
     strcat(deleted_search_string, search_string);
     
     dirp = opendir(base.realname);
     if (! dirp) {
	  *num_returned = num_rec_list;
	  return(rec_list);
     }
     for (dp = readdir(dirp); dp != NULL; dp = readdir(dirp)) {
	  if (is_dotfile(dp->d_name))
	       continue;
	  strcpy(realname, append(base.realname, dp->d_name));
	  printf("checking %s, rest_of_string is %s\n", dp->d_name,
		 rest_of_string);
	  if (lstat(realname, &stat_buf))
	       continue;
	  if (stat_buf.st_mode & S_IFDIR) {
	       re_comp(deleted_search_string);
	       if (re_exec(dp->d_name)) {
		    filerec *retval;
		    int retnum;
		    filerec newbase;
		    
		    strcpy(username, append(base.username, &dp->d_name[2]));
		    newbase.realname = realname;
		    newbase.username = username;
		    retval = match_pattern(newbase, rest_of_string,
					   &retnum);
		    if (retnum != 0) {
			 rec_list = add_arrays(rec_list, &num_rec_list,
					       retval, &retnum);
		    }
	       }
	       else {
		    re_comp(search_string);
		    if (re_exec(dp->d_name)) {
			 filerec *retval;
			 int retnum;
			 filerec newbase;

			 strcpy(username, append(base.username, dp->d_name));
			 newbase.realname = realname;
			 newbase.username = username;
			 retval = match_pattern(newbase, rest_of_string,
						&retnum);
			 if (retnum != 0) {
			      rec_list = add_arrays(rec_list, &num_rec_list,
						    retval, &retnum);
			 }
		    }	
	       }
	  }
	  else {
	       if (*rest_of_string)
		    continue;
	       re_comp(deleted_search_string);
	       if (re_exec(dp->d_name)) {
		    strcpy(username, append(base.username, &dp->d_name[2]));
		    num_rec_list += 1;
		    rec_list = (filerec *) realloc(rec_list, sizeof(filerec) *
						   num_rec_list);
		    
		    rec_list[num_rec_list - 1].realname =
			 malloc(strlen(realname) + 1);
		    strcpy(rec_list[num_rec_list - 1].realname, realname);
		    rec_list[num_rec_list - 1].username =
			 malloc(strlen(username) + 1);
		    strcpy(rec_list[num_rec_list - 1].username, username);
	       }
	  }
     }
     *num_returned = num_rec_list;
     return(rec_list);
}

		    
     


filereccmp(rec1, rec2)
filerec *rec1, *rec2;
{
     return(strcmp(rec1->username, rec2->username));
}


sort_filrecs(data, num_data)
filerec *data;
int num_data;
{
     qsort(data, num_data, sizeof(filerec), filereccmp);
}






char *lastpart(filename)
char *filename;
{
     char *part;

     part = rindex(filename, '/');

     if (! part)
	  part = filename;
     else if (part == filename)
	  part++;
     else if (part - filename + 1 == strlen(filename)) {
	  part = rindex(--part, '/');
	  if (! part)
	       part = filename;
	  else
	       part++;
     }
     else
	  part++;

     return(part);
}





char *firstpart(filename, rest)
char *filename;
char *rest; /* RETURN */
{
     char *part;
     static char buf[MAXPATHLEN];

     strcpy(buf, filename);
     part = index(buf, '/');
     if (! part) {
	  *rest = '\0';
	  return(buf);
     }
     strcpy(rest, part + 1);
     *part = '\0';
     return(buf);
}



is_dotfile(filename)
char *filename;
{
     return (! (strcmp(filename, ".") && strcmp(filename, "..")));
}



/*
 * NOTE: The two string arguments to append must NOT be strings of
 * length zero.  Furthermore, append uses a static array, so its
 * return value must be copied immediately.
 */
char *append(filepath, filename)
char *filepath, *filename;
int print_errors;
{
     static char buf[MAXPATHLEN];

     strcpy(buf, filepath);
     if (buf[strlen(buf) - 1] == '/')
	  buf[strlen(buf) - 1] = '\0';
     if (strlen(buf) + strlen(filename) + 2 > MAXPATHLEN) {
 	  *buf = '\0';
	  return(buf);
     }
     strcat(buf, "/");
     strcat(buf, filename);
     return(buf);
}
