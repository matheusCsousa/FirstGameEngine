#pragma once

#include "../Event/Event.hpp"

namespace Core {
class Logic {
public:
    virtual ~Logic() = default;
    virtual void onUpdate() = 0;
    virtual void onRender() = 0;
    // Return false to defer a native close request (for example, while confirming unsaved edits).
    virtual bool onCloseRequested() { return true; }
    virtual void onEvent(Event& event) {}
};
}
