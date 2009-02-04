#include <Evas.h>
#include <Ecore.h>
#include <Edje.h>
#include <zinnia.h>
#include <math.h>
#include "enscribi_canvas.h" 

typedef struct _Smart_Data Smart_Data;

struct _Smart_Data
{ 
    Evas_Coord       x, y, w, h;
    Evas_Object     *obj;
    Evas_Object     *clip;
    Evas_Object     *img;
    Eina_List       *strokes;
    Ecore_Timer     *hold_timer;
    Enscribi_Recognizer *recognizer;
    Evas_Coord       last_x, last_y; // Last mouse coords for drawing lines. Could this be handled better?
    double           linewidth, lineradius;
}; 

// Gaussian filter coefficients, used for fast antialiasing of drawn lines
static int filter[128] = {
      0,   1,   1,   2,
      3,   3,   4,   5,
      5,   6,   7,   8,
      9,  10,  11,  12,
     13,  14,  15,  16,
     18,  19,  20,  22,
     23,  25,  27,  28,
     30,  32,  34,  36,
     38,  40,  42,  45,
     47,  49,  52,  54,
     57,  59,  62,  65,
     67,  70,  73,  76,
     79,  82,  85,  88,
     91,  95,  98, 101,
    104, 108, 111, 114,
    117, 121, 124, 128,
    131, 134, 138, 141,
    144, 147, 151, 154,
    157, 160, 164, 167,
    170, 173, 176, 179,
    182, 185, 188, 190,
    193, 196, 198, 201,
    203, 206, 208, 210,
    213, 215, 217, 219,
    221, 223, 225, 227,
    228, 230, 232, 233,
    235, 236, 237, 239,
    240, 241, 242, 243,
    244, 245, 246, 247,
    248, 249, 250, 250,
    251, 252, 252, 253,
    254, 254, 255, 255
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

static void _enscribi_canvas_stroke_new(Evas_Object *obj);
static void _enscribi_canvas_stroke_line_add(Evas_Object *obj, Evas_Coord x, Evas_Coord y);
static void _enscribi_canvas_recognition_update(Smart_Data *sd);
static int _enscribi_canvas_cb_hold_timeout(void *data);

/* local subsystem globals */
static Evas_Smart *_e_smart = NULL;

/* externally accessible functions */
Evas_Object *
enscribi_canvas_add(Evas *evas)
{
    _smart_init();
    return evas_object_smart_add(evas, _e_smart);
}

Eina_List *enscribi_canvas_matches_get(Evas_Object *obj)
{
    Smart_Data *sd;

    sd = evas_object_smart_data_get(obj);
    if (!sd) 
        return NULL;

    return enscribi_recognizer_matches_get(sd->recognizer);
}

void 
enscribi_canvas_recognizer_set(Evas_Object *obj, Enscribi_Recognizer *recognizer)
{
    Smart_Data *sd;

    sd = evas_object_smart_data_get(obj);
    if (!sd) 
        return;

    sd->recognizer = recognizer;
}

/*
   Draw anti-aliased line from (x0, y0) to (x1, y1).
   Method implemented from description at 
   http://http.developer.nvidia.com/GPUGems2/gpugems2_chapter22.html
   Calculations are done in 8 bit integers, hence the factor 255 when
   going from float to integer.
*/
static void
_enscribi_canvas_draw_line(Evas_Object *obj, Evas_Coord x0, Evas_Coord y0, Evas_Coord x1, Evas_Coord y1)
{
    Evas_Coord dx, dy, w, h, x, y, min_x, min_y, max_x, max_y;
    unsigned int *data, *p1, *addr;
    int a, r, g, b, aa, i0, i1;
    int k, e0[3], e1[3], e2[3], e3[3];
    int d0, d1, d2, d3, d4, d5, WW, RR;
    double W, R;
    Smart_Data *sd;

    dx = x1 - x0;
    dy = y1 - y0;
    if (dx == 0 && dy == 0)
        return;

    sd = evas_object_smart_data_get(obj);
    data = evas_object_image_data_get(sd->img, 1);
    evas_object_image_size_get(sd->img, &w, &h);

    // Set color. Draw in full white, so color can be set by clipping.
    a = r = g = b = 0xFF;

    // Precalc bounding lines for projection of points
    W = sd->linewidth; WW = 255*W;
    R = sd->lineradius; RR = 255*R;
    k = 255*2/((2*R + W)*sqrt(dx*dx + dy*dy));
    e0[0] = k*(y0-y1); e0[1] = k*(x1-x0); e0[2] = 255 + k*(x0*y1 - x1*y0);
    e1[0] = k*(x1-x0); e1[1] = k*(y1-y0); e1[2] = 255 + k*(x0*x0 + y0*y0 - x0*x1 - y0*y1);
    e2[0] = k*(y1-y0); e2[1] = k*(x0-x1); e2[2] = 255 + k*(x1*y0 - x0*y1);
    e3[0] = k*(x0-x1); e3[1] = k*(y0-y1); e3[2] = 255 + k*(x1*x1 + y1*y1 - x0*x1 - y0*y1);
    
    // Boundary of affected area
    min_x = x1 < x0 ? x1 : x0;
    min_x -= W/2+R;
    if (min_x < 0) min_x = 0;
    max_x = x1 > x0 ? x1 : x0;
    max_x += W/2+R;
    if (max_x > w) max_x = w;
    min_y = y1 < y0 ? y1 : y0;
    min_y -= W/2+R;
    if (min_y < 0) min_y = 0;
    max_y = y1 > y0 ? y1 : y0;
    max_y += W/2+R;
    if (max_y > w) max_y = h;

    // Draw points in affected area
    for (x = min_x; x < max_x; x++) {
        for (y = min_y; y < max_y; y++) {
            // Project points to get distance to bounding lines
            d0 = x * e0[0] + y*e0[1] + e0[2];
            d1 = x * e1[0] + y*e1[1] + e1[2];
            d2 = x * e2[0] + y*e2[1] + e2[2];
            d3 = x * e3[0] + y*e3[1] + e3[2];
            if (d0 > 0 && d1 > 0 && d2 > 0 && d3 > 0) {
                // Get minimum distance to edges and end points
                d5 = d0 < d2 ? d0 : d2;
                d4 = d1 < d3 ? d1 : d3;

                i0 = d4 >= RR ? 127 : 127*d4/RR;
                i1 = d5 >= RR ? 127 : 127*d5/RR;

                // Draw point with filtered intensity
                p1 = data + (y * w) + x;
                aa = filter[i0]*filter[i1]/255;
                if (*p1 >> 24 < aa && aa > 0) 
                    *p1 = ((a*aa)/255 << 24) | ((r*aa)/255 << 16) | ((g*aa)/255 << 8) | (b*aa)/255;
            }
        }
    }

    evas_object_image_data_update_add(sd->img, min_x, min_y, max_x, max_y);
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
    _enscribi_canvas_stroke_new(obj);
    _enscribi_canvas_stroke_line_add(obj, ev->canvas.x, ev->canvas.y);

    printf("Mouse down.\n");
}

static void
_cb_mouse_up(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
    Smart_Data *sd;

    sd = evas_object_smart_data_get(obj);
    sd->hold_timer = ecore_timer_add(1.0, _enscribi_canvas_cb_hold_timeout, sd);

    printf("Mouse up.\n");
}

static void
_cb_mouse_move(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
    Evas_Event_Mouse_Move *ev;

    ev = event_info;
    if (ev->buttons == 1)
        _enscribi_canvas_stroke_line_add(obj, ev->cur.canvas.x, ev->cur.canvas.y);
}

/* private functions */

static void
_enscribi_canvas_stroke_new(Evas_Object *obj)
{
    Stroke *stroke;
    Smart_Data *sd;

    sd = evas_object_smart_data_get(obj);

    stroke = malloc(sizeof(Stroke));
    stroke->points = NULL;
    sd->strokes = eina_list_append(sd->strokes, stroke);
}

static void
_enscribi_canvas_stroke_line_add(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
    Evas_Coord dx, dy, w, h, xx, yy;
    Eina_List *last, *ll;
    Stroke *stroke;
    Point *p, *end_p;
    Smart_Data *sd;

    sd = evas_object_smart_data_get(obj);

    last = eina_list_last(sd->strokes);
    stroke = last->data;
    
    x = x - sd->x;
    y = y - sd->y;


    /* Draw a line and update stroke list */
    if (stroke->points) {
        Point *a, *b;
        int delta = 5;

        last = eina_list_last(stroke->points);
        ll = last->prev;
        b = last->data;
        _enscribi_canvas_draw_line(sd->obj, sd->last_x, sd->last_y, x, y);

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
                }
                else {
                    /* Small change in direction, move last point */
                    b->x = x;
                    b->y = y;
                }
            }
        } else {
            /* Only one point, add point to list of drawn points */
            p = malloc(sizeof(Point));
            p->x = x;
            p->y = y;
            stroke->points = eina_list_append(stroke->points, p);
        }
    }
    else {
        /* No points added, add a first point to list of drawn points */
        p = malloc(sizeof(Point));
        p->x = x;
        p->y = y;
        stroke->points = eina_list_append(stroke->points, p);
    }
    sd->last_x = x;
    sd->last_y = y;
}

