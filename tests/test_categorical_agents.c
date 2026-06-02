#include "categorical_agents.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

static int tests_run    = 0;
static int tests_passed = 0;

#define TEST(name) do { \
    tests_run++; \
    printf("  TEST %-45s ", #name); \
} while(0)

#define PASS() do { tests_passed++; printf("PASS\n"); } while(0)
#define FAIL(msg) do { printf("FAIL: %s\n", msg); } while(0)

/* ============================================================ */

static void test_category_create(void) {
    TEST(category_create);
    Category c;
    cat_init(&c, "TestCat");
    assert(strcmp(c.name, "TestCat") == 0);
    assert(c.num_objects == 0);
    assert(c.num_morphisms == 0);
    PASS();
}

static void test_add_objects(void) {
    TEST(add_objects);
    Category c;
    cat_init(&c, "ObjCat");
    ObjId a = cat_add_object(&c);
    ObjId b = cat_add_object(&c);
    ObjId cc = cat_add_object(&c);
    assert(c.num_objects == 3);
    assert(a == 0 && b == 1 && cc == 2);
    PASS();
}

static void test_add_morphism(void) {
    TEST(add_morphism);
    Category c;
    cat_init(&c, "MorCat");
    ObjId a = cat_add_object(&c);
    ObjId b = cat_add_object(&c);
    Morphism *f = cat_add_morphism(&c, a, b, "f");
    assert(f != NULL);
    assert(f->dom == a && f->cod == b);
    assert(strcmp(f->label, "f") == 0);
    assert(c.num_morphisms == 1);
    PASS();
}

static void test_compose_morphisms(void) {
    TEST(compose_morphisms);
    Category c;
    cat_init(&c, "CompCat");
    ObjId a = cat_add_object(&c);
    ObjId b = cat_add_object(&c);
    ObjId d = cat_add_object(&c);
    Morphism *f = cat_add_morphism(&c, a, b, "f");
    Morphism *g = cat_add_morphism(&c, b, d, "g");
    Morphism *fg = cat_add_morphism(&c, a, d, "f;g");
    bool ok = cat_set_compose(&c, f->id, g->id, fg->id);
    assert(ok);
    MorId result = cat_compose(&c, f->id, g->id);
    assert(result == fg->id);
    PASS();
}

static void test_compose_domain_mismatch(void) {
    TEST(compose_domain_mismatch);
    Category c;
    cat_init(&c, "Mismatch");
    ObjId a = cat_add_object(&c);
    ObjId b = cat_add_object(&c);
    ObjId d = cat_add_object(&c);
    Morphism *f = cat_add_morphism(&c, a, b, "f");
    Morphism *g = cat_add_morphism(&c, d, a, "g"); /* dom(g)=d != cod(f)=b */
    Morphism *fg = cat_add_morphism(&c, a, a, "fg");
    bool ok = cat_set_compose(&c, f->id, g->id, fg->id);
    assert(!ok); /* should fail */
    PASS();
}

static void test_identity_morphism(void) {
    TEST(identity_morphism);
    Category c;
    cat_init(&c, "IdCat");
    ObjId a = cat_add_object(&c);
    Morphism *id_a = cat_identity(&c, a);
    assert(id_a != NULL);
    assert(id_a->dom == a && id_a->cod == a);
    PASS();
}

static void test_identity_laws(void) {
    TEST(identity_laws);
    Category c;
    cat_init(&c, "IdLawCat");
    ObjId a = cat_add_object(&c);
    ObjId b = cat_add_object(&c);
    Morphism *f = cat_add_morphism(&c, a, b, "f");
    Morphism *id_a = cat_identity(&c, a);
    Morphism *id_b = cat_identity(&c, b);
    /* f then id_b == f  (left identity: id_b ∘ f = f) */
    MorId r1 = cat_compose(&c, f->id, id_b->id);
    assert(r1 == f->id);
    /* id_a then f == f  (right identity: f ∘ id_a = f) */
    MorId r2 = cat_compose(&c, id_a->id, f->id);
    assert(r2 == f->id);
    PASS();
}

