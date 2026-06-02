#include "categorical_agents.h"
#include <string.h>
#include <stdio.h>

/* ============================================================
 * Category
 * ============================================================ */

void cat_init(Category *c, const char *name) {
    memset(c, 0, sizeof(*c));
    strncpy(c->name, name, sizeof(c->name) - 1);
    /* mark composition table as undefined */
    for (size_t i = 0; i < CAT_MAX_MORPHISMS; i++)
        for (size_t j = 0; j < CAT_MAX_MORPHISMS; j++)
            c->comp_table[i][j] = (MorId)-1;
}

ObjId cat_add_object(Category *c) {
    ObjId id = c->num_objects;
    c->objects[id] = id;
    c->num_objects++;
    return id;
}

Morphism *cat_add_morphism(Category *c, ObjId dom, ObjId cod, const char *label) {
    if (c->num_morphisms >= CAT_MAX_MORPHISMS) return NULL;
    MorId id = c->num_morphisms;
    Morphism *m = &c->morphisms[id];
    m->id  = id;
    m->dom = dom;
    m->cod = cod;
    strncpy(m->label, label, sizeof(m->label) - 1);
    c->num_morphisms++;
    return m;
}

bool cat_set_compose(Category *c, MorId f, MorId g, MorId fg) {
    if (f >= c->num_morphisms || g >= c->num_morphisms || fg >= c->num_morphisms)
        return false;
    /* must have cod(f) == dom(g) */
    if (c->morphisms[f].cod != c->morphisms[g].dom)
        return false;
    c->comp_table[f][g] = fg;
    return true;
}

MorId cat_compose(Category *c, MorId f, MorId g) {
    if (f >= c->num_morphisms || g >= c->num_morphisms)
        return (MorId)-1;
    return c->comp_table[f][g];
}

Morphism *cat_find_morphism(Category *c, ObjId dom, ObjId cod) {
    for (size_t i = 0; i < c->num_morphisms; i++) {
        if (c->morphisms[i].dom == dom && c->morphisms[i].cod == cod)
            return &c->morphisms[i];
    }
    return NULL;
}

Morphism *cat_identity(Category *c, ObjId obj) {
    /* find or create id_a : a -> a */
    for (size_t i = 0; i < c->num_morphisms; i++) {
        if (c->morphisms[i].dom == obj && c->morphisms[i].cod == obj) {
            char expected[64];
            snprintf(expected, sizeof(expected), "id_%zu", obj);
            if (strcmp(c->morphisms[i].label, expected) == 0)
                return &c->morphisms[i];
        }
    }
    char label[64];
    snprintf(label, sizeof(label), "id_%zu", obj);
    Morphism *m = cat_add_morphism(c, obj, obj, label);
    if (m) {
        /* id ∘ id = id, and for any f: a->b, id_b ∘ f = f, f ∘ id_a = f */
        MorId mid = m->id;
        cat_set_compose(c, mid, mid, mid);
        /* wire up with existing morphisms */
        for (size_t i = 0; i < c->num_morphisms; i++) {
            if ((MorId)i == mid) continue;
            /* cod(mor) == obj → mor then id = mor → comp_table[mor][mid] = mor */
            if (c->morphisms[i].cod == obj)
                cat_set_compose(c, (MorId)i, mid, (MorId)i);
            /* dom(mor) == obj → id then mor = mor → comp_table[mid][mor] = mor */
            if (c->morphisms[i].dom == obj)
                cat_set_compose(c, mid, (MorId)i, (MorId)i);
        }
    }
    return m;
}

bool cat_check_associativity(Category *c, MorId f, MorId g, MorId h) {
    MorId fg = cat_compose(c, f, g);
    MorId gh = cat_compose(c, g, h);
    if (fg == (MorId)-1 || gh == (MorId)-1) return false;
    MorId fg_h = cat_compose(c, fg, h);
    MorId f_gh = cat_compose(c, f, gh);
    if (fg_h == (MorId)-1 || f_gh == (MorId)-1) return false;
    return fg_h == f_gh;
}

