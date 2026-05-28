#include "categorical_agents.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) printf("  TEST: %-55s", #name);
#define PASS() do { printf("PASS\n"); tests_passed++; } while(0)
#define FAIL(msg) do { printf("FAIL: %s\n", msg); tests_failed++; } while(0)
#define ASSERT(cond, msg) do { if (!(cond)) { FAIL(msg); return; } } while(0)

/* Static string pools for tests that need stable string pointers */
static char large_obj_ids[100][16];
static char large_mor_ids[300][16];
static char chain_obj_ids[6][8];
static char chain_mor_ids[5][8];

/* --- Test 1: Object creation and lookup --- */
void test_object_creation(void) {
    TEST(object_creation_and_lookup);
    Category* cat = category_create(10, 10);
    ASSERT(cat != NULL, "category_create returned NULL");

    category_add_object(cat, "agent_A", "sensor", NULL);
    category_add_object(cat, "agent_B", "actuator", NULL);

    CatObject* a = category_find_object(cat, "agent_A");
    CatObject* b = category_find_object(cat, "agent_B");
    CatObject* missing = category_find_object(cat, "nonexistent");

    ASSERT(a != NULL, "agent_A not found");
    ASSERT(b != NULL, "agent_B not found");
    ASSERT(missing == NULL, "nonexistent should be NULL");
    ASSERT(strcmp(a->type, "sensor") == 0, "agent_A type mismatch");
    ASSERT(strcmp(b->type, "actuator") == 0, "agent_B type mismatch");

    category_free(cat);
    PASS();
}

/* --- Test 2: Morphism creation between objects --- */
void test_morphism_creation(void) {
    TEST(morphism_creation_between_objects);
    Category* cat = category_create(10, 10);
    category_add_object(cat, "A", "type_a", NULL);
    category_add_object(cat, "B", "type_b", NULL);

    int rc = category_add_morphism(cat, "f", "A", "B", NULL);
    ASSERT(rc == 0, "add_morphism failed");

    Morphism* m = category_find_morphism(cat, "f");
    ASSERT(m != NULL, "morphism f not found");
    ASSERT(strcmp(m->source->id, "A") == 0, "source mismatch");
    ASSERT(strcmp(m->target->id, "B") == 0, "target mismatch");

    category_free(cat);
    PASS();
}

/* --- Test 3: Identity morphism is neutral --- */
void test_identity_neutral(void) {
    TEST(identity_morphism_is_neutral);
    Category* cat = category_create(10, 20);
    category_add_object(cat, "X", "t", NULL);
    category_add_object(cat, "Y", "t", NULL);
    category_add_morphism(cat, "f", "X", "Y", NULL);

    CatObject* x = category_find_object(cat, "X");
    CatObject* y = category_find_object(cat, "Y");
    Morphism* f = category_find_morphism(cat, "f");

    Morphism* id_x = category_identity(cat, x);
    Morphism* id_y = category_identity(cat, y);
    ASSERT(id_x != NULL, "id_X is NULL");
    ASSERT(id_y != NULL, "id_Y is NULL");

    /* f ∘ id_X should have same source/target as f */
    Morphism* r1 = category_compose(cat, id_x, f);
    ASSERT(r1 != NULL, "f ∘ id_X is NULL");
    ASSERT(strcmp(r1->source->id, "X") == 0, "r1 source != X");
    ASSERT(strcmp(r1->target->id, "Y") == 0, "r1 target != Y");

    /* id_Y ∘ f should have same source/target as f */
    Morphism* r2 = category_compose(cat, f, id_y);
    ASSERT(r2 != NULL, "id_Y ∘ f is NULL");
    ASSERT(strcmp(r2->source->id, "X") == 0, "r2 source != X");
    ASSERT(strcmp(r2->target->id, "Y") == 0, "r2 target != Y");

    category_free(cat);
    PASS();
}

