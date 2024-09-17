# JE::CFeedbackInterface

[Return to `je`](/docs/je.md)

## C++

- [`CFeedbackInterface.hpp`](/src/je/CFeedbackInterface.hpp)
- [`CFeedbackInterface.cpp`](/src/je/CFeedbackInterface.cpp)

## References

- [`JE::HeapTracker`](/docs/je/HeapTracker.md)
- [`JE::GUI::Interface`](/docs/je/GUI/Interface.md)
- [`JE::CEngineInterface`](/docs/je/CEngineInterface.md)

## Inheritance

```mermaid
graph TD;
    JE::HeapTracker --> JE::GUI::Interface
    JE::GUI::Interface --> JE::CEngineInterface
    JE::CEngineInterface --> JE::CFeedbackInterface
```

[Return to `je`](/docs/je.md)