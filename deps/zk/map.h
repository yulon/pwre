#ifndef _ZK_MAP_H
#define _ZK_MAP_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct _zk_pair {
	void *key;
	void *value;
} *_zk_pair_list_t;

typedef struct zk_map {
	size_t base;
	size_t size;
	_zk_pair_list_t list;
} *zk_map_t;

static zk_map_t zk_new_map(size_t base_size) {
	zk_map_t map = calloc(1, sizeof(struct zk_map));
	map->base = base_size;
	map->size = base_size;
	map->list = calloc(base_size, sizeof(struct _zk_pair));
	return map;
}

static bool zk_map_resize(zk_map_t map, size_t size) {
	if (size <= map->base) {
		return false;
	}
	_zk_pair_list_t newList = calloc(size, sizeof(struct _zk_pair));
	_zk_pair_list_t oldList = map->list;
	memcpy(newList, oldList, map->size);
	map->list = newList;
	map->size = size;
	free(oldList);
	return true;
}

static bool _zk_map_set(zk_map_t map, void *key, void *value) {
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
					zk_map_resize(map, size);
					return true;
				}
			} else {
				zk_map_resize(map, size);
				map->list[ix].key = key;
				map->list[ix].value = value;
				return true;
			}
		}
	}
	return false;
}
#define zk_map_set(map, key, value) _zk_map_set(map, (void *)key, (void *)value)

static void *_zk_map_get(zk_map_t map, void *key) {
	for (size_t dvdnd = map->base; dvdnd <= map->size; dvdnd += map->base) {
		size_t ix = (size_t)key % dvdnd;
		if (map->list[ix].key == key) {
			return map->list[ix].value;
		}
	}
	return NULL;
}
#define zk_map_get(map, key) _zk_map_get(map, (void *)key)

static void _zk_map_delete(zk_map_t map, void *key) {
	for (size_t dvdnd = map->base; dvdnd <= map->size; dvdnd += map->base) {
		size_t ix = (size_t)key % dvdnd;
		if (map->list[ix].key == key) {
			map->list[ix].key = NULL;
			map->list[ix].value = NULL;
		}
	}
}
#define zk_map_delete(map, key) _zk_map_delete(map, (void *)key)

static void zk_map_free(zk_map_t map) {
	free(map->list);
	free(map);
}

#endif // !_ZK_MAP_H
