// SPDX-License-Identifier: GPL-3.0-only

#ifndef INVADER__TAG__HEK__CLASS__COMPILE_HPP
#define INVADER__TAG__HEK__CLASS__COMPILE_HPP

#define ADD_CHICAGO_MAP(reflexive_struct) ADD_REFLEXIVE_START(reflexive_struct) { \
                                              ADD_DEPENDENCY_ADJUST_SIZES(reflexive.map); \
                                              DEFAULT_VALUE(reflexive.map_u_scale, 1.0f); \
                                              DEFAULT_VALUE(reflexive.map_v_scale, 1.0f); \
                                              INITIALIZE_SHADER_ANIMATION(reflexive); \
                                          } ADD_REFLEXIVE_END

#define MAKE_WHITE_IF_BLACK(what) if(what.red == 0.0f && what.green == 0.0f && what.blue == 0.0f) { what.red = 1.0f; what.green = 1.0f; what.blue = 1.0f; }
#define MAKE_ONE_IF_ZERO(what) if(what == 0.0f) { what = 1.0f; }

#define INITIALIZE_SHADER_ANIMATION(what) DEFAULT_VALUE(what .u_animation_period, 1.0f); \
                                          DEFAULT_VALUE(what .u_animation_scale, 1.0f); \
                                          DEFAULT_VALUE(what .v_animation_period, 1.0f); \
                                          DEFAULT_VALUE(what .v_animation_scale, 1.0f); \
                                          DEFAULT_VALUE(what .rotation_animation_period, 1.0f); \
                                          DEFAULT_VALUE(what .rotation_animation_scale, 360.0f);

#define COPY_SHADER_DATA COPY_THIS(shader_flags); \
                         COPY_THIS(detail_level); \
                         COPY_THIS(power); \
                         COPY_THIS(color_of_emitted_light); \
                         COPY_THIS(tint_color); \
                         COPY_THIS(physics_flags); \
                         COPY_THIS(material_type); \
                         COPY_THIS(shader_type);

#define COMPILE_OBJECT_DATA ADD_DEPENDENCY_ADJUST_SIZES(tag.model); \
                            ADD_DEPENDENCY_ADJUST_SIZES(tag.animation_graph); \
                            ADD_DEPENDENCY_ADJUST_SIZES(tag.collision_model); \
                            ADD_DEPENDENCY_ADJUST_SIZES(tag.physics); \
                            ADD_DEPENDENCY_ADJUST_SIZES(tag.modifier_shader); \
                            ADD_DEPENDENCY_ADJUST_SIZES(tag.creation_effect); \
                            ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.attachments, type); \
                            ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.widgets, reference); \
                            if(tag.render_bounding_radius.read() < tag.bounding_radius.read()) { \
                                tag.render_bounding_radius = tag.bounding_radius; \
                            } \
                            ADD_REFLEXIVE_START(tag.functions) { \
                                DEFAULT_VALUE(reflexive.period, 1.0f); \
                                if(reflexive.bounds.from == 0.0f && reflexive.bounds.to == 0.0f) { \
                                    reflexive.bounds.to = 1.0f; \
                                } \
                                reflexive.inverse_bounds = 1.0f / (reflexive.bounds.to - reflexive.bounds.from); \
                                if(reflexive.step_count > 1) { \
                                    reflexive.inverse_step = 1.0f / (reflexive.step_count - 1); \
                                } \
                                reflexive.inverse_period = 1.0f / reflexive.period; \
                            } ADD_REFLEXIVE_END; \
                            ADD_REFLEXIVE_START(tag.change_colors) { \
                                ADD_REFLEXIVE_START(reflexive.permutations) {\
                                    DEFAULT_VALUE(reflexive.weight, 1.0F); \
                                } ADD_REFLEXIVE_END; \
                            } ADD_REFLEXIVE_END; \
                            tag.has_change_colors = tag.change_colors.count != 0; \
                            ADD_REFLEXIVE(tag.predicted_resources);

