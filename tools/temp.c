#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>
#include"geometry.h"

struct My_record
{
	unsigned long sec;
	unsigned long susec;
	double ax;
	double ay;
	double az;
	double lax;
	double lay;
	double laz;
	double dz;
	double dx;
	double dy;
	double ndz;
	double ndx;
	double ndy;
	double x;
	double y;
	double speed;
};

struct Speed
{
	unsigned long sec;
	unsigned long susec;
	double x;
	double y;
	double speed;
};

int main(int argc, char**argv)
{
	FILE *fp;
	char buf[256], *strp;
	int hour, minute, second, milisec;
	struct My_record *newr, *aRecord, *bRecord;
	struct Speed *news, *aspeed, *bspeed;
	struct Duallist records, speeds;
	double x, y;
	int n;
	struct Item *aItem, *bItem;

	duallist_init(&records);
	duallist_init(&speeds);
	if( (fp=fopen(argv[1], "r"))!= NULL) {
		memset(buf, 0, 256);
		while(fgets(buf, 256, fp)) {
			newr=(struct My_record*)malloc(sizeof(struct My_record));
			strp = strtok(buf, "-");
			strp = strtok(NULL, "-");
			strp = strtok(NULL, "-");
			strp = strtok(NULL, ":");
			hour = atoi(strp);
			strp = strtok(NULL, ":");
			minute = atoi(strp);
			strp = strtok(NULL, ".");
			second = atoi(strp);
			newr->sec = hour*3600+minute*60+second;
			strp = strtok(NULL, "\t");
			milisec = atoi(strp);
			newr->susec = (newr->sec*1000 + milisec)/10*10;
			strp = strtok(NULL, " ");
			newr->ax = atof(strp);
			strp = strtok(NULL, " ");
			newr->ay = atof(strp);
			strp = strtok(NULL, " ");
			newr->az = atof(strp);
			strp = strtok(NULL, " ");
			newr->lax = atof(strp);
			strp = strtok(NULL, " ");
			newr->lay = atof(strp);
			strp = strtok(NULL, " ");
			newr->laz = atof(strp);
			strp = strtok(NULL, " ");
			newr->dz = 90-atof(strp);
			strp = strtok(NULL, " ");
			newr->dx = atof(strp);
			strp = strtok(NULL, " ");
			newr->dy = atof(strp);
			strp = strtok(NULL, " ");
			newr->ndz = 90-atof(strp);
			strp = strtok(NULL, " ");
			newr->ndx = atof(strp);
			strp = strtok(NULL, " ");
			newr->ndy = atof(strp);
			newr->x = 0.5;
			newr->y = 0.5;
			newr->speed = 0;
			memset(buf, 0, 256);
			duallist_add_to_tail(&records, newr);
		}
		fclose(fp);
	}
	if( (fp=fopen(argv[2], "r"))!= NULL) {
		memset(buf, 0, 256);
		while(fgets(buf, 256, fp)) {
			news=(struct Speed*)malloc(sizeof(struct Speed));
			strp = strtok(buf, "-");
			strp = strtok(NULL, "-");
			strp = strtok(NULL, "-");
			strp = strtok(NULL, ":");
			hour = atoi(strp);
			strp = strtok(NULL, ":");
			minute = atoi(strp);
			strp = strtok(NULL, ".");
			second = atoi(strp);
			news->sec = hour*3600+minute*60+second;
			strp = strtok(NULL, "\t");
			milisec = atoi(strp);
			news->susec = (news->sec*1000 + milisec)/10*10;
			strp = strtok(NULL, " ");
			strp = strtok(NULL, " ");
			news->y = atof(strp);
			strp = strtok(NULL, " ");
			news->x = atof(strp);
			news->speed = 0;
			memset(buf, 0, 256);
			if(news->x == 0)
				free(news);
			else
				duallist_add_to_tail(&speeds, news);
		}
		fclose(fp);
	}

	aItem = speeds.head;
	while (aItem && aItem->next) {
		aspeed = (struct Speed*)aItem->datap;
		bspeed = (struct Speed*)aItem->next->datap;
		x = aspeed->x;
		y = aspeed->y;
		n = 1;
		while (aspeed->sec == bspeed->sec) {
			x += bspeed->x;
			y += bspeed->y;
			n ++;
			duallist_pick_item(&speeds, aItem->next);
			if(aItem->next) 
				bspeed = (struct Speed*)aItem->next->datap;
			else
				break;
		}
		aspeed->x = x/n;
		aspeed->y = y/n;
		aItem = aItem->next;
	}

	aItem = speeds.head;
	while (aItem && aItem->next) {
		aspeed = (struct Speed*)aItem->datap;
		bspeed = (struct Speed*)aItem->next->datap;
		aspeed->speed = distance_in_meter(aspeed->x, aspeed->y, bspeed->x, bspeed->y)/(bspeed->sec-aspeed->sec);
		aItem = aItem->next;
	}

	aItem = speeds.head;
	bItem = records.head;
	while (aItem && bItem) {
		aRecord = (struct My_record*)bItem->datap;
		aspeed = (struct Speed*)aItem->datap;
		while (aRecord->sec < aspeed->sec) {
			bItem = bItem->next;
			if (bItem)
				aRecord = (struct My_record*)bItem->datap;
			else
				break;
		}
		while (aRecord->sec == aspeed->sec) {
			aRecord->speed = aspeed->speed;
			bItem = bItem->next;
			if (bItem)
				aRecord = (struct My_record*)bItem->datap;
			else
				break;
		}

		aItem = aItem->next;
	}

	aItem = records.head;
	while (aItem && aItem->next) {
		aRecord = (struct My_record*)aItem->datap;
		bRecord = (struct My_record*)aItem->next->datap;
		bRecord->x = aRecord->x + distance_in_latitude(cos(M_PI*aRecord->ndz/180)*aRecord->speed*(bRecord->susec-aRecord->susec)/1000);
		bRecord->y = aRecord->y + distance_in_latitude(sin(M_PI*aRecord->ndz/180)*aRecord->speed*(bRecord->susec-aRecord->susec)/1000);
		printf("t001,2007-01-01 %2ld:%2ld:%2ld,%.5lf,%.5lf,%2.0lf,%2.0lf,0\n", bRecord->sec/3600, bRecord->sec%3600/60, bRecord->sec%3600%60, bRecord->x, bRecord->y,bRecord->speed, bRecord->ndz);
		aItem = aItem->next;
	}

	return 0;
}

