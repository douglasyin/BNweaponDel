#include <stdio.h>
#include <jansson.h>

int isWhite( const char *name, char *bl[], int num )
{
  int i=0;

  for( i=0; i<num; i++ ){

    if( !strcmp( name, bl[i] ) ){
      printf("%s\n",name);
      return(1);
	}
  }
  return(0);
}

main ()
{
  json_t *units, *current_unit, *value, *side, *weapons, *v, *currentweapon;
  json_error_t error;
  size_t size;
  const char *unitname, *keyname, *weaponname;
  int progress, laststep;
  char *wlist[]={"veh_tank_mega"};                      // 4

  units = json_load_file("./BattleUnits.json", 0, &error);
  if (!units) {
      /* the error variable contains error information */
    printf("load error \n");
    exit (1);
  }

  size = json_object_size (units);
  printf("Proceeding %d units...\n", size);

  progress = 0;
  laststep = 0;

  json_object_foreach (units, unitname, current_unit) {
	
	progress ++;
	if( (progress-laststep) >= size/10 ){
//		printf("ooo");
		laststep = progress;
	}

    side = json_object_get(current_unit, "side");

    if ( !isWhite( unitname, wlist, 1 )) { 
      weapons = json_object_get(current_unit, "weapons");
      json_object_clear( weapons );           /* clear it */

      //json_object_set (current_unit, "weapons", json_object ());
    } else {
      weapons = json_object_get(current_unit, "weapons");
      json_object_foreach(weapons, weaponname, currentweapon) {
	         /* get weapons here  */
        json_object_set(currentweapon, "backattackAnimation", json_string ("s_sandbags_back_idle"));
        json_object_set(currentweapon, "damageAnimationDelay", json_integer (0));
      }
    }
  }

  json_dump_file(units, "./BattleUnits_formated.json", JSON_INDENT (2)|JSON_SORT_KEYS);
  json_dump_file(units, "output/BU.json", JSON_COMPACT);

  printf("\n");

}
