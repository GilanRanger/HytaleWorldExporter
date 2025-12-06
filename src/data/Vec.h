#pragma once
#include <cmath>

struct Mat4;  // Forward declaration

struct Vec4 {
	float x, y, z, w;

	Vec4() : x(0), y(0), z(0), w(0) {}
	Vec4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}

    Vec4 normalize() const {
        float len = std::sqrt(x * x + y * y + z * z + w * w);
        if (len < 1e-6f) {
            return Identity();
        }
        return Vec4(x / len, y / len, z / len, w / len);
    }

	static Vec4 Identity() {
		return Vec4(0.0f, 0.0f, 0.0f, 1.0f);
	}

    Mat4 toMatrix() const;  // Changed from static to const member function

	// Vector addition
	Vec4 operator+(const Vec4& other) const {
		return Vec4(x + other.x, y + other.y, z + other.z, w + other.w);
	}

	Vec4& operator+=(const Vec4& other) {
		x += other.x;
		y += other.y;
		z += other.z;
		w += other.w;
		return *this;
	}

	// Vector subtraction
	Vec4 operator-(const Vec4& other) const {
		return Vec4(x - other.x, y - other.y, z - other.z, w - other.w);
	}

	Vec4& operator-=(const Vec4& other) {
		x -= other.x;
		y -= other.y;
		z -= other.z;
		w -= other.w;
		return *this;
	}

	// Scalar multiplication
	Vec4 operator*(float scalar) const {
		return Vec4(x * scalar, y * scalar, z * scalar, w * scalar);
	}

	Vec4& operator*=(float scalar) {
		x *= scalar;
		y *= scalar;
		z *= scalar;
		w *= scalar;
		return *this;
	}

	// Component-wise multiplication
	Vec4 operator*(const Vec4& other) const {
		return Vec4(x * other.x, y * other.y, z * other.z, w * other.w);
	}
};

struct Vec3 {
	float x, y, z;

	Vec3() : x(0), y(0), z(0) {}
	Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vec3 normalize() const {
        float len = std::sqrt(x * x + y * y + z * z);
        if (len < 1e-6f) {
            return Identity();
        }
        return Vec3(x / len, y / len, z / len);
    }

    static Vec3 Identity() {
        return Vec3(0.0f, 0.0f, 1.0f);
    }

	// Vector addition
	Vec3 operator+(const Vec3& other) const {
		return Vec3(x + other.x, y + other.y, z + other.z);
	}

	Vec3& operator+=(const Vec3& other) {
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}

	// Vector subtraction
	Vec3 operator-(const Vec3& other) const {
		return Vec3(x - other.x, y - other.y, z - other.z);
	}

	Vec3& operator-=(const Vec3& other) {
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}

	// Scalar multiplication
	Vec3 operator*(float scalar) const {
		return Vec3(x * scalar, y * scalar, z * scalar);
	}

	Vec3& operator*=(float scalar) {
		x *= scalar;
		y *= scalar;
		z *= scalar;
		return *this;
	}

	// Component-wise multiplication
	Vec3 operator*(const Vec3& other) const {
		return Vec3(x * other.x, y * other.y, z * other.z);
	}
};

struct Vec2 {
	float u, v;

	Vec2() : u(0), v(0) {}
	Vec2(float u, float v) : u(u), v(v) {}

	// Vector addition
	Vec2 operator+(const Vec2& other) const {
		return Vec2(u + other.u, v + other.v);
	}

	Vec2& operator+=(const Vec2& other) {
		u += other.u;
		v += other.v;
		return *this;
	}

	// Vector subtraction
	Vec2 operator-(const Vec2& other) const {
		return Vec2(u - other.u, v - other.v);
	}

	Vec2& operator-=(const Vec2& other) {
		u -= other.u;
		v -= other.v;
		return *this;
	}

	// Scalar multiplication
	Vec2 operator*(float scalar) const {
		return Vec2(u * scalar, v * scalar);
	}

