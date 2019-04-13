//通用头文件
#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<time.h>

struct Simulator
{
  // experiment region
  struct Region *region;

  // bus routes
  struct Hashtable routes;

  // vehicles and static storage nodes
  struct Hashtable vnodes;
  struct Hashtable snodes;

  // pairwise contacts
  struct Hashtable cntTable;

  // pairwise icts
  struct Hashtable ictTable;
  
  // oracle
  struct Oracle *oracle;

  // packets traversing and recieved in the simulation
  struct Duallist pkgs;
  unsigned long sentPkgs;
  // traffic generator
  struct TrafficGenerator *trafficGenerator;
  struct Hashtable destinations;

  // driven events
  struct Duallist *eventSlots;
  time_t slotSize;
  unsigned long eventNums;

  // replication control
  int fwdMethod;

  // expr time setting
  time_t exprStartAt;
  time_t exprEndAt;
  time_t clock;

  unsigned int bufSize;

  unsigned int pkgSize;
  int pkgTTL;
  int pkgRcdRoute;

  unsigned long trafficCount;

  // similarity filename
  char *similarityFile;

  unsigned long paintedNodes;
  unsigned long nPaints;
};


int main(){
	
	time_t Starttime=0, Endtime=0		//起始时间
	char *pkgdumpfile = NULL;			//文件输出
	int magicNumber;


	struct Simulator *aSim;				//定义Simulator类型的指针
    srand(randseed);
    aSim = (struct Simulator*)malloc(sizeof(struct Simulator));//定义一个指向Simulator的指针
   
    simulator_init_func(aSim, starttime, endtime, 10);//Simulator初始化


/************将原始的GPS数据信息变为哈希表存储**************/

	if((fsource=fopen(argv[1], "r"))!=NULL) {
		fscanf(fsource, "%d\n", &magicNumber);
		printf("Loading %s file ...\n", argv[1]);
		if(magicNumber == FILE_ORIGINAL_GPS_TAXI) {
			startAt = 0;
			endAt = 0;
			load_contacts_with_hashtable(fsource, NULL, &aSim->vnodes, &startAt, &endAt);
		}
		fclose(fsource);
	}






}