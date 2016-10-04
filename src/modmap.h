#ifndef _MODMAP_H
#define _MODMAP_H

#include <stdbool.h>
#include <string.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *ModMap;

typedef struct _ModMapKv {
	void *key;
	void *value;
} *_ModMapKvList;

typedef struct _ModMapPvt {
	size_t base;
	size_t size;
	_ModMapKvList list;
} *_ModMapPvt;

ModMap new_ModMap(size_t baseSize) {
	_ModMapPvt map = calloc(1, sizeof(struct _ModMapPvt));
	map->base = baseSize;
	map->size = baseSize;
	map->list = calloc(baseSize, sizeof(struct _ModMapKv));
	return (ModMap)map;
}

#define _MODMAP ((_ModMapPvt)map)

bool ModMap_resize(ModMap map, size_t size) {
	if (size <= _MODMAP->base) {
		return false;
	}
	_ModMapKvList newList = calloc(size, sizeof(struct _ModMapKv));
	_ModMapKvList oldList = _MODMAP->list;
	memcpy(newList, oldList, _MODMAP->size);
	_MODMAP->list = newList;
	_MODMAP->size = size;
	free(oldList);
	return true;
}

bool ModMap_set(ModMap map, void *key, void *value) {
	for (size_t dvdnd = _MODMAP->base; dvdnd <= _MODMAP->size; dvdnd += _MODMAP->base) {
		size_t ix = (size_t)key % dvdnd;
		if (!_MODMAP->list[ix].key) {
			_MODMAP->list[ix].key = key;
			_MODMAP->list[ix].value = value;
			return true;
		}
	}
	for (size_t size = _MODMAP->size * 2; size <= SIZE_MAX; size = size * 2) {
		for (size_t dvdnd = _MODMAP->size + _MODMAP->base; dvdnd <= size; dvdnd += _MODMAP->base) {
			size_t ix = (size_t)key % dvdnd;
			if (ix < _MODMAP->size) {
				if (!_MODMAP->list[ix].key) {
					_MODMAP->list[ix].key = key;
					_MODMAP->list[ix].value = value;
					ModMap_resize(map, size);
					return true;
				}
			} else {
				ModMap_resize(map, size);
				_MODMAP->list[ix].key = key;
				_MODMAP->list[ix].value = value;
				return true;
			}
		}
	}
	return false;
}

void *ModMap_get(ModMap map, void *key) {
	for (size_t dvdnd = _MODMAP->base; dvdnd <= _MODMAP->size; dvdnd += _MODMAP->base) {
		size_t ix = (size_t)key % dvdnd;
		if (_MODMAP->list[ix].key == key) {
			return _MODMAP->list[ix].value;
		}
	}
	return NULL;
}

void ModMap_delete(ModMap map, void *key) {
	for (size_t dvdnd = _MODMAP->base; dvdnd <= _MODMAP->size; dvdnd += _MODMAP->base) {
		size_t ix = (size_t)key % dvdnd;
		if (_MODMAP->list[ix].key == key) {
			_MODMAP->list[ix].key = NULL;
			_MODMAP->list[ix].value = NULL;
		}
	}
}

void ModMap_free(ModMap map) {
	free(_MODMAP->list);
	free(_MODMAP);
}

#undef _MODMAP

#ifdef __cplusplus
}
#endif

#endif // !_MODMAP_H
