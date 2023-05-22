#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define HASH_SIZE 256

typedef struct Entry {
    int key;
    int value;
    struct Entry* next;
} Entry;

typedef struct Hashmap {
    Entry* table[HASH_SIZE];
} Hashmap;

Hashmap* hashmap_create();
int hash(int key);
void hashmap_set(Hashmap* map, int key, int value);
int hashmap_get(Hashmap* map, int key);
void hashmap_free(Hashmap* map);
void hashmap_remove(Hashmap* map, int key);
void hashmap_print(Hashmap* map);