/* --- Test 4: Composition is associative --- */
void test_associativity(void) {
    TEST(composition_is_associative);
    Category* cat = category_create(10, 30);
    category_add_object(cat, "A", "t", NULL);
    category_add_object(cat, "B", "t", NULL);
    category_add_object(cat, "C", "t", NULL);
    category_add_object(cat, "D", "t", NULL);

    category_add_morphism(cat, "f", "A", "B", NULL);
    category_add_morphism(cat, "g", "B", "C", NULL);
    category_add_morphism(cat, "h", "C", "D", NULL);

    Morphism* f = category_find_morphism(cat, "f");
    Morphism* g = category_find_morphism(cat, "g");
    Morphism* h = category_find_morphism(cat, "h");

    /* (h ∘ g) ∘ f */
    Morphism* hg = category_compose(cat, g, h);
    ASSERT(hg != NULL, "g∘h is NULL");
    Morphism* hg_f = category_compose(cat, f, hg);
    ASSERT(hg_f != NULL, "(h∘g)∘f is NULL");

    /* h ∘ (g ∘ f) */
    Morphism* gf = category_compose(cat, f, g);
    ASSERT(gf != NULL, "g∘f is NULL");
    Morphism* h_gf = category_compose(cat, gf, h);
    ASSERT(h_gf != NULL, "h∘(g∘f) is NULL");

    /* Both should map A → D */
    ASSERT(strcmp(hg_f->source->id, "A") == 0, "(h∘g)∘f source != A");
    ASSERT(strcmp(hg_f->target->id, "D") == 0, "(h∘g)∘f target != D");
    ASSERT(strcmp(h_gf->source->id, "A") == 0, "h∘(g∘f) source != A");
    ASSERT(strcmp(h_gf->target->id, "D") == 0, "h∘(g∘f) target != D");

    category_free(cat);
    PASS();
}

/* --- Test 5: Composition source/target matching --- */
void test_compose_mismatch(void) {
    TEST(composition_mismatched_returns_null);
    Category* cat = category_create(10, 20);
    category_add_object(cat, "A", "t", NULL);
    category_add_object(cat, "B", "t", NULL);
    category_add_object(cat, "C", "t", NULL);

    category_add_morphism(cat, "f", "A", "B", NULL);
    category_add_morphism(cat, "g", "C", "A", NULL);

    Morphism* f = category_find_morphism(cat, "f");
    Morphism* g = category_find_morphism(cat, "g");

    /* f: A→B, g: C→A — not composable (f->target=B != g->source=C) */
    Morphism* bad = category_compose(cat, f, g);
    ASSERT(bad == NULL, "composing f,g should be NULL");

    /* But g then f is fine: g: C→A, f: A→B */
    Morphism* good = category_compose(cat, g, f);
    ASSERT(good != NULL, "composing g,f should work");
    ASSERT(strcmp(good->source->id, "C") == 0, "g∘f source != C");
    ASSERT(strcmp(good->target->id, "B") == 0, "g∘f target != B");

    category_free(cat);
    PASS();
}

/* --- Test 6: Functor maps objects correctly --- */
static CatObject* mirror_obj(CatObject* obj, void* ctx) {
    (void)ctx;
    return obj;
}
static Morphism* mirror_mor(Morphism* m, void* ctx) {
    (void)ctx;
    return m;
}

void test_functor_maps_objects(void) {
    TEST(functor_maps_objects_correctly);
    Category* c1 = category_create(10, 10);
    category_add_object(c1, "A", "sensor", NULL);
    category_add_object(c1, "B", "actuator", NULL);

    Functor* F = functor_create(c1, c1, mirror_obj, mirror_mor, NULL);
    ASSERT(F != NULL, "functor_create returned NULL");

    CatObject* a = category_find_object(c1, "A");
    CatObject* mapped = functor_map_object(F, a);
    ASSERT(mapped != NULL, "mapped object is NULL");
    ASSERT(strcmp(mapped->id, "A") == 0, "mapped id != A");

    functor_free(F);
    category_free(c1);
    PASS();
}

/* --- Test 7: Functor preserves identity --- */
void test_functor_preserves_identity(void) {
    TEST(functor_preserves_identity);
    Category* c1 = category_create(10, 20);
    category_add_object(c1, "X", "t", NULL);
    CatObject* x = category_find_object(c1, "X");

    Morphism* id_x = category_identity(c1, x);
    ASSERT(id_x != NULL, "id_X NULL");

    Functor* F = functor_create(c1, c1, mirror_obj, mirror_mor, NULL);
    Morphism* f_id = functor_map_morphism(F, id_x);
    ASSERT(f_id != NULL, "F(id_X) NULL");
    ASSERT(strcmp(f_id->source->id, f_id->target->id) == 0,
           "F(id) source != target");
    ASSERT(strcmp(f_id->source->id, "X") == 0, "F(id) != id for X");

    functor_free(F);
    category_free(c1);
    PASS();
}

