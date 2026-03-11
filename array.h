#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <semaphore.h>

#define ARR_SIZE 8
typedef struct{
    char* arr[ARR_SIZE];
    int front;
    int back; //front and back indeces for circular queue implementation
} array;


int array_init(array *s);                 // initialise the array
int array_put(array *s, char *hostname, sem_t *mutex);  // place element into the array, block when full
int array_get(array *s, char **hostname, sem_t *mutex); // remove element from the array, block when empty
void array_free(array *s);                // free the array's resources
bool array_isFull(array *s, sem_t *mutex);
bool array_isEmpty(array *s, sem_t *mutex);