static void test_find_morphism(void) {
    TEST(find_morphism);
    Category c;
    cat_init(&c, "FindCat");
    ObjId a = cat_add_object(&c);
    ObjId b = cat_add_object(&c);
    cat_add_morphism(&c, a, b, "f");
    Morphism *found = cat_find_morphism(&c, a, b);
    assert(found != NULL);
    assert(strcmp(found->label, "f") == 0);
    Morphism *notfound = cat_find_morphism(&c, b, a);
    assert(notfound == NULL);
    PASS();
}

static void test_associativity(void) {
    TEST(associativity);
    Category c;
    cat_init(&c, "AssocCat");
    ObjId a = cat_add_object(&c);
    ObjId b = cat_add_object(&c);
    ObjId d = cat_add_object(&c);
    ObjId e = cat_add_object(&c);
    Morphism *f = cat_add_morphism(&c, a, b, "f");
    Morphism *g = cat_add_morphism(&c, b, d, "g");
    Morphism *h = cat_add_morphism(&c, d, e, "h");
    Morphism *fg = cat_add_morphism(&c, a, d, "fg");
    Morphism *gh = cat_add_morphism(&c, b, e, "gh");
    Morphism *fgh = cat_add_morphism(&c, a, e, "fgh");
    cat_set_compose(&c, f->id, g->id, fg->id);
    cat_set_compose(&c, g->id, h->id, gh->id);
    cat_set_compose(&c, fg->id, h->id, fgh->id);
    cat_set_compose(&c, f->id, gh->id, fgh->id);
    assert(cat_check_associativity(&c, f->id, g->id, h->id));
    PASS();
}

static void test_functor_create(void) {
    TEST(functor_create);
    Category src, dst;
    cat_init(&src, "Src");
    cat_init(&dst, "Dst");
    Functor F;
    fun_init(&F, &src, &dst);
    assert(F.src_cat == &src);
    assert(F.dst_cat == &dst);
    PASS();
}

static void test_functor_obj_map(void) {
    TEST(functor_obj_map);
    Category src, dst;
    cat_init(&src, "S"); cat_init(&dst, "D");
    ObjId sa = cat_add_object(&src);
    ObjId da = cat_add_object(&dst);
    Functor F;
    fun_init(&F, &src, &dst);
    fun_map_obj(&F, sa, da);
    assert(fun_apply_obj(&F, sa) == da);
    PASS();
}

static void test_functor_mor_map(void) {
    TEST(functor_mor_map);
    Category src, dst;
    cat_init(&src, "S"); cat_init(&dst, "D");
    ObjId sa = cat_add_object(&src), sb = cat_add_object(&src);
    ObjId da = cat_add_object(&dst), db = cat_add_object(&dst);
    Morphism *sf = cat_add_morphism(&src, sa, sb, "f");
    Morphism *df = cat_add_morphism(&dst, da, db, "Ff");
    Functor F;
    fun_init(&F, &src, &dst);
    fun_map_obj(&F, sa, da);
    fun_map_obj(&F, sb, db);
    fun_map_mor(&F, sf->id, df->id);
    assert(fun_apply_mor(&F, sf->id) == df->id);
    PASS();
}

static void test_functor_preserves_composition(void) {
    TEST(functor_preserves_composition);
    Category src, dst;
    cat_init(&src, "S"); cat_init(&dst, "D");
    ObjId sa = cat_add_object(&src), sb = cat_add_object(&src), sc = cat_add_object(&src);
    ObjId da = cat_add_object(&dst), db = cat_add_object(&dst), dc = cat_add_object(&dst);
    Morphism *sf = cat_add_morphism(&src, sa, sb, "f");
    Morphism *sg = cat_add_morphism(&src, sb, sc, "g");
    Morphism *sfg = cat_add_morphism(&src, sa, sc, "fg");
    cat_set_compose(&src, sf->id, sg->id, sfg->id);

    Morphism *df = cat_add_morphism(&dst, da, db, "Ff");
    Morphism *dg = cat_add_morphism(&dst, db, dc, "Fg");
    Morphism *dfg = cat_add_morphism(&dst, da, dc, "Ffg");
    cat_set_compose(&dst, df->id, dg->id, dfg->id);

    Functor F;
    fun_init(&F, &src, &dst);
    fun_map_obj(&F, sa, da); fun_map_obj(&F, sb, db); fun_map_obj(&F, sc, dc);
    fun_map_mor(&F, sf->id, df->id); fun_map_mor(&F, sg->id, dg->id);
    fun_map_mor(&F, sfg->id, dfg->id);
    assert(fun_preserves_composition(&F));
    PASS();
}

