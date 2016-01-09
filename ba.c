#include <stdio.h>
#include <jansson.h>

int main ()
{
	json_t *units, *current_unit, *stats, *targetArea;
	json_error_t error;
	const char *unitname;

	units = json_load_file ("./BattleAbilities.json", 0, &error);
	if (!units) {
		/* the error variable contains error information */
		printf ("load error \n");
		exit (1);
	}
	printf ("proceeding %d ablilities...\n", (int)json_object_size (units));

	json_object_foreach (units, unitname, current_unit) {

		json_object_del (current_unit, "damageAnimationType");	/* remove damageAnimationType  */

		stats = json_object_get (current_unit, "stats");
		if (stats) {
			targetArea = json_object_get (stats, "targetArea");
			if (targetArea)
				json_object_set(targetArea, "aoeOrderDelay", json_real (0.0));
		}else
			printf ("get targetArea error\n");

	}

	json_dump_file (units, "./BattleAbilities_formated.json", JSON_INDENT (2)|JSON_SORT_KEYS);
	json_dump_file (units, "output/BattleAbilities.json", JSON_COMPACT);

}
