/*
 *     Campus Cluster Monitering Program
 *       John Welch and Chris Vanharen
 *            MIT's Project Athena
 *                Summer 1990
 */

#include "xcluster.h"		/* global include file. */
#include "net.h"

/*
 * GLOBAL VARIABLES
 */
struct cluster *cluster_list;	/* Pointer to list of clusters */
static char *config[] = {"configplease2"};
char machtypes[10][25];
int num_machtypes;

/*
 *   read_clusters reads the cluster names and locations in from a file and
 *   puts them into a linked list called cluster_list.
 */
void read_clusters()
{
  int num, trip=0, i;
  struct cluster *new, *current = NULL;
  FILE *f;
  int s;

  s = net(progname, 1, config);
  if (s < 1)
    {
      fprintf(stderr,
	      "Error while contacting server, trying to get configuration information.\n");
      exit(-1);
    }

/* read data from socket */

  f = fdopen(s, "r");

  fscanf(f, "%d", &num_machtypes);
  for (i=1; i <= num_machtypes; i++)
    {
      fscanf(f, "%s", machtypes[i]);
    }

  cluster_list = (struct cluster *)malloc ((unsigned)sizeof(struct cluster));
  while(fscanf(f, "%d", &num) != EOF)
    {
      if (trip == 1)
        new = (struct cluster *) malloc ((unsigned) sizeof (struct cluster));
      else
        new = cluster_list;
      if (new == NULL)
        {
          fprintf (stderr, "%s: Out of memory.\n", progname);
          exit (-1);
        }
      new->cluster_number = num;

      fgets(new->button_name, 25, f);

      for (i=0; TRUE; i++)
	{
	  fscanf(f, "%s", new->cluster_names[i]);
	  if(!strcmp(new->cluster_names[i], "XXXXX"))
	    break;
	}

      fscanf(f, "%d%d", &new->x_coord, &new->y_coord);

      for (i=0; TRUE; i++)
	{
	  fscanf(f, "%s", new->prntr_name[i]);
	  strcpy(new->prntr_current_status[i], "?");
	  strcpy(new->prntr_num_jobs[i], "0");
	  if(!strcmp(new->prntr_name[i], "XXXXX"))
	    break;
	}
      new->next = NULL;
      if (trip == 1)
        current->next = new;
      current = new;
      trip = 1;
    }
  (void)fclose(f);
}