static void test_functor_preserves_identity(void) {
    TEST(functor_preserves_identity);
    Category src, dst;
    cat_init(&src, "S"); cat_init(&dst, "D");
    ObjId sa = cat_add_object(&src);
    ObjId da = cat_add_object(&dst);
    Morphism *sid = cat_identity(&src, sa);
    Morphism *did = cat_identity(&dst, da);
    Functor F;
    fun_init(&F, &src, &dst);
    fun_map_obj(&F, sa, da);
    fun_map_mor(&F, sid->id, did->id);
    assert(fun_preserves_identity(&F));
    PASS();
}

static void test_natural_transformation(void) {
    TEST(natural_transformation);
    Category src, dst;
    cat_init(&src, "S"); cat_init(&dst, "D");
    ObjId sa = cat_add_object(&src), sb = cat_add_object(&src);
    ObjId da0 = cat_add_object(&dst), db0 = cat_add_object(&dst);
    ObjId da1 = cat_add_object(&dst), db1 = cat_add_object(&dst);
    /* F, G : src -> dst */
    Functor F, G;
    fun_init(&F, &src, &dst); fun_init(&G, &src, &dst);
    fun_map_obj(&F, sa, da0); fun_map_obj(&F, sb, db0);
    fun_map_obj(&G, sa, da1); fun_map_obj(&G, sb, db1);

    Morphism *sf = cat_add_morphism(&src, sa, sb, "f");
    Morphism *Ff = cat_add_morphism(&dst, da0, db0, "Ff");
    Morphism *Gf = cat_add_morphism(&dst, da1, db1, "Gf");
    fun_map_mor(&F, sf->id, Ff->id);
    fun_map_mor(&G, sf->id, Gf->id);

    /* alpha_a : F(a) -> G(a), alpha_b : F(b) -> G(b) */
    Morphism *alpha_a = cat_add_morphism(&dst, da0, da1, "αa");
    Morphism *alpha_b = cat_add_morphism(&dst, db0, db1, "αb");
    /* naturality: Gf ∘ αa == αb ∘ Ff */
    MorId lhs_unused = cat_compose(&dst, Gf->id, alpha_a->id);
    MorId rhs_unused = cat_compose(&dst, alpha_b->id, Ff->id);
    (void)lhs_unused; (void)rhs_unused;
    /* both undefined — set up composition */
    MorId both = cat_add_morphism(&dst, da0, db1, "nat_sq")->id;
    /* αa first then Gf = both */
    cat_set_compose(&dst, alpha_a->id, Gf->id, both);
    /* Ff first then αb = both */
    cat_set_compose(&dst, Ff->id, alpha_b->id, both);

    NatTrans nt;
    nat_init(&nt, &F, &G);
    nat_set_component(&nt, sa, alpha_a->id);
    nat_set_component(&nt, sb, alpha_b->id);
    assert(nat_check_naturality(&nt, sf->id));
    PASS();
}

static void test_monoidal_tensor_obj(void) {
    TEST(monoidal_tensor_obj);
    MonoidalCategory mc;
    mono_init(&mc, "MonCat", 0);
    ObjId a = cat_add_object(&mc.base);
    ObjId b = cat_add_object(&mc.base);
    ObjId ab = cat_add_object(&mc.base);
    mono_set_tensor_obj(&mc, a, b, ab);
    assert(mono_tensor_obj(&mc, a, b) == ab);
    PASS();
}

static void test_monoidal_unit(void) {
    TEST(monoidal_unit);
    MonoidalCategory mc;
    mono_init(&mc, "MonCat", 0);
    ObjId I = cat_add_object(&mc.base);
    ObjId a = cat_add_object(&mc.base);
    mono_set_tensor_obj(&mc, I, a, a);
    mono_set_tensor_obj(&mc, a, I, a);
    assert(mono_tensor_obj(&mc, I, a) == a);
    assert(mono_tensor_obj(&mc, a, I) == a);
    PASS();
}

