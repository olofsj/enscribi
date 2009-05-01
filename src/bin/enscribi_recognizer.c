#include <Eina.h>
#include <Ecore.h>
#include "enscribi_recognizer.h" 
#include "enscribi_recognizer_exe.h" 

static int _exe_data_cb(void *data, int type, void *event);

void
enscribi_recognizer_lookup(Enscribi_Recognizer *self, Eina_List *strokes, Evas_Object *caller)
{
    int i, sc;
    Eina_List *s, *p, *matches;
    Stroke *stroke;
    Point *point;
    Match *match;
    void *sdata;

    if (!self) return;

    printf("Update recognition\n");
    while (self->matches) {
        free(self->matches->data);
        self->matches = eina_list_remove_list(self->matches, self->matches);
    }

    // Pack the data for sending it to the recognition process
    // The buffer sdata is used for collecting all the output
    int nrof_strokes = eina_list_count(strokes);
    int nrof_points = 0;
    EINA_LIST_FOREACH(strokes, s, stroke)
        nrof_points += eina_list_count(stroke->points);

    int ssize = sizeof(Enscribi_Recognition_Input_Data) + sizeof(int)*(nrof_strokes-1 + 2*nrof_points-nrof_strokes) + sizeof(Enscribi_Recognition_Stroke_Data)*nrof_strokes;
    sdata = malloc(ssize);

    // Setup input data struct which point to all the strokes. 
    // Offsets for the strokes also have to be added.
    int in_data_size = sizeof(Enscribi_Recognition_Input_Data) + sizeof(int)*(nrof_strokes-1);
    Enscribi_Recognition_Input_Data *in_data = malloc(in_data_size);
    in_data->data = caller;
    in_data->w = self->w;
    in_data->h = self->h;
    in_data->nrof_strokes = nrof_strokes;

    // Add all the strokes in their own structs
    int strokenr = 0;
    int spos = in_data_size;
    EINA_LIST_FOREACH(strokes, s, stroke) {
        nrof_points = eina_list_count(stroke->points);

        int stroke_data_size = sizeof(Enscribi_Recognition_Stroke_Data) + sizeof(int)*(2*nrof_points-1);
        Enscribi_Recognition_Stroke_Data *stroke_data = malloc(stroke_data_size);
        stroke_data->nrof_points = nrof_points;

        int pointnr = 0;
        EINA_LIST_FOREACH(stroke->points, p, point) {
            stroke_data->coord[2*pointnr] = point->x;
            stroke_data->coord[2*pointnr+1] = point->y;
            pointnr++;
        }

        // Add offset to in_data and copy to sdata buffer
        in_data->offset[strokenr++] = spos;
        memcpy(sdata+spos, stroke_data, stroke_data_size);
        spos = spos+stroke_data_size;
        free(stroke_data);
    }

    // Copy the completed in_data to sdata buffer
    memcpy(sdata, in_data, in_data_size);
    free(in_data);

    // Send the packed result
    ecore_exe_send(self->exe, sdata, ssize);
    free(sdata);
}

Eina_List *
enscribi_recognizer_matches_get(Enscribi_Recognizer *self)
{
    if (!self) return NULL;
    return self->matches;
}

void
enscribi_recognizer_resize(Enscribi_Recognizer *self, int w, int h)
{
    if (!self) return;
    self->w = w;
    self->h = h;
}

static int _exe_data_cb(void *data, int type, void *event)
{
    Ecore_Exe_Event_Data *e = event;
    void *sdata;
    int ssize, i;
    Enscribi_Recognizer *self;
    Match *match;

    if (!e) return 1;

    self = ecore_exe_data_get(e->exe);

    // Unpack data and insert in object list
    Enscribi_Recognition_Output_Data *out_data = e->data;
    for (i = 0; i < out_data->nrof_matches; i++) {
        match = malloc(sizeof(Match));
        match->str = strdup(&(out_data->match[5*i]));
        match->score = out_data->score[i];
        self->matches = eina_list_append(self->matches, match);
    }

    evas_object_smart_callback_call(out_data->data, "matches,updated", NULL);

    return 1;
}

Enscribi_Recognizer *
enscribi_recognizer_new()
{
    Enscribi_Recognizer *self;
    char * file;

    self = calloc(1, sizeof(Enscribi_Recognizer));
    if (!self) return;

    // Set up event handler for pipe data, and run slave process with low priority
    ecore_event_handler_add(ECORE_EXE_EVENT_DATA, _exe_data_cb, NULL);
    ecore_exe_run_priority_set(19);
    self->exe = ecore_exe_pipe_run("/home/olof/code/enscribi/src/bin/enscribi_recognizer_exe", ECORE_EXE_PIPE_WRITE | ECORE_EXE_PIPE_READ, self);

    self->w = 100;
    self->h = 100;

    return self;
}

void 
enscribi_recognizer_del(Enscribi_Recognizer *self)
{
    if (!self) return;
	
    ecore_exe_terminate(self->exe);
    
    free(self);
}
