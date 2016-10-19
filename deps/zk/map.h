#ifndef _ZK_MAP_H
#define _ZK_MAP_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct _ZKPair {
	void *key;
	void *value;
} *_ZKPairList;

typedef struct ZKMap {
	size_t base;
	size_t size;
	_ZKPairList list;
} *ZKMap;

static ZKMap new_ZKMap(size_t baseSize) {
	ZKMap map = calloc(1, sizeof(struct ZKMap));
	map->base = baseSize;
	map->size = baseSize;
	map->list = calloc(baseSize, sizeof(struct _ZKPair));
	return map;
}

static bool ZKMap_ReSize(ZKMap map, size_t size) {
	if (size <= map->base) {
		return false;
	}
	_ZKPairList newList = calloc(size, sizeof(struct _ZKPair));
	_ZKPairList oldList = map->list;
	memcpy(newList, oldList, map->size);
	map->list = newList;
	map->size = size;
	free(oldList);
	return true;
}

static bool _ZKMap_Set(ZKMap map, void *key, void *value) {
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
					ZKMap_ReSize(map, size);
					return true;
				}
			} else {
				ZKMap_ReSize(map, size);
				map->list[ix].key = key;
				map->list[ix].value = value;
				return true;
			}
		}
	}
	return false;
}
#define ZKMap_Set(ZKMap_map, any_key, any_value) _ZKMap_Set(ZKMap_map, (void *)any_key, (void *)any_value)

static void *_ZKMap_Get(ZKMap map, void *key) {
	for (size_t dvdnd = map->base; dvdnd <= map->size; dvdnd += map->base) {
		size_t ix = (size_t)key % dvdnd;
		if (map->list[ix].key == key) {
			return map->list[ix].value;
		}
	}
	return NULL;
}
#define ZKMap_Get(ZKMap_map, any_key) _ZKMap_Get(ZKMap_map, (void *)any_key)

static void _ZKMap_Delete(ZKMap map, void *key) {
	for (size_t dvdnd = map->base; dvdnd <= map->size; dvdnd += map->base) {
		size_t ix = (size_t)key % dvdnd;
		if (map->list[ix].key == key) {
			map->list[ix].key = NULL;
			map->list[ix].value = NULL;
		}
	}
}
#define ZKMap_Delete(ZKMap_map, any_key) _ZKMap_Delete(ZKMap_map, (void *)any_key)

static void ZKMap_Free(ZKMap map) {
	free(map->list);
	free(map);
}

#endif // !_ZK_MAP_H
