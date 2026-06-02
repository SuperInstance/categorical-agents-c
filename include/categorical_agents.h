#ifndef CATEGORICAL_AGENTS_H
#define CATEGORICAL_AGENTS_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================
 * Core category theory types
 * ============================================================ */

#define CAT_MAX_OBJECTS    64
#define CAT_MAX_MORPHISMS  128
#define CAT_MAX_NAT_COMPS  64
#define MAX_CAPABILITIES   32
#define MAX_STRATEGIES     16

typedef size_t ObjId;
typedef size_t MorId;

/* A morphism: f : dom -> cod */
typedef struct {
    MorId  id;
    ObjId  dom;   /* source object */
    ObjId  cod;   /* target object */
    char   label[64];
} Morphism;

/* A category: collection of objects and morphisms with composition */
typedef struct {
    char       name[64];
    ObjId      objects[CAT_MAX_OBJECTS];
    size_t     num_objects;
    Morphism   morphisms[CAT_MAX_MORPHISMS];
    size_t     num_morphisms;
    /* composition table: comp[i][j] = MorId of compose(morph[i], morph[j])
       -1 means undefined (stored as (MorId)-1 sentinel) */
    MorId      comp_table[CAT_MAX_MORPHISMS][CAT_MAX_MORPHISMS];
} Category;

/* ============================================================
 * Functor: mapping between categories
 * ============================================================ */

typedef struct {
    ObjId  obj_map[CAT_MAX_OBJECTS];   /* obj_map[src] = dst */
    MorId  mor_map[CAT_MAX_MORPHISMS]; /* mor_map[src] = dst */
    Category *src_cat;
    Category *dst_cat;
} Functor;

/* ============================================================
 * Natural transformation
 * ============================================================ */

typedef struct {
    Functor  *from;  /* source functor */
    Functor  *to;    /* target functor */
    MorId    components[CAT_MAX_NAT_COMPS]; /* component per object */
    size_t   num_components;
} NatTrans;

/* ============================================================
 * Monoidal category
 * ============================================================ */

typedef struct {
    Category  base;
    ObjId     tensor_obj_table[CAT_MAX_OBJECTS][CAT_MAX_OBJECTS]; /* tensor(a,b) */
    MorId     tensor_mor_table[CAT_MAX_MORPHISMS][CAT_MAX_MORPHISMS];
    ObjId     unit_obj; /* monoidal unit I */
} MonoidalCategory;

/* Symmetric monoidal: adds braiding */
typedef struct {
    MonoidalCategory mono;
    MorId  braid[CAT_MAX_OBJECTS][CAT_MAX_OBJECTS]; /* sigma_{a,b} : a⊗b -> b⊗a */
} SymMonoidalCategory;

/* ============================================================
 * Agent capabilities & protocols
 * ============================================================ */

typedef struct {
    char  label[64];
    ObjId id;
} Capability;

typedef struct {
    Capability  caps[MAX_CAPABILITIES];
    size_t      num_caps;
} CapSet;

/* A protocol maps one capability set to another (a morphism in the cap category) */
typedef struct {
    char   name[64];
    CapSet dom;
    CapSet cod;
    MorId  morphism_id;
} Protocol;

/* ============================================================
 * Composition strategies
 * ============================================================ */

typedef enum {
    STRAT_SEQUENTIAL,
    STRAT_PARALLEL,
    STRAT_CONDITIONAL
} StrategyKind;

typedef struct {
    StrategyKind kind;
    MorId        first;
    MorId        second;
    MorId        result;   /* composed morphism */
    /* conditional fields */
    bool         condition_flag;
} ComposeStrategy;

typedef struct {
    ComposeStrategy strategies[MAX_STRATEGIES];
    size_t          num_strategies;
} StrategySet;

/* ============================================================
 * API
 * ============================================================ */

/* Category */
void     cat_init(Category *c, const char *name);
ObjId    cat_add_object(Category *c);
Morphism *cat_add_morphism(Category *c, ObjId dom, ObjId cod, const char *label);
bool     cat_set_compose(Category *c, MorId f, MorId g, MorId fg);
MorId    cat_compose(Category *c, MorId f, MorId g);
Morphism *cat_find_morphism(Category *c, ObjId dom, ObjId cod);
Morphism *cat_identity(Category *c, ObjId obj);
bool     cat_check_associativity(Category *c, MorId f, MorId g, MorId h);

/* Functor */
void  fun_init(Functor *f, Category *src, Category *dst);
void  fun_map_obj(Functor *f, ObjId src, ObjId dst);
void  fun_map_mor(Functor *f, MorId src, MorId dst);
ObjId fun_apply_obj(Functor *f, ObjId src);
MorId fun_apply_mor(Functor *f, MorId src);
bool  fun_preserves_composition(Functor *f);
bool  fun_preserves_identity(Functor *f);

/* Natural transformation */
void  nat_init(NatTrans *nt, Functor *from, Functor *to);
void  nat_set_component(NatTrans *nt, ObjId obj, MorId comp);
MorId nat_get_component(NatTrans *nt, ObjId obj);
bool  nat_check_naturality(NatTrans *nt, MorId f);

/* Monoidal */
void  mono_init(MonoidalCategory *mc, const char *name, ObjId unit);
ObjId mono_tensor_obj(MonoidalCategory *mc, ObjId a, ObjId b);
MorId mono_tensor_mor(MonoidalCategory *mc, MorId f, MorId g);
void  mono_set_tensor_obj(MonoidalCategory *mc, ObjId a, ObjId b, ObjId result);
void  mono_set_tensor_mor(MonoidalCategory *mc, MorId f, MorId g, MorId result);

/* Symmetric monoidal */
void   symm_init(SymMonoidalCategory *smc, const char *name, ObjId unit);
void   symm_set_braid(SymMonoidalCategory *smc, ObjId a, ObjId b, MorId sigma);
MorId  symm_get_braid(SymMonoidalCategory *smc, ObjId a, ObjId b);
bool   symm_check_hexagon(SymMonoidalCategory *smc, ObjId a, ObjId b, ObjId c);
bool   symm_check_involution(SymMonoidalCategory *smc, ObjId a, ObjId b);

/* Capabilities & protocols */
void  capset_init(CapSet *cs);
void  capset_add(CapSet *cs, const char *label, ObjId id);
bool  capset_contains(CapSet *cs, ObjId id);
bool  capset_equals(CapSet *a, CapSet *b);
void  proto_init(Protocol *p, const char *name, CapSet *dom, CapSet *cod);
bool  proto_is_valid(Protocol *p, Category *c);

/* Composition strategies */
void  strat_init(StrategySet *ss);
MorId strat_sequential(Category *c, StrategySet *ss, MorId f, MorId g);
MorId strat_parallel(MonoidalCategory *mc, StrategySet *ss, MorId f, MorId g);
MorId strat_conditional(Category *c, StrategySet *ss, MorId f, MorId g, bool cond);

#ifdef __cplusplus
}
#endif

#endif /* CATEGORICAL_AGENTS_H */