/* --- Test 8: Functor preserves composition --- */
void test_functor_preserves_composition(void) {
    TEST(functor_preserves_composition);
    Category* c1 = category_create(10, 30);
    category_add_object(c1, "A", "t", NULL);
    category_add_object(c1, "B", "t", NULL);
    category_add_object(c1, "C", "t", NULL);
    category_add_morphism(c1, "f", "A", "B", NULL);
    category_add_morphism(c1, "g", "B", "C", NULL);

    Morphism* f = category_find_morphism(c1, "f");
    Morphism* g = category_find_morphism(c1, "g");
    Morphism* gf = category_compose(c1, f, g);

    Functor* F = functor_create(c1, c1, mirror_obj, mirror_mor, NULL);

    /* F(g) ∘ F(f) */
    Morphism* Ff = functor_map_morphism(F, f);
    Morphism* Fg = functor_map_morphism(F, g);
    Morphism* Fg_Ff = category_compose(c1, Ff, Fg);

    /* F(g∘f) */
    Morphism* Fgf = functor_map_morphism(F, gf);

    ASSERT(Fg_Ff != NULL, "F(g)∘F(f) NULL");
    ASSERT(Fgf != NULL, "F(g∘f) NULL");
    /* Both should map A→C */
    ASSERT(strcmp(Fg_Ff->source->id, "A") == 0, "F(g)∘F(f) source != A");
    ASSERT(strcmp(Fg_Ff->target->id, "C") == 0, "F(g)∘F(f) target != C");
    ASSERT(strcmp(Fgf->source->id, "A") == 0, "F(g∘f) source != A");
    ASSERT(strcmp(Fgf->target->id, "C") == 0, "F(g∘f) target != C");

    functor_free(F);
    category_free(c1);
    PASS();
}

/* --- Test 9: Natural transformation components --- */
void test_nat_transform_components(void) {
    TEST(natural_transformation_components);
    Category* c1 = category_create(10, 20);
    category_add_object(c1, "A", "t", NULL);
    category_add_object(c1, "B", "t", NULL);
    category_add_morphism(c1, "eta_A", "A", "B", NULL);
    category_add_morphism(c1, "eta_B", "B", "A", NULL);

    Functor* F = functor_create(c1, c1, mirror_obj, mirror_mor, NULL);
    Functor* G = functor_create(c1, c1, mirror_obj, mirror_mor, NULL);

    NaturalTransformation* nt = nat_transform_create(F, G, c1);
    ASSERT(nt != NULL, "nat_transform_create returned NULL");
    ASSERT(nt->num_components == 2, "expected 2 components");

    Morphism* eta_a = category_find_morphism(c1, "eta_A");
    Morphism* eta_b = category_find_morphism(c1, "eta_B");

    ASSERT(nat_transform_set_component(nt, 0, eta_a) == 0, "set comp 0 failed");
    ASSERT(nat_transform_set_component(nt, 1, eta_b) == 0, "set comp 1 failed");

    Morphism* c0 = nat_transform_get_component(nt, 0);
    Morphism* c1p = nat_transform_get_component(nt, 1);
    ASSERT(c0 != NULL && strcmp(c0->id, "eta_A") == 0, "component 0 wrong");
    ASSERT(c1p != NULL && strcmp(c1p->id, "eta_B") == 0, "component 1 wrong");

    nat_transform_free(nt);
    functor_free(F);
    functor_free(G);
    category_free(c1);
    PASS();
}

