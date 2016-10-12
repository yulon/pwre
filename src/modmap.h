#ifndef _MODMAP_H
#define _MODMAP_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ModMap *ModMap;

ModMap new_ModMap(size_t);

bool ModMap_resize(ModMap, size_t);

bool _ModMap_set(ModMap, void *key, void *value);
#define ModMap_set(map, key, value) _ModMap_set(map, (void *)key, (void *)value)

void *_ModMap_get(ModMap, void *key);
#define ModMap_get(map, key) _ModMap_get(map, (void *)key)

void _ModMap_delete(ModMap, void *key);
#define ModMap_delete(map, key) _ModMap_delete(map, (void *)key)

void ModMap_free(ModMap);

#ifdef __cplusplus
}
#endif

#endif // !_MODMAP_H
