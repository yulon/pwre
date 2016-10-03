#ifndef _MODMAP_H
#define _MODMAP_H

#include <stdbool.h>
#include <string.h>

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

#define _MAP ((_ModMapPvt)map)

bool ModMap_resize(ModMap map, size_t size) {
	if (size <= _MAP->base) {
		return false;
	}
	_ModMapKvList newList = calloc(size, sizeof(struct _ModMapKv));
	_ModMapKvList oldList = _MAP->list;
	memcpy(newList, oldList, _MAP->size);
	_MAP->list = newList;
	_MAP->size = size;
	free(oldList);
	return true;
}

bool ModMap_set(ModMap map, void *key, void *value) {
	for (size_t dvdnd = _MAP->base; dvdnd <= _MAP->size; dvdnd += _MAP->base) {
		size_t ix = (size_t)key % dvdnd;
		if (!_MAP->list[ix].key) {
			_MAP->list[ix].key = key;
			_MAP->list[ix].value = value;
			return true;
		}
	}
	for (size_t size = _MAP->size * 2; size <= (size_t)-1; size = size * 2) {
		for (size_t dvdnd = _MAP->size + _MAP->base; dvdnd <= size; dvdnd += _MAP->base) {
			size_t ix = (size_t)key % dvdnd;
			if (ix < _MAP->size) {
				if (!_MAP->list[ix].key) {
					_MAP->list[ix].key = key;
					_MAP->list[ix].value = value;
					ModMap_resize(map, size);
					return true;
				}
			} else {
				ModMap_resize(map, size);
				_MAP->list[ix].key = key;
				_MAP->list[ix].value = value;
				return true;
			}
		}
	}
	return false;
}

void *ModMap_get(ModMap map, void *key) {
	for (size_t dvdnd = _MAP->base; dvdnd <= _MAP->size; dvdnd += _MAP->base) {
		size_t ix = (size_t)key % dvdnd;
		if (_MAP->list[ix].key == key) {
			return _MAP->list[ix].value;
		}
	}
	return NULL;
}

void ModMap_delete(ModMap map, void *key) {
	for (size_t dvdnd = _MAP->base; dvdnd <= _MAP->size; dvdnd += _MAP->base) {
		size_t ix = (size_t)key % dvdnd;
		if (_MAP->list[ix].key == key) {
			_MAP->list[ix].key = NULL;
			_MAP->list[ix].value = NULL;
		}
	}
}

void ModMap_free(ModMap map) {
	free(_MAP->list);
	free(_MAP);
}

#undef _MAP

#ifdef __cplusplus
}
#endif

#endif // !_MODMAP_H
