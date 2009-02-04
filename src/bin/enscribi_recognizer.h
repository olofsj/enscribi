#ifndef _ENSCRIBI_RECOGNIZER_H_
#define _ENSCRIBI_RECOGNIZER_H_ 
   
#include <Eina.h>

typedef struct _Enscribi_Recognizer Enscribi_Recognizer;
typedef struct _Stroke  Stroke;
typedef struct _Point   Point;
typedef struct _Match   Match;

struct _Enscribi_Recognizer
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

Enscribi_Recognizer * enscribi_recognizer_new();
void enscribi_recognizer_del(Enscribi_Recognizer *self);
void enscribi_recognizer_resize(Enscribi_Recognizer *self, int w, int h);
void enscribi_recognizer_lookup(Enscribi_Recognizer *self, Eina_List *strokes);
Eina_List *enscribi_recognizer_matches_get(Enscribi_Recognizer *self);

#endif