	Vec2& operator*=(float scalar) {
		u *= scalar;
		v *= scalar;
		return *this;
	}

	// Component-wise multiplication
	Vec2 operator*(const Vec2& other) const {
		return Vec2(u * other.u, v * other.v);
	}
};

struct Mat4 {
    float m[4][4];

    // Constructors
    Mat4() {
        // Initialize to identity by default
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                m[i][j] = (i == j) ? 1.0f : 0.0f;
            }
        }
    }

    Mat4(float diagonal) {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                m[i][j] = (i == j) ? diagonal : 0.0f;
            }
        }
    }

    // Static factory methods
    static Mat4 Identity() {
        return Mat4(1.0f);
    }

    static Mat4 Translate(const Vec3& translation) {
        Mat4 result;
        result.m[0][3] = translation.x;
        result.m[1][3] = translation.y;
        result.m[2][3] = translation.z;
        return result;
    }

    static Mat4 Scale(const Vec3& scale) {
        Mat4 result;
        result.m[0][0] = scale.x;
        result.m[1][1] = scale.y;
        result.m[2][2] = scale.z;
        result.m[3][3] = 1.0f;
        return result;
    }

    static Mat4 Scale(float uniformScale) {
        return Scale(Vec3(uniformScale, uniformScale, uniformScale));
    }

    static Mat4 RotateX(float angleRadians) {
        Mat4 result;
        float c = std::cos(angleRadians);
        float s = std::sin(angleRadians);
        result.m[1][1] = c;
        result.m[1][2] = -s;
        result.m[2][1] = s;
        result.m[2][2] = c;
        return result;
    }

    static Mat4 RotateY(float angleRadians) {
        Mat4 result;
        float c = std::cos(angleRadians);
        float s = std::sin(angleRadians);
        result.m[0][0] = c;
        result.m[0][2] = s;
        result.m[2][0] = -s;
        result.m[2][2] = c;
        return result;
    }

    static Mat4 RotateZ(float angleRadians) {
        Mat4 result;
        float c = std::cos(angleRadians);
        float s = std::sin(angleRadians);
        result.m[0][0] = c;
        result.m[0][1] = -s;
        result.m[1][0] = s;
        result.m[1][1] = c;
        return result;
    }

    // Matrix multiplication
    Mat4 operator*(const Mat4& other) const {
        Mat4 result(0.0f);
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                for (int k = 0; k < 4; ++k) {
                    result.m[i][j] += m[i][k] * other.m[k][j];
                }
            }
        }
        return result;
    }

    // Transform a 3D point (w=1)
    Vec3 transformPoint(const Vec3& point) const {
        float x = m[0][0] * point.x + m[0][1] * point.y + m[0][2] * point.z + m[0][3];
        float y = m[1][0] * point.x + m[1][1] * point.y + m[1][2] * point.z + m[1][3];
        float z = m[2][0] * point.x + m[2][1] * point.y + m[2][2] * point.z + m[2][3];
        float w = m[3][0] * point.x + m[3][1] * point.y + m[3][2] * point.z + m[3][3];

        // Perspective divide if needed
        if (w != 1.0f && w != 0.0f) {
            return Vec3(x / w, y / w, z / w);
        }
        return Vec3(x, y, z);
    }

    // Transform a 3D vector/direction (w=0)
    Vec3 transformVector(const Vec3& vec) const {
        float x = m[0][0] * vec.x + m[0][1] * vec.y + m[0][2] * vec.z;
        float y = m[1][0] * vec.x + m[1][1] * vec.y + m[1][2] * vec.z;
        float z = m[2][0] * vec.x + m[2][1] * vec.y + m[2][2] * vec.z;
        return Vec3(x, y, z);
    }

    // Transpose
    Mat4 transpose() const {
        Mat4 result;
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                result.m[i][j] = m[j][i];
            }
        }
        return result;
    }

    // Determinant
    float determinant() const {
        // Using cofactor expansion along first row
        float a = m[0][0], b = m[0][1], c = m[0][2], d = m[0][3];

        float det3x3_1 =
            m[1][1] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) -
            m[1][2] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) +
            m[1][3] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]);

        float det3x3_2 =
            m[1][0] * (m[2][2] * m[3][3] - m[2][3] * m[3][2]) -
            m[1][2] * (m[2][0] * m[3][3] - m[2][3] * m[3][0]) +
            m[1][3] * (m[2][0] * m[3][2] - m[2][2] * m[3][0]);

        float det3x3_3 =
            m[1][0] * (m[2][1] * m[3][3] - m[2][3] * m[3][1]) -
            m[1][1] * (m[2][0] * m[3][3] - m[2][3] * m[3][0]) +
            m[1][3] * (m[2][0] * m[3][1] - m[2][1] * m[3][0]);

        float det3x3_4 =
            m[1][0] * (m[2][1] * m[3][2] - m[2][2] * m[3][1]) -
            m[1][1] * (m[2][0] * m[3][2] - m[2][2] * m[3][0]) +
            m[1][2] * (m[2][0] * m[3][1] - m[2][1] * m[3][0]);

        return a * det3x3_1 - b * det3x3_2 + c * det3x3_3 - d * det3x3_4;
    }

    // Inverse (for general 4x4 matrices)
    Mat4 inverse() const {
        float det = determinant();
        if (std::abs(det) < 1e-6f) {
            // Matrix is singular, return identity
            return Identity();
        }

        // For transformation matrices, we can use a faster method
        // This is a simplified version for affine transforms
        Mat4 result;

        // Extract 3x3 rotation part
        Mat4 rot;
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                rot.m[i][j] = m[i][j];
            }
        }

        // Transpose rotation part (inverse of rotation)
        Mat4 rotInv = rot.transpose();

        // Copy transposed rotation
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                result.m[i][j] = rotInv.m[i][j];
            }
        }

        // Inverse translation: -R^T * T
        Vec3 translation(m[0][3], m[1][3], m[2][3]);
        Vec3 invTranslation = rotInv.transformVector(translation) * -1.0f;
        result.m[0][3] = invTranslation.x;
        result.m[1][3] = invTranslation.y;
        result.m[2][3] = invTranslation.z;

        result.m[3][3] = 1.0f;

        return result;
    }

    // Extract just the 3x3 rotation part
    Mat4 extractRotation() const {
        Mat4 result = Identity();
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 3; ++j) {
                result.m[i][j] = m[i][j];
            }
        }
        return result;
    }
};