/* --- Test 10: Category with 50+ objects and 100+ morphisms --- */
void test_large_category(void) {
    TEST(large_category_50_objects_100_morphisms);
    Category* cat = category_create(100, 300);
    ASSERT(cat != NULL, "create failed");

    /* Create 60 objects using static string pool */
    for (int i = 0; i < 60; i++) {
        snprintf(large_obj_ids[i], sizeof(large_obj_ids[i]), "obj_%d", i);
        category_add_object(cat, large_obj_ids[i], "t", NULL);
    }
    ASSERT(cat->num_objects == 60, "expected 60 objects");

    /* Create 120 morphisms: obj_i → obj_{i+1} */
    for (int i = 0; i < 120; i++) {
        int src = i % 60;
        int tgt = (i + 1) % 60;
        snprintf(large_mor_ids[i], sizeof(large_mor_ids[i]), "mor_%d", i);
        category_add_morphism(cat, large_mor_ids[i],
                              large_obj_ids[src], large_obj_ids[tgt], NULL);
    }
    ASSERT(cat->num_morphisms == 120, "expected 120 morphisms");

    /* Verify some lookups */
    CatObject* o42 = category_find_object(cat, "obj_42");
    ASSERT(o42 != NULL, "obj_42 not found");
    Morphism* m50 = category_find_morphism(cat, "mor_50");
    ASSERT(m50 != NULL, "mor_50 not found");

    category_free(cat);
    PASS();
}

/* --- Test 11: Compose chain of 5 morphisms --- */
void test_compose_chain_5(void) {
    TEST(compose_chain_of_5_morphisms);
    Category* cat = category_create(10, 30);
    for (int i = 0; i < 6; i++) {
        snprintf(chain_obj_ids[i], sizeof(chain_obj_ids[i]), "N%d", i);
        category_add_object(cat, chain_obj_ids[i], "t", NULL);
    }

    for (int i = 0; i < 5; i++) {
        snprintf(chain_mor_ids[i], sizeof(chain_mor_ids[i]), "step%d", i);
        category_add_morphism(cat, chain_mor_ids[i],
                              chain_obj_ids[i], chain_obj_ids[i+1], NULL);
    }

    /* Compose step0, step1, step2, step3, step4 */
    Morphism* result = category_find_morphism(cat, "step0");
    for (int i = 1; i < 5; i++) {
        Morphism* next = category_find_morphism(cat, chain_mor_ids[i]);
        result = category_compose(cat, result, next);
        ASSERT(result != NULL, "chain composition failed");
    }

    ASSERT(strcmp(result->source->id, "N0") == 0, "chain source != N0");
    ASSERT(strcmp(result->target->id, "N5") == 0, "chain target != N5");

    category_free(cat);
    PASS();
}

/* --- Test 12: Error handling — compose NULL --- */
void test_compose_null(void) {
    TEST(compose_null_morphisms_returns_null);
    Category* cat = category_create(10, 10);
    Morphism* r1 = category_compose(cat, NULL, NULL);
    ASSERT(r1 == NULL, "compose(NULL, NULL) should be NULL");

    category_add_object(cat, "A", "t", NULL);
    category_add_morphism(cat, "f", "A", "A", NULL);
    Morphism* f = category_find_morphism(cat, "f");

    Morphism* r2 = category_compose(cat, f, NULL);
    Morphism* r3 = category_compose(cat, NULL, f);
    ASSERT(r2 == NULL, "compose(f, NULL) should be NULL");
    ASSERT(r3 == NULL, "compose(NULL, f) should be NULL");

    category_free(cat);
    PASS();
}

/* --- Test 13: Identity is idempotent --- */
void test_identity_idempotent(void) {
    TEST(identity_is_idempotent);
    Category* cat = category_create(10, 20);
    category_add_object(cat, "Z", "t", NULL);
    CatObject* z = category_find_object(cat, "Z");

    Morphism* id1 = category_identity(cat, z);
    Morphism* id2 = category_identity(cat, z);
    ASSERT(id1 == id2, "identity not idempotent (different pointers)");
    ASSERT(strcmp(id1->id, "id_Z") == 0, "identity id wrong");

    category_free(cat);
    PASS();
}

int main(void) {
    printf("=== Categorical Agents C — Test Suite ===\n\n");

    test_object_creation();
    test_morphism_creation();
    test_identity_neutral();
    test_associativity();
    test_compose_mismatch();
    test_functor_maps_objects();
    test_functor_preserves_identity();
    test_functor_preserves_composition();
    test_nat_transform_components();
    test_large_category();
    test_compose_chain_5();
    test_compose_null();
    test_identity_idempotent();

    printf("\n=== Results: %d passed, %d failed ===\n",
           tests_passed, tests_failed);

    return tests_failed > 0 ? 1 : 0;
}
