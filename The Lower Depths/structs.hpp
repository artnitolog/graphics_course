#ifndef STRUCTS_HPP
#define STRUCTS_HPP

#include<cmath>
#include<array>


template <typename E>
constexpr auto to_underlying(E e) noexcept {
    return static_cast<std::underlying_type_t<E>>(e);
}

enum class Direction : int {UP, DOWN, LEFT, RIGHT};

template<class T>
struct Point {
    T x;
    T y;

    template<class ConvTo>
    operator Point<ConvTo>();

    // bool operator<(Point other);

    Point Shift(Direction dir, T step);
    T SqrDist(Point other);
};

template<class T>
T Point<T>::SqrDist(Point<T> other) {
    return (x - other.x) * (x - other.x) + (y - other.y) * (y - other.y);
}

template<class T>
bool operator==(Point<T> a, Point<T> b) {
    return a.x == b.x && a.y == b.y;
}

// template<class T>
// bool Point<T>::operator<(Point<T> other) {
//     return x < other.x || (x == other.x && y < other.y);
// }

template<class T>
Point<T> Point<T>::Shift(Direction dir, T step) {
    switch (dir) {
        case Direction::UP: return {x, y - step};
        case Direction::DOWN: return {x, y + step};
        case Direction::LEFT: return {x - step, y};
        case Direction::RIGHT: return {x + step, y};
        default: return *this;
    }
}

template<class T>
template<class ConvTo>
Point<T>::operator Point<ConvTo>() {
    return {static_cast<ConvTo>(x), static_cast<ConvTo>(y)};
}

#endif