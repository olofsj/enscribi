#include <Evas.h>
#include <Ecore.h>
#include <Edje.h>
#include <zinnia.h>
#include <math.h>
#include "ekanji_canvas.h" 

typedef struct _Smart_Data Smart_Data;
typedef struct _Stroke  Stroke;
typedef struct _Point   Point;

struct _Stroke
{
    Eina_List *points;
    Eina_List *lines;
};

struct _Point
{
    Evas_Coord x;
    Evas_Coord y;
};

struct _Smart_Data
{ 
    Evas_Coord       x, y, w, h;
    Evas_Object     *obj;
    Evas_Object     *clip;
    Evas_Object     *bg;
    Eina_List       *strokes;
    Eina_List       *matches;
    Ecore_Timer     *hold_timer;
    zinnia_recognizer_t *recognizer;
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

static void _ekanji_canvas_stroke_new(Evas_Object *obj);
static void _ekanji_canvas_stroke_line_add(Evas_Object *obj, Evas_Coord x, Evas_Coord y);
static void _ekanji_canvas_recognition_update(Smart_Data *sd);
static int _ekanji_canvas_cb_hold_timeout(void *data);

/* local subsystem globals */
static Evas_Smart *_e_smart = NULL;

/* externally accessible functions */
Evas_Object *
ekanji_canvas_add(Evas *evas)
{
    _smart_init();
    return evas_object_smart_add(evas, _e_smart);
}

Eina_List *ekanji_canvas_matches_get(Evas_Object *obj)
{
    Smart_Data *sd;

    sd = evas_object_smart_data_get(obj);
    if (!sd) 
        return NULL;

    return sd->matches;
}

/* callbacks */
static void
_cb_mouse_down(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
    Evas_Event_Mouse_Up *ev;
    Smart_Data *sd;

    sd = evas_object_smart_data_get(obj);
    if (sd->hold_timer) ecore_timer_del(sd->hold_timer);
    sd->hold_timer = NULL;

    ev = event_info;
    _ekanji_canvas_stroke_new(obj);
    _ekanji_canvas_stroke_line_add(obj, ev->canvas.x, ev->canvas.y);

    printf("Mouse down.\n");
}

static void
_cb_mouse_up(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
    Smart_Data *sd;

    sd = evas_object_smart_data_get(obj);
    sd->hold_timer = ecore_timer_add(1.0, _ekanji_canvas_cb_hold_timeout, sd);

    printf("Mouse up.\n");
}

static void
_cb_mouse_move(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
    Evas_Event_Mouse_Move *ev;

    ev = event_info;
    if (ev->buttons == 1)
        _ekanji_canvas_stroke_line_add(obj, ev->cur.canvas.x, ev->cur.canvas.y);
}

/* private functions */

static void
_ekanji_canvas_stroke_new(Evas_Object *obj)
{
    Stroke *stroke;
    Smart_Data *sd;

    sd = evas_object_smart_data_get(obj);

    stroke = malloc(sizeof(Stroke));
    stroke->lines = NULL;
    stroke->points = NULL;
    sd->strokes = eina_list_append(sd->strokes, stroke);
}

static void
_ekanji_canvas_stroke_line_add(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
    Evas_Coord dx, dy;
    Evas_Object *line;
    Eina_List *last, *ll;
    Stroke *stroke;
    Point *p, *end_p;
    Smart_Data *sd;

    sd = evas_object_smart_data_get(obj);

    last = eina_list_last(sd->strokes);
    stroke = last->data;

    /* Draw a line, if this is not the starting point */
    if (stroke->points) {
        Point *a, *b;
        int delta = 5;

        last = eina_list_last(stroke->points);
        ll = last->prev;
        b = last->data;

        if (ll) {
            if (abs(x - b->x) > delta || abs(y - b->y) > delta) {
                /* Some movement, check to see in what direction */
                float old_x, old_y, new_x, new_y;
                float diff = 0.1; // This seems to be ok...

                a = ll->data;

                old_x = (float)(b->x - a->x)/sqrt((b->x - a->x)*(b->x - a->x) + (b->y - a->y)*(b->y - a->y));
                old_y = (float)(b->y - a->y)/sqrt((b->x - a->x)*(b->x - a->x) + (b->y - a->y)*(b->y - a->y));
                new_x = (float)(x - b->x)/sqrt((x - b->x)*(x - b->x) + (y - b->y)*(y - b->y));
                new_y = (float)(y - b->y)/sqrt((x - b->x)*(x - b->x) + (y - b->y)*(y - b->y));

                if (fabs(old_x - new_x) > diff || fabs(old_y - new_y) > diff) {
                    /* Change in direction, add new point */
                    p = malloc(sizeof(Point));
                    p->x = x;
                    p->y = y;
                    stroke->points = eina_list_append(stroke->points, p);

                    /* Draw line from last point */
                    line = evas_object_line_add(evas_object_evas_get(sd->obj));
                    evas_object_smart_member_add(line, sd->obj);
                    evas_object_clip_set(line, sd->clip);
                    evas_object_line_xy_set(line, x, y, b->x, b->y);
                    evas_object_color_set(line, 55, 55, 55, 255);
                    evas_object_show(line);
                    stroke->lines = eina_list_append(stroke->lines, line);
                }
                else {
                    /* Small change in direction, move last point */
                    b->x = x;
                    b->y = y;

                    /* Redraw line */
                    last = eina_list_last(stroke->lines);
                    line = last->data;
                    evas_object_line_xy_set(line, x, y, a->x, a->y);
                }
            }
        } else {
            /* Only one point, add point to list of drawn points */
            p = malloc(sizeof(Point));
            p->x = x;
            p->y = y;
            stroke->points = eina_list_append(stroke->points, p);

            /* Draw line from last point */
            line = evas_object_line_add(evas_object_evas_get(sd->obj));
            evas_object_smart_member_add(line, sd->obj);
            evas_object_clip_set(line, sd->clip);
            evas_object_line_xy_set(line, x, y, b->x, b->y);
            evas_object_color_set(line, 55, 55, 55, 255);
            evas_object_show(line);
            stroke->lines = eina_list_append(stroke->lines, line);
        }
    }
    else {
        /* No points added, add a first point to list of drawn points */
        p = malloc(sizeof(Point));
        p->x = x;
        p->y = y;
        stroke->points = eina_list_append(stroke->points, p);
    }
}

static void
_ekanji_canvas_clear(Evas_Object *obj)
{
    Smart_Data *sd;
    Stroke *stroke;
    Evas_Object *line;

    sd = evas_object_smart_data_get(obj);
    while (sd->strokes) {
        stroke = sd->strokes->data;
        if (stroke) {
            while (stroke->lines) {
                line = stroke->lines->data;
                evas_object_del(line);
                stroke->lines = eina_list_remove_list(stroke->lines, stroke->lines);
            }
            while (stroke->points) {
                free(stroke->points->data);
                stroke->points = eina_list_remove_list(stroke->points, stroke->points);
            }
            free(stroke);
        }
        sd->strokes = eina_list_remove_list(sd->strokes, sd->strokes);
    }
}

static int
_ekanji_canvas_cb_hold_timeout(void *data)
{
    Smart_Data *sd;

    sd = data;
    _ekanji_canvas_recognition_update(sd);
    return 0;
}

static void
_ekanji_canvas_recognition_update(Smart_Data *sd)
{
    int i, sc;
    Eina_List *s, *p;
    Stroke *stroke;
    Point *point;
    Match *match;
    zinnia_character_t *character;
    zinnia_result_t *result;

    printf("Update recognition\n");
    while (sd->matches) {
        free(sd->matches->data);
        sd->matches = eina_list_remove_list(sd->matches, sd->matches);
    }

    /* convert internal stroke data to zinnia format */
    character  = zinnia_character_new();
    zinnia_character_clear(character);
    zinnia_character_set_width(character, sd->w);
    zinnia_character_set_height(character, sd->h);
    sc = -1;
    for (s = sd->strokes; s; s = s->next) {
        sc++;
        stroke = s->data;
        for (p = stroke->points; p; p = p->next) {
            point = p->data;
            zinnia_character_add(character, sc, point->x, point->y);
        }
    }

    /* classify stroke data and update matches */
    result = zinnia_recognizer_classify(sd->recognizer, character, 10);
    if (result == NULL) {
        fprintf(stderr, "%s\n", zinnia_recognizer_strerror(sd->recognizer));
        return;
    }

    for (i = 0; i < zinnia_result_size(result); ++i) {
        match = malloc(sizeof(Match));
        match->str = zinnia_result_value(result, i);
        match->score = zinnia_result_score(result, i);
        sd->matches = eina_list_append(sd->matches, match);
    }
    
    /* emit signal to edje parent about update */
    Evas_Object *parent;
    parent = evas_object_smart_parent_get(sd->obj);
    if (parent) {
        edje_object_signal_emit(parent, "canvas,matches,updated", "canvas");
    }

    zinnia_result_destroy(result);
    zinnia_character_destroy(character);

    /*
    for (s = sd->matches; s; s = s->next) {
        match = s->data;
        printf("%s\t%f\n", match->str, match->score);
    }
    */

    _ekanji_canvas_clear(sd->obj);
}

static void
_smart_init(void)
{
    if (_e_smart) return;
    {
        static const Evas_Smart_Class sc =
        {
            "ekanji_canvas",
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
    sd->hold_timer = NULL;
    sd->clip = evas_object_rectangle_add(evas_object_evas_get(obj));
    evas_object_smart_member_add(sd->clip, obj);

    sd->bg = evas_object_rectangle_add(evas_object_evas_get(obj));
    evas_object_color_set(sd->bg, 240, 240, 240, 255);
    evas_object_clip_set(sd->bg, sd->clip);
    evas_object_smart_member_add(sd->bg, obj);
    evas_object_show(sd->bg);
    
    /* Initialize zinnia recognition engine */
    sd->recognizer = zinnia_recognizer_new();
    if (!zinnia_recognizer_open(sd->recognizer, "/usr/lib/zinnia/model/tomoe/handwriting-ja.model")) {
        fprintf(stderr, "ERROR: %s\n", zinnia_recognizer_strerror(sd->recognizer));
        return;
    }

    /* Initialize lists */
    sd->strokes = NULL;
    sd->matches = NULL;

    /* Set up callbacks */
    evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_DOWN, _cb_mouse_down, NULL);
    evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_UP, _cb_mouse_up, NULL);
    evas_object_event_callback_add(obj, EVAS_CALLBACK_MOUSE_MOVE, _cb_mouse_move, NULL);

    evas_object_smart_data_set(obj, sd);
}

static void
_smart_del(Evas_Object *obj)
{
    Smart_Data *sd;

    sd = evas_object_smart_data_get(obj);
    if (!sd) return;
    evas_object_del(sd->clip);
    evas_object_del(sd->bg);
    zinnia_recognizer_destroy(sd->recognizer);
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
    evas_object_move(sd->bg, x, y);
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
    evas_object_resize(sd->bg, w, h);
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

