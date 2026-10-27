#include <array>

struct Mat4 {
    std::array<float, 16> m{};   // column-major : m[col * 4 + row]

    static Mat4 identity() {
        Mat4 r;
        r.m[0] = r.m[5] = r.m[10] = r.m[15] = 1.0f;
        return r;
    }

    float& operator()(int row, int col) { return m[col * 4 + row]; }
    float  operator()(int row, int col) const { return m[col * 4 + row]; }

    Mat4 operator*(const Mat4& o) const {
        Mat4 r;
        for (int c = 0; c < 4; ++c)
            for (int row = 0; row < 4; ++row) {
                float sum = 0.0f;
                for (int k = 0; k < 4; ++k)
                    sum += (*this)(row, k) * o(k, c);
                r(row, c) = sum;
            }
        return r;
    }

    static Mat4 translation(float x, float y, float z) {
        Mat4 r = identity();
        r(0, 3) = x;
        r(1, 3) = y;
        r(2, 3) = z;
        return r;
    }
};