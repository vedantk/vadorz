/*An ArrayList Implementation */
#ifndef _ArrayList_H
#define _ArrayList_H

#include <stdlib.h>
#include <iso646.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

#define c(this, method, args...) this.method(&(this), ## args)
typedef struct ArrayList ArrayList;
struct ArrayList{
	void ** data;
	uint32_t slots; //"slots" (total mem / sizeof objs  allocated to the list
	uint32_t len; //number of objects in the list
	size_t size; //size of the objects
	bool (* add) (ArrayList *, void * obj);
	bool (* insert) (ArrayList *, size_t, void *);
	bool (* push) (ArrayList *, void * obj);
	bool (* push_back)(ArrayList *, void *);
	bool (* isEmpty)(ArrayList *);
	void * (* peek) (ArrayList *);
	void * (* peek_back) (ArrayList *);
	void * (* pop) (ArrayList * );
	void * (* pop_back) (ArrayList *);
	void * (* get) (ArrayList *, size_t);
	void * (* set) (ArrayList *, size_t, void *);
	void * (* remove) (ArrayList *, size_t);
	void (* printStats) (ArrayList *);
	uint32_t (*getSize) (ArrayList *);
	uint32_t (* length) (ArrayList *);
	uint32_t (* units) (ArrayList *); 
	
};



static bool refactor_ArrayList(ArrayList * list){//makes sure the arraylist has space
	int i;
	for(i = list->len; i < list->slots; i++){
			list->data[i] = NULL;
	}

	if(list->slots <= (list->len + 1)){
		void ** temp = realloc( list->data,  list->size * (list->slots  + 10));
		list->slots += 10;

		if(temp == NULL){
			return false;
		}

		list->data = temp;
	}



	return true;
}

static bool add_ArrayList(ArrayList * list, void * obj){ //if false, list is at max size, and space cannot be allocated
	//otherwise true
	if(!refactor_ArrayList(list)){
		return false;
	}
	list->data[list->len] = obj;
	list->len += 1;
	return true;
}

static bool insert_ArrayList(ArrayList * list, size_t index, void * obj){
	if(index == list->len){
		return add_ArrayList(list,obj);
	}

	if(index >= list->len || index < 0){
		return false;
	}

	if(!refactor_ArrayList(list)){
		return false;
	}
	int i;
	for(i = list->len; i >= index ; i--){
		list->data[i + 1] = list->data[i];
	}
	list->len += 1;
	list->data[index] = obj;
	return true;
}

static void * index_ArrayList(ArrayList * list, size_t index){
	if(index >= list->len || index < 0){
		return NULL;
	}else{
		return list->data[index];
	}
}

static void * set_ArrayList(ArrayList * list, size_t index, void * obj){
	if(index >= list->len || index < 0){
		return NULL;
	}else{
		void * temp = list->data[index];
		list->data[index] = obj;
		return temp;
	}
}

static void * remove_ArrayList(ArrayList * list, size_t index){
	if(index >= list->len || index < 0){
		return NULL;
	}else{
		void * temp = list->data[index];
		int i;
		for( i = (index + 1); i < list->len; i++){
			list->data[i - 1] = list->data[i];
		}
		list->len -= 1;

		refactor_ArrayList(list);
		return temp;
	}
}

static unsigned int length_ArrayList(ArrayList * list){
	return list->len;
}

static unsigned int slots_ArrayList(ArrayList * list){
	return list->len;
}

static size_t size_ArrayList(ArrayList * list){
	return list->size;
}

static void printStats_ArrayList(ArrayList * list){
	printf("length : %d\nslots : %d\nsize : %d\n",list->len,list->slots,list->size);
}

void delete_ArrayList(ArrayList * list){
	free(list->data);
	list->slots = 0;
	list->len = 0;
	list->size = 0;
}

static void * peek_ArrayList(ArrayList * this){
	return index_ArrayList(this, 0);
}

static void * pop_back_ArrayList(ArrayList * this){
	return remove_ArrayList(this, length_ArrayList(this) -1);
}

static void * peek_back_ArrayList(ArrayList *this){
	return  index_ArrayList(this, length_ArrayList(this) -1);
}

static bool push_back_ArrayList(ArrayList * this, void * obj){
	return add_ArrayList(this,obj);
}

static bool push_ArrayList(ArrayList * this, void * obj){
	return insert_ArrayList(this,0, obj);
}

static bool isEmpty_ArrayList(ArrayList * this){
	return length_ArrayList(this) <= 0;
}

static void * pop_ArrayList(ArrayList * this){
	return remove_ArrayList(this,0);
}

bool new_ArrayList(ArrayList * list, size_t size){//inits an ArrayList, requires the size of the objects to be stored
	//returns false if mem allocation fails, otherwise true
	void ** temp = calloc(0, size);
	if(temp == NULL){
		return false;
	}
	list->data = temp;
	list->len = 0;
	list->slots = 0;
	list->size = size;
	list->add = &add_ArrayList;
	list->remove = &remove_ArrayList;
	list->length = &length_ArrayList;
	list->get = &index_ArrayList;
	list->set = &set_ArrayList;
	list->units = &slots_ArrayList;
	list->insert = &insert_ArrayList;
	list->getSize = &size_ArrayList;
	list->printStats = &printStats_ArrayList;
	list->pop = &pop_ArrayList;
	list->pop_back = &pop_back_ArrayList;
	list->peek = &peek_ArrayList;
	list->peek_back = &peek_back_ArrayList;
	list->push = &push_ArrayList;
	list->push_back = &push_back_ArrayList;
	list->isEmpty = &isEmpty_ArrayList;
	return true;
}

#endif /* _ArrayList_H */
