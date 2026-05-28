#include "categorical_agents.h"
#include <stdlib.h>
#include <string.h>

/* Find an object by id. Returns pointer into the objects array or NULL. */
CatObject* category_find_object(Category* cat, const char* id) {
    if (!cat || !id) return NULL;
    for (int i = 0; i < cat->num_objects; i++) {
        if (strcmp(cat->objects[i].id, id) == 0)
            return &cat->objects[i];
    }
    return NULL;
}

/* Find a morphism by id. */
Morphism* category_find_morphism(Category* cat, const char* id) {
    if (!cat || !id) return NULL;
    for (int i = 0; i < cat->num_morphisms; i++) {
        if (strcmp(cat->morphisms[i].id, id) == 0)
            return &cat->morphisms[i];
    }
    /* Also check dynamic morphisms (identities, composites) */
    for (int i = 0; i < cat->num_dynamic; i++) {
        if (strcmp(cat->dynamic_morphisms[i].id, id) == 0)
            return &cat->dynamic_morphisms[i];
    }
    return NULL;
}
