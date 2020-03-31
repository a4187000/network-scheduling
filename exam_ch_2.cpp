#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>
#include <algorithm>

#include <iostream>
using namespace std;

//system counter
double drop = 0;
double total = 0;
double throughput = 0;

//node
typedef struct package
{
    int tp;
    int flow;
    int number;
    double data_remain;
    package *pre;
    package *next;
}package;

//UE
typedef struct UE
{
    int CQI;
    int CQI_dr;
    int QCI;
    int DP;
    double CL;
    int flow;
    int drop_package;
    double through_put;
    package *head;
    package *tail;
    UE *pre;
    UE *next;
    int QCI_index;
    int count;
    double data;
}UE;

//UE manager
typedef struct flow_mgr
{
    int mgr_type;// 0->normal 1->GTT
    UE *head;
    UE *tail;
    int flow_count;
}flow_mgr;

//function for quicksort
int cmp(const void *a, const void *b){
   return ((double*)b)[1] - ((double*)a)[1];
}

//GBR function
flow_mgr* gen_mgr(int, int);
void gen_new_package(flow_mgr* ,int);
void calculation_normal(flow_mgr*, flow_mgr*);
int calculation(UE *);
void delet_package(UE *, package *);
void add_package(flow_mgr *, package *);
void afterTTI_GBR(flow_mgr *, flow_mgr *, int );

int main(int argc, char *argv[]){
    //a.exe times(ms) choose(1=GBR, 2=MT, 3=PF) flow(ue) QCI
    //case_number 1->my_method 2->MT 3->PF

    srand(time(NULL));
    int times = atoi(argv[1]);
    int choose = atoi(argv[2]);
    int Max = atoi(argv[3]);
    int QCI = atoi(argv[4]);

    package *temp;
    flow_mgr *GTT, *normal;
    normal = gen_mgr(Max, QCI);
    GTT = gen_mgr(Max, QCI);
    GTT->mgr_type = 1;
    int time = 0;

    //GBR
    while(choose == 1){
        if(total/Max >= times){
            printf("system drop rate = %lf\n", drop/total);
            printf("system through put =%lf\n", (throughput*data)/(times*1000));
            return 0;
        }

        printf("Time(%d)\n", ++time);
        total+=Max;

        gen_new_package(normal, time);

        calculation_normal(normal, GTT);

        
    }

}

//generate new flow manager
flow_mgr* gen_mgr(int Max, int QCI){
    int QCI_arr[9]={1,2,3,4,66,6,7,8,9};
    int DP[9]={100,150,50, 300, 100, 300, 100, 300,300};
    double CL[9]={1e-02, 1e-03, 1e-03, 1e-06, 1e-02, 1e-06, 1e-03, 1e-06, 1e-06};
    double data[9] = {8.3, 242, 0, 0, 0, 12, 0, 0, 0};

    flow_mgr *new_mgr;
    UE *new_UE;

    new_mgr = (flow_mgr*)malloc(sizeof(flow_mgr));
    new_mgr->head = NULL;
    new_mgr->tail = NULL;
    new_mgr->mgr_type = 0;
    new_mgr->flow_count = 0;

    for(int i=0; i<Max; i++){
        new_UE = (UE*)malloc(sizeof(UE));
        new_UE->CQI = 0;//須完成
        new_UE->QCI = QCI_arr[QCI];
        new_UE->CL = CL[QCI];
        new_UE->DP = DP[QCI];
        new_UE->flow = i;
        new_UE->drop_package = 0;
        new_UE->through_put = 0;
        new_UE->head = NULL;
        new_UE->tail = NULL;
        new_UE->pre = NULL;
        new_UE->next = NULL;
        new_UE->count = 0;
        new_UE->QCI_index = QCI;
        new_UE->data = data[QCI];

        if(i == 0){
            new_mgr->head = new_UE;
            new_mgr->tail = new_UE;
            new_mgr->flow_count++;
        }
        else{
            new_UE->pre = new_mgr->tail;
            new_mgr->tail->next = new_UE;
            new_mgr->tail = new_UE;
            new_mgr->flow_count++;
        }
    }
    return new_mgr;
}

