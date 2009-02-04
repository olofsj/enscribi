#ifndef _EKANJI_RECOGNIZER_H_
#define _EKANJI_RECOGNIZER_H_ 
   
#include <Eina.h>

typedef struct _Ekanji_Recognizer Ekanji_Recognizer;
typedef struct _Stroke  Stroke;
typedef struct _Point   Point;
typedef struct _Match   Match;

struct _Ekanji_Recognizer
{ 
    int              w, h;
    Eina_List       *matches;
    void            *recognizer;
}; 

struct _Stroke
{
    Eina_List *points;
};

struct _Point
{
    int x;
    int y;
};

struct _Match
{
    const char *str;
    float score;
};

Ekanji_Recognizer * ekanji_recognizer_new();
void ekanji_recognizer_del(Ekanji_Recognizer *self);
void ekanji_recognizer_resize(Ekanji_Recognizer *self, int w, int h);
void ekanji_recognizer_lookup(Ekanji_Recognizer *self, Eina_List *strokes);
Eina_List *ekanji_recognizer_matches_get(Ekanji_Recognizer *self);

#endif

