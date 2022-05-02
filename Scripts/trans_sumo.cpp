#include <stdio.h>
#include <iostream>
using namespace std;

/*
脚本：将SUMO导出的数据格式，转换为和先前一致的格式，以使得SUMO仿真和之前的MAC仿真对接起来
输入：fin输入文件路径 ；fout输出文件路径 （修改这俩文件名即可）
注意：每次更改了格式就重新修改一下fsanf和fprint即可
*/

int main()
{
   FILE *fin = NULL;
   char buff[255];
    fin = fopen("/media/lion/Elements SE/Maxwell/BubbleMAC/SumoSimu/MyCross/DemoCross.csv", "r");
    int timestep;
    char id[20];
    double x;
    double y;
    double angle;
    char type[15];
    double speed;
    double pos;
    char lane[10];
    double slope;
    double flow;
    double speed2;
    double fo;
    char file_path[100];

    while(fscanf(fin, "%d", &timestep)!=-1){
        fscanf(fin, "%s", id);
        fscanf(fin, "%lf", &x);
        fscanf(fin, "%lf", &y);
        fscanf(fin, "%lf", &angle);
        fscanf(fin, "%s", type);
        fscanf(fin, "%lf", &speed);
        fscanf(fin, "%lf", &pos);
        fscanf(fin, "%s", lane);
        fscanf(fin, "%lf", &slope);
        fscanf(fin, "%lf", &flow);
        fscanf(fin, "%lf", &speed2);

        sprintf(file_path, "/media/lion/Elements SE/Maxwell/BubbleMAC/SumoSimu/MyCross/transformed/carposition_%d.txt", timestep); // slot number
        FILE* fout = fopen (file_path,"a+");
        fprintf(fout, "%d %s %lf %lf %lf %s %lf %lf %s %lf %lf %lf\n",timestep,id, x, y, angle, type, speed, pos, lane, slope, flow, speed2);
        fclose (fout);

        //cout<<" slot= "<<timestep<<" id= "<<id<<" x= "<<x<<" y="<<y<<" angle= "<<angle<<" type= "<<type<<" speed= "<<speed<<" pos= "<<pos<<" lane= "<<lane<<" slope= "<<slope<<" flow= "<<flow<<" speed2= "<<speed2<<endl;
    }

}