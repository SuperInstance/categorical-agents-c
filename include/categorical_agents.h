#ifndef CATEGORICAL_AGENTS_H
#define CATEGORICAL_AGENTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/* --- Objects --- */
/* Objects are capabilities or agents identified by string IDs. */
typedef struct {
    const char* id;
    const char* type;
    void* data;
} CatObject;

/* --- Morphisms --- */
/* Morphisms are protocols/transformations between objects. */
typedef struct Morphism {
    const char* id;
    CatObject* source;
    CatObject* target;
    void* (*apply)(void* input);
} Morphism;

/* --- Category --- */
/* A category: collection of objects + morphisms with composition. */
typedef struct {
    CatObject* objects;
    int num_objects;
    int max_objects;
    Morphism* morphisms;
    int num_morphisms;
    int max_morphisms;
    /* Internal storage for identity and composite morphisms */
    Morphism* dynamic_morphisms;
    int num_dynamic;
    int max_dynamic;
    /* Internal string pool for generated ids (identities, composites) */
    char** id_pool;
    int num_ids;
    int max_ids;
} Category;

/* Category lifecycle */
Category* category_create(int max_objects, int max_morphisms);
void category_free(Category* cat);

/* Add objects and morphisms */
int category_add_object(Category* cat, const char* id, const char* type, void* data);
int category_add_morphism(Category* cat, const char* id,
                          const char* source_id, const char* target_id,
                          void* (*apply)(void*));

/* Lookup */
CatObject* category_find_object(Category* cat, const char* id);
Morphism* category_find_morphism(Category* cat, const char* id);

/* Composition: g ∘ f (apply f then g). Returns NULL if not composable. */
Morphism* category_compose(Category* cat, Morphism* f, Morphism* g);

/* Identity morphism for an object */
Morphism* category_identity(Category* cat, CatObject* obj);

/* --- Functors --- */
/* Structure-preserving maps between categories. */
typedef struct {
    Category* source;
    Category* target;
    CatObject* (*map_object)(CatObject*, void* ctx);
    Morphism* (*map_morphism)(Morphism*, void* ctx);
    void* ctx;
} Functor;

Functor* functor_create(Category* src, Category* target,
                        CatObject* (*map_obj)(CatObject*, void* ctx),
                        Morphism* (*map_mor)(Morphism*, void* ctx),
                        void* ctx);
void functor_free(Functor* f);

/* Apply functor */
CatObject* functor_map_object(Functor* f, CatObject* obj);
Morphism* functor_map_morphism(Functor* f, Morphism* m);

/* --- Natural Transformations --- */
/* Family of morphisms between functors. */
typedef struct {
    Functor* source;
    Functor* target;
    Morphism** components;  /* one per object in the source category */
    int num_components;
} NaturalTransformation;

NaturalTransformation* nat_transform_create(Functor* src, Functor* tgt,
                                             Category* dom);
void nat_transform_free(NaturalTransformation* nt);

int nat_transform_set_component(NaturalTransformation* nt, int index,
                                Morphism* m);
Morphism* nat_transform_get_component(NaturalTransformation* nt, int index);

#ifdef __cplusplus
}
#endif

#endif /* CATEGORICAL_AGENTS_H */
