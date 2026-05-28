#include "categorical_agents.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Allocate a stable copy of a string in the category's id pool */
static const char* pool_strdup(Category* cat, const char* s) {
    if (cat->num_ids >= cat->max_ids) {
        int new_max = cat->max_ids * 2;
        char** new_pool = (char**)realloc(cat->id_pool, new_max * sizeof(char*));
        if (!new_pool) return NULL;
        cat->id_pool = new_pool;
        cat->max_ids = new_max;
    }
    char* copy = (char*)malloc(strlen(s) + 1);
    if (!copy) return NULL;
    strcpy(copy, s);
    cat->id_pool[cat->num_ids++] = copy;
    return copy;
}

Category* category_create(int max_objects, int max_morphisms) {
    Category* cat = (Category*)calloc(1, sizeof(Category));
    if (!cat) return NULL;

    cat->max_objects = max_objects;
    cat->max_morphisms = max_morphisms;
    cat->objects = (CatObject*)calloc(max_objects, sizeof(CatObject));
    cat->morphisms = (Morphism*)calloc(max_morphisms, sizeof(Morphism));
    if (!cat->objects || !cat->morphisms) {
        free(cat->objects);
        free(cat->morphisms);
        free(cat);
        return NULL;
    }

    cat->max_dynamic = 256;
    cat->dynamic_morphisms = (Morphism*)calloc(cat->max_dynamic, sizeof(Morphism));
    if (!cat->dynamic_morphisms) {
        free(cat->objects);
        free(cat->morphisms);
        free(cat);
        return NULL;
    }

    cat->max_ids = 256;
    cat->id_pool = (char**)calloc(cat->max_ids, sizeof(char*));
    if (!cat->id_pool) {
        free(cat->objects);
        free(cat->morphisms);
        free(cat->dynamic_morphisms);
        free(cat);
        return NULL;
    }

    return cat;
}

void category_free(Category* cat) {
    if (!cat) return;
    for (int i = 0; i < cat->num_ids; i++)
        free(cat->id_pool[i]);
    free(cat->id_pool);
    free(cat->objects);
    free(cat->morphisms);
    free(cat->dynamic_morphisms);
    free(cat);
}

int category_add_object(Category* cat, const char* id, const char* type, void* data) {
    if (!cat || cat->num_objects >= cat->max_objects) return -1;
    CatObject* obj = &cat->objects[cat->num_objects++];
    obj->id = id;
    obj->type = type;
    obj->data = data;
    return 0;
}

int category_add_morphism(Category* cat, const char* id,
                          const char* source_id, const char* target_id,
                          void* (*apply)(void*)) {
    if (!cat || cat->num_morphisms >= cat->max_morphisms) return -1;
    CatObject* src = category_find_object(cat, source_id);
    CatObject* tgt = category_find_object(cat, target_id);
    if (!src || !tgt) return -1;

    Morphism* m = &cat->morphisms[cat->num_morphisms++];
    m->id = id;
    m->source = src;
    m->target = tgt;
    m->apply = apply;
    return 0;
}

/* Grow dynamic morphism array if needed */
static int ensure_dynamic_capacity(Category* cat) {
    if (cat->num_dynamic >= cat->max_dynamic) {
        int new_max = cat->max_dynamic * 2;
        Morphism* new_arr = (Morphism*)realloc(cat->dynamic_morphisms,
                                                new_max * sizeof(Morphism));
        if (!new_arr) return -1;
        cat->dynamic_morphisms = new_arr;
        cat->max_dynamic = new_max;
    }
    return 0;
}

/* Add a dynamic morphism (identity or composite). Returns pointer to it. */
static Morphism* add_dynamic(Category* cat, const char* id,
                             CatObject* src, CatObject* tgt,
                             void* (*apply)(void*)) {
    if (ensure_dynamic_capacity(cat) < 0) return NULL;
    const char* stable_id = pool_strdup(cat, id);
    if (!stable_id) return NULL;
    Morphism* m = &cat->dynamic_morphisms[cat->num_dynamic++];
    m->id = stable_id;
    m->source = src;
    m->target = tgt;
    m->apply = apply;
    return m;
}

Morphism* category_identity(Category* cat, CatObject* obj) {
    if (!cat || !obj) return NULL;

    char idbuf[128];
    snprintf(idbuf, sizeof(idbuf), "id_%s", obj->id);

    Morphism* existing = category_find_morphism(cat, idbuf);
    if (existing) return existing;

    return add_dynamic(cat, idbuf, obj, obj, NULL);
}

Morphism* category_compose(Category* cat, Morphism* f, Morphism* g) {
    if (!cat || !f || !g) return NULL;
    if (!f->target || !g->source) return NULL;
    if (strcmp(f->target->id, g->source->id) != 0) return NULL;

    char idbuf[256];
    snprintf(idbuf, sizeof(idbuf), "comp_%s_%s", f->id, g->id);
    Morphism* existing = category_find_morphism(cat, idbuf);
    if (existing) return existing;

    return add_dynamic(cat, idbuf, f->source, g->target, NULL);
}
