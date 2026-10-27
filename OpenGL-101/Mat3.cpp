#pragma 
#include <array>
#include <iostream>
#include "Point.h"

struct Mat3 {
    std::array<float, 9> m{};   // column-major : m[col * 4 + row]

    static Mat3 identity() {
        Mat3 r;
        r.m[0] = r.m[4] = r.m[8] = 1.0f;
        return r;
    }

    float& operator()(int row, int col) { return m[col * 3 + row]; }
    float  operator()(int row, int col) const { return m[col * 3 + row]; }

    Point operator*(const Point p) const {
        std::array<float, 3> v = { 0.0f, 0.0f, 0.0f };
        for (int c = 0; c < 3; ++c)
            for (int row = 0; row < 3; ++row) {
                float sum = 0.0f;
                sum += (*this)(row, 0) * p.x;
                sum += (*this)(row, 1) * p.y;
                sum += (*this)(row, 2) * 1;
                v[row] = sum;
            }

        return Point({ v[0], v[1] });
    }

    Mat3 operator*(const Mat3& o) const {
        Mat3 r;
        for (int c = 0; c < 3; ++c)
            for (int row = 0; row < 3; ++row) {
                float sum = 0.0f;
                for (int k = 0; k < 3; ++k)
                    sum += (*this)(row, k) * o(k, c);
                r(row, c) = sum;
            }
        return r;
    }

    void operator*=(const Mat3& o) {
        (*this) = (*this) * o;
    }
    //M   iX            iY          i1
    //oX  cos(a)*sX   -sin(a)*sY    tX
    //oY  sin(a)*sX   cos(a)*sY     tY
    //o1  0             0           1
    //
    //  a => angle rotation
    // V = (x,  y,  z,  1)
    // V*M = (sX*x + tX ... , 1)
    //  1   0   x1 
    //  0   1   x2
    //  0   0   x3 

    static Mat3 translation(float x, float y) {
        Mat3 r = identity();
        r(0, 2) = x;
        r(1, 2) = y;

        return r;
    }

    static Mat3 scale(float x, float y) {
        Mat3 r = identity();
        r(0, 0) = x;
        r(1, 1) = y;

        return r;
    }

    static Mat3 rotate(float a) {
        Mat3 r = identity();
        float cA = cos(a);
        float sA = sin(a);
        r(0, 0) = cA;
        r(0, 1) = -sA;
        r(1, 0) = sA;
        r(1, 1) = cA;

        return r;
    }

    void show() {

        for (int c = 0; c < 3; ++c) {
            for (int row = 0; row < 3; ++row) {
                std::cout << (*this)(c, row) << '\t';
            }
            std::cout << '\n';
        }
    }
};