static void test_monoidal_tensor_mor(void) {
    TEST(monoidal_tensor_mor);
    MonoidalCategory mc;
    mono_init(&mc, "MonCat", 0);
    ObjId a = cat_add_object(&mc.base);
    ObjId b = cat_add_object(&mc.base);
    Morphism *f = cat_add_morphism(&mc.base, a, b, "f");
    Morphism *g = cat_add_morphism(&mc.base, a, b, "g");
    Morphism *fg = cat_add_morphism(&mc.base, a, b, "f⊗g");
    mono_set_tensor_mor(&mc, f->id, g->id, fg->id);
    assert(mono_tensor_mor(&mc, f->id, g->id) == fg->id);
    PASS();
}

static void test_symmetric_braid(void) {
    TEST(symmetric_braid);
    SymMonoidalCategory smc;
    symm_init(&smc, "SymMon", 0);
    ObjId a = cat_add_object(&smc.mono.base);
    ObjId b = cat_add_object(&smc.mono.base);
    ObjId ab = cat_add_object(&smc.mono.base);
    ObjId ba = cat_add_object(&smc.mono.base);
    mono_set_tensor_obj(&smc.mono, a, b, ab);
    mono_set_tensor_obj(&smc.mono, b, a, ba);
    MorId sigma = cat_add_morphism(&smc.mono.base, ab, ba, "σ")->id;
    symm_set_braid(&smc, a, b, sigma);
    assert(symm_get_braid(&smc, a, b) == sigma);
    PASS();
}

static void test_symmetric_involution(void) {
    TEST(symmetric_involution);
    SymMonoidalCategory smc;
    symm_init(&smc, "SymMon", 0);
    ObjId a = cat_add_object(&smc.mono.base);
    ObjId b = cat_add_object(&smc.mono.base);
    ObjId ab = cat_add_object(&smc.mono.base);
    ObjId ba = cat_add_object(&smc.mono.base);
    mono_set_tensor_obj(&smc.mono, a, b, ab);
    mono_set_tensor_obj(&smc.mono, b, a, ba);
    MorId sab = cat_add_morphism(&smc.mono.base, ab, ba, "σab")->id;
    MorId sba = cat_add_morphism(&smc.mono.base, ba, ab, "σba")->id;
    MorId idab = cat_identity(&smc.mono.base, ab)->id;
    symm_set_braid(&smc, a, b, sab);
    symm_set_braid(&smc, b, a, sba);
    cat_set_compose(&smc.mono.base, sba, sab, idab);
    assert(symm_check_involution(&smc, a, b));
    PASS();
}

static void test_symmetric_hexagon(void) {
    TEST(symmetric_hexagon);
    SymMonoidalCategory smc;
    symm_init(&smc, "SymMon", 0);
    ObjId a = cat_add_object(&smc.mono.base);
    ObjId b = cat_add_object(&smc.mono.base);
    ObjId c = cat_add_object(&smc.mono.base);
    /* set up braids */
    symm_set_braid(&smc, a, b, cat_add_morphism(&smc.mono.base, 0, 1, "σab")->id);
    symm_set_braid(&smc, b, c, cat_add_morphism(&smc.mono.base, 1, 2, "σbc")->id);
    symm_set_braid(&smc, a, c, cat_add_morphism(&smc.mono.base, 0, 2, "σac")->id);
    assert(symm_check_hexagon(&smc, a, b, c));
    PASS();
}

static void test_capability_set(void) {
    TEST(capability_set);
    CapSet cs;
    capset_init(&cs);
    capset_add(&cs, "perceive", 0);
    capset_add(&cs, "reason", 1);
    capset_add(&cs, "act", 2);
    assert(cs.num_caps == 3);
    assert(capset_contains(&cs, 1));
    assert(!capset_contains(&cs, 5));
    PASS();
}

static void test_capability_equality(void) {
    TEST(capability_equality);
    CapSet a, b;
    capset_init(&a); capset_init(&b);
    capset_add(&a, "x", 0); capset_add(&a, "y", 1);
    capset_add(&b, "x", 0); capset_add(&b, "y", 1);
    assert(capset_equals(&a, &b));
    CapSet c;
    capset_init(&c);
    capset_add(&c, "x", 0); capset_add(&c, "z", 2);
    assert(!capset_equals(&a, &c));
    PASS();
}

