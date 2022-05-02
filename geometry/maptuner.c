#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include "common.h" 
#include "geometry.h"


int main(int argc, char **argv) 
{
  double scrapLength = -1;
  double cellSize = -1;
  char *regnPoints = NULL;

  char *districtFile = NULL;
  char *riverFile = NULL;
  char *roadFile = NULL;
  char *roadAttrFile = NULL;

  char *smapfile = NULL;
  char *omapfile = NULL;

  char *editfile = NULL;

  FILE *fDistrict, *fRiver, *fRoad;
  FILE *fTmp = NULL;
  FILE *fOutput;

  struct Polygon *chosen_polygon = NULL;
  struct Region *region = NULL;

  struct Point point;

//若未输入参数，给出提示
  if(argc < 2) {
	printf("Usage: %s [-r \"x1 y1 x2 y2 x3 y3...\"] [-j scrapLength length]  [-c cell size] [-s districts.shp rivers.shp roads.shp roads.txt | -m source.map] [-e editmap.txt] output.map\n", argv[0]);
	exit(1);
  }
//将用户输入的参数保存起来
  while(argc>1 && (argv[1][0])=='-' ) {
    switch ( argv[1][1]) {
      case 'r':
	regnPoints = argv[2];
        argc-=2;
        argv+=2;
        break;

      case 'j':
        scrapLength = atof(argv[2]);
        argc-=2;
        argv+=2;
        break;

      case 'c':
        cellSize = atof(argv[2]);
        argc-=2;
        argv+=2;
        break;

      case 's':
	districtFile = argv[2];
	riverFile = argv[3];
	roadFile = argv[4];
	roadAttrFile = argv[5];
	argc-=5;
	argv+=5;
        break;

      case 'm':
	smapfile = argv[2];
        argc-=2;
        argv+=2;
        break;

      case 'e':
	editfile = argv[2];
        argc-=2;
        argv+=2;
        break;

      default:
        printf("Bad option %s\n", argv[1]);
	printf("Usage: %s [-r \"x1 y1 x2 y2 x3 y3...\"] [-j scrapLength length]  [-c cell size] [-s districts.shp rivers.shp roads.shp roads.txt | -m source.map] [-e editmap.txt] output.map\n", argv[0]);
        exit(1);
      }
  }


//这里做一个判断，如果参数只有1，即用户未输入参数，则使用默认地图
  if(argc > 1) 
	omapfile = argv[1];
  else
	omapfile = "default.map";


//画出用户指定的多边形区域
  if(regnPoints != NULL) {
	double x, y;
	char *px, *py;

	px = strtok(regnPoints, " ");
	py = strtok(NULL, " ");
	x = atof(px);
	y = atof(py);
	point.x = x, point.y = y;
	build_polygon(&chosen_polygon, &point);
	while(1) {	
		px = strtok(NULL, " ");
		if(px==NULL) break;
		py = strtok(NULL, " ");
		x = atof(px);
		y = atof(py);
		point.x = x, point.y = y;
		build_polygon(&chosen_polygon, &point);
	}
	close_polygon(chosen_polygon);
  }


  if(districtFile != NULL || riverFile != NULL || roadFile != NULL) {

//将fTem指向一个文件（依次检查Road、District，River）
	if((fRoad=fopen(roadFile, "r"))!=NULL) {           			//如果顺利打开Road地图文件
	      fTmp = fRoad;											//将fTmp指向指定的Road地图文件
	} else if((fDistrict=fopen(districtFile, "r"))!=NULL){
	      fTmp = fDistrict;
	} else if((fRiver=fopen(riverFile, "r"))!=NULL)  {
	      fTmp = fRiver;
	}

//如果没有选择区域，并且选择了地图文件，那么给定一个默认的四边形区域
	if(chosen_polygon==NULL && fTmp != NULL) {
	      double xmin, xmax, ymin, ymax;
	      fseek(fTmp, 36, SEEK_SET);
	      fread(&xmin, sizeof(double), 1, fTmp);    //取出xmin,ymin,xmax,ymax 在文件开始36个字符处依次取出
	      fread(&ymin, sizeof(double), 1, fTmp);
	      fread(&xmax, sizeof(double), 1, fTmp);
	      fread(&ymax, sizeof(double), 1, fTmp);
	      fclose(fTmp);
	      point.x = xmin, point.y = ymin;
	      build_polygon(&chosen_polygon, &point);	//以这四个点画出多边形
	      point.x = xmin, point.y = ymax;
	      build_polygon(&chosen_polygon, &point);
	      point.x = xmax, point.y = ymax;
	      build_polygon(&chosen_polygon, &point);
	      point.x = xmax, point.y = ymin;
	      build_polygon(&chosen_polygon, &point);
	      close_polygon(chosen_polygon);
	}


	region = build_geographical_region(districtFile, riverFile, roadFile, roadAttrFile, chosen_polygon, cellSize==-1?DEFAULT_CELLSIZE:cellSize);

  } else if (smapfile != NULL) {
  	if((fTmp=fopen(smapfile, "rb"))!=NULL) {
		region = region_load_func(fTmp, chosen_polygon, cellSize);
		fclose(fTmp);
		if(region == NULL) {
			printf("Load .map failed.\n");
			return -2;
		}
	}
  }


  if(editfile != NULL && region != NULL) {
	if((fTmp = fopen(editfile, "r"))!=NULL) {
		edit_region(fTmp, region);
		check_max_degree(region); 
		fclose(fTmp);
	}
  } 


  if(region) {
	printf("Dumping region to .map file.\n");
	if((fOutput=fopen(omapfile, "wb"))== NULL) {
	      printf("Cannot open file %s to write!\n", omapfile);
	      return -1;
	}
	region_dump_func(fOutput, region);
	printf("Summary: #crosses: %6ld, #roads: %6ld, maxdgr: %2d\n", region->crosses.nItems, region->roads.nItems, region->maxdgr);
	region_free_func(region);
	fclose(fOutput);

  } else {
	printf("Got a NULL region. No .map file is created.\n");
  }

  return 0;
}
