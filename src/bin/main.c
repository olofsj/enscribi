#include <stdlib.h>
#include <Evas.h>
#include <Ecore.h>
#include <Ecore_Evas.h>
#include <Ecore_X.h>
#include <Edje.h>
#include "ekanji_canvas.h" 
#include "ekanji_input_frame.h" 

static void
_cb_exit(Ecore_Evas *ee)
{
    printf("Ekanji: _cb_exit\n");
    ecore_main_loop_quit();
}

static void
_cb_move(Ecore_Evas *ee)
{
    printf("Ekanji: _cb_move\n");
}

static void
_cb_show(Ecore_Evas *ee)
{
    printf("Ekanji: _cb_show\n");
    ecore_evas_show(ee);
}

static void
_cb_hide(Ecore_Evas *ee)
{
    printf("Ekanji: _cb_hide\n");
    ecore_evas_hide(ee);
}

static void
_cb_resize(Ecore_Evas *ee)
{
    Evas_Object *kbd;
    int w, h;

    /* get the geometry of ee. we don't need the x,y coords so we send NULL. */
    ecore_evas_geometry_get(ee, NULL, NULL, &w, &h);

    /* find our bg object and resize it to the window size (if it exists) */
    kbd = evas_object_name_find(ecore_evas_get(ee), "kbd");
    if (kbd) evas_object_resize(kbd, w, h);
}

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
    ee = ecore_evas_new(NULL, 0, 0, w, h, NULL);
    ecore_evas_title_set(ee, "EKanji");
	ecore_evas_callback_destroy_set(ee, _cb_exit);
    ecore_evas_callback_delete_request_set(ee, _cb_exit);
	ecore_evas_callback_move_set(ee, _cb_move);
    ecore_evas_callback_show_set(ee, _cb_show);
    ecore_evas_callback_hide_set(ee, _cb_hide);
    ecore_evas_callback_resize_set(ee, _cb_resize);
    ecore_event_handler_add(ECORE_EVENT_SIGNAL_EXIT, _cb_exit, ee);

    /* get a pointer our new Evas canvas */
    evas = ecore_evas_get(ee);

    /* Load and set up the edje objects */
    edje = edje_object_add(evas);
    evas_object_name_set(edje, "kbd");
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

    //bg = edje_object_add(evas);
    //edje_object_file_set(bg, "../../data/themes/ekanji.edj", "ekanji/kbd/illume");
    //edje_object_part_swallow(bg, "e.swallow.content", edje);
    //evas_object_move(bg, 0, 0);
    //evas_object_resize(bg, w, h);
    //evas_object_show(bg);

    /* show the window */
    ecore_evas_show(ee);

    /* set window states and properties */
    Ecore_X_Window_State states[2];

    states[0] = ECORE_X_WINDOW_STATE_SKIP_TASKBAR;
    states[1] = ECORE_X_WINDOW_STATE_SKIP_PAGER;
    ecore_x_netwm_window_state_set(ecore_evas_software_x11_window_get(ee), states, 2);
    //ecore_event_handler_add(ECORE_X_EVENT_CLIENT_MESSAGE, client_message_handler, ee);

    ecore_evas_title_set(ee, "Virtual Keyboard");
    ecore_evas_name_class_set(ee, "Ekanji", "Virtual-Keyboard");

    /* tell e that this is a vkbd window */
    ecore_x_e_virtual_keyboard_set(ecore_evas_software_x11_window_get(ee), 1);
    
    /* start the main event loop */
    ecore_main_loop_begin();

    /* when the main event loop exits, shutdown our libraries */
    ecore_evas_shutdown();
    ecore_shutdown();
    evas_shutdown();
}
