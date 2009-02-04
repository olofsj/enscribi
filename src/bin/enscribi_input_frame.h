#ifndef _ENSCRIBI_CANVAS_H_
#define _ENSCRIBI_CANVAS_H_ 
   
#include <Evas.h>
#include "enscribi_recognizer.h" 

Evas_Object *enscribi_input_frame_add(Evas *evas, Evas_Object *parent);
void enscribi_input_frame_recognizer_set(Evas_Object *obj, Enscribi_Recognizer *recognizer);

#endif

