#pragma once

#include "../Event/Event.hpp"

namespace Core {
class Logic {
public:
    virtual ~Logic() = default;
    virtual void onUpdate() = 0;
    virtual void onRender() = 0;
    virtual void onEvent(Event& event) {}
};
}
