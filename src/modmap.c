#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "modmap.h"

typedef struct Pair {
	void *key;
	void *value;
} *PairList;

struct ModMap {
	size_t base;
	size_t size;
	PairList list;
};

ModMap new_ModMap(size_t baseSize) {
	ModMap map = calloc(1, sizeof(struct ModMap));
	map->base = baseSize;
	map->size = baseSize;
	map->list = calloc(baseSize, sizeof(struct Pair));
	return map;
}

bool ModMap_resize(ModMap map, size_t size) {
	if (size <= map->base) {
		return false;
	}
	PairList newList = calloc(size, sizeof(struct Pair));
	PairList oldList = map->list;
	memcpy(newList, oldList, map->size);
	map->list = newList;
	map->size = size;
	free(oldList);
	return true;
}

bool _ModMap_set(ModMap map, void *key, void *value) {
	for (size_t dvdnd = map->base; dvdnd <= map->size; dvdnd += map->base) {
		size_t ix = (size_t)key % dvdnd;
		if (!map->list[ix].key) {
			map->list[ix].key = key;
			map->list[ix].value = value;
			return true;
		}
	}
	for (size_t size = map->size * 2; size <= SIZE_MAX; size = size * 2) {
		for (size_t dvdnd = map->size + map->base; dvdnd <= size; dvdnd += map->base) {
			size_t ix = (size_t)key % dvdnd;
			if (ix < map->size) {
				if (!map->list[ix].key) {
					map->list[ix].key = key;
					map->list[ix].value = value;
					ModMap_resize(map, size);
					return true;
				}
			} else {
				ModMap_resize(map, size);
				map->list[ix].key = key;
				map->list[ix].value = value;
				return true;
			}
		}
	}
	return false;
}

void *_ModMap_get(ModMap map, void *key) {
	for (size_t dvdnd = map->base; dvdnd <= map->size; dvdnd += map->base) {
		size_t ix = (size_t)key % dvdnd;
		if (map->list[ix].key == key) {
			return map->list[ix].value;
		}
	}
	return NULL;
}

void _ModMap_delete(ModMap map, void *key) {
	for (size_t dvdnd = map->base; dvdnd <= map->size; dvdnd += map->base) {
		size_t ix = (size_t)key % dvdnd;
		if (map->list[ix].key == key) {
			map->list[ix].key = NULL;
			map->list[ix].value = NULL;
		}
	}
}

void ModMap_free(ModMap map) {
	free(map->list);
	free(map);
}
