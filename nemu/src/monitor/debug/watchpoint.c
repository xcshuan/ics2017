#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp(char *str){
	if(free_ != NULL){
		WP* q;
		q = free_;
		free_ = free_->next;
		q->next = head->next;
		head = q;
		strcpy(head->expr, str);
		return q;
	}
	else{
		printf("No more free space for new watchpoint!");
		return NULL;
	}
}

void free_wp(WP wp){
	WP *p = head;
	WP *q = head->next;
	while(q != NULL){
		if(q->NO == wp.NO){
			p->next = q->next;
			q->next = free_;
			free_=q;
			break;
		}
		p = p->next;
		q = p->next;
	}
}

void info_w()
{
	printf("Num\tWhat\tValue\t");
	WP* p = head;
	printf("%d\t%s\t%d",p->NO,p->expr,p->old_value);
	while(p != NULL)
	{
		p = p->next;
	}
}
