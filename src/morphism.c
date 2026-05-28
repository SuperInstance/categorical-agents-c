#include "categorical_agents.h"
#include <stdlib.h>
#include <string.h>

/* Morphism creation is handled by category_add_morphism.
   This file provides helper utilities. */

/* Check if two morphisms are composable: f's target == g's source */
int morphism_composable(Morphism* f, Morphism* g) {
    if (!f || !g) return 0;
    if (!f->target || !g->source) return 0;
    return strcmp(f->target->id, g->source->id) == 0;
}