static void test_protocol(void) {
    TEST(protocol);
    CapSet dom, cod;
    capset_init(&dom); capset_init(&cod);
    capset_add(&dom, "perceive", 0);
    capset_add(&cod, "reason", 1);
    Protocol p;
    proto_init(&p, "perceive-to-reason", &dom, &cod);
    assert(strcmp(p.name, "perceive-to-reason") == 0);
    Category c; cat_init(&c, "CapCat");
    assert(proto_is_valid(&p, &c));
    PASS();
}

static void test_protocol_invalid(void) {
    TEST(protocol_invalid);
    CapSet empty;
    capset_init(&empty);
    CapSet cod;
    capset_init(&cod);
    capset_add(&cod, "reason", 1);
    Protocol p;
    proto_init(&p, "bad", &empty, &cod);
    Category c; cat_init(&c, "CapCat");
    assert(!proto_is_valid(&p, &c));
    PASS();
}

static void test_strategy_sequential(void) {
    TEST(strategy_sequential);
    Category c;
    cat_init(&c, "SeqCat");
    ObjId a = cat_add_object(&c), b = cat_add_object(&c), d = cat_add_object(&c);
    MorId f = cat_add_morphism(&c, a, b, "f")->id;
    MorId g = cat_add_morphism(&c, b, d, "g")->id;
    MorId fg = cat_add_morphism(&c, a, d, "fg")->id;
    cat_set_compose(&c, f, g, fg);
    StrategySet ss; strat_init(&ss);
    MorId r = strat_sequential(&c, &ss, f, g);
    assert(r == fg);
    assert(ss.num_strategies == 1);
    assert(ss.strategies[0].kind == STRAT_SEQUENTIAL);
    PASS();
}

static void test_strategy_parallel(void) {
    TEST(strategy_parallel);
    MonoidalCategory mc;
    mono_init(&mc, "ParCat", 0);
    ObjId a = cat_add_object(&mc.base), b = cat_add_object(&mc.base);
    MorId f = cat_add_morphism(&mc.base, a, b, "f")->id;
    MorId g = cat_add_morphism(&mc.base, a, b, "g")->id;
    MorId fg = cat_add_morphism(&mc.base, a, b, "f⊗g")->id;
    mono_set_tensor_mor(&mc, f, g, fg);
    StrategySet ss; strat_init(&ss);
    MorId r = strat_parallel(&mc, &ss, f, g);
    assert(r == fg);
    assert(ss.strategies[0].kind == STRAT_PARALLEL);
    PASS();
}

static void test_strategy_conditional_true(void) {
    TEST(strategy_conditional_true);
    Category c;
    cat_init(&c, "CondCat");
    ObjId a = cat_add_object(&c), b = cat_add_object(&c), d = cat_add_object(&c);
    MorId f = cat_add_morphism(&c, a, b, "f")->id;
    MorId g = cat_add_morphism(&c, b, d, "g")->id;
    MorId fg = cat_add_morphism(&c, a, d, "fg")->id;
    cat_set_compose(&c, f, g, fg);
    StrategySet ss; strat_init(&ss);
    MorId r = strat_conditional(&c, &ss, f, g, true);
    assert(r == fg);
    assert(ss.strategies[0].kind == STRAT_CONDITIONAL);
    assert(ss.strategies[0].condition_flag == true);
    PASS();
}

static void test_strategy_conditional_false(void) {
    TEST(strategy_conditional_false);
    Category c;
    cat_init(&c, "CondCat");
    ObjId a = cat_add_object(&c), b = cat_add_object(&c);
    MorId f = cat_add_morphism(&c, a, b, "f")->id;
    MorId g = cat_add_morphism(&c, a, b, "g")->id;
    StrategySet ss; strat_init(&ss);
    MorId r = strat_conditional(&c, &ss, f, g, false);
    assert(r == f); /* condition false => just return f */
    assert(ss.strategies[0].condition_flag == false);
    PASS();
}

