# categorical-agents-c

A minimal C99 library implementing a categorical agent protocol for embedded systems. This is the C counterpart to the [categorical-agents](https://github.com/SuperInstance/categorical-agents) Rust library, designed for bare-metal and resource-constrained environments.

## Features

- **Category theory primitives**: Objects (agents/capabilities), Morphisms (protocols/transformations), Categories, Functors, and Natural Transformations
- **Zero dependencies** beyond C99 stdlib/string
- **Heap-friendly**: pre-allocated arrays with configurable capacity
- **Composable**: supports morphism composition, identity laws, associativity

## Building

```bash
make            # builds libcategorical_agents.a
make test       # builds and runs the test suite
make clean      # clean build artifacts
```

## Usage

```c
#include <categorical_agents.h>

// Create a category
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

## API

| Function | Description |
|----------|-------------|
| `category_create(max_obj, max_mor)` | Create a new category |
| `category_free(cat)` | Free a category |
| `category_add_object(cat, id, type, data)` | Add an object |
| `category_add_morphism(cat, id, src, tgt, fn)` | Add a morphism |
| `category_find_object(cat, id)` | Look up an object |
| `category_find_morphism(cat, id)` | Look up a morphism |
| `category_identity(cat, obj)` | Get identity morphism for an object |
| `category_compose(cat, f, g)` | Compose morphisms (g ∘ f) |
| `functor_create(src, tgt, map_obj, map_mor, ctx)` | Create a functor |
| `nat_transform_create(src, tgt, dom)` | Create a natural transformation |

## License

MIT

Part of the [SuperInstance OpenConstruct](https://github.com/SuperInstance/OpenConstruct) ecosystem.
