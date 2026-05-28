#include "categorical_agents.h"
#include <stdlib.h>
#include <string.h>

Functor* functor_create(Category* src, Category* target,
                        CatObject* (*map_obj)(CatObject*, void* ctx),
                        Morphism* (*map_mor)(Morphism*, void* ctx),
                        void* ctx) {
    Functor* f = (Functor*)calloc(1, sizeof(Functor));
    if (!f) return NULL;
    f->source = src;
    f->target = target;
    f->map_object = map_obj;
    f->map_morphism = map_mor;
    f->ctx = ctx;
    return f;
}

void functor_free(Functor* f) {
    free(f);
}

CatObject* functor_map_object(Functor* f, CatObject* obj) {
    if (!f || !f->map_object) return NULL;
    return f->map_object(obj, f->ctx);
}

Morphism* functor_map_morphism(Functor* f, Morphism* m) {
    if (!f || !f->map_morphism) return NULL;
    return f->map_morphism(m, f->ctx);
}
