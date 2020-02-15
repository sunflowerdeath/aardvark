#pragma once

#include <nod/nod.hpp>

namespace aardvark {

class Connection {
  public:
    virtual void disconnect(){};
};

class NodConnection : public Connection {
  public:
    NodConnection(nod::connection conn) : conn(std::move(conn)){};
    void disconnect() override { conn.disconnect(); };

  private:
    nod::connection conn;
};

struct Value {
    enum class ValueType { none, abs, rel };

    Value() = default;
    Value(ValueType type, float value) : type(type), value(value){};

    ValueType type = ValueType::none;
    float value = 0;

    float calc(float total, float fallback = 0) {
        if (type == ValueType::none) return fallback;
        if (type == ValueType::abs) return value;
        return value * total;  // rel
    };

    bool is_none() const { return type == ValueType::none; };

    static Value abs(float value) { return Value(ValueType::abs, value); };
    static Value rel(float value) { return Value(ValueType::rel, value); };
    static Value none() { return Value(ValueType::none, 0); };
};

struct Scale {
    float horiz = 1;
    float vert = 1;
};

struct Size {
    float width = 0;
    float height = 0;
    static bool is_equal(Size a, Size b);
};

inline bool operator==(const Size& lhs, const Size& rhs) {
    return lhs.width == rhs.width && lhs.height == rhs.height;
};

inline bool operator!=(const Size& lhs, const Size& rhs) {
    return !(lhs == rhs);
};

struct Position {
    Position() = default;
    Position(float left, float top) : left(left), top(top) {};
    float left = 0;
    float top = 0;
    static Position add(Position a, Position b);
};

inline bool operator==(const Position& lhs, const Position& rhs) {
    return lhs.left == rhs.left && lhs.top == rhs.top;
};

inline bool operator!=(const Position& lhs, const Position& rhs) {
    return !(lhs == rhs);
};

inline Position operator+(const Position& lhs, const Position& rhs) {
    return Position(
        lhs.left + rhs.left,  // left
        lhs.top + rhs.top     // top
    );
};

/*
struct Color {
    int red;
    int green;
    int blue;
    int alpha;

    SkColor to_sk_color() {
        return SkColorSetARGB(alpha, red, green, blue);
    }

    static Color from_sk_color(const SkColor& sk_color) {
        return Color{SkColorGetR(sk_color), SkColorGetG(sk_color),
                     SkColorGetB(sk_color), SkColorGetA(sk_color)};
    }
};
*/

}  // namespace aardvark

