# categorical-agents-c

C99 port of [categorical-agents](https://github.com/SuperInstance/categorical-agents) — category theory primitives for multi-agent systems on embedded and bare-metal targets.

## What This Gives You

- **Category theory in C** — Objects (agents/capabilities), Morphisms (protocols), Categories, Functors, Natural Transformations
- **Zero dependencies** — C99 stdlib only, no heap allocation required
- **Pre-allocated arrays** — Configurable capacity for objects and morphisms, no dynamic memory
- **Full composition** — Morphism composition, identity laws, associativity checks
- **Functor mapping** — Structure-preserving maps between categories

## Quick Start

```c
#include <categorical_agents.h>

// Create a category with capacity for 32 objects and 64 morphisms
Category* cat = category_create(32, 64);

// Add objects (agents/capabilities)
category_add_object(cat, "sensor", "capability", NULL);
category_add_object(cat, "actuator", "capability", NULL);

// Add morphisms (protocols)
category_add_morphism(cat, "read", "sensor", "actuator", NULL);

// Get identity morphism
CatObject* sensor = category_find_object(cat, "sensor");
Morphism* id = category_identity(cat, sensor);

// Compose morphisms
Morphism* f = category_find_morphism(cat, "read");
// ... compose with other morphisms

category_free(cat);
```

## Build & Test

```bash
make            # build libcategorical_agents.a
make test       # build and run test suite
make clean
```

Requires a C99 compiler (gcc, clang) and make.

## API Reference

| Function | Description |
|----------|-------------|
| `category_create(max_obj, max_mor)` | Create a new category |
| `category_free(cat)` | Free a category |
| `category_add_object(cat, id, type, data)` | Add an object |
| `category_add_morphism(cat, id, src, tgt, fn)` | Add a morphism |
| `category_find_object(cat, id)` | Look up an object by ID |
| `category_find_morphism(cat, id)` | Look up a morphism by ID |
| `category_identity(cat, obj)` | Get identity morphism for an object |
| `category_compose(cat, f, g)` | Compose morphisms (g ∘ f) |
| `functor_create(src, tgt, map_obj, map_mor, ctx)` | Create a functor between categories |
| `nat_transform_create(src, tgt, dom)` | Create a natural transformation |

## How It Fits

- **[categorical-agents](https://github.com/SuperInstance/categorical-agents)** — The Rust original; this is the C port for constrained environments
- **[cocapn-health-rs](https://github.com/SuperInstance/cocapn-health-rs)** — Fleet health checks on embedded agents use this library
- **[conservation-protocol](https://github.com/SuperInstance/conservation-protocol)** — Spectral identity verification for C-based agents

## Testing

13 tests covering object/morphism creation, identity laws, composition, functor mapping, and naturality.

```bash
make test
```

## Installation

```bash
git clone https://github.com/SuperInstance/categorical-agents-c.git
cd categorical-agents-c
make
```

## License

MIT

Part of the [SuperInstance OpenConstruct](https://github.com/SuperInstance) ecosystem.
