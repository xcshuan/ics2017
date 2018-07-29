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
		bool sucess = false;
		int value;
		value = expr(str,&sucess);
		if(sucess == true){
				WP* q;
				q = free_;
				free_ = free_->next;
				strcpy(q->expr, str);
				q->old_value = value;
				if(head == NULL){
					head = q;
					head->next = NULL;
				}
				else{
					q->next = head;
					head = q;
					//printf("head = %d\n",head->NO);
					//if(head->next!=NULL) printf("head-next = %d\n",head->NO);
				}
				return q;
			}
		else{
			printf("Input Error!");
			return NULL;
		}
	}
	else{
		printf("No more free space for new watchpoint!");
		return NULL;
	}
}

void free_wp(int num){
	if(head ==NULL) return;
	WP *p = head;
	WP *q = head->next;
	int flag = 0;
	if(p->NO == num){
		p->next = free_;
		free_ = p;
		head = NULL;
		flag++;
	}
	else{
		while(q != NULL){
			if(q->NO == num){
				p->next = q->next;
				q->next = free_;
				free_=q;
				flag++;
				break;
			}
			p = p->next;
			q = p->next;
		}
	}
	if(flag > 0) printf("watchpoint %d is deleted\n", num);
	else printf("The %d watchpoint is not existed\n", num);
}

void info_w()
{
	printf("Num\tWhat\tValue\t\n");
	WP *p = head;
	if(p == NULL) printf("No watchpoint now\n");
	while(p != NULL)
	{
		printf("%d\t%s\t%d\n",p->NO,p->expr,p->old_value);
		p = p->next;

	}
}
bool check_w(){
	WP *p = head;
	int flag = 0;
	bool sucess = false;
	while(p != NULL){
		uint32_t new_value = expr(p->expr,&sucess);
		if(sucess == false) printf("Error!");
		else{
			if(new_value != p->old_value){
				printf("Watchpoint %d is triggered\n", p->NO);
				printf("old value = %d\nnew value = %d\n",p->old_value,new_value);
				flag++;
				p->old_value = new_value;
			}
		}
		p = p->next;
	}
	if(flag != 0) return false;
	else return true;
}