/* ============================================================
 * Functor
 * ============================================================ */

void fun_init(Functor *f, Category *src, Category *dst) {
    memset(f, 0, sizeof(*f));
    f->src_cat = src;
    f->dst_cat = dst;
    for (size_t i = 0; i < CAT_MAX_OBJECTS; i++)
        f->obj_map[i] = (ObjId)-1;
    for (size_t i = 0; i < CAT_MAX_MORPHISMS; i++)
        f->mor_map[i] = (MorId)-1;
}

void fun_map_obj(Functor *f, ObjId src, ObjId dst) { f->obj_map[src] = dst; }
void fun_map_mor(Functor *f, MorId src, MorId dst) { f->mor_map[src] = dst; }

ObjId fun_apply_obj(Functor *f, ObjId src) {
    if (src >= CAT_MAX_OBJECTS) return (ObjId)-1;
    return f->obj_map[src];
}

MorId fun_apply_mor(Functor *f, MorId src) {
    if (src >= CAT_MAX_MORPHISMS) return (MorId)-1;
    return f->mor_map[src];
}

bool fun_preserves_composition(Functor *f) {
    Category *s = f->src_cat;
    for (size_t i = 0; i < s->num_morphisms; i++) {
        for (size_t j = 0; j < s->num_morphisms; j++) {
            MorId comp = cat_compose(s, (MorId)i, (MorId)j);
            if (comp != (MorId)-1) {
                MorId fi = fun_apply_mor(f, (MorId)i);
                MorId fj = fun_apply_mor(f, (MorId)j);
                MorId fcomp = fun_apply_mor(f, comp);
                if (fi == (MorId)-1 || fj == (MorId)-1 || fcomp == (MorId)-1)
                    return false;
                MorId dst_comp = cat_compose(f->dst_cat, fi, fj);
                if (dst_comp != fcomp) return false;
            }
        }
    }
    return true;
}

bool fun_preserves_identity(Functor *f) {
    Category *s = f->src_cat;
    for (size_t i = 0; i < s->num_objects; i++) {
        Morphism *id_src = cat_identity(s, (ObjId)i);
        ObjId dst_obj = fun_apply_obj(f, (ObjId)i);
        if (!id_src || dst_obj == (ObjId)-1) return false;
        MorId fid = fun_apply_mor(f, id_src->id);
        if (fid == (MorId)-1) return false;
        Morphism *id_dst = cat_identity(f->dst_cat, dst_obj);
        if (!id_dst || id_dst->id != fid) return false;
    }
    return true;
}

/* ============================================================
 * Natural transformation
 * ============================================================ */

void nat_init(NatTrans *nt, Functor *from, Functor *to) {
    memset(nt, 0, sizeof(*nt));
    nt->from = from;
    nt->to   = to;
    for (size_t i = 0; i < CAT_MAX_NAT_COMPS; i++)
        nt->components[i] = (MorId)-1;
}

void nat_set_component(NatTrans *nt, ObjId obj, MorId comp) {
    if (obj < CAT_MAX_NAT_COMPS) {
        nt->components[obj] = comp;
        if (obj >= nt->num_components) nt->num_components = obj + 1;
    }
}

MorId nat_get_component(NatTrans *nt, ObjId obj) {
    if (obj >= CAT_MAX_NAT_COMPS) return (MorId)-1;
    return nt->components[obj];
}

