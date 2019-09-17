/*
 * Invader (c) 2018 Kavawuvi
 *
 * This program is free software under the GNU General Public License v3.0. See LICENSE for more information.
 */

#ifndef INVADER__TAG__HEK__CLASS__DIALOGUE_HPP
#define INVADER__TAG__HEK__CLASS__DIALOGUE_HPP

#include "../../compiled_tag.hpp"
#include "../../../hek/data_type.hpp"
#include "../header.hpp"

namespace Invader::HEK {
    ENDIAN_TEMPLATE(EndianType) struct Dialogue {
        PAD(0x2);
        PAD(0x2);
        PAD(0xC);
        TagDependency<EndianType> idle_noncombat; // sound
        TagDependency<EndianType> idle_combat; // sound
        TagDependency<EndianType> idle_flee; // sound
        PAD(0x10);
        PAD(0x10);
        PAD(0x10);
        TagDependency<EndianType> pain_body_minor; // sound
        TagDependency<EndianType> pain_body_major; // sound
        TagDependency<EndianType> pain_shield; // sound
        TagDependency<EndianType> pain_falling; // sound
        TagDependency<EndianType> scream_fear; // sound
        TagDependency<EndianType> scream_pain; // sound
        TagDependency<EndianType> maimed_limb; // sound
        TagDependency<EndianType> maimed_head; // sound
        TagDependency<EndianType> death_quiet; // sound
        TagDependency<EndianType> death_violent; // sound
        TagDependency<EndianType> death_falling; // sound
        TagDependency<EndianType> death_agonizing; // sound
        TagDependency<EndianType> death_instant; // sound
        TagDependency<EndianType> death_flying; // sound
        PAD(0x10);
        TagDependency<EndianType> damaged_friend; // sound
        TagDependency<EndianType> damaged_friend_player; // sound
        TagDependency<EndianType> damaged_enemy; // sound
        TagDependency<EndianType> damaged_enemy_cm; // sound
        PAD(0x10);
        PAD(0x10);
        PAD(0x10);
        PAD(0x10);
        TagDependency<EndianType> hurt_friend; // sound
        TagDependency<EndianType> hurt_friend_re; // sound
        TagDependency<EndianType> hurt_friend_player; // sound
        TagDependency<EndianType> hurt_enemy; // sound
        TagDependency<EndianType> hurt_enemy_re; // sound
        TagDependency<EndianType> hurt_enemy_cm; // sound
        TagDependency<EndianType> hurt_enemy_bullet; // sound
        TagDependency<EndianType> hurt_enemy_needler; // sound
        TagDependency<EndianType> hurt_enemy_plasma; // sound
        TagDependency<EndianType> hurt_enemy_sniper; // sound
        TagDependency<EndianType> hurt_enemy_grenade; // sound
        TagDependency<EndianType> hurt_enemy_explosion; // sound
        TagDependency<EndianType> hurt_enemy_melee; // sound
        TagDependency<EndianType> hurt_enemy_flame; // sound
        TagDependency<EndianType> hurt_enemy_shotgun; // sound
        TagDependency<EndianType> hurt_enemy_vehicle; // sound
        TagDependency<EndianType> hurt_enemy_mountedweapon; // sound
        PAD(0x10);
        PAD(0x10);
        PAD(0x10);
        TagDependency<EndianType> killed_friend; // sound
        TagDependency<EndianType> killed_friend_cm; // sound
        TagDependency<EndianType> killed_friend_player; // sound
        TagDependency<EndianType> killed_friend_player_cm; // sound
        TagDependency<EndianType> killed_enemy; // sound
        TagDependency<EndianType> killed_enemy_cm; // sound
        TagDependency<EndianType> killed_enemy_player; // sound
        TagDependency<EndianType> killed_enemy_player_cm; // sound
        TagDependency<EndianType> killed_enemy_covenant; // sound
        TagDependency<EndianType> killed_enemy_covenant_cm; // sound
        TagDependency<EndianType> killed_enemy_floodcombat; // sound
        TagDependency<EndianType> killed_enemy_floodcombat_cm; // sound
        TagDependency<EndianType> killed_enemy_floodcarrier; // sound
        TagDependency<EndianType> killed_enemy_floodcarrier_cm; // sound
        TagDependency<EndianType> killed_enemy_sentinel; // sound
        TagDependency<EndianType> killed_enemy_sentinel_cm; // sound
        TagDependency<EndianType> killed_enemy_bullet; // sound
        TagDependency<EndianType> killed_enemy_needler; // sound
        TagDependency<EndianType> killed_enemy_plasma; // sound
        TagDependency<EndianType> killed_enemy_sniper; // sound
        TagDependency<EndianType> killed_enemy_grenade; // sound
        TagDependency<EndianType> killed_enemy_explosion; // sound
        TagDependency<EndianType> killed_enemy_melee; // sound
        TagDependency<EndianType> killed_enemy_flame; // sound
        TagDependency<EndianType> killed_enemy_shotgun; // sound
        TagDependency<EndianType> killed_enemy_vehicle; // sound
        TagDependency<EndianType> killed_enemy_mountedweapon; // sound
        TagDependency<EndianType> killing_spree; // sound
        PAD(0x10);
        PAD(0x10);
        PAD(0x10);
        TagDependency<EndianType> player_kill_cm; // sound
        TagDependency<EndianType> player_kill_bullet_cm; // sound
        TagDependency<EndianType> player_kill_needler_cm; // sound
        TagDependency<EndianType> player_kill_plasma_cm; // sound
        TagDependency<EndianType> player_kill_sniper_cm; // sound
        TagDependency<EndianType> anyone_kill_grenade_cm; // sound
        TagDependency<EndianType> player_kill_explosion_cm; // sound
        TagDependency<EndianType> player_kill_melee_cm; // sound
        TagDependency<EndianType> player_kill_flame_cm; // sound
        TagDependency<EndianType> player_kill_shotgun_cm; // sound
        TagDependency<EndianType> player_kill_vehicle_cm; // sound
        TagDependency<EndianType> player_kill_mountedweapon_cm; // sound
        TagDependency<EndianType> player_killling_spree_cm; // sound
        PAD(0x10);
        PAD(0x10);
        PAD(0x10);
        TagDependency<EndianType> friend_died; // sound
        TagDependency<EndianType> friend_player_died; // sound
        TagDependency<EndianType> friend_killed_by_friend; // sound
        TagDependency<EndianType> friend_killed_by_friendly_player; // sound
        TagDependency<EndianType> friend_killed_by_enemy; // sound
        TagDependency<EndianType> friend_killed_by_enemy_player; // sound
        TagDependency<EndianType> friend_killed_by_covenant; // sound
        TagDependency<EndianType> friend_killed_by_flood; // sound
        TagDependency<EndianType> friend_killed_by_sentinel; // sound
        TagDependency<EndianType> friend_betrayed; // sound
        PAD(0x10);
        PAD(0x10);
        TagDependency<EndianType> new_combat_alone; // sound
        TagDependency<EndianType> new_enemy_recent_combat; // sound
        TagDependency<EndianType> old_enemy_sighted; // sound
        TagDependency<EndianType> unexpected_enemy; // sound
        TagDependency<EndianType> dead_friend_found; // sound
        TagDependency<EndianType> alliance_broken; // sound
        TagDependency<EndianType> alliance_reformed; // sound
        TagDependency<EndianType> grenade_throwing; // sound
        TagDependency<EndianType> grenade_sighted; // sound
        TagDependency<EndianType> grenade_startle; // sound
        TagDependency<EndianType> grenade_danger_enemy; // sound
        TagDependency<EndianType> grenade_danger_self; // sound
        TagDependency<EndianType> grenade_danger_friend; // sound
        PAD(0x10);
        PAD(0x10);
        TagDependency<EndianType> new_combat_group_re; // sound
        TagDependency<EndianType> new_combat_nearby_re; // sound
        TagDependency<EndianType> alert_friend; // sound
        TagDependency<EndianType> alert_friend_re; // sound
        TagDependency<EndianType> alert_lost_contact; // sound
        TagDependency<EndianType> alert_lost_contact_re; // sound
        TagDependency<EndianType> blocked; // sound
        TagDependency<EndianType> blocked_re; // sound
        TagDependency<EndianType> search_start; // sound
        TagDependency<EndianType> search_query; // sound
        TagDependency<EndianType> search_query_re; // sound
        TagDependency<EndianType> search_report; // sound
        TagDependency<EndianType> search_abandon; // sound
        TagDependency<EndianType> search_group_abandon; // sound
        TagDependency<EndianType> group_uncover; // sound
        TagDependency<EndianType> group_uncover_re; // sound
        TagDependency<EndianType> advance; // sound
        TagDependency<EndianType> advance_re; // sound
        TagDependency<EndianType> retreat; // sound
        TagDependency<EndianType> retreat_re; // sound
        TagDependency<EndianType> cover; // sound
        PAD(0x10);
        PAD(0x10);
        PAD(0x10);
        PAD(0x10);
        TagDependency<EndianType> sighted_friend_player; // sound
        TagDependency<EndianType> shooting; // sound
        TagDependency<EndianType> shooting_vehicle; // sound
        TagDependency<EndianType> shooting_berserk; // sound
        TagDependency<EndianType> shooting_group; // sound
        TagDependency<EndianType> shooting_traitor; // sound
        TagDependency<EndianType> taunt; // sound
        TagDependency<EndianType> taunt_re; // sound
        TagDependency<EndianType> flee; // sound
        TagDependency<EndianType> flee_re; // sound
        TagDependency<EndianType> flee_leader_died; // sound
        TagDependency<EndianType> attempted_flee; // sound
        TagDependency<EndianType> attempted_flee_re; // sound
        TagDependency<EndianType> lost_contact; // sound
        TagDependency<EndianType> hiding_finished; // sound
        TagDependency<EndianType> vehicle_entry; // sound
        TagDependency<EndianType> vehicle_exit; // sound
        TagDependency<EndianType> vehicle_woohoo; // sound
        TagDependency<EndianType> vehicle_scared; // sound
        TagDependency<EndianType> vehicle_collision; // sound
        TagDependency<EndianType> partially_sighted; // sound
        TagDependency<EndianType> nothing_there; // sound
        TagDependency<EndianType> pleading; // sound
        PAD(0x10);
        PAD(0x10);
        PAD(0x10);
        PAD(0x10);
        PAD(0x10);
        PAD(0x10);
        TagDependency<EndianType> surprise; // sound
        TagDependency<EndianType> berserk; // sound
        TagDependency<EndianType> melee_attack; // sound
        TagDependency<EndianType> dive; // sound
        TagDependency<EndianType> uncover_exclamation; // sound
        TagDependency<EndianType> leap_attack; // sound
        TagDependency<EndianType> resurrection; // sound
        PAD(0x10);
        PAD(0x10);
        PAD(0x10);
        PAD(0x10);
        TagDependency<EndianType> celebration; // sound
        TagDependency<EndianType> check_body_enemy; // sound
        TagDependency<EndianType> check_body_friend; // sound
        TagDependency<EndianType> shooting_dead_enemy; // sound
        TagDependency<EndianType> shooting_dead_enemy_player; // sound
        PAD(0x10);
        PAD(0x10);
        PAD(0x10);
        PAD(0x10);
        TagDependency<EndianType> alone; // sound
        TagDependency<EndianType> unscathed; // sound
        TagDependency<EndianType> seriously_wounded; // sound
        TagDependency<EndianType> seriously_wounded_re; // sound
        TagDependency<EndianType> massacre; // sound
        TagDependency<EndianType> massacre_re; // sound
        TagDependency<EndianType> rout; // sound
        TagDependency<EndianType> rout_re; // sound
        PAD(0x10);
        PAD(0x10);
        PAD(0x10);
        PAD(0x10);
        PAD(0x2F0);
        ENDIAN_TEMPLATE(NewType) operator Dialogue<NewType>() const noexcept {
            Dialogue<NewType> copy = {};
            COPY_THIS(idle_noncombat);
            COPY_THIS(idle_combat);
            COPY_THIS(idle_flee);
            COPY_THIS(pain_body_minor);
            COPY_THIS(pain_body_major);
            COPY_THIS(pain_shield);
            COPY_THIS(pain_falling);
            COPY_THIS(scream_fear);
            COPY_THIS(scream_pain);
            COPY_THIS(maimed_limb);
            COPY_THIS(maimed_head);
            COPY_THIS(death_quiet);
            COPY_THIS(death_violent);
            COPY_THIS(death_falling);
            COPY_THIS(death_agonizing);
            COPY_THIS(death_instant);
            COPY_THIS(death_flying);
            COPY_THIS(damaged_friend);
            COPY_THIS(damaged_friend_player);
            COPY_THIS(damaged_enemy);
            COPY_THIS(damaged_enemy_cm);
            COPY_THIS(hurt_friend);
            COPY_THIS(hurt_friend_re);
            COPY_THIS(hurt_friend_player);
            COPY_THIS(hurt_enemy);
            COPY_THIS(hurt_enemy_re);
            COPY_THIS(hurt_enemy_cm);
            COPY_THIS(hurt_enemy_bullet);
            COPY_THIS(hurt_enemy_needler);
            COPY_THIS(hurt_enemy_plasma);
            COPY_THIS(hurt_enemy_sniper);
            COPY_THIS(hurt_enemy_grenade);
            COPY_THIS(hurt_enemy_explosion);
            COPY_THIS(hurt_enemy_melee);
            COPY_THIS(hurt_enemy_flame);
            COPY_THIS(hurt_enemy_shotgun);
            COPY_THIS(hurt_enemy_vehicle);
            COPY_THIS(hurt_enemy_mountedweapon);
            COPY_THIS(killed_friend);
            COPY_THIS(killed_friend_cm);
            COPY_THIS(killed_friend_player);
            COPY_THIS(killed_friend_player_cm);
            COPY_THIS(killed_enemy);
            COPY_THIS(killed_enemy_cm);
            COPY_THIS(killed_enemy_player);
            COPY_THIS(killed_enemy_player_cm);
            COPY_THIS(killed_enemy_covenant);
            COPY_THIS(killed_enemy_covenant_cm);
            COPY_THIS(killed_enemy_floodcombat);
            COPY_THIS(killed_enemy_floodcombat_cm);
            COPY_THIS(killed_enemy_floodcarrier);
            COPY_THIS(killed_enemy_floodcarrier_cm);
            COPY_THIS(killed_enemy_sentinel);
            COPY_THIS(killed_enemy_sentinel_cm);
            COPY_THIS(killed_enemy_bullet);
            COPY_THIS(killed_enemy_needler);
            COPY_THIS(killed_enemy_plasma);
            COPY_THIS(killed_enemy_sniper);
            COPY_THIS(killed_enemy_grenade);
            COPY_THIS(killed_enemy_explosion);
            COPY_THIS(killed_enemy_melee);
            COPY_THIS(killed_enemy_flame);
            COPY_THIS(killed_enemy_shotgun);
            COPY_THIS(killed_enemy_vehicle);
            COPY_THIS(killed_enemy_mountedweapon);
            COPY_THIS(killing_spree);
            COPY_THIS(player_kill_cm);
            COPY_THIS(player_kill_bullet_cm);
            COPY_THIS(player_kill_needler_cm);
            COPY_THIS(player_kill_plasma_cm);
            COPY_THIS(player_kill_sniper_cm);
            COPY_THIS(anyone_kill_grenade_cm);
            COPY_THIS(player_kill_explosion_cm);
            COPY_THIS(player_kill_melee_cm);
            COPY_THIS(player_kill_flame_cm);
            COPY_THIS(player_kill_shotgun_cm);
            COPY_THIS(player_kill_vehicle_cm);
            COPY_THIS(player_kill_mountedweapon_cm);
            COPY_THIS(player_killling_spree_cm);
            COPY_THIS(friend_died);
            COPY_THIS(friend_player_died);
            COPY_THIS(friend_killed_by_friend);
            COPY_THIS(friend_killed_by_friendly_player);
            COPY_THIS(friend_killed_by_enemy);
            COPY_THIS(friend_killed_by_enemy_player);
            COPY_THIS(friend_killed_by_covenant);
            COPY_THIS(friend_killed_by_flood);
            COPY_THIS(friend_killed_by_sentinel);
            COPY_THIS(friend_betrayed);
            COPY_THIS(new_combat_alone);
            COPY_THIS(new_enemy_recent_combat);
            COPY_THIS(old_enemy_sighted);
            COPY_THIS(unexpected_enemy);
            COPY_THIS(dead_friend_found);
            COPY_THIS(alliance_broken);
            COPY_THIS(alliance_reformed);
            COPY_THIS(grenade_throwing);
            COPY_THIS(grenade_sighted);
            COPY_THIS(grenade_startle);
            COPY_THIS(grenade_danger_enemy);
            COPY_THIS(grenade_danger_self);
            COPY_THIS(grenade_danger_friend);
            COPY_THIS(new_combat_group_re);
            COPY_THIS(new_combat_nearby_re);
            COPY_THIS(alert_friend);
            COPY_THIS(alert_friend_re);
            COPY_THIS(alert_lost_contact);
            COPY_THIS(alert_lost_contact_re);
            COPY_THIS(blocked);
            COPY_THIS(blocked_re);
            COPY_THIS(search_start);
            COPY_THIS(search_query);
            COPY_THIS(search_query_re);
            COPY_THIS(search_report);
            COPY_THIS(search_abandon);
            COPY_THIS(search_group_abandon);
            COPY_THIS(group_uncover);
            COPY_THIS(group_uncover_re);
            COPY_THIS(advance);
            COPY_THIS(advance_re);
            COPY_THIS(retreat);
            COPY_THIS(retreat_re);
            COPY_THIS(cover);
            COPY_THIS(sighted_friend_player);
            COPY_THIS(shooting);
            COPY_THIS(shooting_vehicle);
            COPY_THIS(shooting_berserk);
            COPY_THIS(shooting_group);
            COPY_THIS(shooting_traitor);
            COPY_THIS(taunt);
            COPY_THIS(taunt_re);
            COPY_THIS(flee);
            COPY_THIS(flee_re);
            COPY_THIS(flee_leader_died);
            COPY_THIS(attempted_flee);
            COPY_THIS(attempted_flee_re);
            COPY_THIS(lost_contact);
            COPY_THIS(hiding_finished);
            COPY_THIS(vehicle_entry);
            COPY_THIS(vehicle_exit);
            COPY_THIS(vehicle_woohoo);
            COPY_THIS(vehicle_scared);
            COPY_THIS(vehicle_collision);
            COPY_THIS(partially_sighted);
            COPY_THIS(nothing_there);
            COPY_THIS(pleading);
            COPY_THIS(surprise);
            COPY_THIS(berserk);
            COPY_THIS(melee_attack);
            COPY_THIS(dive);
            COPY_THIS(uncover_exclamation);
            COPY_THIS(leap_attack);
            COPY_THIS(resurrection);
            COPY_THIS(celebration);
            COPY_THIS(check_body_enemy);
            COPY_THIS(check_body_friend);
            COPY_THIS(shooting_dead_enemy);
            COPY_THIS(shooting_dead_enemy_player);
            COPY_THIS(alone);
            COPY_THIS(unscathed);
            COPY_THIS(seriously_wounded);
            COPY_THIS(seriously_wounded_re);
            COPY_THIS(massacre);
            COPY_THIS(massacre_re);
            COPY_THIS(rout);
            COPY_THIS(rout_re);
            return copy;
        }
    };
    static_assert(sizeof(Dialogue<BigEndian>) == 0x1010);

    void compile_dialogue_tag(CompiledTag &compiled, const std::byte *data, std::size_t size);
}
#endif
