#ifndef _MODMAP_H
#define _MODMAP_H

#include <stdbool.h>
#include <string.h>

typedef void *ModMap;

typedef struct _Kav {
	void *key;
	void *value;
} *_Kavs;

typedef struct _ModMapPvt {
	size_t base;
	size_t size;
	_Kavs list;
} *_ModMapPvt;

ModMap mdmpNew(size_t baseSize) {
	_ModMapPvt map = calloc(1, sizeof(struct _ModMapPvt));
	map->base = baseSize;
	map->size = baseSize;
	map->list = calloc(baseSize, sizeof(struct _Kav));
	return (ModMap)map;
}

#define _MAP ((_ModMapPvt)map)

bool mdmpResize(ModMap map, size_t size) {
	if (size <= _MAP->base) {
		return false;
	}
	_Kavs newList = calloc(size, sizeof(struct _Kav));
	_Kavs oldList = _MAP->list;
	memcpy(newList, oldList, _MAP->size);
	_MAP->list = newList;
	_MAP->size = size;
	free(oldList);
	return true;
}

bool mdmpSet(ModMap map, void *key, void *value) {
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
					mdmpResize(map, size);
					return true;
				}
			} else {
				mdmpResize(map, size);
				_MAP->list[ix].key = key;
				_MAP->list[ix].value = value;
				return true;
			}
		}
	}
	return false;
}

void *mdmpGet(ModMap map, void *key) {
	for (size_t dvdnd = _MAP->base; dvdnd <= _MAP->size; dvdnd += _MAP->base) {
		size_t ix = (size_t)key % dvdnd;
		if (_MAP->list[ix].key == key) {
			return _MAP->list[ix].value;
		}
	}
	return NULL;
}

void mdmpDelete(ModMap map, void *key) {
	for (size_t dvdnd = _MAP->base; dvdnd <= _MAP->size; dvdnd += _MAP->base) {
		size_t ix = (size_t)key % dvdnd;
		if (_MAP->list[ix].key == key) {
			_MAP->list[ix].key = NULL;
			_MAP->list[ix].value = NULL;
		}
	}
}

void mdmpFree(ModMap map) {
	free(_MAP->list);
	free(_MAP);
}

#undef _MAP
#endif // !_MODMAP_H
