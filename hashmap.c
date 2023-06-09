#include "hashmap.h"

int hash(int key) {
    return key % HASH_SIZE;
}

Hashmap* hashmap_create() {
    Hashmap* map = malloc(sizeof(Hashmap));
    memset(map, 0, sizeof(Hashmap));
    return map;
}

void hashmap_set(Hashmap* map, int key, int value) {
    int index = hash(key);
    Entry* entry = map->table[index];
    while (entry != NULL) {
        if (entry->key == key) {
            entry->value = value;
            return;
        }
        entry = entry->next;
    }
    entry = malloc(sizeof(Entry));
    entry->key = key;
    entry->value = value;
    entry->next = map->table[index];
    map->table[index] = entry;
}

int hashmap_get(Hashmap* map, int key) {
    int index = hash(key);
    Entry* entry = map->table[index];
    while (entry != NULL) {
        if (entry->key == key) {
            return entry->value;
        }
        entry = entry->next;
    }
    return -1; 
}

void hashmap_free(Hashmap* map) {
    for (int i = 0; i < HASH_SIZE; i++) {
        Entry* entry = map->table[i];
        while (entry != NULL) {
            Entry* temp = entry;
            entry = entry->next;
            free(temp);
        }
    }
    free(map);
}

void hashmap_print(Hashmap* map) {
    for (int i = 0; i < HASH_SIZE; i++) {
        Entry* entry = map->table[i];
        while (entry != NULL) {
            printf("Key: %d, Value: %d\n", entry->key, entry->value);
            entry = entry->next;
        }
    }
}
void hashmap_remove(Hashmap* map, int key) {
    int index = hash(key);
    Entry* entry = map->table[index];
    Entry* prev = NULL;

    while (entry != NULL) {
        if (entry->key == key) {
            if (prev != NULL) {
                prev->next = entry->next;
            } else {
                map->table[index] = entry->next;
            }
            free(entry);
            return;
        }

        prev = entry;
        entry = entry->next;
    }
}