static void test_multiple_compositions(void) {
    TEST(multiple_compositions);
    Category c;
    cat_init(&c, "Multi");
    ObjId a = cat_add_object(&c);
    ObjId b = cat_add_object(&c);
    ObjId d = cat_add_object(&c);
    ObjId e = cat_add_object(&c);
    MorId f = cat_add_morphism(&c, a, b, "f")->id;
    MorId g = cat_add_morphism(&c, b, d, "g")->id;
    MorId h = cat_add_morphism(&c, d, e, "h")->id;
    MorId fg = cat_add_morphism(&c, a, d, "fg")->id;
    MorId gh = cat_add_morphism(&c, b, e, "gh")->id;
    MorId fgh = cat_add_morphism(&c, a, e, "fgh")->id;
    cat_set_compose(&c, f, g, fg);
    cat_set_compose(&c, g, h, gh);
    cat_set_compose(&c, fg, h, fgh);
    cat_set_compose(&c, f, gh, fgh);
    /* verify (f ∘ g) ∘ h == f ∘ (g ∘ h) */
    assert(cat_compose(&c, cat_compose(&c, f, g), h) == cat_compose(&c, f, cat_compose(&c, g, h)));
    PASS();
}

static void test_compose_undefined(void) {
    TEST(compose_undefined);
    Category c;
    cat_init(&c, "Undef");
    ObjId a = cat_add_object(&c);
    ObjId b = cat_add_object(&c);
    MorId f = cat_add_morphism(&c, a, b, "f")->id;
    MorId g = cat_add_morphism(&c, a, b, "g")->id;
    MorId r = cat_compose(&c, f, g);
    assert(r == (MorId)-1);
    PASS();
}

static void test_functor_not_preserving(void) {
    TEST(functor_not_preserving);
    Category src, dst;
    cat_init(&src, "S"); cat_init(&dst, "D");
    ObjId sa = cat_add_object(&src), sb = cat_add_object(&src), sc = cat_add_object(&src);
    ObjId da = cat_add_object(&dst), db = cat_add_object(&dst), dc = cat_add_object(&dst);
    MorId sf = cat_add_morphism(&src, sa, sb, "f")->id;
    MorId sg = cat_add_morphism(&src, sb, sc, "g")->id;
    MorId sfg = cat_add_morphism(&src, sa, sc, "fg")->id;
    cat_set_compose(&src, sf, sg, sfg);
    MorId df = cat_add_morphism(&dst, da, db, "Ff")->id;
    MorId dg = cat_add_morphism(&dst, db, dc, "Fg")->id;
    MorId dfg_wrong = cat_add_morphism(&dst, da, dc, "wrong")->id;
    cat_set_compose(&dst, df, dg, dfg_wrong);
    /* map F(fg) to something different from F(f);F(g) */
    MorId dfg_other = cat_add_morphism(&dst, da, dc, "other")->id;
    Functor F;
    fun_init(&F, &src, &dst);
    fun_map_obj(&F, sa, da); fun_map_obj(&F, sb, db); fun_map_obj(&F, sc, dc);
    fun_map_mor(&F, sf, df); fun_map_mor(&F, sg, dg);
    fun_map_mor(&F, sfg, dfg_other); /* deliberately wrong */
    assert(!fun_preserves_composition(&F));
    PASS();
}

int main(void) {
    printf("=== Categorical Agents C Test Suite ===\n\n");

    test_category_create();
    test_add_objects();
    test_add_morphism();
    test_compose_morphisms();
    test_compose_domain_mismatch();
    test_identity_morphism();
    test_identity_laws();
    test_find_morphism();
    test_associativity();
    test_functor_create();
    test_functor_obj_map();
    test_functor_mor_map();
    test_functor_preserves_composition();
    test_functor_preserves_identity();
    test_natural_transformation();
    test_monoidal_tensor_obj();
    test_monoidal_unit();
    test_monoidal_tensor_mor();
    test_symmetric_braid();
    test_symmetric_involution();
    test_symmetric_hexagon();
    test_capability_set();
    test_capability_equality();
    test_protocol();
    test_protocol_invalid();
    test_strategy_sequential();
    test_strategy_parallel();
    test_strategy_conditional_true();
    test_strategy_conditional_false();
    test_multiple_compositions();
    test_compose_undefined();
    test_functor_not_preserving();

    printf("\n=== Results: %d / %d passed ===\n", tests_passed, tests_run);
    return (tests_passed == tests_run) ? 0 : 1;
}
