# JE::CDebugToolsInterface

[Return to `JE`](/docs/je.md)

## C++

- [`CDebugToolsInterface.hpp`](/src/je/CDebugToolsInterface.hpp)
- [`CDebugToolsInterface.cpp`](/src/je/CDebugToolsInterface.cpp)

## References

- [`JE::HeapTracker`](/docs/je/HeapTracker.md)
- [`JE::GUI::Interface`](/docs/je/GUI/Interface.md)
- [`JE::CEngineInterface`](/docs/je/CEngineInterface.md)

## Inheritance

```mermaid
graph TD;
    JE::HeapTracker --> JE::GUI::Interface
    JE::GUI::Interface --> JE::CEngineInterface
    JE::CEngineInterface --> JE::CDebugToolsInterface
```

[Return to `JE`](/docs/je.md)