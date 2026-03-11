#include "array.h"
#include <pthread.h>


array arr;
sem_t mutex;

int i = 0;

void produce(){
    array_put(&arr, i, &mutex);
    sem_wait(&mutex);
        i++;
    sem_post(&mutex);
}

void consume(){
    
}


int main(){
    if(!array_init(&arr)){
        printf("array init failure\n");
    }


    array_free(&arr);
}