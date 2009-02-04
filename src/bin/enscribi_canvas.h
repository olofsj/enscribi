#ifndef _ENSCRIBI_CANVAS_H_
#define _ENSCRIBI_CANVAS_H_ 
   
#include <Evas.h>
#include "enscribi_recognizer.h" 

Evas_Object *enscribi_canvas_add(Evas *evas);
Eina_List *enscribi_canvas_matches_get(Evas_Object *obj);
void enscribi_canvas_recognizer_set(Evas_Object *obj, Enscribi_Recognizer *recognizer);

#endif