static void
_enscribi_canvas_clear(Evas_Object *obj)
{
    Smart_Data *sd;
    Stroke *stroke;
    unsigned int *data, *p1;
    int i, a, r, g, b, w, h;

    sd = evas_object_smart_data_get(obj);

    // Clear image object (set to fully transparent)
    evas_object_image_size_get(sd->img, &w, &h);
    data = evas_object_image_data_get(sd->img, 1);
    a = 0x00;
    r = g = b = 0x00;
    for (i = 0; i < w*h; i++) {
	 p1 = data + i;
	 *p1 = (a << 24) | (r << 16) | (g << 8) | b;
    }

    // Clear stroke list
    while (sd->strokes) {
        stroke = sd->strokes->data;
        if (stroke) {
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
_enscribi_canvas_cb_hold_timeout(void *data)
{
    Smart_Data *sd;

    sd = data;
    _enscribi_canvas_recognition_update(sd);
    return 0;
}

static void
_enscribi_canvas_recognition_update(Smart_Data *sd)
{
    int i, sc;
    Eina_List *s, *p;

    enscribi_recognizer_lookup(sd->recognizer, sd->strokes);

    /* emit signal to edje parent about update */
    Evas_Object *parent;
    parent = evas_object_smart_parent_get(sd->obj);
    if (parent) {
        edje_object_signal_emit(parent, "canvas,matches,updated", "canvas");
    }

    _enscribi_canvas_clear(sd->obj);
}

static void
_smart_init(void)
{
    if (_e_smart) return;
    {
        static const Evas_Smart_Class sc =
        {
            "enscribi_canvas",
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
    sd->linewidth = 2.0;
    sd->lineradius = 1.0;

    sd->img = evas_object_image_filled_add(evas_object_evas_get(obj));
    evas_object_color_set(sd->img, 0, 0, 128, 255);
    evas_object_clip_set(sd->img, sd->clip);
    evas_object_smart_member_add(sd->img, obj);
    evas_object_image_alpha_set(sd->img, 1);
    evas_object_show(sd->img);

    /* Initialize recognition engine */
    sd->recognizer = NULL;

    /* Initialize lists */
    sd->strokes = NULL;

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
    evas_object_del(sd->img);
    enscribi_recognizer_del(sd->recognizer);
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
    evas_object_move(sd->img, x, y);
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
    evas_object_resize(sd->img, w, h);
    evas_object_image_size_set(sd->img, w, h);
    enscribi_recognizer_resize(sd->recognizer, w, h);
    _enscribi_canvas_clear(sd->obj);
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

