#include<stdio.h>
#include<stdlib.h>
#include<string.h>
typedef struct list
{
    int index;
    int start;
    int size;
    int end;
    struct list* next;
    void* address;
}list;

void* heap;
int heapSize=11;
list *freeList[11];
list *allocList=NULL;

void initLists()
{
    heap=malloc(1024);
    for(int i=0;i<heapSize-1;i++)
    {
        freeList[i]=NULL;
    }
    freeList[heapSize-1]=(list*)malloc(sizeof(list));
    freeList[heapSize-1]->start=0;
    freeList[heapSize-1]->end=1023;
    freeList[heapSize-1]->size=1024;
    freeList[heapSize-1]->index=10;
    freeList[heapSize-1]->next=NULL;
    freeList[heapSize-1]->address=heap+freeList[heapSize-1]->start;
}

void printLists()
{
    printf("\n------------------------------------------list printing---------------------------------------------\n");
    printf("free list: ");
    for(int i=0;i<heapSize;i++)
    {
        printf("{");
        list* temp=freeList[i];
        while(temp!=NULL)
        {
            printf("(%d,%d)",temp->start,temp->end);
            temp=temp->next;
        }
        printf("},");
    }
    printf("\n");
    printf("Alloc list: ");
    list *tmp=allocList;
    while(tmp!=NULL)
    {
        printf("(%d,%d)->",tmp->start,tmp->end);
        tmp=tmp->next;
    }
    printf("\n----------------------------------------------------------------------------------------------------\n");
}

int getIndex(int size)
{
    int ind=0;
    int num=1;
    while(num<size){
        num*=2;
        ind++;
    }
    return ind;
}

list* getAllockBlock(int index)
{
    if(index==heapSize-1 && freeList[index]==NULL)
        return NULL;
    else if(freeList[index]==NULL)
    {
        list* retval=getAllockBlock(index+1);
        return retval;
    }
    else
    {
        list* retval=freeList[index];
        while(retval->next!=NULL)
        {
            retval=retval->next;
        }
        return retval;
    }
}

void appendTo(int index,list* new)
{
    if(freeList[index]==NULL)
    {
        freeList[index]=new;
    }
    else
    {
        list* temp=freeList[index];
        while(temp->next!=NULL)
        {
            temp=temp->next;
        }
        temp->next=new;
    }
}

void* updateFreeList(list* block, int index)
{
    if(block->index!=index)
    {
        list* new1=(list*)malloc(sizeof(list));
        new1->start=(block->end + block->start)/2 + 1;
        new1->end=block->end;
        new1->size=block->size/2;
        new1->next=NULL;
        new1->index=block->index-1;
        new1->address=heap+new1->start;
        appendTo(block->index-1,new1);

        list* new2=(list*)malloc(sizeof(list));
        new2->start=block->start;
        new2->end=(block->end + block->start)/2;
        new2->size=block->size/2;
        new2->next=NULL;
        new2->index=block->index-1;
        new2->address=heap+new2->start;
        void* retval=updateFreeList(new2,index);
        return retval;
    }
    else
    {
        list* new3=(list*)malloc(sizeof(list));
        new3->start=block->start;
        new3->end=block->end;
        new3->size=block->size;
        new3->next=NULL;
        new3->index=block->index;
        new3->address=heap+new3->start;
        if(allocList==NULL)
        {
            allocList=new3;
        }
        else
        {
            list* temp=allocList;
            while(temp->next!=NULL)
            {
                temp=temp->next;
            }
            temp->next=new3;
        }
        return new3->address;
    }
}

void* myMalloc(int allocSize)
{
    int index=getIndex(allocSize);
    list *blockToAlloc=getAllockBlock(index);
    //printf("(%d,%d)\n",blockToAlloc->start,blockToAlloc->end);
    //printf("blockToAlloc index:%d\n",blockToAlloc->index);
    void* retval=updateFreeList(blockToAlloc,index);
    list *temp=freeList[blockToAlloc->index];
    if(temp->next==NULL)
    {
        freeList[blockToAlloc->index]=NULL;
        free(temp);
    }
    else
    {
        while(temp->next!=blockToAlloc)
        {
            temp=temp->next;
        }
        temp->next=NULL;
        free(blockToAlloc);
    }
    return retval;
}

list* removeFromAllocList(void* address)
{
    list *prev=NULL;
    list *curr=allocList;
    while (curr->address!=address){
        prev=curr;
        curr=curr->next;
    }
    if(!prev)
        allocList=allocList->next;
    else
    {
        prev->next=curr->next;
    }
    return curr;
}

void insertIntoFreeList(list *node, int index)
{
    list *tmp=freeList[index];
    freeList[index]=node;
    node->next=tmp;
}

list* findBuddy(int address, int index)
{
    list *prev=NULL;
    list *curr=freeList[index];
    while(curr!=NULL && curr->start!=address)
    {
        prev=curr;
        curr=curr->next;
    }
    if(curr!=NULL)
        prev->next=curr->next;
    return curr;
}

void coalesceBuddies(list *node, int index)
{
    int buddyNum=node->start/node->size;
    int buddyAddress;
    if(buddyNum%2==0)
        buddyAddress=node->start + node->size;
    else
        buddyAddress=node->start - node->size;
    list *buddy=findBuddy(buddyAddress,index);
    if(buddy!=NULL)
    {
        list *newNode=(list*)malloc(sizeof(list));
        newNode->start=(node->start<buddy->start)?node->start:buddy->start;
        newNode->end=(node->end>buddy->end)?node->end:buddy->end;
        newNode->size=node->size*2;
        newNode->index=getIndex(newNode->size);
        newNode->next=NULL;
        newNode->address=heap+newNode->start;
        free(buddy);
        freeList[index]=freeList[index]->next;
        free(node);
        insertIntoFreeList(newNode,newNode->index);
        coalesceBuddies(newNode,index+1);
    }
}

void myFree(void** address)
{
    list  *freed=removeFromAllocList(*address);
    *address=NULL;
//    printf("\nfreed:(%d,%d)",freed->start,freed->end);
    int index=getIndex(freed->size);
    insertIntoFreeList(freed,index);
    coalesceBuddies(freed,index);
}

int main()
{
    initLists();
    printLists();
    int *x1=(int*)myMalloc(sizeof(int)*4);
    printLists();
    int *x2=(int*)myMalloc(sizeof(int)*4);
    printLists();
    int *x3=(int*)myMalloc(sizeof(int)*4);
    printLists();
    int *x4=(int*)myMalloc(sizeof(int)*4);
    printLists();
    //myFree(&x1);
    //printLists();
    //myFree(&x3);
    //printLists();
    //myFree(&x2);
    //printLists();
    //myFree(&x4);
    //printLists();
}