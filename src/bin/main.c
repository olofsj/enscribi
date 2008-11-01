#include <stdlib.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include "ekanji_canvas.h" 

int
main(int argc, char **argv)
{
    Ecore_Evas *ee;
    Evas *evas;
    Evas_Object *bg, *edje;
    Evas_Coord w, h;
    Evas_Object *canvas;
    w = 350;
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

    /* create our white background */
    /*
    bg = evas_object_rectangle_add(evas);
    evas_object_color_set(bg, 255, 255, 255, 255);
    evas_object_move(bg, 0, 0);
    evas_object_resize(bg, 300, 300);
    evas_object_name_set(bg, "background");
    evas_object_show(bg);
    */

    /* load the edje */
    edje = edje_object_add(evas);
    edje_object_file_set(edje, "../../data/themes/ekanji.edj", "main");
    evas_object_move(edje, 0, 0);
    evas_object_resize(edje, w, h);
    evas_object_show(edje);

    /* create the canvas objects */
    canvas = ekanji_canvas_add(evas);
    edje_object_part_swallow(edje, "base.swallow.canvas.1", canvas);
    canvas = ekanji_canvas_add(evas);
    edje_object_part_swallow(edje, "base.swallow.canvas.2", canvas);

    /* show the window */
    ecore_evas_show(ee);

    /* start the main event loop */
    ecore_main_loop_begin();

    /* when the main event loop exits, shutdown our libraries */
    ecore_evas_shutdown();
    ecore_shutdown();
    evas_shutdown();
}
