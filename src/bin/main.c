#include <stdlib.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Edje.h>
#include "ekanji_canvas.h" 
#include "ekanji_input_frame.h" 

int
main(int argc, char **argv)
{
    Ecore_Evas *ee;
    Evas *evas;
    Evas_Object *bg, *edje, *o, *canvas;
    Evas_Coord w, h;
    w = 400;
    h = 200;

    /* initialize our libraries */
    evas_init();
    ecore_init();
    ecore_evas_init();
    edje_init();

    /* create our Ecore_Evas */
    ee = ecore_evas_software_x11_new(0, 0, 0, 0, w, h);
    ecore_evas_title_set(ee, "EKanji");

    /* get a pointer our new Evas canvas */
    evas = ecore_evas_get(ee);

    /* Load and set up the edje objects */
    edje = edje_object_add(evas);
    edje_object_file_set(edje, "../../data/themes/ekanji.edj", "ekanji/kbd");
    evas_object_move(edje, 0, 0);
    evas_object_resize(edje, w, h);
    evas_object_show(edje);
    o = ekanji_input_frame_add(evas, edje);
    edje_object_part_swallow(edje, "input/1", o);
    o = ekanji_input_frame_add(evas, edje);
    edje_object_part_swallow(edje, "input/2", o);
    o = ekanji_input_frame_add(evas, edje);
    edje_object_part_swallow(edje, "input/3", o);

    /* show the window */
    ecore_evas_show(ee);

    /* start the main event loop */
    ecore_main_loop_begin();

    /* when the main event loop exits, shutdown our libraries */
    ecore_evas_shutdown();
    ecore_shutdown();
    evas_shutdown();
}
