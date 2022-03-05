#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <queue>

#include "common.h" 
#include "geometry.h"
#include "trace.h"
#include "blossom.h"

#define false 0
#define true 1
#define N 800

struct Edge {
    int to,next;
} edge[N*N*2];

int head[N],tot;
int n;//n个点
int father[N],pre[N];//father记录一个点属于哪个一个点为根的花
int Q[N*N*2],first,tail;//bfs队列
int match[N];//匹配
int odd[N],vis[N];//odd记录一个点为奇点/偶点，1为奇，0为偶
int timeBlock;//LCA时间戳
int top[N],rinedge[N];
 
void addEdge(int x,int y) {//添边
    edge[tot].to=y;
    edge[tot].next=head[x];
    head[x]=tot++;
}
int Find(int x){//并查集寻找根节点
    if(father[x]!=x)
        return father[x]=Find(father[x]);
    return x;
}
int lca(int x, int y){//求解最近公共祖先
    timeBlock++;
    while(x){
        rinedge[x]=timeBlock;
        x=Find(top[x]);
    }
    x=y;
    while(rinedge[x]!=timeBlock)
        x=Find(top[x]);
    return x;
}
void blossom(int x, int y, int k) {//将奇环缩成一个点并将原来是奇点的点变为偶点并加入队列
    while(Find(x)!=Find(k)){
        pre[x]=y;
 
        y=match[x];
        odd[y]=false;
        Q[tail++]=y;
 
        father[Find(x)]=k;
        father[Find(y)]=k;
 
        x=pre[y];
    }
}
int bfs(int s) {
    memset(top,0,sizeof(top));
    memset(pre,0,sizeof(pre));
    memset(odd,false,sizeof(odd));
    memset(vis,false,sizeof(vis));
    for(int i=1;i<=n;i++)
        father[i]=i;
 
    vis[s]=true;
    first=tail=0;
    Q[tail++]=s;
 
    while(first!=tail){
        int now=Q[first++];
        for(int i=head[now];i!=-1;i=edge[i].next){
            int to=edge[i].to;
            if(!vis[to]){
                top[to]=now;
                pre[to]=now;
                odd[to]=true;
                vis[to]=true;
 
                if(!match[to]){
                    int j=to;
                    while(j){
                        int x=pre[j];
                        int y=match[x];
                        match[j]=x;
                        match[x]=j;
                        j=y;
                    }
                    return true;
                }
 
                vis[match[to]]=true;
                top[match[to]]=to;
                Q[tail++]=match[to];
            }
            else if(Find(now)!=Find(to) && odd[to]==false) {
                int k=lca(now,to);
                blossom(now,to,k);
                blossom(to,now,k);
            }
        }
    }
    return false;
}
 
int blossom_algorithm (struct Region *region, int car_num) {
 
    struct Cell *aCell;
    struct Item *aItem, *nItem;
    struct vehicle *aCar, *nCar;
    struct neighbour_car *neigh;

    memset(head,-1,sizeof(head));
    memset(match,0,sizeof(match));
    tot=0;
 
    n = car_num;

    int total_num = 0;
	int index=0;

	for(int i = 0; i<region->hCells; i++){       
		for(int j = 0; j<region->vCells;j++) {
			aCell = region->mesh + i*region->vCells + j;
			if (aCell->cars.head == NULL) continue;

			aItem = aCell->cars.head;
			while (aItem != NULL){
				aCar = (struct vehicle*)aItem->datap;
				aCar->match_id = index;
				index++;
				aItem = aItem->next;
			}

		}
	}
				



    for(int i = 0; i<region->hCells; i++){       
		for(int j = 0; j<region->vCells;j++) {
			aCell = region->mesh + i*region->vCells + j;
			if (aCell->cars.head == NULL) continue;

			aItem = aCell->cars.head;
			while (aItem != NULL){
				aCar = (struct vehicle*)aItem->datap;
				total_num += aCar->known_neigh.nItems;
				nItem = aCar->known_neigh.head;
				while (nItem != NULL){
					neigh = (struct neighbour_car*)nItem->datap;
					nCar = (struct vehicle*)neigh->carItem->datap;
					if (neigh->packet_num != 0) addEdge(aCar->match_id+1, nCar->match_id+1);
					nItem = nItem->next;
				}
				aItem = aItem->next;
			}		
		}
    }

    total_num /=2;

    //printf("total edge number: %d\n", total_num);
    int res=0;
	int start_point = rand()%n+1;

    for(int i=start_point;i<=n;i++)
        if(!match[i])
            res+=bfs(i);
    for(int i=1;i<start_point;i++)
        if(!match[i])
            res+=bfs(i);

    for(int i = 0; i<region->hCells; i++){       
		for(int j = 0; j<region->vCells;j++) {
			aCell = region->mesh + i*region->vCells + j;
			if (aCell->cars.head == NULL) continue;

			aItem = aCell->cars.head;
			while (aItem != NULL){
				aCar = (struct vehicle*)aItem->datap;
				nItem = aCar->known_neigh.head;
				while (nItem != NULL){
					neigh = (struct neighbour_car*)nItem->datap;
					nCar = (struct vehicle*)neigh->carItem->datap;
					if (match[aCar->match_id+1] == nCar->match_id+1) {
						aCar->role =2;
						aCar->choose_car_Item = neigh->carItem;
					}  // aCar and nCar has communicated in this round 
					nItem = nItem->next;
				}
				aItem = aItem->next;
			}		
		}
    }
 
    return res;
}
