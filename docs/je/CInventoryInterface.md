# JE::CInventoryInterface

[Return to `je`](/docs/je.md)

## C++

- [`CInventoryInterface.hpp`](/src/je/CInventoryInterface.hpp)
- [`CInventoryInterface.cpp`](/src/je/CInventoryInterface.cpp)

## References

- [`JE::HeapTracker`](/docs/je/HeapTracker.md)
- [`JE::GUI::Interface`](/docs/je/GUI/Interface.md)
- [`JE::CEngineInterface`](/docs/je/CEngineInterface.md)

## Inheritance

```mermaid
graph TD;
    JE::HeapTracker --> JE::GUI::Interface
    JE::GUI::Interface --> JE::CEngineInterface
    JE::CEngineInterface --> JE::CInventoryInterface
```

[Return to `je`](/docs/je.md)