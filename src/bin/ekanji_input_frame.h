#ifndef _EKANJI_CANVAS_H_
#define _EKANJI_CANVAS_H_ 
   
#include <Evas.h>
#include "ekanji_recognizer.h" 

Evas_Object *ekanji_input_frame_add(Evas *evas, Evas_Object *parent);
void ekanji_input_frame_recognizer_set(Evas_Object *obj, Ekanji_Recognizer *recognizer);

#endif

