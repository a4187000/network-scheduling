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
    int tp; //remaining time
    int flow; //flow number
    int number;
    double data_remain;
    package *pre;
    package *next;
}package;

//UE
typedef struct UE
{
    int CQI;
    double CQI_dr;
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

void free_package(package *);
void print_all(flow_mgr *);

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
    normal->mgr_type = 0;
    int time = 0;

    //GBR
    while(choose == 1){
        if(total/Max >= times){
            for(int i=0; i<Max; i++){

            }
            printf("drop: %lf total: %lf\n", drop, total);
            printf("system drop rate = %lf\n", drop/total);
            printf("system throughput =%lf\n", throughput);
            return 0;
        }

        printf("Time(%d)\n", ++time);
        total+=Max;

        gen_new_package(normal, time);
        //print_all(normal);

        calculation_normal(normal, GTT);

        afterTTI_GBR(normal, GTT, 20);

        //print_all(normal);
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
        new_UE->CQI = 13;//更新在calculation_normal
        new_UE->CQI_dr = 2000; // 更新在calculation_normal
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
        new_UE->data = data[QCI]/1000;

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
        newnode->tp = 90; //使用CQI預估
        newnode->flow = i;
        newnode->number = num;
        newnode->data_remain = temp_UE->data;
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
    UE *temp_ue_normal;
    package *temp;
    package *temp_del;
    temp_ue = normal->head;
    int time=0;
    double CQI_dr[16]= {0, 0.318841, 0.318841, 0.318841, 0.318841, 0.318841, 0.318841, 
    0.563768, 0.563768, 0.563768, 0.888406, 0.888406, 0.888406, 0.888406, 0.888406, 0.888406};

    //calculation nomal if there is any package overtime
    while(temp_ue != NULL){
        temp = temp_ue->head;
        while(temp != NULL){
            if(temp->tp < (temp->data_remain / (double)temp_ue->CQI_dr)){
                //printf("%d %lf\n", temp->tp, (temp->data_remain / temp_ue->CQI_dr));
                temp_del = temp;
                temp = temp->next;
                delet_package(temp_ue, temp_del);
                free_package(temp_del);
                drop++;
                //printf("drop %d\n", temp_ue->flow);
                temp_ue->drop_package++;
            }
            else{
                temp->tp--;
                temp = temp->next;
            }
        }
        //CQI 15% +1 15% -1 70% 維持
        int change = rand()%100+1;
        if(change > 85){
            if(temp_ue->CQI <15){
                temp_ue->CQI++;
                temp_ue->CQI_dr = CQI_dr[temp_ue->CQI];
            }
        }
        else if(change > 70 && change <= 85){
            if(temp_ue->CQI > 1){
                temp_ue->CQI--;
                temp_ue->CQI_dr = CQI_dr[temp_ue->CQI];
            }
        }
        temp_ue = temp_ue->next;
    }

    //calculation GTT if there is any package overtime
    temp_ue = GTT->head;
    temp_ue_normal = normal->head;
    while(temp_ue != NULL){
        temp = temp_ue->head;
        while(temp != NULL){
            if(temp->tp < (temp->data_remain / temp_ue->CQI_dr)){
                temp_del = temp;
                temp = temp->next;
                delet_package(temp_ue, temp_del);
                free_package(temp_del);
                drop++;
                temp_ue_normal->drop_package++;
            }
            else{
                temp->tp--;
                temp = temp->next;
            }
        }
        temp_ue->CQI = temp_ue_normal->CQI;
        temp_ue->CQI_dr = temp_ue_normal->CQI_dr;
        temp_ue->drop_package = temp_ue_normal->drop_package;
        temp_ue = temp_ue->next;
    }

    temp_ue = normal->head;
    while(temp_ue != NULL){
        temp = temp_ue->head;
        if(temp_ue->QCI != 6 && temp_ue->count != 0){ //要改成正確ＱＣＩ指標
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
    left = ((double)temp->tp/(double)ue->DP)*(1/-log10(ue->CL));
    right = ((temp->data_remain/ue->CQI_dr)/temp->tp)*(1/-log10(ue->drop_package/ue->tail->number));

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

void free_package(package *del){
    free(del);
}

void print_all(flow_mgr *mgr){
    UE *temp_ue = mgr->head;
    package *temp;

    printf("UE: %d\n", mgr->flow_count);
    for(int i=1; i<=mgr->flow_count; i++){
        printf("flow %d\n", temp_ue->flow);
        printf("CL:%lf ", temp_ue->CL);
        printf("count:%d ", temp_ue->count);
        printf("CQI:%d ", temp_ue->CQI);
        printf("dr:%lf ", temp_ue->CQI_dr);
        printf("data:%lf ", temp_ue->data);
        printf("DP:%d ", temp_ue->DP);
        printf("drop package:%d ", temp_ue->drop_package);
        printf("QCI:%d\n", temp_ue->QCI);
        temp = temp_ue->head;
        for(int j=1; j<=temp_ue->count; j++){
            printf("package:%d ", j);
            printf("data remain:%lf ", temp->data_remain);
            printf("tp:%d\n", temp->tp);
            temp = temp->next;
        }
        temp_ue = temp_ue->next;
        printf("\n");
    }
}

void afterTTI_GBR(flow_mgr *normal, flow_mgr *GTT, int rb){
    int send;
    int gtt_count = 0;
    int gtt_send[GTT->flow_count];
    int data_remain;
    double normal_throughput[normal->flow_count][2];
    UE *temp_ue_normal;
    package *temp, *temp_normal, *del;
    srand(time(NULL));

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
        if(gtt_send[i] == 0){ //no package in GTT
            normal_throughput[i][0] = i;
            normal_throughput[i][1] = temp_ue_normal->through_put;
        }
        else{ // if there is any package in GTT mark -1
            normal_throughput[i][0] = i;
            normal_throughput[i][1] = -1;
        }
        temp_ue_normal = temp_ue_normal->next;
    }

    qsort(normal_throughput, normal->flow_count, sizeof(normal_throughput[0]), cmp);
    rb -= gtt_count;
    int index = normal->flow_count;
    for(int i=0; i<rb && index > 0; ){ // mark the sending UE in normal
        if((int)normal_throughput[index][0]!=-1){
            gtt_send[(int)normal_throughput[index][0]] = 2;
            i++;
            index--;
        }
        else{
            index--;
        }
    }

    temp_ue_normal = normal->head;
    temp_ue = GTT->head;
    for(int i=0; i<GTT->flow_count; i++){
        data_remain = temp_ue->CQI_dr;
        int send_random = rand()%100;
        if(temp_ue_normal->CQI >= 10){
            send = 1;
        }
        else if(temp_ue_normal->CQI >= 7 && temp_ue_normal->CQI <10){
            if(send_random <= 90){
                send = 1;
            }
            else{
                send = 0;
            }
        }
        else if(temp_ue_normal->CQI >0 && temp_ue_normal->CQI <7){
            if(send_random <= 70){
                send = 1;
            }
            else{
                send = 0;
            }
        }
        else{
            send = 0;
        } 
        if(gtt_send[i] == 1 && send ==1){ // there is any package in GTT
            printf("send\n");
            while(data_remain > 0 && temp_ue->count > 0){
                if(data_remain >= temp_ue->head->data_remain){
                    throughput++;
                    data_remain -= temp_ue->head->data_remain;
                    temp_ue_normal->through_put++;
                    temp = temp_ue->head;
                    delet_package(temp_ue, temp_ue->head);
                    free_package(temp);
                }
                else{
                    temp_ue->head->data_remain -= data_remain;
                    data_remain = 0;
                }
            }
            while(data_remain > 0 && temp_ue_normal->count > 0){
                if(data_remain >= temp_ue_normal->head->data_remain){
                    throughput++;
                    data_remain -= temp_ue_normal->head->data_remain;
                    temp_ue_normal->through_put++;
                    temp = temp_ue_normal->head;
                    delet_package(temp_ue_normal, temp);
                    free_package(temp);
                }
                else{
                    temp_ue_normal->head->data_remain -= data_remain;
                    data_remain = 0;
                }
            }
        }
        else if(gtt_send[i] == 2 && send == 1){ //there is only in normal and send in this TTI
            while(data_remain > 0 && temp_ue_normal->count > 0){
                if(data_remain >= temp_ue_normal->head->data_remain){
                    throughput++;
                    data_remain -= temp_ue_normal->head->data_remain;
                    temp_ue_normal->through_put++;
                    temp = temp_ue_normal->head;
                    delet_package(temp_ue_normal, temp);
                    free_package(temp);
                }
                else{
                    temp_ue_normal->head->data_remain -= data_remain;
                    data_remain = 0;
                }
            }
        }
        temp_ue_normal = temp_ue_normal->next;
        temp_ue = temp_ue->next;
    }

}