# categorical-agents-c

A C11 implementation of **category theory for agent composition** — a port of the Rust [categorical-agents](https://github.com/SuperInstance/categorical-agents) crate.

## Concepts

This library models agents and their interactions using category theory:

- **Objects** → Agent capabilities (sets of things an agent can do)
- **Morphisms** → Protocols (transformations between capability sets)
- **Composition** → Sequential chaining of protocols
- **Functors** → Structure-preserving maps between capability categories
- **Natural transformations** → Mappings between functors
- **Monoidal categories** → Parallel (tensor) composition of protocols
- **Symmetric monoidal categories** → Reordering of parallel compositions (braiding)
- **Composition strategies** → Sequential, parallel, and conditional protocol composition

## Building

```bash
make            # build static library
make test       # build and run tests
make clean      # remove build artifacts
```

Requirements: C11 compiler (gcc/clang), no external dependencies.

## API Overview

### Category

```c
Category c;
cat_init(&c, "MyCat");
ObjId a = cat_add_object(&c);
ObjId b = cat_add_object(&c);
Morphism *f = cat_add_morphism(&c, a, b, "f");
Morphism *id_a = cat_identity(&c, a);
```

### Functor

```c
Functor F;
fun_init(&F, &src_cat, &dst_cat);
fun_map_obj(&F, src_obj, dst_obj);
fun_map_mor(&F, src_mor, dst_mor);
bool ok = fun_preserves_composition(&F);
```

### Monoidal Category

```c
MonoidalCategory mc;
mono_init(&mc, "MonCat", unit_obj);
mono_set_tensor_obj(&mc, a, b, tensor_ab);
MorId par = mono_tensor_mor(&mc, f, g);
```

### Symmetric Monoidal

```c
SymMonoidalCategory smc;
symm_init(&smc, "SymMon", unit_obj);
symm_set_braid(&smc, a, b, sigma_ab);
bool invol = symm_check_involution(&smc, a, b);
```

### Agent Capabilities & Protocols

```c
CapSet caps;
capset_init(&caps);
capset_add(&caps, "perceive", 0);
capset_add(&caps, "reason", 1);

Protocol p;
proto_init(&p, "sense-think", &dom_caps, &cod_caps);
```

### Composition Strategies

```c
StrategySet ss;
strat_init(&ss);
MorId seq = strat_sequential(&cat, &ss, f, g);
MorId par = strat_parallel(&mc, &ss, f, g);
MorId cond = strat_conditional(&cat, &ss, f, g, should_compose);
```

## Test Results

32 tests covering: category operations, morphism composition, identity laws, associativity, functors, natural transformations, monoidal tensor products, symmetric braiding, involution checks, capability sets, protocols, and all three composition strategies.

## License

MIT
