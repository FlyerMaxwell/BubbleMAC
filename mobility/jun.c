#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<time.h>
#include<math.h>
#include"common.h"
#include"files.h"
#include"contact.h"
#include"geometry.h"
 

struct Car 
{
  char name[NAME_LENGTH];

  double *redundency;
  double *redundency_a;

  struct Hashtable friends;
  struct Duallist vectors;
  unsigned long totalContacts;
};

struct Friend
{
  char name[NAME_LENGTH];
  struct Duallist contacts;
};

void friend_init_func(struct Friend *aFriend, char *name)
{
	if(aFriend == NULL) 
		return;
	strncpy(aFriend->name, name, strlen(name)+1);
	duallist_init(&aFriend->contacts);
}


int friend_has_name(char *names, struct Friend *aFriend)
{
	return !strcmp(names, aFriend->name);
}

void friend_free_func(struct Friend *aFriend)
{
	if (aFriend == NULL)
		return;
	duallist_destroy(&aFriend->contacts, NULL);
	free(aFriend);
}

void car_init_func(struct Car *aCar, char *name, int nSlots)
{
	if(aCar == NULL) 
		return;
	strncpy(aCar->name, name, strlen(name)+1);
	aCar->redundency = (double*)malloc(sizeof(double)*(nSlots-1));
	aCar->redundency_a = (double*)malloc(sizeof(double)*(nSlots-1));
	memset(aCar->redundency, 0, sizeof(double)*(nSlots-1));
	memset(aCar->redundency_a, 0, sizeof(double)*(nSlots-1));
  	hashtable_init(&aCar->friends, 1000, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))friend_has_name);
	duallist_init(&aCar->vectors);
	aCar->totalContacts = 0;
}

int car_has_name(char *names, struct Car *aCar)
{
	return !strcmp(names, aCar->name);
}

void car_free_func(struct Car *aCar)
{
	if (aCar == NULL)
		return;
	free(aCar->redundency);
	free(aCar->redundency_a);
  	hashtable_destroy(&aCar->friends, (void(*)(void*))friend_free_func);
	duallist_destroy(&aCar->vectors, free);
	free(aCar);
}


