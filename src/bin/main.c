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
    Evas_Object *bg;

    /* initialize our libraries */
    evas_init();
    ecore_init();
    ecore_evas_init();

    /* create our Ecore_Evas and show it */
    ee = ecore_evas_software_x11_new(0, 0, 0, 0, 300, 300);
    ecore_evas_title_set(ee, "EKanji");
    ecore_evas_show(ee);

    /* get a pointer our new Evas canvas */
    evas = ecore_evas_get(ee);

    /* create our white background */
    bg = evas_object_rectangle_add(evas);
    evas_object_color_set(bg, 255, 255, 255, 255);
    evas_object_move(bg, 0, 0);
    evas_object_resize(bg, 300, 300);
    evas_object_name_set(bg, "background");
    evas_object_show(bg);

    /* create the canvas object */
    Evas_Object *canvas;
    canvas = ekanji_canvas_add(evas);
    evas_object_move(canvas, 50, 50);
    evas_object_resize(canvas, 200, 200);
    evas_object_show(canvas);

    /* start the main event loop */
    ecore_main_loop_begin();

    /* when the main event loop exits, shutdown our libraries */
    ecore_evas_shutdown();
    ecore_shutdown();
    evas_shutdown();
}
