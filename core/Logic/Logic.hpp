#pragma once

namespace Core {
class Logic {
public:
    virtual ~Logic() = default;
    virtual void onUpdate() = 0;
    virtual void onRender() = 0;
};
}
