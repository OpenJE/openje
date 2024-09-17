# JE::CLoadingBarInterface

[Return to `JE`](/docs/je.md)

## C++

- [`CLoadingBarInterface.hpp`](/src/je/CLoadingBarInterface.hpp)
- [`CLoadingBarInterface.cpp`](/src/je/CLoadingBarInterface.cpp)

## References

- [`JE::HeapTracker`](/docs/je/HeapTracker.md)
- [`JE::GUI::Interface`](/docs/je/GUI/Interface.md)
- [`JE::CEngineInterface`](/docs/je/CEngineInterface.md)

## Inheritance

```mermaid
graph TD;
    JE::HeapTracker --> JE::GUI::Interface
    JE::GUI::Interface --> JE::CEngineInterface
    JE::CEngineInterface --> JE::CLoadingBarInterface
```

[Return to `JE`](/docs/je.md)