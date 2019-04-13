#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<math.h>
#include"common.h"
#include"files.h"
#include"contact.h"
#include"geometry.h"


int main( int   argc,
          char *argv[] )
{
  char *graphfile=NULL, *strp1, *strp2, buf[128];
  FILE *fsource, *fdump;
  
  unsigned long pairTableSize = 10e6;
  struct Hashtable pairTable;
  struct Item *aPairItem;
  struct Pair *aPair;
  struct Item *aContactItem;
  struct Contact *aContact;

  int checkSize = 30 * 60; // half an hour
  double ratio = 0.95;

  int *counts;
  unsigned long nItems, zeros, total;

  unsigned long k,i;
  time_t fromtime = 0, totime = 0;

  if(argc < 2) {
	printf("Usage: %s [-s check_window_size(min)] [-r required_ratio(0-1)] [-w graphfile] fromtime totime .cont|.lst ...\n", argv[0]);
	exit(1);
  }
  while(argv[1][0] == '-') {
	switch(argv[1][1]) {
	case 's':
		checkSize = atoi(argv[2])*60;
		argc-=2;
		argv+=2;
		break;


	case 'r':
		ratio = atof(argv[2]);
		argc-=2;
		argv+=2;
		break;

	case 'w':
		graphfile = argv[2];
		argc-=2;
		argv+=2;
		break;
		
	default:
		printf("Usage: %s [-s check_window_size(min)] [-r required_ratio(0-1)] [-w graphfile] fromtime totime .cont|.lst ...\n", argv[0]);
		exit(1);
	}
  }

  hashtable_init(&pairTable, pairTableSize, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))pair_has_names);
  strp1 = argv[1];
  strp2 = argv[2];
  argc-=2;
  argv+=2;

  if(graphfile == NULL) {
	sprintf(buf, "from%sto%s_window%dmin_ratio%.0f%%.edge", strp1, strp2, checkSize/60, ratio*100);
	graphfile = buf;
  }

  while(argc>1) {
	if((fsource=fopen(argv[1], "r"))!=NULL) {
		printf("Loading %s file ...\n", argv[1]);
		load_source_file(fsource, NULL, NULL, NULL, NULL, NULL, &pairTable, PAIRWISE_TABLE, (void*(*)(FILE *, struct Region*, void *, int, time_t *, time_t *))load_contacts_with_hashtable, NULL, NULL,NULL,NULL,NULL,NULL, NULL, NULL, NULL);
		fclose(fsource);

	}
	argc--;
	argv++;
  }

  if((fdump=fopen(graphfile, "w"))!=NULL) {
  	  fromtime = strtot(strp1);
	  totime = strtot(strp2);

	  printf("Abstracting edges saved in file %s...\n", graphfile);
	  for (k=0;k<pairTable.size;k++) {
		  aPairItem = pairTable.head[k];
		  while (aPairItem != NULL) {
			  aPair = (struct Pair*)aPairItem->datap;	
			  nItems = ceil((totime - fromtime)*1.0/checkSize);
			  counts = (int*)malloc(sizeof(int)*nItems);
			  memset(counts, 0, sizeof(int)*nItems);
			  aContactItem = aPair->contents.head;
			  while(aContactItem!=NULL) {
				  aContact = (struct Contact*)aContactItem->datap;
			
				  if(aContact->startAt > totime)
					  break;
				  if(aContact->startAt >= fromtime)
					  counts[(aContact->startAt - fromtime)/checkSize] ++;

				  aContactItem = aContactItem->next;
			  }
			  zeros = 0;
			  total = 0;
			  for(i=0;i<nItems;i++) {
				  if (counts[i] == 0) zeros ++;
				  total += counts[i];
			  }
			  if(1 - zeros*1.0/nItems >= ratio) 
				  fprintf(fdump, "%s %s %.2f\n", aPair->vName1, aPair->vName2, total*1.0/nItems);
			  free(counts);
			  aPairItem = aPairItem->next;
		  }
	  }		
	  fclose(fdump);
  }

  hashtable_destroy(&pairTable, (void(*)(void*))pair_free_func);
  return 0;
}