int main( int   argc,
          char *argv[] )
{
  char *fromTime, *toTime;
  FILE *fsource;
  FILE *fdump, *fdump1;
  
  struct Item *aCarItem, *bCarItem, *aFriendItem, *bFriendItem, *prevItem, *aPairItem, *aContactItem, *aVectorItem;

  struct Contact *aContact;
  struct Pair *aPair;
  struct Car *aCar, *bCar;
  struct Friend *aFriend, *bFriend;

  unsigned long *aVector, *currentVector, *prevVector;
  unsigned long *aggVector;

  struct Hashtable globalTable;

  struct Hashtable pairTable;
  unsigned long pairTableSize = 10e6;
  unsigned long k, i, j, l;

  double entropy1, entropy2, jointEntropy;

  int magicNumber, nSlots, slotLength, cGran, slotContacts, filtered, totalFilter, slotFilter;
  time_t startAt, endAt, clock;


  if(argc < 2) {
	printf("Usage: %s -time fromTime toTime slot_length(hours) -filter total_contacts contacts_per_slot -gc relation_extraction_gran(#contacts) .cont ...\n", argv[0]);
	exit(1);
  }

  while(argv[1][0] == '-') {
	switch(argv[1][1]) {
	case 't':
		fromTime = argv[2];
		toTime = argv[3];
		slotLength = atoi(argv[4])*3600;
		argc-=4;
		argv+=4;
		break;

	case 'g':
		if(argv[1][2] == 'c')
			cGran = atoi(argv[2]);
		argc-=2;
		argv+=2;
		break;

	case 'f':
		totalFilter = atoi(argv[2]);
		slotFilter = atoi(argv[3]);
		argc-=3;
		argv+=3;
		break;
	default:
		printf("Usage: %s -time fromTime toTime slot_length(hours) -filter total_contacts contacts_per_slot -gc relation_extraction_gran(#contacts) .cont ...\n", argv[0]);
		exit(1);
	}
  }

  startAt = strtot(fromTime);
  endAt = strtot(toTime);
  
  nSlots = (endAt-startAt)/slotLength;

  hashtable_init(&globalTable, 1000, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))car_has_name);

  //load contacts and set up friends
  while(argc>1) {
	if((fsource=fopen(argv[1], "r"))!=NULL) {
		fscanf(fsource, "%d\n", &magicNumber);
		printf("Loading %s file ...\n", argv[1]);
		if(magicNumber == FILE_CONTACT) {
			hashtable_init(&pairTable, pairTableSize, (unsigned long(*)(void*))sdbm, (int(*)(void*, void*))pair_has_names);
			load_contacts_with_hashtable(fsource, NULL, &pairTable, PAIRWISE_TABLE, NULL, NULL);

			for (k=0;k<pairTable.size;k++) {
				aPairItem = pairTable.head[k];
				while (aPairItem != NULL) {
					aPair = (struct Pair*)aPairItem->datap;

					/* set up the coresponding pair of cars in the global car table */
					aCarItem = hashtable_find(&globalTable, aPair->vName1);
					if(aCarItem == NULL) {
						aCar = (struct Car*)malloc(sizeof(struct Car));
						car_init_func(aCar, aPair->vName1, nSlots);
						hashtable_add(&globalTable, aCar->name, aCar);
					} else {
						aCar = (struct Car*)aCarItem->datap;
					}

					aFriendItem = hashtable_find(&aCar->friends, aPair->vName2);
					if(aFriendItem == NULL) {
						aFriend = (struct Friend*)malloc(sizeof(struct Friend));
						friend_init_func(aFriend, aPair->vName2);
						hashtable_add(&aCar->friends, aPair->vName2, aFriend);
					} else {
						aFriend = (struct Friend*)aFriendItem->datap;
					}

					bCarItem = hashtable_find(&globalTable, aPair->vName2);
					if(bCarItem == NULL) {
						bCar = (struct Car*)malloc(sizeof(struct Car));
						car_init_func(bCar, aPair->vName2, nSlots);
						hashtable_add(&globalTable, bCar->name, bCar);
					} else {
						bCar = (struct Car*)bCarItem->datap;
					}

					bFriendItem = hashtable_find(&bCar->friends, aPair->vName1);
					if(bFriendItem == NULL) {
						bFriend = (struct Friend*)malloc(sizeof(struct Friend));
						friend_init_func(bFriend, aPair->vName1);
						hashtable_add(&bCar->friends, aPair->vName1, bFriend);
					} else {
						bFriend = (struct Friend*)bFriendItem->datap;
					}

					// note that we use datap to store the timestamp of a contact
					aContactItem = aPair->contents.head;
					while(aContactItem) {
						aContact = (struct Contact*)aContactItem->datap;
						duallist_add_to_tail(&aFriend->contacts, (void*)aContact->startAt);
						aCar->totalContacts ++;
					
						duallist_add_to_tail(&bFriend->contacts, (void*)aContact->startAt);
						bCar->totalContacts ++;

						aContactItem = aContactItem->next;
					}

					aPairItem = aPairItem->next;
				}
			}

		} else {
			printf("Wrong file type! File %s has been ignored.\n", argv[1]);
		}
		fclose(fsource);
		hashtable_destroy(&pairTable, (void(*)(void*))pair_free_func);
	}
	argc--;
	argv++;
  }
  // filter out those cars that cannot meet the total number of contacts
  printf("%ld cars loaded. Start to filter those cars have less total contacts ...\n", globalTable.count);
  for (k=0;k<globalTable.size;k++) {
	  aCarItem = globalTable.head[k];
	  while (aCarItem != NULL) {
	  	aCar = (struct Car*)aCarItem->datap;
		if(aCar->totalContacts < totalFilter)
			car_free_func((struct Car*)hashtable_pick(&globalTable, aCar->name));
		aCarItem = aCarItem->next;
	  }
  }
  printf("Now %ld cars left.\n", globalTable.count);

  //set up contact vectors
  printf("Start to setup contact vectors for each car.\n");
  for (k=0;k<globalTable.size;k++) {
	  aCarItem = globalTable.head[k];
	  while (aCarItem != NULL) {
	  	aCar = (struct Car*)aCarItem->datap;
		clock = startAt;
		for (i=0; i<nSlots; i++) {
			aVector = (unsigned long *)malloc(sizeof(unsigned long)*aCar->friends.count);
			memset(aVector, 0, sizeof(unsigned long)*aCar->friends.count);
			duallist_add_to_tail(&aCar->vectors, aVector);
			j = -1;
			for ( l=0;l<aCar->friends.size;l++) {
				aFriendItem = aCar->friends.head[l];
				while(aFriendItem!=NULL) {
					aFriend = (struct Friend*)aFriendItem->datap;
					j ++;
					aContactItem = aFriend->contacts.head;
					while (aContactItem && (time_t)aContactItem->datap <= clock + slotLength) {
						aContactItem = aContactItem->next;
						duallist_pick_head(&aFriend->contacts);
						aVector[j] += 1;
					}
					aFriendItem = aFriendItem->next;
				}
			}
			clock += slotLength;
						

		}
		aCarItem = aCarItem->next;

	  }
  }

  // filter out those cars that cannot meet the contact number of each slot 
  printf("Start to filter those cars have less contacts in each slot ...\n");
  for (k=0;k<globalTable.size;k++) {
	  aCarItem = globalTable.head[k];
	  while (aCarItem != NULL) {
	  	aCar = (struct Car*)aCarItem->datap;
		filtered = 0;
		aVectorItem = aCar->vectors.head;
		while(aVectorItem) {
			aVector = (unsigned long*)aVectorItem->datap;
			slotContacts = 0;
			for ( i=0;i<aCar->friends.count;i++) { 
				slotContacts += aVector[i];
				aVector[i] /= cGran;
			} 
			if (slotContacts < slotFilter) {
				filtered = 1;
				break;
			}
			aVectorItem = aVectorItem->next;
		}
		if (filtered) {
			aCarItem = aCarItem->next;
			car_free_func((struct Car*)hashtable_pick(&globalTable, aCar->name));
		} else
			aCarItem = aCarItem->next;
	  }
  }
  printf("Now %ld cars left.\n", globalTable.count);


  //calculate the redundency 
  printf("Start to calculate redundency of contacts between current slot and history slots.\n");
  for (k=0;k<globalTable.size;k++) {
	  aCarItem = globalTable.head[k];
	  while (aCarItem != NULL) {
		  aCar = (struct Car*)aCarItem->datap;
		  if(aCar->vectors.nItems < nSlots) {
			printf("Car %s has less records, %ld slots recorded\n", aCar->name, aCar->vectors.nItems);
			aCarItem = aCarItem->next;
			continue;
		  }
		  currentVector = (unsigned long*)aCar->vectors.head->prev->datap;
		  entropy1 = vector_entropy(currentVector, aCar->friends.count);
		  aggVector = (unsigned long*)malloc(sizeof(unsigned long)*aCar->friends.count);
		  memset(aggVector, 0, sizeof(unsigned long)*aCar->friends.count); 	
		  prevItem = aCar->vectors.head->prev->prev;
		  for(i=0;i<nSlots-1;i++) {
			if(prevItem == aCar->vectors.head->prev)
				break;
			prevVector = (unsigned long*)prevItem->datap;
			entropy2 = vector_entropy(prevVector, aCar->friends.count);
		  	jointEntropy = vectors_joint_entropy(currentVector, prevVector, aCar->friends.count);
			aCar->redundency[i] = (entropy1+entropy2-jointEntropy)/(entropy1+entropy2);
			for(j = 0; j< aCar->friends.count; j++)
				aggVector[j] = aggVector[j] + prevVector[j];
			entropy2 = vector_entropy(aggVector, aCar->friends.count);
		  	jointEntropy = vectors_joint_entropy(currentVector, aggVector, aCar->friends.count);
			aCar->redundency_a[i] = (entropy1+entropy2-jointEntropy)/(entropy1+entropy2);
			prevItem = prevItem->prev;
		  }
		  aCarItem = aCarItem->next;
	  }
  }


  fdump = fopen("redundency.txt", "w");
  fdump1 = fopen("redundency_a.txt", "w");
  for (k=0;k<globalTable.size;k++) {
	aCarItem = globalTable.head[k];
	while (aCarItem != NULL) {
		  aCar = (struct Car*)aCarItem->datap;
		  fprintf(fdump, "%s:", aCar->name);
		  fprintf(fdump1, "%s:", aCar->name);
		  for (i=0;i<nSlots-1;i++) {
			  fprintf(fdump, "%.4lf ", aCar->redundency[i]);
			  fprintf(fdump1, "%.4lf ", aCar->redundency_a[i]);
		  }
		  fprintf(fdump, "\n");
		  fprintf(fdump1, "\n");
		  aCarItem = aCarItem->next;
	}
  }
  fclose(fdump);
  fclose(fdump1);

  hashtable_destroy(&globalTable, (void(*)(void*))car_free_func);
  return 0;
}
