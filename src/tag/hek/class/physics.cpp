/*
 * Invader (c) 2019 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0. See LICENSE for more information.
 */

#include <cmath>
#include <vector>

#include "../compile.hpp"
#include "physics.hpp"

namespace Invader::HEK {
    void compile_physics_tag(CompiledTag &compiled, const std::byte *data, std::size_t size) {
        BEGIN_COMPILE(Physics);

        std::size_t matrix_count = tag.inertial_matrix_and_inverse.count;
        std::size_t matrix_offset = compiled.data.size();
        ADD_REFLEXIVE(tag.inertial_matrix_and_inverse);
        ADD_REFLEXIVE(tag.powered_mass_points);

        std::size_t mass_points_count = tag.mass_points.count;

        // Some of these calculations are from MosesofEgypt's reclaimer source. You can get it at https://bitbucket.org/Moses_of_Egypt/reclaimer/src/5d710221979fecbb8e71fa57c768f17f42f0010d/hek/defs/objs/phys.py?at=default&fileviewer=file-view-default
        // I added this stuff because it is actually recalculated when the physics tag is run through tool.exe when building a cache file.
        if(mass_points_count > 0) {
            std::size_t mass_points_offsets = compiled.data.size();

            // Get all of the stuff
            float total_relative_mass = 0.0f;

            ADD_REFLEXIVE_START(tag.mass_points) {
                total_relative_mass += reflexive.relative_mass;
            } ADD_REFLEXIVE_END;

            float average_relative_mass = total_relative_mass / mass_points_count;
            float density_scale = tag.density / mass_points_count;
            float density_scale_co = 0.0f;

            float xx = 1e-30f;
            float yy = 1e-30f;
            float zz = 1e-30f;

            float neg_zx = 0.0f;
            float neg_xy = 0.0f;
            float neg_yz = 0.0f;

            Point3D<NativeEndian> com = tag.center_of_mass;

            // Iterate once to get density and masses as well as to get inertial stuff.
            auto *mass_points = reinterpret_cast<PhysicsMassPoint<LittleEndian> *>(compiled.data.data() + mass_points_offsets);
            for(std::size_t i = 0; i < mass_points_count; i++) {
                float mass_percent = mass_points[i].relative_mass / total_relative_mass;
                float mass = mass_percent * tag.mass;
                mass_points[i].mass = mass;

                if(mass_points[i].relative_density != 0.0f) {
                    density_scale_co += mass_points[i].relative_mass / mass_points[i].relative_density;
                }

                Point3D<NativeEndian> pos = mass_points[i].position;

                float dist_xx = std::pow(com.y - pos.y, 2.0f) + std::pow(com.z - pos.z, 2.0f);
                float dist_yy = std::pow(com.x - pos.x, 2.0f) + std::pow(com.z - pos.z, 2.0f);
                float dist_zz = std::pow(com.x - pos.x, 2.0f) + std::pow(com.y - pos.y, 2.0f);

                float dist_zx = (com.x - pos.x) * (com.z - pos.z);
                float dist_xy = (com.x - pos.x) * (com.y - pos.y);
                float dist_yz = (com.y - pos.y) * (com.z - pos.z);

                float radius_term = 0.0f;
                if(mass_points[i].radius > 0.0f) {
                    radius_term = 4 * std::pow(10.0f, (2 * std::log10(mass_points[i].radius) - 1.0f));
                }

                xx += (dist_xx + radius_term) * mass;
                yy += (dist_yy + radius_term) * mass;
                zz += (dist_zz + radius_term) * mass;

                neg_zx -= dist_zx * mass;
                neg_xy -= dist_xy * mass;
                neg_yz -= dist_yz * mass;
            }
            density_scale *= density_scale_co;

            float moment_scale = tag.moment_scale;
            xx *= moment_scale;
            yy *= moment_scale;
            zz *= moment_scale;
            neg_zx *= moment_scale;
            neg_xy *= moment_scale;
            neg_yz *= moment_scale;

            // Finally write down density
            float mass_scale = 1.0f / average_relative_mass;
            for(std::size_t i = 0; i < mass_points_count; i++) {
                mass_points[i].density = density_scale * mass_scale * mass_points[i].relative_density;
            }

            // ...And matrix stuff
            if(matrix_count == 2) {
                auto *matrices = reinterpret_cast<PhysicsInertialMatrix<LittleEndian> *>(compiled.data.data() + matrix_offset);

                Matrix<LittleEndian> m;
                m.matrix[0][0] = xx;
                m.matrix[0][1] = neg_xy;
                m.matrix[0][2] = neg_zx;
                m.matrix[1][0] = neg_xy;
                m.matrix[1][1] = yy;
                m.matrix[1][2] = neg_yz;
                m.matrix[2][0] = neg_zx;
                m.matrix[2][1] = neg_yz;
                m.matrix[2][2] = zz;
                tag.xx_moment = m.matrix[0][0];
                tag.yy_moment = m.matrix[1][1];
                tag.zz_moment = m.matrix[2][2];
                matrices[0].matrix = m;
                matrices[1].matrix = invert_matrix(m);
            }
        }

        FINISH_COMPILE
    }
}
