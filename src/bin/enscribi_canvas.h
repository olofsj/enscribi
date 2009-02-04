#ifndef _EKANJI_CANVAS_H_
#define _EKANJI_CANVAS_H_ 
   
#include <Evas.h>
#include "ekanji_recognizer.h" 

Evas_Object *ekanji_canvas_add(Evas *evas);
Eina_List *ekanji_canvas_matches_get(Evas_Object *obj);
void ekanji_canvas_recognizer_set(Evas_Object *obj, Ekanji_Recognizer *recognizer);

#endif

