#include <stdlib.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Edje.h>
#include "ekanji_canvas.h" 

static void _ekanji_cb_matches(void *data, Evas_Object *obj, const char *emission, const char *source);
static void _ekanji_cb_finished(void *data, Evas_Object *obj, const char *emission, const char *source);

static void
_ekanji_cb_matches(void *data, Evas_Object *obj, const char *emission, const char *source)
{
    Match *match;
    Eina_List *matches, *l;
    int i;
    Edje_Message_String_Set *msg;
    
    matches = ekanji_canvas_matches_get(data);
        
    msg = calloc(1, sizeof(Edje_Message_String_Set) - sizeof(char *) + (9 * sizeof(char *)));
    msg->count = 9;
    for (i = 0; i < 8; i++) {
        l = eina_list_nth_list(matches, i);
        match = l->data;
        msg->str[i] = match->str;
        printf("%s\t\n", msg->str[i]);
    }
    msg->str[8] = ""; // Why do we have to set a 9th element to not get scrap in 8th?
    edje_object_message_send(obj, EDJE_MESSAGE_STRING_SET, 1, msg);
    free(msg);
}

static void
_ekanji_cb_finished(void *data, Evas_Object *obj, const char *emission, const char *source)
{
    const char *str;

    str = edje_object_part_text_get(obj, "result");
    printf("Result: %s\n", str);
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
    edje_object_signal_callback_add(edje, "canvas,matches,updated", "canvas", _ekanji_cb_matches, canvas);
    edje_object_signal_callback_add(edje, "result,finished", "result", _ekanji_cb_finished, canvas);

    /* show the window */
    ecore_evas_show(ee);

    /* start the main event loop */
    ecore_main_loop_begin();

    /* when the main event loop exits, shutdown our libraries */
    ecore_evas_shutdown();
    ecore_shutdown();
    evas_shutdown();
}
