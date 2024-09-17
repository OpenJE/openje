# JE::POIWaypoint

[Return to `JE`](/docs/je.md)

## C++

- [`POIWaypoint.hpp`](/src/je/POIWaypoint.hpp)
- [`POIWaypoint.cpp`](/src/je/POIWaypoint.cpp)

## References

- [`JE::GUI::Base`](/docs/je/GUI/Base.md)
- [`JE::HeapTracker`](/docs/je/HeapTracker.md)
- [`JE::GUI::Window`](/docs/je/GUI/Window.md)
- [`JE::GUI::Label`](/docs/je/GUI/Label.md)

## Inheritance

```mermaid
graph TD;
    JE::GUI::Base --> JE::GUI::Window
    JE::HeapTracker --> JE::GUI::Window
    JE::GUI::Window --> JE::GUI::Label
    JE::GUI::Label --> JE::POIWaypoint
```

[Return to `JE`](/docs/je.md)