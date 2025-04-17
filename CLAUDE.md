# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build Commands
- Build: `make -j`
- Run tests: `make test`
- Run single test: `./build/test --gtest_filter=TEST_SUITE.TEST_NAME`
- Debug build: `make debug`
- Miyoo Mini cross-compile: `make miyoo-mini-shell` then `./cross-compile/miyoo-mini/create_packages.sh <version>`

## Code Style Guidelines
- **Headers**: Include guards use `_H_` suffix. Include system headers first, then project headers
- **Naming**: `snake_case` for functions/variables, `PascalCase` for classes/types, `ALL_CAPS` for constants
- **Types**: Use `std::unique_ptr` for memory management, custom type aliases for domain objects (e.g., `DocAddr`)
- **Error handling**: Return booleans for success/failure, use early returns for error conditions
- **Classes**: Implement virtual destructors for base classes, follow RAII patterns
- **UI components**: Follow View interface pattern with `render()`, `on_keypress()`, `is_done()` methods
- **Memory management**: Prefer smart pointers over raw pointers, use RAII patterns