// Define Vec4::toMatrix() after Mat4 is fully defined
inline Mat4 Vec4::toMatrix() const {
    // Normalize first to ensure it's a unit quaternion
    Vec4 q = normalize();

    float xx = q.x * q.x;
    float yy = q.y * q.y;
    float zz = q.z * q.z;
    float xy = q.x * q.y;
    float xz = q.x * q.z;
    float yz = q.y * q.z;
    float wx = q.w * q.x;
    float wy = q.w * q.y;
    float wz = q.w * q.z;

    Mat4 result;

    result.m[0][0] = 1.0f - 2.0f * (yy + zz);
    result.m[0][1] = 2.0f * (xy - wz);
    result.m[0][2] = 2.0f * (xz + wy);
    result.m[0][3] = 0.0f;

    result.m[1][0] = 2.0f * (xy + wz);
    result.m[1][1] = 1.0f - 2.0f * (xx + zz);
    result.m[1][2] = 2.0f * (yz - wx);
    result.m[1][3] = 0.0f;

    result.m[2][0] = 2.0f * (xz - wy);
    result.m[2][1] = 2.0f * (yz + wx);
    result.m[2][2] = 1.0f - 2.0f * (xx + yy);
    result.m[2][3] = 0.0f;

    result.m[3][0] = 0.0f;
    result.m[3][1] = 0.0f;
    result.m[3][2] = 0.0f;
    result.m[3][3] = 1.0f;

    return result;
}