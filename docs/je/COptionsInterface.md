# JE::COptionsInterface

[Return to `je`](/docs/je.md)

## C++

- [`COptionsInterface.hpp`](/src/je/COptionsInterface.hpp)
- [`COptionsInterface.cpp`](/src/je/COptionsInterface.cpp)

## References

- [`JE::HeapTracker`](/docs/je/HeapTracker.md)
- [`JE::GUI::Interface`](/docs/je/GUI/Interface.md)
- [`JE::CEngineInterface`](/docs/je/CEngineInterface.md)

## Inheritance

```mermaid
graph TD;
    JE::HeapTracker --> JE::GUI::Interface
    JE::GUI::Interface --> JE::CEngineInterface
    JE::CEngineInterface --> JE::COptionsInterface
```

[Return to `je`](/docs/je.md)