#define COMPILE_DEVICE_DATA COMPILE_OBJECT_DATA \
                            ADD_DEPENDENCY_ADJUST_SIZES(tag.open); \
                            ADD_DEPENDENCY_ADJUST_SIZES(tag.close); \
                            ADD_DEPENDENCY_ADJUST_SIZES(tag.opened); \
                            ADD_DEPENDENCY_ADJUST_SIZES(tag.closed); \
                            ADD_DEPENDENCY_ADJUST_SIZES(tag.depowered); \
                            ADD_DEPENDENCY_ADJUST_SIZES(tag.repowered); \
                            ADD_DEPENDENCY_ADJUST_SIZES(tag.delay_effect); \
                            tag.inverse_power_transition_time = 1.0F / (TICK_RATE * tag.power_transition_time); \
                            tag.inverse_power_acceleration_time = 1.0F / (TICK_RATE * tag.power_acceleration_time); \
                            tag.inverse_position_transition_time = 1.0f / (TICK_RATE * tag.position_transition_time); \
                            tag.inverse_position_acceleration_time = 1.0f / (TICK_RATE * tag.position_acceleration_time);

#define COMPILE_ITEM_DATA COMPILE_OBJECT_DATA\
                          ADD_DEPENDENCY_ADJUST_SIZES(tag.material_effects);\
                          ADD_DEPENDENCY_ADJUST_SIZES(tag.collision_sound);\
                          ADD_DEPENDENCY_ADJUST_SIZES(tag.detonating_effect);\
                          ADD_DEPENDENCY_ADJUST_SIZES(tag.detonation_effect);

#define COMPILE_UNIT_DATA COMPILE_OBJECT_DATA \
                          ADD_DEPENDENCY_ADJUST_SIZES(tag.integrated_light_toggle); \
                          ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.camera_tracks, track); \
                          ADD_DEPENDENCY_ADJUST_SIZES(tag.spawned_actor); \
                          ADD_DEPENDENCY_ADJUST_SIZES(tag.melee_damage); \
                          ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.new_hud_interfaces, hud); \
                          ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.dialogue_variants, dialogue); \
                          ADD_REFLEXIVE(tag.powered_seats); \
                          ADD_BASIC_DEPENDENCY_REFLEXIVE(tag.weapons, weapon); \
                          ADD_REFLEXIVE_START(tag.seats) { \
                              ADD_BASIC_DEPENDENCY_REFLEXIVE(reflexive.camera_tracks, track); \
                              ADD_BASIC_DEPENDENCY_REFLEXIVE(reflexive.unit_hud_interface, hud); \
                              ADD_DEPENDENCY_ADJUST_SIZES(reflexive.built_in_gunner); \
                          } ADD_REFLEXIVE_END

#define COMPILE_MULTITEXTURE_OVERLAY(reflexive_struct) ADD_REFLEXIVE_START(reflexive_struct) { \
                                                           ADD_DEPENDENCY_ADJUST_SIZES(reflexive.primary); \
                                                           ADD_DEPENDENCY_ADJUST_SIZES(reflexive.secondary); \
                                                           ADD_DEPENDENCY_ADJUST_SIZES(reflexive.tertiary); \
                                                           ADD_REFLEXIVE(reflexive.effectors); \
                                                       } ADD_REFLEXIVE_END

#define ADD_MODEL_COLLISION_BSP(reflexive_struct) ADD_REFLEXIVE_START(reflexive_struct) { \
                                                        ADD_REFLEXIVE(reflexive.bsp3d_nodes); \
                                                        ADD_REFLEXIVE(reflexive.planes); \
                                                        ADD_REFLEXIVE(reflexive.leaves); \
                                                        ADD_REFLEXIVE(reflexive.bsp2d_references); \
                                                        ADD_REFLEXIVE(reflexive.bsp2d_nodes); \
                                                        ADD_REFLEXIVE(reflexive.surfaces); \
                                                        ADD_REFLEXIVE(reflexive.edges); \
                                                        ADD_REFLEXIVE(reflexive.vertices); \
                                                 } ADD_REFLEXIVE_END;

#endif
