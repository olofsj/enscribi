#ifndef _EKANJI_CANVAS_H_
#define _EKANJI_CANVAS_H_ 
   
#include <Evas.h>

typedef struct _Match   Match;

struct _Match
{
    const char *str;
    float score;
};

Evas_Object *ekanji_canvas_add(Evas *evas);
Eina_List *ekanji_canvas_matches_get(Evas_Object *obj);

#endif

