#ifndef _ZK_MAP_H
#define _ZK_MAP_H

#ifndef _ZK_LINK
#define _ZK_LINK(p, n) ___##p##___##n
#define _ZK_LINK_M(p, n) _ZK_LINK(p, n)
#endif

#ifdef ZK_SCOPE
#define _ZK_NAME(n) _ZK_LINK_M(ZK_SCOPE, n)
#else
#define _ZK_NAME(n) _ZK_LINK(zk, n)
#endif

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ZKMap *ZKMap;

ZKMap _ZK_NAME(new_ZKMap)(size_t);
#define new_ZKMap(size_t) _ZK_NAME(new_ZKMap)(size_t)

bool _ZK_NAME(ZKMap_resize)(ZKMap, size_t);
#define ZKMap_resize(ZKMap, size_t) _ZK_NAME(ZKMap_resize)(ZKMap, size_t)

bool _ZK_NAME(ZKMap_set)(ZKMap, void *key, void *value);
#define ZKMap_set(ZKMap_map, any_key, any_value) _ZK_NAME(ZKMap_set)(ZKMap_map, (void *)any_key, (void *)any_value)

void *_ZK_NAME(ZKMap_get)(ZKMap, void *key);
#define ZKMap_get(ZKMap_map, any_key) _ZK_NAME(ZKMap_get)(ZKMap_map, (void *)any_key)

void _ZK_NAME(ZKMap_delete)(ZKMap, void *key);
#define ZKMap_delete(ZKMap_map, any_key) _ZK_NAME(ZKMap_delete)(ZKMap_map, (void *)any_key)

void _ZK_NAME(ZKMap_free)(ZKMap);
#define ZKMap_free(ZKMap) _ZK_NAME(ZKMap_free)(ZKMap)

#ifdef __cplusplus
}
#endif

#ifdef ZK_IMPL
#ifndef _ZK_MAP_BODY
#define _ZK_MAP_BODY

#include <stdlib.h>
#include <string.h>

typedef struct Pair {
	void *key;
	void *value;
} *PairList;

struct ZKMap {
	size_t base;
	size_t size;
	PairList list;
};

ZKMap _ZK_NAME(new_ZKMap)(size_t baseSize) {
	ZKMap map = calloc(1, sizeof(struct ZKMap));
	map->base = baseSize;
	map->size = baseSize;
	map->list = calloc(baseSize, sizeof(struct Pair));
	return map;
}

bool _ZK_NAME(ZKMap_resize)(ZKMap map, size_t size) {
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

bool _ZK_NAME(ZKMap_set)(ZKMap map, void *key, void *value) {
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
					ZKMap_resize(map, size);
					return true;
				}
			} else {
				ZKMap_resize(map, size);
				map->list[ix].key = key;
				map->list[ix].value = value;
				return true;
			}
		}
	}
	return false;
}

void *_ZK_NAME(ZKMap_get)(ZKMap map, void *key) {
	for (size_t dvdnd = map->base; dvdnd <= map->size; dvdnd += map->base) {
		size_t ix = (size_t)key % dvdnd;
		if (map->list[ix].key == key) {
			return map->list[ix].value;
		}
	}
	return NULL;
}

void _ZK_NAME(ZKMap_delete)(ZKMap map, void *key) {
	for (size_t dvdnd = map->base; dvdnd <= map->size; dvdnd += map->base) {
		size_t ix = (size_t)key % dvdnd;
		if (map->list[ix].key == key) {
			map->list[ix].key = NULL;
			map->list[ix].value = NULL;
		}
	}
}

void _ZK_NAME(ZKMap_free)(ZKMap map) {
	free(map->list);
	free(map);
}

#endif // !_ZK_MAP_BODY
#endif // !ZK_IMPL

#endif // !_ZK_MAP_H
