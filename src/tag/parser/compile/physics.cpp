// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/definition/physics.hpp>
#include <invader/build/build_workload.hpp>

namespace Invader::Parser {
    void Physics::post_cache_deformat() {
        this->postprocess_hek_data(); // do this to mitigate precision issues
    }

    void Physics::postprocess_hek_data() {
        // Some of these calculations are from MosesofEgypt's reclaimer source. You can get it at https://bitbucket.org/Moses_of_Egypt/reclaimer/src/5d710221979fecbb8e71fa57c768f17f42f0010d/hek/defs/objs/phys.py?at=default&fileviewer=file-view-default
        // I added this stuff because it is actually recalculated when the physics tag is run through tool.exe when building a cache file.
        float total_relative_mass = 0.0f;
        for(auto &mp : this->mass_points) {
            total_relative_mass += mp.relative_mass;
        }

        std::size_t mass_points_count = this->mass_points.size();
        float average_relative_mass = total_relative_mass / mass_points_count;
        float density_scale = this->density / mass_points_count;
        float density_scale_co = 0.0f;

        float xx = 1e-30f;
        float yy = 1e-30f;
        float zz = 1e-30f;

        float neg_zx = 0.0f;
        float neg_xy = 0.0f;
        float neg_yz = 0.0f;

        Point3D<NativeEndian> com = this->center_of_mass;

        // Iterate once to get density and masses as well as to get inertial stuff.
        for(auto &mp : this->mass_points) {
            float mass_percent = mp.relative_mass / total_relative_mass;
            float mass = mass_percent * this->mass;
            mp.mass = mass;

            if(mp.relative_density != 0.0f) {
                density_scale_co += mp.relative_mass / mp.relative_density;
            }

            Point3D<NativeEndian> pos = mp.position;

            float dist_xx = std::pow(com.y - pos.y, 2.0f) + std::pow(com.z - pos.z, 2.0f);
            float dist_yy = std::pow(com.x - pos.x, 2.0f) + std::pow(com.z - pos.z, 2.0f);
            float dist_zz = std::pow(com.x - pos.x, 2.0f) + std::pow(com.y - pos.y, 2.0f);

            float dist_zx = (com.x - pos.x) * (com.z - pos.z);
            float dist_xy = (com.x - pos.x) * (com.y - pos.y);
            float dist_yz = (com.y - pos.y) * (com.z - pos.z);

            float radius_term = 0.0f;
            if(mp.radius > 0.0f) {
                radius_term = 4 * std::pow(10.0f, (2 * std::log10(mp.radius) - 1.0f));
            }

            xx += (dist_xx + radius_term) * mass;
            yy += (dist_yy + radius_term) * mass;
            zz += (dist_zz + radius_term) * mass;

            neg_zx -= dist_zx * mass;
            neg_xy -= dist_xy * mass;
            neg_yz -= dist_yz * mass;
        }
        density_scale *= density_scale_co;

        float moment_scale = this->moment_scale;
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
        this->xx_moment = m.matrix[0][0];
        this->yy_moment = m.matrix[1][1];
        this->zz_moment = m.matrix[2][2];
        this->inertial_matrix_and_inverse.resize(2);
        this->inertial_matrix_and_inverse[0].matrix = m;
        this->inertial_matrix_and_inverse[1].matrix = invert_matrix(m);
    }
}
