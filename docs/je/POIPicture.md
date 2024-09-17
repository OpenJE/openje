# JE::POIPicture

[Return to `je`](/docs/je.md)

## C++

- [`POIPicture.hpp`](/src/je/POIPicture.hpp)
- [`POIPicture.cpp`](/src/je/POIPicture.cpp)

## References

- [`JE::GUI::Base`](/docs/je/GUI/Base.md)
- [`JE::HeapTracker`](/docs/je/HeapTracker.md)
- [`JE::GUI::Window`](/docs/je/GUI/Window.md)
- [`JE::GUI::Picture`](/docs/je/GUI/Picture.md)

## Inheritance

```mermaid
graph TD;
    JE::GUI::Base --> JE::GUI::Window
    JE::HeapTracker --> JE::GUI::Window
    JE::GUI::Window --> JE::GUI::Picture
    JE::GUI::Picture --> JE::POIPicture
```

[Return to `je`](/docs/je.md)