#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include"common.h"

struct VidCount
{
  char id[256];
  int count;
};

int count_has_id(char *name, struct VidCount *aVid)
{
	return !strncmp(name, aVid->id, strlen(name));
}

char* basename (char* path)
{
    char *ptr = strrchr (path, '/');
    
    return ptr ? ptr + 1 : (char*)path;
}

char* filename (char* basename)
{
    char *ptr = strrchr (basename, '.');
    if (ptr)
	*ptr = '\0';
    
    return (char*)basename;
}

int main(int argc, char**argv)
{
  FILE *fsource;
  char buf[128], *key;
  struct Hashtable vidTable;
  struct VidCount *aVid;
  struct Item *aItem; 
  char *strp = NULL, *extp = NULL, *ptr;
  int total=0, i, fcomplete = 0, uncommon = 0, omitext = 0, magicNumber = -1;

  if(argc < 2) {
	printf("Usage: %s [-fullset] [-m magicnumber] [-uncommon] [-omitext] [-e ext] [-p prefix] .lst ... \n", argv[0]);
	exit(1);
  }
	
  
  while(argv[1][0] == '-') {
	  switch(argv[1][1]) {
	  case 'm':
		  magicNumber = atoi(argv[2]);
		  argc-=2;
		  argv+=2;
		  break;
	  case 'f':
		  fcomplete = 1;
		  argc-=1;
		  argv+=1;
		  break;
	  case 'u':
		  uncommon = 1;
		  argc-=1;
		  argv+=1;
		  break;
	  case 'o':
		  omitext = 1;
		  argc-=1;
		  argv+=1;
		  break;
	  case 'e':
		  extp = argv[2];
		  argc-=2;
		  argv+=2;
		  break;
	  case 'p':
		  strp = argv[2];
		  argc-=2;
		  argv+=2;
		  break;
	  }
  }

  hashtable_init(&vidTable, 3000, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))count_has_id);
  while(argc > 1) {
	if((fsource=fopen(argv[1], "r"))!=NULL) {
		fgets(buf, 127, fsource);
		while (fgets(buf, 127, fsource))
		{
			key = basename(buf);
			ptr = strrchr(key, '\n');
			if(ptr)
				*ptr = 0;
			if(omitext)
				key = filename(key);
			aItem = hashtable_find(&vidTable, key);
			if(aItem == NULL) {
				aVid = (struct VidCount*)malloc(sizeof(struct VidCount));
				strncpy(aVid->id, key, strlen(key)+1);
				aVid->count = 1;
				hashtable_add(&vidTable, key, aVid);
			} else {
				aVid = (struct VidCount*)aItem->datap;
				aVid->count ++;
			}
		}
	}
	fclose(fsource);
	total ++;
	argc --;
	argv ++;
  }

  if(magicNumber != -1) 
	printf("%d\n", magicNumber);

  for (i=0;i<vidTable.size;i++) {
	aItem = vidTable.head[i];
	while (aItem != NULL) {
		aVid = (struct VidCount*)aItem->datap;
		if(fcomplete == 0 && uncommon == 0 && aVid->count == total) {
			if(strp==NULL && extp == NULL)
				printf("%s\n", aVid->id);
			else if(strp==NULL && extp )
				printf("%s.%s\n", aVid->id, extp);
			else if(strp && extp == NULL)
				printf("%s%s\n", strp, aVid->id);
			else if(strp && extp )
				printf("%s%s.%s\n", strp, aVid->id, extp);
		} else if (fcomplete == 0 && uncommon == 1 && aVid->count != total) {
			if(strp==NULL && extp == NULL)
				printf("%s\n", aVid->id);
			else if(strp==NULL && extp )
				printf("%s.%s\n", aVid->id, extp);
			else if(strp && extp == NULL)
				printf("%s%s\n", strp, aVid->id);
			else if(strp && extp )
				printf("%s%s.%s\n", strp, aVid->id, extp);
		} else if(fcomplete) {
			if(strp==NULL && extp == NULL)
				printf("%s\n", aVid->id);
			else if(strp==NULL && extp )
				printf("%s.%s\n", aVid->id, extp);
			else if(strp && extp == NULL)
				printf("%s%s\n", strp, aVid->id);
			else if(strp && extp )
				printf("%s%s.%s\n", strp, aVid->id, extp);
		}
		aItem = aItem->next;
	}
  }
  
  hashtable_destroy(&vidTable, free);
  return 0;
}
