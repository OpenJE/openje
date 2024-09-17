# JE::DialogueInterface

[Return to `je`](/docs/je.md)

## C++

- [`DialogueInterface.hpp`](/src/je/DialogueInterface.hpp)
- [`DialogueInterface.cpp`](/src/je/DialogueInterface.cpp)

## References

- [`JE::HeapTracker`](/docs/je/HeapTracker.md)
- [`JE::GUI::Interface`](/docs/je/GUI/Interface.md)
- [`JE::CEngineInterface`](/docs/je/CEngineInterface.md)

## Inheritance

```mermaid
graph TD;
    JE::HeapTracker --> JE::GUI::Interface
    JE::GUI::Interface --> JE::CEngineInterface
    JE::CEngineInterface --> JE::DialogueInterface
```

[Return to `je`](/docs/je.md)