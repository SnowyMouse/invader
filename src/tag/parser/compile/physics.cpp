// SPDX-License-Identifier: GPL-3.0-only

#include <invader/tag/parser/parser.hpp>
#include <invader/build/build_workload.hpp>

namespace Invader::Parser {
    void Physics::pre_compile(BuildWorkload &workload, std::size_t tag_index, std::size_t, std::size_t) {
        // Some of these calculations are from MosesofEgypt's reclaimer source. You can get it at https://github.com/Sigmmma/reclaimer/blob/master/reclaimer/hek/defs/objs/phys.py
        // I added this stuff because it is actually recalculated when the physics tag is run through tool.exe when building a cache file.

        // Check moment scale. Gearbox tool.exe will exception if this is 0, MCC tool.exe will make a map with NaNs in the inertial matrices.
        // This *should* be a 0->1 default, but sapien and friends blow up as well so this can help find the tag that does it.
        if(this->moment_scale == 0.0F) {
            REPORT_ERROR_PRINTF(workload, ERROR_TYPE_FATAL_ERROR, tag_index, "Physics moment scale is exactly 0. This is invalid as moment scale must be non-zero.");
            throw InvalidTagDataException();
        }

        double total_relative_mass = 0.0F;
        double density_scale = 0.0F;
        for(auto &mp : this->mass_points) {
            total_relative_mass += mp.relative_mass;
            if(mp.relative_density != 0.0) {
                density_scale += mp.relative_mass / mp.relative_density;
            }
        }

        double comx = 0.0, comy = 0.0, comz = 0.0;
        for(auto &mp : this->mass_points) {
            if(total_relative_mass != 0.0) {
                mp.mass = this->mass * mp.relative_mass / total_relative_mass;
                mp.density = this->density * mp.relative_density * density_scale / total_relative_mass;
            }

            comx += mp.mass * mp.position.x;
            comy += mp.mass * mp.position.y;
            comz += mp.mass * mp.position.z;
        }

        if(this->mass != 0.0) {
            double mass_inverse = 1.0 / this->mass;
            this->center_of_mass.x = mass_inverse * comx;
            this->center_of_mass.y = mass_inverse * comy;
            this->center_of_mass.z = mass_inverse * comz;
        }

        // Get inertial matrix stuff and set base moments.
        // 32-bit floats here match tool precision.
        float mxx = 0.0F;
        float myy = 0.0F;
        float mzz = 0.0F;
        float mxy = 0.0F;
        float myz = 0.0F;
        float mzx = 0.0F;
        for(auto &mp : this->mass_points) {
            double moment = this->moment_scale * mp.mass; // We checked earlier that moment_scale was non-zero.
            double x = mp.position.x - this->center_of_mass.x;
            double y = mp.position.y - this->center_of_mass.y;
            double z = mp.position.z - this->center_of_mass.z;
            double xx = x * x;
            double yy = y * y;
            double zz = z * z;

            this->xx_moment += moment * (yy + zz);
            this->yy_moment += moment * (zz + xx);
            this->zz_moment += moment * (xx + yy);

            double radius_term = 0.4 * moment * mp.radius * mp.radius;

            // If the base radius is anything below 0 then we account for local mass point radius.
            if(this->radius < 0.0F) {
                this->xx_moment += radius_term;
                this->yy_moment += radius_term;
                this->zz_moment += radius_term;
            }

            // We always acount for local mass point radius in the inertial matrices. Is this a tool oversight?
            mxx += radius_term + moment * (yy + zz);
            myy += radius_term + moment * (zz + xx);
            mzz += radius_term + moment * (xx + yy);
            mxy += -moment * x * y;
            myz += -moment * y * z;
            mzx += -moment * z * x;
        }

        HEK::Matrix<HEK::LittleEndian> m = {};
        m.matrix[0][0] = mxx;
        m.matrix[0][1] = mxy;
        m.matrix[0][2] = mzx;
        m.matrix[1][0] = mxy;
        m.matrix[1][1] = myy;
        m.matrix[1][2] = myz;
        m.matrix[2][0] = mzx;
        m.matrix[2][1] = myz;
        m.matrix[2][2] = mzz;

        this->inertial_matrix_and_inverse.resize(2);
        this->inertial_matrix_and_inverse[0].matrix = m;
        this->inertial_matrix_and_inverse[1].matrix = invert_matrix(m);
    }
}