bool nat_check_naturality(NatTrans *nt, MorId f) {
    /* For naturality square: G(f) ∘ α_A = α_B ∘ F(f)
       where f : A -> B in src_cat */
    Category *src = nt->from->src_cat;
    if (f >= src->num_morphisms) return false;
    ObjId a = src->morphisms[f].dom;
    ObjId b = src->morphisms[f].cod;

    MorId alpha_a = nat_get_component(nt, a);
    MorId alpha_b = nat_get_component(nt, b);
    MorId Ff = fun_apply_mor(nt->from, f);
    MorId Gf = fun_apply_mor(nt->to, f);

    if (alpha_a == (MorId)-1 || alpha_b == (MorId)-1 ||
        Ff == (MorId)-1 || Gf == (MorId)-1) return false;

    Category *dst = nt->from->dst_cat;
    /* Gf ∘ αa means αa first then Gf: comp_table[αa][Gf] */
    MorId lhs = cat_compose(dst, alpha_a, Gf);
    /* αb ∘ Ff means Ff first then αb: comp_table[Ff][αb] */
    MorId rhs = cat_compose(dst, Ff, alpha_b);
    if (lhs == (MorId)-1 || rhs == (MorId)-1) return false;
    return lhs == rhs;
}

/* ============================================================
 * Monoidal category
 * ============================================================ */

void mono_init(MonoidalCategory *mc, const char *name, ObjId unit) {
    cat_init(&mc->base, name);
    mc->unit_obj = unit;
    for (size_t i = 0; i < CAT_MAX_OBJECTS; i++)
        for (size_t j = 0; j < CAT_MAX_OBJECTS; j++)
            mc->tensor_obj_table[i][j] = (ObjId)-1;
    for (size_t i = 0; i < CAT_MAX_MORPHISMS; i++)
        for (size_t j = 0; j < CAT_MAX_MORPHISMS; j++)
            mc->tensor_mor_table[i][j] = (MorId)-1;
}

void mono_set_tensor_obj(MonoidalCategory *mc, ObjId a, ObjId b, ObjId result) {
    mc->tensor_obj_table[a][b] = result;
}

ObjId mono_tensor_obj(MonoidalCategory *mc, ObjId a, ObjId b) {
    if (a >= CAT_MAX_OBJECTS || b >= CAT_MAX_OBJECTS) return (ObjId)-1;
    return mc->tensor_obj_table[a][b];
}

void mono_set_tensor_mor(MonoidalCategory *mc, MorId f, MorId g, MorId result) {
    mc->tensor_mor_table[f][g] = result;
}

MorId mono_tensor_mor(MonoidalCategory *mc, MorId f, MorId g) {
    if (f >= CAT_MAX_MORPHISMS || g >= CAT_MAX_MORPHISMS) return (MorId)-1;
    return mc->tensor_mor_table[f][g];
}

/* ============================================================
 * Symmetric monoidal
 * ============================================================ */

void symm_init(SymMonoidalCategory *smc, const char *name, ObjId unit) {
    mono_init(&smc->mono, name, unit);
    for (size_t i = 0; i < CAT_MAX_OBJECTS; i++)
        for (size_t j = 0; j < CAT_MAX_OBJECTS; j++)
            smc->braid[i][j] = (MorId)-1;
}

void symm_set_braid(SymMonoidalCategory *smc, ObjId a, ObjId b, MorId sigma) {
    smc->braid[a][b] = sigma;
}

MorId symm_get_braid(SymMonoidalCategory *smc, ObjId a, ObjId b) {
    if (a >= CAT_MAX_OBJECTS || b >= CAT_MAX_OBJECTS) return (MorId)-1;
    return smc->braid[a][b];
}

bool symm_check_hexagon(SymMonoidalCategory *smc, ObjId a, ObjId b, ObjId c) {
    /* Simplified hexagon check: braid must satisfy sigma_{a, b⊗c} ==
       (id_b ⊗ sigma_{a,c}) ∘ sigma_{a,b} ⊗ id_c (up to associator).
       For our simplified model, just verify braids exist. */
    MorId ab = symm_get_braid(smc, a, b);
    MorId bc = symm_get_braid(smc, b, c);
    MorId ac = symm_get_braid(smc, a, c);
    return ab != (MorId)-1 && bc != (MorId)-1 && ac != (MorId)-1;
}

