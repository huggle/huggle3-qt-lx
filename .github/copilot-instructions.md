# Huggle 3 AI Development Guide

Huggle 3 is a C++/Qt-based anti-vandalism tool for Wikipedia and MediaWiki sites. This guide helps AI coding agents understand the project's unique architecture and patterns.

## Project Architecture

### Core Module Structure
- **huggle_core**: Core logic, MediaWiki API handling, extension system
- **huggle_ui**: Qt-based UI components, web engine integration  
- **huggle**: Main executable that combines core + UI
- **huggle_res**: Shared resources (images, stylesheets)
- **huggle_l10n**: Localization/translation files

### Key Classes and Patterns
- `WikiEdit`: Central entity representing page changes with 3-stage lifecycle (None → Processed → Postprocessed)
- `Core::HuggleCore`: Global singleton controlling GC, extensions, configuration
- `QueryPool` (HGQP): Manages asynchronous MediaWiki API calls
- All classes inherit from `Collectable` for custom garbage collection
- Thread-safe smart pointers (`collectable_smartptr`) instead of raw pointers

### Extension System
Extensions run in separate domains and use hook-based communication:
```cpp
// Extension hooks are called through Hooks namespace
Hooks::EditAfterPreProcess(edit);  // Calls all registered extensions + scripts

// Extensions implement iExtension interface
class MyExtension : public iExtension {
    bool Hook_EditPreProcess(void* edit) override;
};
```

## Build System & Development Workflow

### Essential Build Commands
```bash
# Configure (enable extensions + web engine)
./configure --extension --web-engine

# Build in release directory
cd release && make

# Build with Qt6 instead of Qt5
./configure --qt6 --extension
```

### CMake Options (set via configure script)
- `HUGGLE_EXT=TRUE`: Build extensions
- `WEB_ENGINE=true`: Use Chromium instead of WebKit
- `QT6_BUILD=true`: Target Qt6 (forces WebEngine)
- `CMAKE_BUILD_TYPE=Debug/Release`: Debug vs release builds

### Project-Specific Conventions

#### Memory Management
- Use `collectable_smartptr<T>` for shared object references
- All major objects inherit from `Collectable` for GC tracking
- Manual `delete` calls use `HUGGLE_CLEAR_PQ_LIST()` macro for lists

#### Logging & Debugging  
```cpp
HUGGLE_LOG("Info message");
HUGGLE_WARNING("Warning text");
HUGGLE_ERROR("Error occurred");
HUGGLE_DEBUG("Debug info", verbosity_level);
```

#### Localization System
Huggle uses XML-based localization files stored in `src/huggle_l10n/Localization/` and `src/Localization/`:
```cpp
// Use _l() function for all user-facing strings
UiGeneric::pMessageBox(this, _l("main-title"), _l("main-message"));

// With parameters (use $1, $2, etc. in XML)
Syslog::HuggleLogs->Log(_l("main-stat", edits_count, reverts_count));
```

**Adding new localizations:**
1. Add string to `src/huggle_l10n/Localization/en.xml` and `src/Localization/en.xml`:
   ```xml
   <string name="main-new-message">Your message here with $1 parameter</string>
   ```
2. Use in code with `_l("main-new-message")` or `_l("main-new-message", param1, param2)`
3. Never hardcode user-facing strings; always use localization keys

#### Configuration Access
```cpp
// Global config via hcfg singleton
hcfg->GetExtensionConfig("extension_name", "key", "default");

// Platform-specific paths
HUGGLE_GLOBAL_EXTENSION_PATH  // Platform-dependent extension directory
```

#### Qt Version Compatibility
Code must support both Qt5 and Qt6:
```c
#ifdef QT6_BUILD
    #define HMUTEX_TYPE QRecursiveMutex
    #define HREGEX_TYPE QRegularExpression
#else
    #define HMUTEX_TYPE QMutex
    #define HREGEX_TYPE QRegExp
#endif
```

### Hook System for Extensions & Scripts
Hook IDs are defined in `script.hpp`:
- `HUGGLE_SCRIPT_HOOK_EDIT_PRE_PROCESS` - Before edit processing
- `HUGGLE_SCRIPT_HOOK_EDIT_POST_PROCESS` - After edit processing  
- `HUGGLE_SCRIPT_HOOK_EDIT_ON_REVERT` - When edit is reverted

JavaScript extensions register hooks:
```javascript
huggle.register_hook("edit_pre_process", "myFunction");
```

### MediaWiki Integration Patterns
- All API calls go through `ApiQuery` class and `QueryPool`
- Use `MediaWiki::FromMWTimestamp()` / `ToMWTimestamp()` for time conversion
- Edit objects track MediaWiki revision IDs (`revid_ht` type)
- Namespace handling uses `MEDIAWIKI_DEFAULT_NS_*` constants

### Thread Safety Notes
- `WikiEdit_ProcessorThread` handles edit postprocessing asynchronously
- Use `HMUTEX_TYPE` locks for thread-safe operations
- Extensions run in separate threads/domains

When modifying core logic, always consider the extension hook points and ensure backward compatibility with the plugin architecture.