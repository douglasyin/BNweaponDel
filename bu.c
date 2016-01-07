#include <stdio.h>
#include <string.h>
#include <jansson.h>

int
isBlack(const char *name, char *bl[], int num)
{
	int		i = 0;

	for (i = 0; i < num; i++) {

		if (!strcmp(name, bl[i])) {
			printf("%s\n", name);
			return (1);
		}
	}
	return (0);
}

int
main()
{
	json_t         *units, *current_unit, *side, *weapons, *currentweapon;
	json_error_t	error;
	size_t		size;
	const char     *unitname, *weaponname;
	int		progress  , laststep, spawnable;
	char           *blacklist[] = {
		/*
		 * "veh_tank_chem_heavy",    // heavy chem tank
		 * "veh_tank_plasma", "veh_artillery_mega",
		 * "veh_artillery_super",    // earth shaker
		 * "veh_tank_tesla",     // Lighting Tank "veh_tank_mega",
		 * // MT "veh_tank_laser", "veh_plasma_turret",
		 * "s_chem_trooper", "s_trooper_plasma", "ship_mini_sub",
		 * "ship_mini_sub_65", "s_flame_heavy",  // fire lizzard
		 * "veh_combine_tank", "s_hitman", "veh_tank_basilisk", //
		 * Basilisk "veh_tank_flame_heavy",  // Dragon
		 * "ship_raft_trooper_65", "def_tesla_coil",   // tesla tower
		 * "def_blockhouse", "s_trooper_fire_ice", // elemental
		 * trooper "ship_minelayer",  // minelayer destroyer
		 * "veh_hoverbike",  //T16 Aero Bike
		 * "veh_hoverbike_rebel_60", "s_trooper_saboteur_heavy",
		 * "veh_tank_heavier", // Heavier tank "veh_tank_chem_light",
		 * "veh_tank_cryo", "s_arctic_trooper" //  29
		 *
		 * }; */
		"air_warthog_2", //B10 II
		"air_warthog", //B10
		"veh_elemental_rover", //nomad
		"ancient_sandworm_player", //legendary sandworm
		"veh_tank_plasma", //plasma tank
		"veh_cannon_plasma", //plasma cannon
		"veh_ancient_robot_2_player_right", //meele
		"veh_ancient_robot_2_player_left", //ranged
		"s_warmonger_white", //lulu
		"s_warmonger_black", //The black drink
		"s_warmonger_yellow", //little yellow
		"veh_scuba_trooper", //MK2
		"veh_phalanx_tank_player", //phalanx
		"def_spiderwasp_egg_player", //egg
		"ancient_raptor_player", //Legendary raptor
		"ancient_boar_player", //L - boar
		"ancient_mammoth_player" // L - mammoth
	};
	/*
	 *
	 * "s_laser_machingun_mission","ship_mini_sub","veh_tank_arctic_heavy",
	 * "ancient_sandworm_player","veh_tank_plasma","veh_tank_mega","s_troo
	 * per_veteran","s_flame_heavy","s_trooper_underdog",          // 6
	 * "s_trooper_fire_ice","s_trooper_dragoon_heavy","s_trooper_lightning
	 * ","veh_tank_tesla",      // 4
	 * "s_trooper_verteran","s_midrange_agent","s_trooper_specialAgent","v
	 * eh_tank_arctic_heavy",   // 4
	 * "s_chem_trooper","s_grenadier_bio","veh_tank_chem_heavy","veh_flame
	 * _turret","s_hitman",     // 5
	 * "veh_tank_flame_heavy","veh_tank_basilisk","s_trooper_saboteur_heav
	 * y","s_arctic_trooper",   // 4
	 * "veh_artillery_mega","veh_tank_heavier","veh_artillery_super","veh_
	 * tankdestroyer",          // 4
	 * "ship_raft_trooper_65","ship_minelayer_65","ship_mini_sub_65",
	 * // 3
	 * "veh_combine_tank","veh_tank_cryo","s_ninja","veh_rockettruck_light
	 * "};                      // 4
	 */

	units = json_load_file("./BattleUnits.json", 0, &error);
	if (!units) {
		/* the error variable contains error information */
		printf("load error \n");
		exit(1);
	}
	size = json_object_size(units);
	printf("Proceeding %d units...\n", (int)size);

	progress = 0;
	laststep = 0;
	spawnable = 0;

	json_object_foreach(units, unitname, current_unit) {

		progress++;
		if ((progress - laststep) >= size / 10) {
			//printf("ooo");
			laststep = progress;
		}
		/*
		 * if( !json_object_del( current_unit, "deathSpawnedUnit" )
		 * ){ spawnable ++; }
		 */
		json_object_set(current_unit, "deathAnimationName", json_string("s_sandbags_front_idle"));
		side = json_object_get(current_unit, "side");

		if (!strcmp(json_string_value(side), "Hostile") || isBlack(unitname, blacklist, 0)) {
			weapons = json_object_get(current_unit, "weapons");
			json_object_clear(weapons);	/* clear it */

			//json_object_set(current_unit, "weapons", json_object());
		} else {
			weapons = json_object_get(current_unit, "weapons");
			json_object_foreach(weapons, weaponname, currentweapon) {
				/* get weapons here  */
				json_object_set(currentweapon, "backattackAnimation", json_string("s_sandbags_back_idle"));
				json_object_set(currentweapon, "frontattackAnimation", json_string("s_sandbags_front_idle"));
				json_object_set(currentweapon, "damageAnimationDelay", json_integer(0));
			}
		}
	}

	json_dump_file(units, "./BattleUnits_formated.json", JSON_INDENT(2) | JSON_SORT_KEYS);
	json_dump_file(units, "output/BattleUnits.json", JSON_COMPACT);

	printf("Spawnables:%d\n", spawnable);

}
