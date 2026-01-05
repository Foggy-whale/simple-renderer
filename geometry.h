#pragma once
#include <cmath>
#include <cassert>
#include <iostream>

/* 定义向量类 */
template<int n> struct vec {
    float data[n] = {0};
    vec<n>() = default;
    vec<n>(const vec<n>& v) { for(int i = 0; i < n; i++) data[i] = v.data[i]; }
    vec<n>(std::initializer_list<float> l) { 
        assert(l.size() == n); 
        for(int i = 0; i < n; i++) data[i] = *(l.begin() + i); 
    }
    float& operator[](const int i)       { assert(i>=0 && i<n); return data[i]; }
    float  operator[](const int i) const { assert(i>=0 && i<n); return data[i]; }
    float norm() const { return std::sqrt(*this * *this); }
    vec<n> normalized() const { return *this / norm(); }
};

// ---> 重载向量运算符
template<int n> float operator*(const vec<n>& lhs, const vec<n>& rhs) {
    float ret = 0;
    for(int i = 0; i < n; i++) ret += lhs[i] * rhs[i]; 
    return ret;
}

template<int n> vec<n> operator+(const vec<n>& lhs, const vec<n>& rhs) {
    vec<n> ret = lhs;
    for(int i = 0; i < n; i++) ret[i] += rhs[i];
    return ret;
}

template<int n> vec<n> operator-(const vec<n>& lhs, const vec<n>& rhs) {
    vec<n> ret = lhs;
    for(int i = 0; i < n; i++) ret[i] -= rhs[i];
    return ret;
}

template<int n> vec<n> operator*(const vec<n>& lhs, const float& rhs) {
    vec<n> ret = lhs;
    for(int i = 0; i < n; i++) ret[i] *= rhs;
    return ret;
}

template<int n> vec<n> operator*(const float& lhs, const vec<n> &rhs) {
    return rhs * lhs;
}

template<int n> vec<n> operator/(const vec<n>& lhs, const float& rhs) {
    vec<n> ret = lhs;
    for(int i = 0; i < n; i++) ret[i] /= rhs;
    return ret;
}

template<int n> vec<n>& operator+=(vec<n>& lhs, const vec<n>& rhs) {
    for(int i = 0; i < n; i++) lhs[i] += rhs[i];
    return lhs;
}

template<int n> vec<n>& operator-=(vec<n>& lhs, const vec<n>& rhs) {
    for(int i = 0; i < n; i++) lhs[i] -= rhs[i];
    return lhs;
}

template<int n> vec<n>& operator*=(vec<n>& lhs, const float& rhs) {
    for(int i = 0; i < n; i++) lhs[i] *= rhs;
    return lhs;
}

template<int n> vec<n>& operator/=(vec<n>& lhs, const float& rhs) {
    for(int i = 0; i < n; i++) lhs[i] /= rhs;
    return lhs;
}

template<int n> std::ostream& operator<<(std::ostream& out, const vec<n>& v) {
    for(int i = 0; i < n; i++) out << v[i] << " ";
    return out;
}
// <--- 重载向量运算符

// ---> 定义vec2, vec3, vec4
template<> struct vec<2> {
    float x = 0, y = 0;
    vec<2>() = default;
    vec<2>(const vec<2>& v) : x(v.x), y(v.y) {}
    vec<2>(float x, float y) : x(x), y(y) {}
    vec<2>(std::initializer_list<float> l) { 
        assert(l.size() == 2); 
        x = *l.begin(), y = *(l.begin() + 1); 
    }
    float& operator[](const int i)       { assert(i>=0 && i<2); return i ? y : x; }
    float  operator[](const int i) const { assert(i>=0 && i<2); return i ? y : x; }
    float norm() const { return std::sqrt(*this * *this); }
    vec<2> normalized() const { return *this / norm(); }
};

template<> struct vec<3> {
    float x = 0, y = 0, z = 0;
    vec<3>() = default;
    vec<3>(const vec<3>& v) : x(v.x), y(v.y), z(v.z) {}
    vec<3>(float x, float y, float z) : x(x), y(y), z(z) {}
    vec<3>(std::initializer_list<float> l) { 
        assert(l.size() == 3); 
        x = *l.begin(), y = *(l.begin() + 1), z = *(l.begin() + 2); 
    }
    float& operator[](const int i)       { assert(i>=0 && i<3); return i ? (1==i ? y : z) : x; }
    float  operator[](const int i) const { assert(i>=0 && i<3); return i ? (1==i ? y : z) : x; }
    float norm() const { return std::sqrt(*this * *this); }
    vec<3> normalized() const { return *this / norm(); }
};

