#include <stdlib.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include "ekanji_canvas.h" 

static void _ekanji_cb_matches(void *data, Evas_Object *obj, const char *emission, const char *source);

static Evas_Object *_results = NULL;

static void
_ekanji_cb_matches(void *data, Evas_Object *obj, const char *emission, const char *source)
{
    Match *match;
    Eina_List *matches, *l;
    int i;

    matches = ekanji_canvas_matches_get(data);
    i = 0;
    for (l = matches; l; l = l->next) {
        match = l->data;
        printf("%s\t%f\n", match->str, match->score);
        if (i < 8) {
            char part[8];
            sprintf(part, "result/%d", i);
            edje_object_part_text_set(obj, part, match->str);
            if (i == 0) 
                edje_object_part_text_set(obj, "result", match->str);
        }
        i++;
    }
}

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


    /* Load and set up the edje objects and canvas */
    edje = edje_object_add(evas);
    edje_object_file_set(edje, "../../data/themes/ekanji.edj", "ekanji/input");
    evas_object_move(edje, 0, 0);
    evas_object_resize(edje, w, h);
    evas_object_show(edje);
    canvas = ekanji_canvas_add(evas);
    edje_object_part_swallow(edje, "canvas", canvas);
    edje_object_signal_callback_add(edje, "canvas,results,updated", "canvas", _ekanji_cb_matches, canvas);

    /* show the window */
    ecore_evas_show(ee);

    /* start the main event loop */
    ecore_main_loop_begin();

    /* when the main event loop exits, shutdown our libraries */
    ecore_evas_shutdown();
    ecore_shutdown();
    evas_shutdown();
}
