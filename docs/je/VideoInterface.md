# JE::VideoInterface

[Return to `JE`](/docs/je.md)

## C++

- [`VideoInterface.hpp`](/src/je/VideoInterface.hpp)
- [`VideoInterface.cpp`](/src/je/VideoInterface.cpp)

## References

- [`JE::HeapTracker`](/docs/je/HeapTracker.md)
- [`JE::GUI::Interface`](/docs/je/GUI/Interface.md)
- [`JE::CEngineInterface`](/docs/je/CEngineInterface.md)

## Inheritance

```mermaid
graph TD;
    JE::HeapTracker --> JE::GUI::Interface
    JE::GUI::Interface --> JE::CEngineInterface
    JE::CEngineInterface --> JE::VideoInterface
```

[Return to `JE`](/docs/je.md)