#include <Eina.h>
#include <zinnia.h>
#include "ekanji_recognizer.h" 

void
ekanji_recognizer_lookup(Ekanji_Recognizer *self, Eina_List *strokes)
{
    int i, sc;
    Eina_List *s, *p, *matches;
    Stroke *stroke;
    Point *point;
    Match *match;
    zinnia_character_t *character;
    zinnia_result_t *result;

    if (!self) return;

    printf("Update recognition\n");
    while (self->matches) {
        free(self->matches->data);
        self->matches = eina_list_remove_list(self->matches, self->matches);
    }

    /* convert internal stroke data to zinnia format */
    character = zinnia_character_new();
    zinnia_character_clear(character);
    zinnia_character_set_width(character, self->w);
    zinnia_character_set_height(character, self->h);
    sc = -1;
    for (s = strokes; s; s = s->next) {
        sc++;
        stroke = s->data;
        for (p = stroke->points; p; p = p->next) {
            point = p->data;
            zinnia_character_add(character, sc, point->x, point->y);
        }
    }

    /* classify stroke data and update matches */
    result = zinnia_recognizer_classify(self->recognizer, character, 10);
    if (result == NULL) {
        fprintf(stderr, "%s\n", zinnia_recognizer_strerror(self->recognizer));
        return;
    }

    for (i = 0; i < zinnia_result_size(result); ++i) {
        match = malloc(sizeof(Match));
        match->str = zinnia_result_value(result, i);
        match->score = zinnia_result_score(result, i);
        self->matches = eina_list_append(self->matches, match);
    }
    
    zinnia_result_destroy(result);
    zinnia_character_destroy(character);
}

Eina_List *
ekanji_recognizer_matches_get(Ekanji_Recognizer *self)
{
    if (!self) return NULL;
    return self->matches;
}

void
ekanji_recognizer_resize(Ekanji_Recognizer *self, int w, int h)
{
    if (!self) return;
    self->w = w;
    self->h = h;
}

Ekanji_Recognizer *
ekanji_recognizer_new()
{
    Ekanji_Recognizer *self;

    self = calloc(1, sizeof(Ekanji_Recognizer));
    if (!self) return;

    self->w = 100;
    self->h = 100;

    self->recognizer = zinnia_recognizer_new();
    if (!zinnia_recognizer_open(self->recognizer, "/usr/lib/zinnia/model/tomoe/handwriting-ja.model")) {
        fprintf(stderr, "ERROR: %s\n", zinnia_recognizer_strerror(self->recognizer));
    }

    return self;
}

void 
ekanji_recognizer_del(Ekanji_Recognizer *self)
{
    if (!self) return;

    zinnia_recognizer_destroy(self->recognizer);
    free(self);
}
