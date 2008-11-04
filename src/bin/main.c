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
    w = 200;
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
    o = ekanji_input_frame_add(evas);
    evas_object_move(o, 0, 0);
    evas_object_resize(o, w, h);
    evas_object_show(o);

    /* show the window */
    ecore_evas_show(ee);

    /* start the main event loop */
    ecore_main_loop_begin();

    /* when the main event loop exits, shutdown our libraries */
    ecore_evas_shutdown();
    ecore_shutdown();
    evas_shutdown();
}
