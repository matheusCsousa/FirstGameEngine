#pragma once

#include <string>
#include <functional>

namespace Core {

enum class EventType {
    None = 0,
    WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
    KeyPressed, KeyReleased, KeyTyped,
    MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
};

class Event {
public:
    virtual ~Event() = default;

    bool Handled = false;

    virtual EventType getEventType() const = 0;
    virtual const char* getName() const = 0;
    virtual std::string toString() const { return getName(); }
};

class EventDispatcher {
public:
    EventDispatcher(Event& event)
        : m_event(event) {}

    template<typename T, typename F>
    bool dispatch(const F& func) {
        if (m_event.getEventType() == T::getStaticType()) {
            m_event.Handled |= func(static_cast<T&>(m_event));
            return true;
        }
        return false;
    }

private:
    Event& m_event;
};

// --- Window Events ---

class WindowResizeEvent : public Event {
public:
    WindowResizeEvent(unsigned int width, unsigned int height)
        : m_width(width), m_height(height) {}

    unsigned int getWidth() const { return m_width; }
    unsigned int getHeight() const { return m_height; }

    std::string toString() const override {
        return std::string(getName()) + ": " + std::to_string(m_width) + ", " + std::to_string(m_height);
    }

    static EventType getStaticType() { return EventType::WindowResize; }
    EventType getEventType() const override { return getStaticType(); }
    const char* getName() const override { return "WindowResize"; }

private:
    unsigned int m_width, m_height;
};

class WindowCloseEvent : public Event {
public:
    WindowCloseEvent() = default;

    static EventType getStaticType() { return EventType::WindowClose; }
    EventType getEventType() const override { return getStaticType(); }
    const char* getName() const override { return "WindowClose"; }
};

// --- Key Events ---

class KeyEvent : public Event {
public:
    int getKeyCode() const { return m_keyCode; }

protected:
    KeyEvent(int keyCode)
        : m_keyCode(keyCode) {}

    int m_keyCode;
};

class KeyPressedEvent : public KeyEvent {
public:
    KeyPressedEvent(int keyCode, int repeatCount)
        : KeyEvent(keyCode), m_repeatCount(repeatCount) {}

    int getRepeatCount() const { return m_repeatCount; }

    std::string toString() const override {
        return std::string(getName()) + ": " + std::to_string(m_keyCode) + " (repeat: " + std::to_string(m_repeatCount) + ")";
    }

    static EventType getStaticType() { return EventType::KeyPressed; }
    EventType getEventType() const override { return getStaticType(); }
    const char* getName() const override { return "KeyPressed"; }

private:
    int m_repeatCount;
};

class KeyReleasedEvent : public KeyEvent {
public:
    KeyReleasedEvent(int keyCode)
        : KeyEvent(keyCode) {}

    std::string toString() const override {
        return std::string(getName()) + ": " + std::to_string(m_keyCode);
    }

    static EventType getStaticType() { return EventType::KeyReleased; }
    EventType getEventType() const override { return getStaticType(); }
    const char* getName() const override { return "KeyReleased"; }
};

// --- Mouse Events ---

class MouseMovedEvent : public Event {
public:
    MouseMovedEvent(float x, float y)
        : m_mouseX(x), m_mouseY(y) {}

    float getX() const { return m_mouseX; }
    float getY() const { return m_mouseY; }

    std::string toString() const override {
        return std::string(getName()) + ": " + std::to_string(m_mouseX) + ", " + std::to_string(m_mouseY);
    }

    static EventType getStaticType() { return EventType::MouseMoved; }
    EventType getEventType() const override { return getStaticType(); }
    const char* getName() const override { return "MouseMoved"; }

private:
    float m_mouseX, m_mouseY;
};

class MouseButtonEvent : public Event {
public:
    int getMouseButton() const { return m_button; }

protected:
    MouseButtonEvent(int button)
        : m_button(button) {}

    int m_button;
};

class MouseButtonPressedEvent : public MouseButtonEvent {
public:
    MouseButtonPressedEvent(int button)
        : MouseButtonEvent(button) {}

    std::string toString() const override {
        return std::string(getName()) + ": " + std::to_string(m_button);
    }

    static EventType getStaticType() { return EventType::MouseButtonPressed; }
    EventType getEventType() const override { return getStaticType(); }
    const char* getName() const override { return "MouseButtonPressed"; }
};

class MouseButtonReleasedEvent : public MouseButtonEvent {
public:
    MouseButtonReleasedEvent(int button)
        : MouseButtonEvent(button) {}

    std::string toString() const override {
        return std::string(getName()) + ": " + std::to_string(m_button);
    }

    static EventType getStaticType() { return EventType::MouseButtonReleased; }
    EventType getEventType() const override { return getStaticType(); }
    const char* getName() const override { return "MouseButtonReleased"; }
};

}
