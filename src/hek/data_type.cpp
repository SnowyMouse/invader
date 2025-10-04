// SPDX-License-Identifier: GPL-3.0-only

#include <cmath>

#include <invader/hek/data_type.hpp>
#include <invader/tag/parser/compile/model.hpp>
#include <invader/tag/hek/definition.hpp>

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

    Matrix<NativeEndian> euler_to_matrix(const Euler3D<NativeEndian> &rotation) noexcept {
        return quaternion_to_matrix(euler_to_quaternion(rotation));
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
        float determinant = rotation.matrix[0][0] * rotation.matrix[1][1] * rotation.matrix[2][2] +
                            rotation.matrix[0][1] * rotation.matrix[1][2] * rotation.matrix[2][0] +
                            rotation.matrix[0][2] * rotation.matrix[1][0] * rotation.matrix[2][1] -
                            rotation.matrix[0][0] * rotation.matrix[1][2] * rotation.matrix[2][1] -
                            rotation.matrix[0][1] * rotation.matrix[1][0] * rotation.matrix[2][2] -
                            rotation.matrix[0][2] * rotation.matrix[1][1] * rotation.matrix[2][0];

        float ood = 1.0F / determinant;
        Matrix<NativeEndian> inverse;
        for(int i = 0; i < 3; i++) {
            for(int j = 0; j < 3; j++) {
                int ip = i < 2 ? i + 1 : 0;
                int im = i > 0 ? i - 1 : 2;
                int jp = j < 2 ? j + 1 : 0;
                int jm = j > 0 ? j - 1 : 2;
                inverse.matrix[j][i] = ood * (rotation.matrix[ip][jp] * rotation.matrix[im][jm] - rotation.matrix[ip][jm] * rotation.matrix[im][jp]);
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

    template<unsigned int bits> constexpr std::int32_t compress_float(float f) {
        constexpr const std::uint32_t SIGNED_BIT = 1 << (bits - 1);
        constexpr const std::uint32_t MASK = SIGNED_BIT - 1;

        // Clamp to -1 to +1
        f = std::max(std::min(f, 1.0F), -1.0F);

        // Compressing a float basically means taking a -1 to 1 (inclusive) value and turning it into a signed integer.
        // So, if a float is 1.0 and we need to compress it to a 16-bit integer, the result is 32767.
        // If a float is -1.0 and we need to compress it to a 16-bit integer, the result is -1 (or 32768). 0 is always 0.
        if(f >= 0.0F) {
            return static_cast<std::uint32_t>(f * MASK + 0.5F);
        }
        else {
            return static_cast<std::uint32_t>((1.0 + f) * MASK + 0.5) | SIGNED_BIT;
        }
    }

    template<unsigned int bits> constexpr float decompress_float(std::uint32_t f) {
        constexpr const std::uint32_t SIGNED_BIT = 1 << (bits - 1);
        constexpr const std::uint32_t MASK = SIGNED_BIT - 1;
        auto number = static_cast<double>(f & MASK) / MASK;

        if(f & SIGNED_BIT) {
            return number - 1.0;
        }
        else {
            return number;
        }
    }

    static_assert(decompress_float<16>(32768) == -1.0);
    static_assert(compress_float<16>(-1.0) == 32768);
    static_assert(decompress_float<16>(32767) == 1.0);
    static_assert(compress_float<16>(1.0) == 32767);
    static_assert(decompress_float<16>(0) == 0.0);
    static_assert(compress_float<16>(0) == 0);

    std::uint32_t compress_vector(float i, float j, float k) noexcept {
        return (compress_float<11>(i)) | (compress_float<11>(j) << 11) | (compress_float<10>(k) << 22);
    }

    void decompress_vector(std::uint32_t v, float &i, float &j, float &k) {
        i = decompress_float<11>(v);
        j = decompress_float<11>(v >> 11);
        k = decompress_float<10>(v >> 22);
    }

    ModelVertexCompressed<NativeEndian> compress_model_vertex(const ModelVertexUncompressed<NativeEndian> &vertex) noexcept {
        ModelVertexCompressed<NativeEndian> r;
        r.position = vertex.position;
        r.node0_index = vertex.node0_index > Invader::Parser::MaxCompressedModelNodeIndex::MAX_COMPRESSED_MODEL_NODE_INDEX ? -3 : vertex.node0_index * 3;
        r.node0_weight = static_cast<std::int16_t>(compress_float<16>(vertex.node0_weight));
        r.node1_index = vertex.node1_index > Invader::Parser::MaxCompressedModelNodeIndex::MAX_COMPRESSED_MODEL_NODE_INDEX ? -3 : vertex.node1_index * 3;
        r.texture_coordinate_u = static_cast<std::int16_t>(compress_float<16>(vertex.texture_coords.x));
        r.texture_coordinate_v = static_cast<std::int16_t>(compress_float<16>(vertex.texture_coords.y));
        r.normal = compress_vector(vertex.normal.i, vertex.normal.j, vertex.normal.k);
        r.binormal = compress_vector(vertex.binormal.i, vertex.binormal.j, vertex.binormal.k);
        r.tangent = compress_vector(vertex.tangent.i, vertex.tangent.j, vertex.tangent.k);
        return r;
    }

    ModelVertexUncompressed<NativeEndian> decompress_model_vertex(const ModelVertexCompressed<NativeEndian> &vertex) noexcept {
        ModelVertexUncompressed<NativeEndian> r;
        r.position = vertex.position;
        r.node0_index = vertex.node0_index < 0 ? 65535 : vertex.node0_index / 3;
        r.node0_weight = decompress_float<16>(vertex.node0_weight);
        r.node1_index = vertex.node1_index < 0 ? 65535 : vertex.node1_index / 3;
        r.node1_weight = 1.0F - r.node0_weight; // this is just derived from node0_weight
        r.texture_coords.x = decompress_float<16>(vertex.texture_coordinate_u);
        r.texture_coords.y = decompress_float<16>(vertex.texture_coordinate_v);

        float normal_i, normal_j, normal_k;

        decompress_vector(vertex.normal, normal_i, normal_j, normal_k);
        r.normal.i = normal_i;
        r.normal.j = normal_j;
        r.normal.k = normal_k;

        decompress_vector(vertex.binormal, normal_i, normal_j, normal_k);
        r.binormal.i = normal_i;
        r.binormal.j = normal_j;
        r.binormal.k = normal_k;

        decompress_vector(vertex.tangent, normal_i, normal_j, normal_k);
        r.tangent.i = normal_i;
        r.tangent.j = normal_j;
        r.tangent.k = normal_k;
        return r;
    }

    ScenarioStructureBSPMaterialCompressedRenderedVertex<NativeEndian> compress_sbsp_rendered_vertex(const ScenarioStructureBSPMaterialUncompressedRenderedVertex<NativeEndian> &vertex) noexcept {
        ScenarioStructureBSPMaterialCompressedRenderedVertex<NativeEndian> r;
        r.position = vertex.position;
        r.texture_coords = vertex.texture_coords;
        r.normal = compress_vector(vertex.normal.i, vertex.normal.j, vertex.normal.k);
        r.binormal = compress_vector(vertex.binormal.i, vertex.binormal.j, vertex.binormal.k);
        r.tangent = compress_vector(vertex.tangent.i, vertex.tangent.j, vertex.tangent.k);
        return r;
    }

    ScenarioStructureBSPMaterialUncompressedRenderedVertex<NativeEndian> decompress_sbsp_rendered_vertex(const ScenarioStructureBSPMaterialCompressedRenderedVertex<NativeEndian> &vertex) noexcept {
        ScenarioStructureBSPMaterialUncompressedRenderedVertex<NativeEndian> r;
        r.position = vertex.position;
        r.texture_coords = vertex.texture_coords;

        float normal_i, normal_j, normal_k;

        decompress_vector(vertex.normal, normal_i, normal_j, normal_k);
        r.normal.i = normal_i;
        r.normal.j = normal_j;
        r.normal.k = normal_k;
        r.normal = r.normal.normalize();

        decompress_vector(vertex.binormal, normal_i, normal_j, normal_k);
        r.binormal.i = normal_i;
        r.binormal.j = normal_j;
        r.binormal.k = normal_k;
        r.binormal = r.binormal.normalize();

        decompress_vector(vertex.tangent, normal_i, normal_j, normal_k);
        r.tangent.i = normal_i;
        r.tangent.j = normal_j;
        r.tangent.k = normal_k;
        r.tangent = r.tangent.normalize();
        return r;
    }

    ScenarioStructureBSPMaterialCompressedLightmapVertex<NativeEndian> compress_sbsp_lightmap_vertex(const ScenarioStructureBSPMaterialUncompressedLightmapVertex<NativeEndian> &vertex) noexcept {
        ScenarioStructureBSPMaterialCompressedLightmapVertex<NativeEndian> r;

        r.normal = compress_vector(vertex.normal.i, vertex.normal.j, vertex.normal.k);
        r.texture_coordinate_x = compress_float<16>(vertex.texture_coords.x);
        r.texture_coordinate_y = compress_float<16>(vertex.texture_coords.y);

        return r;
    }

    ScenarioStructureBSPMaterialUncompressedLightmapVertex<NativeEndian> decompress_sbsp_lightmap_vertex(const ScenarioStructureBSPMaterialCompressedLightmapVertex<NativeEndian> &vertex) noexcept {
        ScenarioStructureBSPMaterialUncompressedLightmapVertex<NativeEndian> r;

        float normal_i, normal_j, normal_k;

        decompress_vector(vertex.normal, normal_i, normal_j, normal_k);
        r.normal.i = normal_i;
        r.normal.j = normal_j;
        r.normal.k = normal_k;
        //r.normal = r.normal.normalize(); // NOTE: these vectors are not always normalized, and normalizing them may change the apparent intensity of some bumpmaps

        r.texture_coords.x = decompress_float<16>(vertex.texture_coordinate_x);
        r.texture_coords.y = decompress_float<16>(vertex.texture_coordinate_y);

        return r;
    }

}