template<> struct vec<4> {
    float x = 0, y = 0, z = 0, w = 0;
    vec<4>() = default;
    vec<4>(const vec<4>& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}
    vec<4>(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
    vec<4>(std::initializer_list<float> l) { 
        assert(l.size() == 4); 
        x = *l.begin(), y = *(l.begin() + 1), z = *(l.begin() + 2), w = *(l.begin() + 3); 
    }
    float& operator[](const int i)       { assert(i>=0 && i<4); return i<2 ? (i ? y : x) : (2==i ? z : w); }
    float  operator[](const int i) const { assert(i>=0 && i<4); return i<2 ? (i ? y : x) : (2==i ? z : w); }

    vec<2> xy()  const { return {x, y}; }
    vec<3> xyz() const { return {x, y, z}; }
    float norm() const { return std::sqrt(*this * *this); }
    vec<4> normalized() const { return *this / norm(); }
};

typedef vec<2> vec2;
typedef vec<3> vec3;
typedef vec<4> vec4;
// <--- 定义vec2, vec3, vec4

inline vec2 cross_product(const vec2 &v1, const vec2 &v2) {
    return {v1.x*v2.y - v1.y*v2.x};
}

inline vec3 cross_product(const vec3 &v1, const vec3 &v2) {
    return {v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x};
}

inline vec4 to_vec4(const vec3& v) {
    return vec4(v.x, v.y, v.z, 1);
}

template<typename T, int N> struct MatInitializer { // 仿照 Eigen 写的矩阵初始化器
    /* 支持初始化格式：
        mat4 << 1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1;
    */
    T& m;
    int idx;
    MatInitializer(T& m_in, float f) : m(m_in), idx(0) {
        if (idx < N * N) {
            m[idx / N][idx % N] = f;
            idx++;
        }
    }
    MatInitializer& operator,(float f) {
        if (idx < N * N) {
            m[idx / N][idx % N] = f;
            idx++;
        }
        return *this;
    }
};

/* 定义矩阵类 */
template<int n> struct dt;
template<int n> struct mat {
    vec<n>  data[n] = {};
    vec<n>& operator[](const int i)       { assert(i>=0 && i<n); return data[i]; }
    vec<n>  operator[](const int i) const { assert(i>=0 && i<n); return data[i]; }

    MatInitializer<mat<n>, n> operator<<(const float f) {
        return MatInitializer<mat<n>, n>(*this, f);
    }

    float det() const {
        return dt<n>::det(*this);
    }

    float cofactor(const int row, const int col) const {
        mat<n-1> sub;
        for(int i = 0, si = 0; i < n; i++) if(i != row) {
            for(int j = 0, sj = 0; j < n; j++) if(j != col) {
                sub[si][sj++] = data[i][j];
            }
            si++;
        }
        return sub.det() * ((row + col) % 2 ? -1 : 1);
    }

    mat<n> transpose() const {
        mat<n> ret;
        for(int i = 0; i < n; i++)
            for(int j = 0; j < n; j++)
                ret[i][j] = (*this)[j][i];
        return ret;
    }

    mat<n> invert() const {
        float det = this->det();
        assert(det != 0);
        mat<n> ret;
        for(int i = 0; i < n; i++)
            for(int j = 0; j < n; j++)
                ret[i][j] = cofactor(i, j) / det;
        return ret;
    }
};

typedef mat<2> mat2;
typedef mat<3> mat3;
typedef mat<4> mat4;

// ---> 重载矩阵运算符
template<int n> std::ostream& operator<<(std::ostream& out, const mat<n>& m) {
    for(int i = 0; i < n; i++) {
        for(int j = 0; j < n; j++) {
            out << m[i][j] << (j + 1 < n ? ' ' : '\n');
        }
    }
    return out;
}

template<int n> mat<n> operator+(const mat<n>& lhs, const mat<n>& rhs) {
    mat<n> ret;
    for(int i = 0; i < n; i++)
        ret[i] = lhs[i] + rhs[i];
    return ret;
}

template<int n> mat<n> operator-(const mat<n>& lhs, const mat<n>& rhs) {
    mat<n> ret;
    for(int i = 0; i < n; i++)
        ret[i] = lhs[i] - rhs[i];
    return ret;
}

template<int n> mat<n> operator*(const mat<n>& lhs, const float& rhs) {
    mat<n> ret;
    for(int i = 0; i < n; i++)
        ret[i] = lhs[i] * rhs;
    return ret;
}

template<int n> mat<n> operator*(const float& lhs, const mat<n>& rhs) {
    return rhs * lhs;
}

template<int n> mat<n> operator/(const mat<n>& lhs, const float& rhs) {
    mat<n> ret;
    for(int i = 0; i < n; i++)
        ret[i] = lhs[i] / rhs;
    return ret;
}

template<int n> vec<n> operator*(const mat<n>& m, const vec<n>& v) {
    vec<n> ret{};
    for(int i = 0; i < n; i++)
        for(int j = 0; j < n; j++)
            ret[i] += m[i][j] * v[j];
    return ret;
}

template<int n> mat<n> operator*(const mat<n>& lhs, const mat<n>& rhs) {
    mat<n> ret{};
    for(int i = 0; i < n; i++)
        for(int j = 0; j < n; j++)
            for(int k = 0; k < n; k++)
                ret[i][j] += lhs[i][k] * rhs[k][j];
    return ret;
}

template<int n> mat<n>& operator+=(mat<n>& lhs, const mat<n>& rhs) {
    for(int i = 0; i < n; i++) lhs[i] += rhs[i];
    return lhs;
}

template<int n> mat<n>& operator-=(mat<n>& lhs, const mat<n>& rhs) {
    for(int i = 0; i < n; i++) lhs[i] -= rhs[i];
    return lhs;
}

template<int n> mat<n>& operator*=(mat<n>& lhs, const float& rhs) {
    for(int i = 0; i < n; i++) lhs[i] *= rhs;
    return lhs;
}

template<int n> mat<n>& operator/=(mat<n>& lhs, const float& rhs) {
    for(int i = 0; i < n; i++) lhs[i] /= rhs;
    return lhs;
}
// <--- 重载矩阵运算符

template<int n> mat<n> identity() {
    mat<n> ret{};
    for(int i = 0; i < n; i++)
        ret[i][i] = 1.0f;
    return ret;
}

// ---> 定义行列式
template<int n> struct dt { 
    static double det(const mat<n>& src) {
        double ret = 0;
        for(int i = 0; i < n; i++)
            ret += src[0][i] * src.cofactor(0, i);
        return ret;
    }
};

template<> struct dt<1> {   
    static double det(const mat<1>& src) {
        return src[0][0];
    }
};
// <--- 定义行列式
