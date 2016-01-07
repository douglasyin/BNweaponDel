#include <stdio.h>
#include <jansson.h>

main()
{
    json_t *units, *current_unit, *value, *side, *weapon, *v;
    json_error_t error;
	size_t size;
	const char *unitname, *keyname;
	int b;

    units = json_load_file("./test.json", 0, &error);
    if(!units) {
	        /* the error variable contains error information */
		printf("load error \n");
		exit(1);
    }
	printf(" %d \n", json_object_size(units) );

	b=0;
	json_object_foreach( units, unitname, current_unit ){

		side = json_object_get(current_unit, "side");

		if( ! strcmp( json_string_value(side), "Hostile" ) ){
			json_object_set(current_unit, "weapons", json_object());
		}else{
			/* get weapons here  */
			json_object_set(current_unit, "backattackAnimation", json_string( "s_sandbags_back_idle") );
			json_object_set(current_unit, "damageAnimationDelay", json_integer(0) );
		}

		
		
//		json_dump_file(current_unit, "./haha.json", JSON_INDENT(4) );
		b++;
//		printf( "%d ", b );

	}

	json_dump_file(units, "./ha.json", JSON_INDENT(2) );
	json_dump_file(units, "output/BattleUnits.json", JSON_COMPACT );
}

