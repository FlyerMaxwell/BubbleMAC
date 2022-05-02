#include<stdlib.h>
#include<time.h>
#include"common.h"
#include"event.h"

void event_init_func(struct Event *aEvent, time_t timestamp, void *byWho, void *datap, int(*handler_func)(struct Simulator*, void*, void*))//初始化事件
{
	if(aEvent == NULL) {//判断要初始化的事件aEvent 是否为NULL
		printf("A NULL event!\n");
	}
	aEvent->timestamp = timestamp;//把各个参数赋给该Event
	aEvent->byWho = byWho;
	aEvent->datap = datap;
	aEvent->handler_func = handler_func;
}

int event_has_earlier_timestamp_than(struct Event *aEvent, struct Event *bEvent)//比较两个Event发生的时间
{
	return aEvent->timestamp < bEvent->timestamp;
}


int event_has_later_timestamp_than(struct Event *aEvent, struct Event *bEvent)
{
	return aEvent->timestamp > bEvent->timestamp;
}

void add_event(struct Simulator *aSim, struct Event *aEvent)//给simulator增加事件。
{
	unsigned long whichslot;

	if(aSim == NULL) {//判断simulator是否为空
		printf("A NULL simulator!\n");
	}
	whichslot = (aEvent->timestamp-aSim->exprStartAt)/aSim->slotSize;//计算新加入的Event应该在第几个slot
	duallist_add_in_sequence_from_tail(aSim->eventSlots+whichslot, aEvent, (int(*)(void*, void*))event_has_earlier_timestamp_than);//将Event加入到一个序列的后面
	aSim->eventNums ++;//Event的数目加一
}

						
int consume_an_event(struct Simulator *aSim)//消耗event，输入为simulator
{
	struct Event *aEvent;
	unsigned long i;

	if(aSim == NULL) {//异常检测
		printf("A NULL simulator!\n");
		return -1;
	}
	for(i=0;i<(aSim->exprEndAt - aSim->exprStartAt)/aSim->slotSize;i++) {
		if(aSim->eventSlots[i].head!=NULL) {
			aEvent = (struct Event*)duallist_pick_head(&aSim->eventSlots[i]);//把这个事件找出来
			aSim->clock = aEvent->timestamp;//把simulator的时间更新到aEvent的时间
			aEvent->handler_func(aSim, aEvent->byWho, aEvent->datap);
			free(aEvent);
			return 1;
		}
	}
	return 0;
}

