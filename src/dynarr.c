#include "container/dynarr.h"

#include <stdio.h>
#include <string.h>
#include <stdint.h>


struct dynarr {
    vector vector;

    Alloc * alloc;
    size_t dtype;
    size_t capacity;
	size_t size;
    char * front;

    bool (*push_back)(dynarr*, void *);
    size_t iter;
};


static void * dynarr_next(dynarr * self) {
    if(self->iter < self->size) {
        return self->front + ((self->iter++) * self->dtype);
    } else {
        self->iter = 0;
        return NULL;
    }
}


dynarr * dynarr_default_new(Alloc * alloc, size_t dtype, size_t capacity) {
    dynarr * self = new(alloc, sizeof(dynarr));
    char * array = capacity > 0 ? new(alloc, dtype * capacity) : NULL;

    *self = (dynarr) {
        . vector = {.next = (void*(*)(const vector*)) dynarr_next}
        , .alloc = alloc
        , .dtype = dtype
        , .capacity = capacity
        , .front = array
    };

    return self;
}


static inline bool dynarr_realloc(dynarr * self) {
    size_t capacity = (self->capacity * 2) + 1;
    char * array = new(self->alloc, capacity * self->dtype);

    if(array != NULL) {
		if(self->size > 0) {
			memcpy(array, self->front, self->dtype * self->size);
			delete(self->alloc, self->front);
		}

        self->front = array;
        self->iter = 0;
        self->capacity = capacity;

        return true;
    } else {
        return false;
    }
}


bool dynarr_push_back(dynarr * self, void * value) {
    if(self->size >= self->capacity) {
        if(dynarr_realloc(self) == false) {
            return false;
        }
    } 

    switch(self->dtype) {
        case 1:
            *((uint8_t*) self->front + self->size)  = *(uint8_t*) value;
            break;
        case 2:
            *((uint16_t*) self->front + self->size) = *(uint16_t*) value;
            break;
        case 4:
            *((uint32_t*) self->front + self->size) = *(uint32_t*) value;
            break;
        case 8:
            *((uint64_t*) self->front + self->size) = *(uint64_t*) value;
            break;
        default:
            memcpy(self->front + (self->size*self->dtype), value, self->dtype);
    }

	self->size++;

    return true;
}


bool dynarr_push_front(dynarr * self, void * value) {
    if(self->size >= self->capacity) {
        if(dynarr_realloc(self) == false) {
            return false;
        }
    }

    memcpy(self->front + self->dtype, self->front, self->size * self->dtype);
    memcpy(self->front, value, self->dtype);

	self->size ++;

    return true;
}


void * dynarr_begin(dynarr * self) {
    return self->front;
}


void * dynarr_end(dynarr * self) {
    return self->front + (self->size * self->dtype);
}


void dynarr_finalize(dynarr * self) {
    if(self->alloc != NULL) {
        finalize(self->alloc);
    }
}



