# olpie-esp32-modules

This repository is a starter layout for reusable ESP32 modules. It keeps the
folders, build metadata, and umbrella includes ready so you can drop in your own
implementations without restructuring a project.

## Getting started

1. Add this repository as a submodule in your PlatformIO project:
   ```bash
   git submodule add https://github.com/yourname/olpie-esp32-modules lib/olpie-esp32-modules
   ```
2. Point PlatformIO to the `include` and `src` folders (for example with
   `lib_extra_dirs`).
3. Create new headers under `include/espmods/` and implementations under `src/`
   matching the structure you prefer.

The existing `espmods::core::Module` placeholder illustrates how to wire a
module with `begin()` and `update()` hooks—replace it with your own logic.

## Building with CMake / ESP-IDF

The provided `CMakeLists.txt` exposes the include directory as an interface
library so it can be consumed from a parent project:

```cmake
add_subdirectory(path/to/olpie-esp32-modules)
target_link_libraries(your_app PRIVATE esp32-modules)
```

## License

MIT
