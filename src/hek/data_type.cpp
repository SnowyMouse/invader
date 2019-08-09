/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0 or later. See LICENSE for more information.
 */

#include <cmath>

#include "data_type.hpp"

namespace Invader::HEK {
    Vector3D<NativeEndian> euler2d_to_vector(const Euler2D<NativeEndian> &rotation) noexcept {
        Vector3D<NativeEndian> vector;
        float sp = std::sin(rotation.pitch);
        float cp = std::cos(rotation.pitch);
        float sy = std::sin(rotation.yaw);
        float cy = std::cos(rotation.yaw);
        vector.i = cy * cp;
        vector.j = sy * cp;
        vector.k = sp;
        return vector;
    }

    Quaternion<NativeEndian> vector_to_quaternion(const Vector3D<NativeEndian> &vector) noexcept {
        Quaternion<NativeEndian> returned_quaternion;
        float angle = std::atan2(vector.i, vector.k);
        returned_quaternion.i = vector.i * std::sin(angle / 2.0f);
        returned_quaternion.j = vector.j * std::sin(angle / 2.0f);
        returned_quaternion.k = vector.k * std::sin(angle / 2.0f);
        returned_quaternion.w = std::cos(angle / 2.0f);
        return returned_quaternion;
    }

    Quaternion<NativeEndian> euler_to_quaternion(const Euler3D<NativeEndian> &rotation) noexcept {
        Quaternion<NativeEndian> returned_quaternion;
        float cy = std::cos(rotation.yaw * 0.5f);
        float sy = std::sin(rotation.yaw * 0.5f);
        float cr = std::cos(rotation.roll * 0.5f);
        float sr = std::sin(rotation.roll * 0.5f);
        float cp = std::cos(rotation.pitch * 0.5f);
        float sp = std::sin(rotation.pitch * 0.5f);
        returned_quaternion.w = cy * cr * cp + sy * sr * sp;
        returned_quaternion.i = cy * sr * cp - sy * cr * sp;
        returned_quaternion.j = cy * cr * sp + sy * sr * cp;
        returned_quaternion.k = sy * cr * cp - cy * sr * sp;
        return returned_quaternion;
    }

    Matrix<NativeEndian> quaternion_to_matrix(const Quaternion<NativeEndian> &rotation) noexcept {
        Matrix<NativeEndian> returned_matrix;

        float w = rotation.w;
        float i = rotation.i;
        float j = rotation.j;
        float k = rotation.k;

        float ww = w*w;
        float ii = i*i;
        float jj = j*j;
        float kk = k*k;

        float inverse = 1.0f / (ii + jj + kk + ww);
        returned_matrix.matrix[0][0] = ( ii - jj - kk + ww) * inverse;
        returned_matrix.matrix[1][1] = (-ii + jj - kk + ww) * inverse;
        returned_matrix.matrix[2][2] = (-ii - jj + kk + ww) * inverse;

        float ij = i*j;
        float kw = k*w;
        returned_matrix.matrix[0][1] = 2.0f * (ij + kw) * inverse;
        returned_matrix.matrix[1][0] = 2.0f * (ij - kw) * inverse;

        float ik = i*k;
        float jw = j*w;
        returned_matrix.matrix[0][2] = 2.0f * (ik - jw) * inverse;
        returned_matrix.matrix[2][0] = 2.0f * (ik + jw) * inverse;

        float jk = j*k;
        float iw = i*w;
        returned_matrix.matrix[1][2] = 2.0f * (jk + iw) * inverse;
        returned_matrix.matrix[2][1] = 2.0f * (jk - iw) * inverse;

        return returned_matrix;
    }

    Vector3D<NativeEndian> add_vector(const Vector3D<NativeEndian> &vector, const Vector3D<NativeEndian> &value) noexcept {
        Vector3D<NativeEndian> returned_value;
        returned_value.i = vector.i + value.i;
        returned_value.j = vector.j + value.j;
        returned_value.k = vector.k + value.k;
        return returned_value;
    }

    Vector3D<NativeEndian> multiply_vector(const Vector3D<NativeEndian> &vector, float value) noexcept {
        Vector3D<NativeEndian> returned_value;
        returned_value.i = vector.i * value;
        returned_value.j = vector.j * value;
        returned_value.k = vector.k * value;
        return returned_value;
    }

