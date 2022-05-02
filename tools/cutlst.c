#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include"common.h"
#include"files.h"

char* filename (char* basename)
{
    char *ptr = strrchr (basename, '.');
    if (ptr)
	*ptr = '\0';
    
    return (char*)basename;
}

int main(int argc, char**argv)
{
  FILE *fsource, *fdump;
  char buf[1024], fname[1024], *strp;
  struct Duallist aDuallist;
  int nItems=0, k;
  unsigned long count;

  if(argc < 3) {
	printf("Usage: %s .lst nItems\n", argv[0]);
	exit(1);
  }
	
  
  
  if((fsource=fopen(argv[1], "r"))!=NULL) {
	  duallist_init(&aDuallist);
	  fgets(buf, 1024, fsource);
	  while (fgets(buf, 1024, fsource))
	  {
		strp = (char*)malloc(sizeof(char)*1024);
		strncpy(strp, buf, strlen(buf)+1);
		duallist_add_to_tail(&aDuallist, strp);
	  }
  }
  fclose(fsource);

  nItems = atoi(argv[2]);
 
  if(nItems) {
	  k = 1;
	  count = 0;
	  sprintf(fname, "%s_%d.lst", filename(argv[1]), k);
	  fdump=fopen(fname, "w");  
	  fprintf(fdump,"%d\n", FILE_LIST);
	  while(aDuallist.nItems) {
	        strp = (char*)duallist_pick_head(&aDuallist);
	        fprintf(fdump, "%s", strp);
	        free(strp);
		count +=1;

		if (count%nItems == 0 && aDuallist.nItems) {
			fclose(fdump);
			k +=1;
			sprintf(fname, "%s_%d.lst", argv[1], k);
			fdump=fopen(fname, "w");  
			fprintf(fdump,"%d\n", FILE_LIST);
		}
	
	  }
	  fclose(fdump);
  }

  duallist_destroy(&aDuallist, free);

  return 0;
}