void gen_new_package(flow_mgr* mgr, int num){
    double data[9] = {8.3, 242, 0, 0, 0, 12, 0, 0, 0};
    package *newnode;
    UE *temp_UE;
    temp_UE = mgr->head;
    for(int i=0; i<mgr->flow_count; i++){
        newnode = (package*)malloc(sizeof(package));
        newnode->tp = rand()%30+1;
        newnode->flow = i;
        newnode->number = num;
        newnode->data_remain = data[temp_UE->QCI_index];
        newnode->pre = NULL;
        newnode->next = NULL;
        if(temp_UE->count == 0){
            temp_UE->head = newnode;
            temp_UE->tail = newnode;
        }
        else{
            temp_UE->tail->next = newnode;
            newnode->pre = temp_UE->tail;
            temp_UE->tail = newnode;
        }
        temp_UE->count++;
        temp_UE = temp_UE->next;
    }
}

void calculation_normal(flow_mgr *normal, flow_mgr *GTT){
    UE *temp_ue;
    package *temp;
    temp_ue = normal->head;

    while(temp_ue != NULL){
        temp = temp_ue->head;
        if(temp_ue->QCI != 6){
            if(calculation(temp_ue)){
                delet_package(temp_ue, temp);
                add_package(GTT, temp);
            }
            else{
                temp_ue = temp_ue->next;
            }
        }
        else{
            temp_ue = temp_ue->next;
        }
    }
}

int calculation(UE *ue){
    package *temp;
    temp = ue->head;

    double left, right;
    left = ((double)temp->tp/(double)ue->CL)*(1/-log10(ue->DP));
    right = ((node->data_remain/ue->CQI_dr)/temp->tp)*(1/-log10(ue->drop_package/ue->tail->number));

    if(left <= right){
        return 1;
    }
    else{
        return 0;
    }
}

void delet_package(UE *ue, package *node){
    if(ue->count == 1){
        ue->head = NULL;
        ue->tail = NULL;
    }
    else if(node == ue->head){
        ue->head = node->next;
        node->next->pre = NULL;
    }
    else if(node->next == NULL){
        node->pre->next = NULL;
        ue->tail = node->pre;
    }
    else{
        node->pre->next = node->next;
        node->next->pre = node->pre;
    }
    ue->count--;
}

void add_package(flow_mgr *mgr, package *node){
    UE *temp;
    temp = mgr->head;

    for(int i=1; i<node->flow; i++){
        temp = temp->next;
    }

    if(temp->count == 0){
        temp->head = node;
        temp->tail = node;
        node->next = NULL;
        node->pre = NULL;
    }
    else{
        node->pre = temp->tail;
        node->pre->next = node;
        temp->tail = node;
        node->next = NULL;
    }
    temp->count++;
}

void afterTTI_GBR(flow_mgr *normal, flow_mgr *GTT, int rb){
    int send;
    int gtt_count = 0;
    int gtt_send[GTT->flow_count];
    int data_remain;
    double normal_throughput[normal->flow_count][2];
    UE *temp_ue_normal;
    package *temp, *temp_normal, *del;

    UE *temp_ue = GTT->head;
    for(int i=0; i<GTT->flow_count; i++){
        if(temp_ue->count!=0){
            gtt_send[i] = 1;
            gtt_count++;
        }
        else{
            gtt_send[i] = 0;
        }
        temp_ue = temp_ue->next;
    }

    temp_ue_normal = normal->head;
    for(int i=0; i<normal->flow_count; i++){
        if(gtt_send[i] == 0){
            normal_throughput[i][0] = i;
            normal_throughput[i][1] = temp_ue_normal->through_put;
        }
        temp_ue_normal = temp_ue_normal->next;
    }

    qsort(normal_throughput, normal->flow_count, sizeof(normal_throughput[0]), cmp);
    rb -= gtt_count;

    for(int i=0; i<rb; i++){
        gtt_send[normal_throughput[i][0]] = 2;
    }

    temp_ue_normal = normal->head;
    temp_ue = GTT->head;
    for(int i=0; i<GTT->flow_count; i++){
        if(gtt_send[i] == 1){

        }
        else if(gtt_send[i] == 2){

        }
        temp_ue_normal = temp_ue_normal->next;
        temp_ue = temp_ue->next;
    }

}