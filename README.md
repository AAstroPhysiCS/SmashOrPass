# SmashOrPass
SmashOrPass - The Game

# Build

Linux prerequisites:
- git
- cmake 3.25 or newer
- ninja
- a C++23 compiler
- clang-format for formatting

Bootstrap vcpkg and configure Debug/Release:

```sh
./scripts_linux/bootstrap.sh
```

Build:

```sh
./scripts_linux/build.sh          # Debug
./scripts_linux/build.sh release  # Release
./scripts_linux/build.sh all      # Debug and Release
```

Run:

```sh
./scripts_linux/run.sh          # Debug
./scripts_linux/run.sh release  # Release
```

Clean:

```sh
./scripts_linux/clean.sh
```

Format:

```sh
./scripts_linux/format.sh
```
