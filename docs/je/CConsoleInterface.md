# JE::CConsoleInterface

[Return to `JE`](/docs/je.md)

## C++

- [`CConsoleInterface.hpp`](/src/je/CConsoleInterface.hpp)
- [`CConsoleInterface.cpp`](/src/je/CConsoleInterface.cpp)

## References

- [`JE::HeapTracker`](/docs/je/HeapTracker.md)
- [`JE::GUI::Interface`](/docs/je/GUI/Interface.md)
- [`JE::CEngineInterface`](/docs/je/CEngineInterface.md)

## Inheritance

```mermaid
graph TD;
    JE::HeapTracker --> JE::GUI::Interface
    JE::GUI::Interface --> JE::CEngineInterface
    JE::CEngineInterface --> JE::CConsoleInterface
```

[Return to `JE`](/docs/je.md)