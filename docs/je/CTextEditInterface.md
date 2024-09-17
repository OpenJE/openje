# JE::CTextEditInterface

[Return to `JE`](/docs/je.md)

## C++

- [`CTextEditInterface.hpp`](/src/je/CTextEditInterface.hpp)
- [`CTextEditInterface.cpp`](/src/je/CTextEditInterface.cpp)

## References

- [`JE::HeapTracker`](/docs/je/HeapTracker.md)
- [`JE::GUI::Interface`](/docs/je/GUI/Interface.md)
- [`JE::CEngineInterface`](/docs/je/CEngineInterface.md)

## Inheritance

```mermaid
graph TD;
    JE::HeapTracker --> JE::GUI::Interface
    JE::GUI::Interface --> JE::CEngineInterface
    JE::CEngineInterface --> JE::CTextEditInterface
```

[Return to `JE`](/docs/je.md)