    Matrix<NativeEndian> multiply_matrix(const Matrix<NativeEndian> &rotation, const Matrix<NativeEndian> &value) noexcept {
        Matrix<NativeEndian> new_rotation = {};
        for(int i = 0; i < 3; i++) {
            for(int j = 0; j < 3; j++) {
                float v = 0.0f;
                for(int k = 0; k < 3; k++) {
                    v += rotation.matrix[i][k] * value.matrix[k][j];
                }
                new_rotation.matrix[i][j] = v;
            }
        }
        return new_rotation;
    }

    Matrix<NativeEndian> multiply_matrix(const Matrix<NativeEndian> &rotation, float value) noexcept {
        Matrix<NativeEndian> new_rotation;
        for(int i = 0; i < 3; i++) {
            for(int j = 0; j < 3; j++) {
                new_rotation.matrix[i][j] = rotation.matrix[i][j] * value;
            }
        }
        return new_rotation;
    }

    Matrix<NativeEndian> invert_matrix(const Matrix<NativeEndian> &rotation) noexcept {
        // Find minor
        Matrix<NativeEndian> minor;
        for(int x = 0; x < 3; x++) {
            for(int y = 0; y < 3; y++) {
                float m[4];
                int m_i = 0;

                for(int xa = 0; xa < 3; xa++) {
                    for(int ya = 0; ya < 3; ya++) {
                        if(xa == x || ya == y) {
                            continue;
                        }
                        m[m_i++] = rotation.matrix[xa][ya];
                    }
                }

                minor.matrix[x][y] = (m[0] * m[3]) - (m[1] * m[2]);
            }
        }

        // Get determinant
        float determinant = rotation.matrix[0][0] * minor.matrix[0][0] - rotation.matrix[0][1] * minor.matrix[0][1] + rotation.matrix[0][2] * minor.matrix[0][2];

        // Cofactor, adjugate, and divide by determinant
        Matrix<NativeEndian> inverse;
        int sign = 1;
        for(int x = 0; x < 3; x++) {
            for(int y = 0; y < 3; y++) {
                inverse.matrix[x][y] = minor.matrix[y][x] * sign / determinant;
                sign = -sign;
            }
        }

        return inverse;
    }

    Vector3D<NativeEndian> rotate_vector(const Vector3D<NativeEndian> &vector, const Quaternion<NativeEndian> &rotation) noexcept {
        return rotate_vector(vector, quaternion_to_matrix(rotation));
    }

    Vector3D<NativeEndian> rotate_vector(const Vector3D<NativeEndian> &vector, const Matrix<NativeEndian> &rotation) noexcept {
        Vector3D<NativeEndian> returned_vector;
        returned_vector.i = vector.i * rotation.matrix[0][0] + vector.j * rotation.matrix[1][0] + vector.k * rotation.matrix[2][0];
        returned_vector.j = vector.i * rotation.matrix[0][1] + vector.j * rotation.matrix[1][1] + vector.k * rotation.matrix[2][1];
        returned_vector.k = vector.i * rotation.matrix[0][2] + vector.j * rotation.matrix[1][2] + vector.k * rotation.matrix[2][2];
        return returned_vector;
    }

    bool intersect_plane_with_points(const Plane3D<NativeEndian> &plane, const Point3D<NativeEndian> &point_a, const Point3D<NativeEndian> &point_b, Point3D<NativeEndian> *intersection, float epsilon) {
        // Get point a and b's distance from the plane
        float point_a_distance = point_a.distance_from_plane(plane);
        float point_b_distance = point_b.distance_from_plane(plane);

        // Make sure they are on opposite sides of the plane
        if(point_b_distance * point_a_distance >= 0.0F) {
            return false;
        }

        // If that's all we need, return true
        if(!intersection) {
            return true;
        }

        // Find the points in the front and back
        float back_distance;
        const Point3D<NativeEndian> *front_point;
        const Point3D<NativeEndian> *back_point;

        if(point_a_distance > point_b_distance) {
            front_point = &point_a;
            back_point = &point_b;
            back_distance = point_b_distance;
        }
        else {
            front_point = &point_b;
            back_point = &point_a;
            back_distance = point_a_distance;
        }

        // Next, find the difference between the front and back points and normalize, then add to point_b to get the intersection
        Vector3D<LittleEndian> vector = (*back_point - *front_point).normalize();
        *intersection = *back_point + vector * (back_distance + epsilon);
        return true;
    }
}
