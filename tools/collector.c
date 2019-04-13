#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"common.h"

#define BUFF_SIZE 32768

struct Node 
{
  char name[32];
  double aValue;
  struct Duallist values;  
};

void node_init_func(struct Node *aNode)
{
	memset(aNode->name, 0,32);
	duallist_init(&aNode->values);
}

void node_free_func(struct Node *aNode)
{
	duallist_destroy(&aNode->values, free);
	free(aNode);
}

int node_has_name(char *name, struct Node *aNode)
{
	return !strncmp(name, aNode->name, 32);
}

void mount_a_file(FILE *fp, struct Duallist *collection, char *dchars, int files);

int main(int argc, char**argv)
{
	FILE *fp;
	char *dchars = " \n", *output= NULL;
	int files;

	struct Duallist collection;
	struct Item *aNodeItem, *aValueItem;
	struct Node *aNode;

	if(argc < 2) {
		printf("Usage: %s [-d cutchar] [-o output] file1 file2 ...\n", argv[0]);
		exit(1);
	}

	while(argv[1][0] == '-') {
		switch(argv[1][1]) {
		case 'd':
			dchars = argv[2];
			argc-=2;
			argv+=2;
			break;
		case 'o':
			output = argv[2];
			argc-=2;
			argv+=2;
			break;
			
		}
	}


	duallist_init(&collection);

	files = 0;
	while(argc>1) {
		if( (fp=fopen(argv[1], "r"))!= NULL) {
			files ++;
			printf("loading %s ...\n", argv[1]);
			mount_a_file(fp, &collection, dchars, files);
			fclose(fp);
		}
		argc--;
		argv++;
	}

	fp = fopen(output, "w");
	aNodeItem = collection.head;
	while (aNodeItem != NULL) {
		aNode = (struct Node*)aNodeItem->datap;
		if(fp)
			fprintf(fp, "%s ", aNode->name);
		else
			printf("%s ", aNode->name);
		aValueItem = aNode->values.head;
		while(aValueItem) {
			if (fp)
				fprintf(fp, "%.2lf ", *((double*)aValueItem->datap));
			else
				printf("%.2lf ", *((double*)aValueItem->datap));
			aValueItem = aValueItem->next;
		}
		if(fp)
			fprintf(fp, "\n");
		else
			printf("\n");
		aNodeItem = aNodeItem->next;
	}
	duallist_destroy(&collection, (void(*)(void*))node_free_func);
	return 0;
}

void mount_a_file(FILE *fp, struct Duallist *collection, char *dchars, int files)
{
	struct Item *aNodeItem, *aItem;
	char buf[BUFF_SIZE], *strp;
	struct Hashtable aTable;
	struct Node *aNode, *newNode;
	unsigned long i;
	int j;
	double *aValue;


	hashtable_init(&aTable, 2000, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))node_has_name);
	memset(buf, 0, BUFF_SIZE);
	while(fgets(buf, BUFF_SIZE, fp)) {
		aNode = (struct Node*)malloc(sizeof(struct Node));
		node_init_func(aNode);
		strp = strtok(buf, dchars);
		strncpy(aNode->name, strp, 32);
		strp = strtok(NULL, dchars);
		aNode->aValue = atof(strp);
		hashtable_add(&aTable, aNode->name, aNode);
		memset(buf, 0, BUFF_SIZE);
	}

	aNodeItem = collection->head;
	while(aNodeItem) {
		aNode = (struct Node*)aNodeItem->datap;
		aItem = hashtable_find(&aTable, aNode->name);
		if(aItem) {
			aValue = (double*)malloc(sizeof(double));
			*aValue = ((struct Node*)aItem->datap)->aValue;
			duallist_add_to_tail(&aNode->values, aValue);
			node_free_func(hashtable_pick(&aTable, aNode->name));
		} else {
			aValue = (double*)malloc(sizeof(double));
			*aValue = 0;
			duallist_add_to_tail(&aNode->values, aValue);
		}
		aNodeItem = aNodeItem->next;
	} 

	for (i=0;i<aTable.size;i++) {
		aNodeItem = aTable.head[i];
		while (aNodeItem != NULL) {
			aNode = (struct Node*)aNodeItem->datap;
			newNode = (struct Node*)malloc(sizeof(struct Node));
			node_init_func(newNode);
			strncpy(newNode->name, aNode->name, 32);
			for(j=0;j<files-1;j++) {
				aValue = (double*)malloc(sizeof(double));
				*aValue = 0;
				duallist_add_to_tail(&newNode->values, aValue);
			}
			aValue = (double*)malloc(sizeof(double));
			*aValue = aNode->aValue;
			duallist_add_to_tail(&newNode->values, aValue);
			duallist_add_to_tail(collection, newNode);
			aNodeItem = aNodeItem->next;
		}
	}
	hashtable_destroy(&aTable, (void(*)(void*))node_free_func);
}
