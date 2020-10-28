#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h> 
#include "ring.hpp"

#define MaxItems 5 // Maximum items a producer can produce or a consumer can consume
#define BufferSize 5 // Size of the buffer

int buffer[BufferSize];
pthread_mutex_t mutex;
Ring ring(50);

void *producer(void *pno) {  
    int nice = rand()%10;
    for(int i = 0; i < nice; i++) {
        pthread_mutex_lock(&mutex);
        ring.insert(*((int *)pno));
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

void *consumer(void *cno) {
    pthread_mutex_lock(&mutex);
    ring.print();
    printf("\nprinted by %d\n", *((int *)cno));
    pthread_mutex_unlock(&mutex);
    return NULL;
}

int main()
{   
    printf("%s\n", __TIME__);
    pthread_t pro[5],con[5];
    pthread_mutex_init(&mutex, NULL);

    int a[5] = {1,2,3,4,5}; //Just used for numbering the producer and consumer

    srand(time(0));

    int nice = rand()%5+5;
    for(int i = 0; i < nice; i++) {
        for(int i = 0; i < 5; i++) pthread_create(&pro[i], NULL, producer, (void *)&a[i]);
        for(int i = 0; i < 5; i++) pthread_create(&con[i], NULL, consumer, (void *)&a[i]);
        for(int i = 0; i < 5; i++) pthread_join(pro[i], NULL);
        for(int i = 0; i < 5; i++) pthread_join(con[i], NULL);
    }

    pthread_mutex_destroy(&mutex);
    printf("dicks %d\n", dicks);

    return 0;
}

int dicks=0;