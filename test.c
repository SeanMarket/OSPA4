#include "array.h"
#include <pthread.h>


array arr;
sem_t mutex;

void produce(){

}

void consume(){
    
}


int main(){
    if(!array_init(&arr)){
        printf("array init failure\n");
    }


    array_free(&arr);
}