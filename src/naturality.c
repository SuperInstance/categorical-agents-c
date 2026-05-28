#include "categorical_agents.h"
#include <stdlib.h>
#include <string.h>

NaturalTransformation* nat_transform_create(Functor* src, Functor* tgt,
                                             Category* dom) {
    if (!src || !tgt || !dom) return NULL;
    NaturalTransformation* nt = (NaturalTransformation*)calloc(1, sizeof(NaturalTransformation));
    if (!nt) return NULL;
    nt->source = src;
    nt->target = tgt;
    nt->num_components = dom->num_objects;
    nt->components = (Morphism**)calloc(nt->num_components, sizeof(Morphism*));
    if (!nt->components) {
        free(nt);
        return NULL;
    }
    return nt;
}

void nat_transform_free(NaturalTransformation* nt) {
    if (!nt) return;
    free(nt->components);
    free(nt);
}

int nat_transform_set_component(NaturalTransformation* nt, int index, Morphism* m) {
    if (!nt || index < 0 || index >= nt->num_components) return -1;
    nt->components[index] = m;
    return 0;
}

Morphism* nat_transform_get_component(NaturalTransformation* nt, int index) {
    if (!nt || index < 0 || index >= nt->num_components) return NULL;
    return nt->components[index];
}
