# JE::CMessageBoxInterface

[Return to `JE`](/docs/je.md)

## C++

- [`CMessageBoxInterface.hpp`](/src/je/CMessageBoxInterface.hpp)
- [`CMessageBoxInterface.cpp`](/src/je/CMessageBoxInterface.cpp)

## References

- [`JE::HeapTracker`](/docs/je/HeapTracker.md)
- [`JE::GUI::Interface`](/docs/je/GUI/Interface.md)
- [`JE::CEngineInterface`](/docs/je/CEngineInterface.md)

## Inheritance

```mermaid
graph TD;
    JE::HeapTracker --> JE::GUI::Interface
    JE::GUI::Interface --> JE::CEngineInterface
    JE::CEngineInterface --> JE::CMessageBoxInterface
```

[Return to `JE`](/docs/je.md)