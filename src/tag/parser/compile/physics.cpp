// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/build/build_workload.hpp>

namespace Invader::Parser {
    void Physics::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        // Some of these calculations are from MosesofEgypt's reclaimer source. You can get it at https://github.com/Sigmmma/reclaimer/blob/master/reclaimer/hek/defs/objs/phys.py
        // I added this stuff because it is actually recalculated when the physics tag is run through tool.exe when building a cache file.

        // Check moment scale. Gearbox tool.exe will exception if this is 0, MCC tool.exe will make a map with NaNs in the inertial matrices.
        // This *should* be a 0->1 default, but sapien and friends blow up as well so this can help find the tag that does it.
        if(this->moment_scale == 0.0f) {
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Physics moment scale is exactly 0. This is invalid as moment scale must be non-zero.");
            throw InvalidTagDataException();
        }

        double total_relative_mass = 0.0f;
        HEK::Point3D<HEK::NativeEndian> com = {};

        for(auto &mp : this->mass_points) {
            total_relative_mass += mp.relative_mass;
        }

        double comx = 0.0, comy = 0.0, comz = 0.0;

        for(auto &mp : this->mass_points) {
            comx += mp.position.x * mp.relative_mass;
            comy += mp.position.y * mp.relative_mass;
            comz += mp.position.z * mp.relative_mass;
        }

        double total_relative_mass_inverse = 1.0 / total_relative_mass;

        com.x = comx * total_relative_mass_inverse;
        com.y = comy * total_relative_mass_inverse;
        com.z = comz * total_relative_mass_inverse;

        this->center_of_mass = com;

        std::size_t mass_points_count = this->mass_points.size();
        double average_relative_mass = total_relative_mass / mass_points_count;
        double density_scale = this->density / mass_points_count;
        double density_scale_co = 0.0f;
        double xx = 0.0f;
        double yy = 0.0f;
        double zz = 0.0f;
        double old_xx = 0.0f;
        double old_yy = 0.0f;
        double old_zz = 0.0f;
        double neg_zx = 0.0f;
        double neg_xy = 0.0f;
        double neg_yz = 0.0f;

        // Iterate once to get density and masses as well as to get inertial stuff.
        for(auto &mp : this->mass_points) {
            double mass_percent = mp.relative_mass * total_relative_mass_inverse;
            double mass = mass_percent * this->mass;
            mp.mass = mass;

            if(mp.relative_density != 0.0f) {
                density_scale_co += mp.relative_mass / mp.relative_density;
            }

            HEK::Point3D<HEK::NativeEndian> pos = mp.position;

            double dist_xx = std::pow(com.y - pos.y, 2.0f) + std::pow(com.z - pos.z, 2.0f);
            double dist_yy = std::pow(com.x - pos.x, 2.0f) + std::pow(com.z - pos.z, 2.0f);
            double dist_zz = std::pow(com.x - pos.x, 2.0f) + std::pow(com.y - pos.y, 2.0f);
            double dist_zx = (com.x - pos.x) * (com.z - pos.z);
            double dist_xy = (com.x - pos.x) * (com.y - pos.y);
            double dist_yz = (com.y - pos.y) * (com.z - pos.z);
            double radius_term = 0.0f;
            if(mp.radius > 0.0f) {
                radius_term = 4 * std::pow(10.0f, (2 * std::log10(mp.radius) - 1.0f));
            }

            xx += (dist_xx + radius_term) * mass;
            yy += (dist_yy + radius_term) * mass;
            zz += (dist_zz + radius_term) * mass;

            old_xx += dist_xx * mass;
            old_yy += dist_yy * mass;
            old_zz += dist_zz * mass;

            neg_zx -= dist_zx * mass;
            neg_xy -= dist_xy * mass;
            neg_yz -= dist_yz * mass;
        }
        density_scale *= density_scale_co;

        double moment_scale = this->moment_scale;
        xx *= moment_scale;
        yy *= moment_scale;
        zz *= moment_scale;
        old_xx *= moment_scale;
        old_yy *= moment_scale;
        old_zz *= moment_scale;
        neg_zx *= moment_scale;
        neg_xy *= moment_scale;
        neg_yz *= moment_scale;

        // Set base moments. If the base radius is anything above 0 then we use values that do not account for local mass point radius.
        if(this->radius > 0.0f) {
            this->xx_moment = old_xx;
            this->yy_moment = old_yy;
            this->zz_moment = old_zz;
        }
        else {
            this->xx_moment = xx;
            this->yy_moment = yy;
            this->zz_moment = zz;
        }

        // Finally write down density
        double mass_scale = 1.0F / average_relative_mass;
        for(std::size_t i = 0; i < mass_points_count; i++) {
            mass_points[i].density = density_scale * mass_scale * mass_points[i].relative_density;
        }

        // ...And matrix stuff
        HEK::Matrix<HEK::LittleEndian> m;
        m.matrix[0][0] = xx;
        m.matrix[0][1] = neg_xy;
        m.matrix[0][2] = neg_zx;
        m.matrix[1][0] = neg_xy;
        m.matrix[1][1] = yy;
        m.matrix[1][2] = neg_yz;
        m.matrix[2][0] = neg_zx;
        m.matrix[2][1] = neg_yz;
        m.matrix[2][2] = zz;
        this->inertial_matrix_and_inverse.resize(2);
        this->inertial_matrix_and_inverse[0].matrix = m;
        this->inertial_matrix_and_inverse[1].matrix = invert_matrix(m);
    }
}
