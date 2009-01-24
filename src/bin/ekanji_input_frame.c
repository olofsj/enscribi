#include <Evas.h>
#include <Ecore.h>
#include <Edje.h>
#include "ekanji_canvas.h" 

typedef struct _Smart_Data Smart_Data;

struct _Smart_Data
{ 
    Evas_Coord       x, y, w, h;
    Evas_Object     *parent;
    Evas_Object     *obj;
    Evas_Object     *clip;
    Evas_Object     *edje;
    Evas_Object     *canvas;
}; 

/* local subsystem functions */
static void _smart_init(void);
static void _smart_add(Evas_Object *obj);
static void _smart_del(Evas_Object *obj);
static void _smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y);
static void _smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h);
static void _smart_show(Evas_Object *obj);
static void _smart_hide(Evas_Object *obj);
static void _smart_color_set(Evas_Object *obj, int r, int g, int b, int a);
static void _smart_clip_set(Evas_Object *obj, Evas_Object *clip);
static void _smart_clip_unset(Evas_Object *obj);
static void _smart_parent_set(Evas_Object *obj, Evas_Object *parent);

static void _ekanji_input_frame_cb_matches(void *data, Evas_Object *obj, 
        const char *emission, const char *source);
static void _ekanji_input_frame_cb_finished(void *data, Evas_Object *obj, 
        const char *emission, const char *source);

/* local subsystem globals */
static Evas_Smart *_e_smart = NULL;

/* externally accessible functions */
Evas_Object *
ekanji_input_frame_add(Evas *evas, Evas_Object *parent)
{
    Evas_Object *obj;

    _smart_init();
    obj = evas_object_smart_add(evas, _e_smart);
    _smart_parent_set(obj, parent);
    return obj;
}

/* callbacks */
static void
_ekanji_input_frame_cb_matches(void *data, Evas_Object *obj, const char *emission, const char *source)
{
    Match *match;
    Eina_List *matches, *l;
    int i;
    Edje_Message_String_Set *msg;
    Smart_Data *sd;
    
    sd = data;
    matches = ekanji_canvas_matches_get(sd->canvas);
        
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
_ekanji_input_frame_cb_finished(void *data, Evas_Object *obj, const char *emission, const char *source)
{
    Smart_Data *sd;
    Edje_Message_String msg;
    char *str;

    sd = data;

    msg.str = edje_object_part_text_get(obj, "result");
    printf("Result: %s\n", msg.str);
    if (sd->parent)
        edje_object_message_send(sd->parent, EDJE_MESSAGE_STRING, 188, &msg);
}

/* private functions */

static void
_smart_init(void)
{
    if (_e_smart) return;
    {
        static const Evas_Smart_Class sc =
        {
            "ekanji_input_frame",
            EVAS_SMART_CLASS_VERSION,
            _smart_add,
            _smart_del,
            _smart_move,
            _smart_resize,
            _smart_show,
            _smart_hide,
            _smart_color_set,
            _smart_clip_set,
            _smart_clip_unset,
            NULL,
            NULL,
            NULL,
            NULL
        };
        _e_smart = evas_smart_class_new(&sc);
    }
}

static void
_smart_add(Evas_Object *obj)
{
    Smart_Data *sd;

    /* Initialize smart data and clipping */
    sd = calloc(1, sizeof(Smart_Data));
    if (!sd) return;
    sd->obj = obj;
    sd->parent = NULL;
    sd->clip = evas_object_rectangle_add(evas_object_evas_get(obj));
    evas_object_smart_member_add(sd->clip, obj);
   
    /* Set up edje object and canvas */
    sd->edje = edje_object_add(evas_object_evas_get(obj));
    edje_object_file_set(sd->edje, "/home/olof/code/ekanji/data/themes/ekanji.edj", "ekanji/input");
    evas_object_move(sd->edje, 0, 0);
    evas_object_show(sd->edje);
    sd->canvas = ekanji_canvas_add(evas_object_evas_get(obj));
    edje_object_part_swallow(sd->edje, "canvas", sd->canvas);

    /* Set up callbacks */
    edje_object_signal_callback_add(sd->edje, "canvas,matches,updated", "canvas",
            _ekanji_input_frame_cb_matches, sd);
    edje_object_signal_callback_add(sd->edje, "result,finished", "result",
            _ekanji_input_frame_cb_finished, sd);

    evas_object_smart_data_set(obj, sd);
}

static void
_smart_del(Evas_Object *obj)
{
    Smart_Data *sd;

    sd = evas_object_smart_data_get(obj);
    if (!sd) return;
    evas_object_del(sd->clip);
    evas_object_del(sd->edje);
    evas_object_del(sd->canvas);
    free(sd);
}

static void
_smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
    Smart_Data *sd;
    Eina_List *l;
    Evas_Coord dx, dy;

    sd = evas_object_smart_data_get(obj);
    if (!sd) return;
    dx = x - sd->x;
    dy = y - sd->y;
    sd->x = x;
    sd->y = y;
    evas_object_move(sd->clip, x, y);
    evas_object_move(sd->edje, x, y);
    evas_object_move(sd->canvas, x, y);
}

static void
_smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
    Smart_Data *sd;

    sd = evas_object_smart_data_get(obj);
    if (!sd) return;
    sd->w = w;
    sd->h = h;
    evas_object_resize(sd->clip, w, h);
    evas_object_resize(sd->edje, w, h);
    evas_object_resize(sd->canvas, w, h);
}

static void
_smart_show(Evas_Object *obj)
{
    Smart_Data *sd;

    sd = evas_object_smart_data_get(obj);
    if (!sd) return;
    evas_object_show(sd->clip);
}

static void
_smart_hide(Evas_Object *obj)
{
    Smart_Data *sd;

    sd = evas_object_smart_data_get(obj);
    if (!sd) return;
    evas_object_hide(sd->clip);
}

static void
_smart_color_set(Evas_Object *obj, int r, int g, int b, int a)
{
    Smart_Data *sd;

    sd = evas_object_smart_data_get(obj);
    if (!sd) return;   
    evas_object_color_set(sd->clip, r, g, b, a);
}

static void
_smart_clip_set(Evas_Object *obj, Evas_Object *clip)
{
    Smart_Data *sd;

    sd = evas_object_smart_data_get(obj);
    if (!sd) return;
    evas_object_clip_set(sd->clip, clip);
}

static void
_smart_clip_unset(Evas_Object *obj)
{
    Smart_Data *sd;

    sd = evas_object_smart_data_get(obj);
    if (!sd) return;
    evas_object_clip_unset(sd->clip);
}  

static void
_smart_parent_set(Evas_Object *obj, Evas_Object *parent)
{
    Smart_Data *sd;

    sd = evas_object_smart_data_get(obj);
    if (!sd) return;
    sd->parent = parent;
}  