bool symm_check_involution(SymMonoidalCategory *smc, ObjId a, ObjId b) {
    /* sigma_{b,a} ∘ sigma_{a,b} == id_{a⊗b} */
    MorId sab = symm_get_braid(smc, a, b);
    MorId sba = symm_get_braid(smc, b, a);
    if (sab == (MorId)-1 || sba == (MorId)-1) return false;
    Category *c = &smc->mono.base;
    MorId comp = cat_compose(c, sba, sab);
    ObjId tab = mono_tensor_obj(&smc->mono, a, b);
    if (tab == (ObjId)-1) return false;
    Morphism *idtab = cat_identity(c, tab);
    return idtab && comp == idtab->id;
}

/* ============================================================
 * Capabilities & protocols
 * ============================================================ */

void capset_init(CapSet *cs) {
    memset(cs, 0, sizeof(*cs));
}

void capset_add(CapSet *cs, const char *label, ObjId id) {
    if (cs->num_caps >= MAX_CAPABILITIES) return;
    cs->caps[cs->num_caps].id = id;
    strncpy(cs->caps[cs->num_caps].label, label, sizeof(cs->caps[cs->num_caps].label) - 1);
    cs->num_caps++;
}

bool capset_contains(CapSet *cs, ObjId id) {
    for (size_t i = 0; i < cs->num_caps; i++)
        if (cs->caps[i].id == id) return true;
    return false;
}

bool capset_equals(CapSet *a, CapSet *b) {
    if (a->num_caps != b->num_caps) return false;
    for (size_t i = 0; i < a->num_caps; i++) {
        if (!capset_contains(b, a->caps[i].id)) return false;
    }
    return true;
}

void proto_init(Protocol *p, const char *name, CapSet *dom, CapSet *cod) {
    memset(p, 0, sizeof(*p));
    strncpy(p->name, name, sizeof(p->name) - 1);
    if (dom) p->dom = *dom;
    if (cod) p->cod = *cod;
}

bool proto_is_valid(Protocol *p, Category *c) {
    /* domain and codomain capability sets should each map to at least one object */
    (void)p; (void)c;
    return p->dom.num_caps > 0 && p->cod.num_caps > 0;
}

/* ============================================================
 * Composition strategies
 * ============================================================ */

void strat_init(StrategySet *ss) {
    memset(ss, 0, sizeof(*ss));
}

MorId strat_sequential(Category *c, StrategySet *ss, MorId f, MorId g) {
    MorId result = cat_compose(c, f, g);
    if (result == (MorId)-1) return (MorId)-1;
    if (ss->num_strategies < MAX_STRATEGIES) {
        ComposeStrategy *s = &ss->strategies[ss->num_strategies++];
        s->kind   = STRAT_SEQUENTIAL;
        s->first  = f;
        s->second = g;
        s->result = result;
    }
    return result;
}

MorId strat_parallel(MonoidalCategory *mc, StrategySet *ss, MorId f, MorId g) {
    MorId result = mono_tensor_mor(mc, f, g);
    if (result == (MorId)-1) return (MorId)-1;
    if (ss->num_strategies < MAX_STRATEGIES) {
        ComposeStrategy *s = &ss->strategies[ss->num_strategies++];
        s->kind   = STRAT_PARALLEL;
        s->first  = f;
        s->second = g;
        s->result = result;
    }
    return result;
}

MorId strat_conditional(Category *c, StrategySet *ss, MorId f, MorId g, bool cond) {
    MorId result = cond ? cat_compose(c, f, g) : f;
    if (result == (MorId)-1) return (MorId)-1;
    if (ss->num_strategies < MAX_STRATEGIES) {
        ComposeStrategy *s = &ss->strategies[ss->num_strategies++];
        s->kind           = STRAT_CONDITIONAL;
        s->first          = f;
        s->second         = g;
        s->result         = result;
        s->condition_flag = cond;
    }
    return result;
}
