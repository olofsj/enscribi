#ifndef _ENSCRIBI_RECOGNIZER_EXE_H_
#define _ENSCRIBI_RECOGNIZER_EXE_H_ 

typedef struct _Enscribi_Recognition_Input_Data Enscribi_Recognition_Input_Data;
typedef struct _Enscribi_Recognition_Stroke_Data Enscribi_Recognition_Stroke_Data;
typedef struct _Enscribi_Recognition_Output_Data Enscribi_Recognition_Output_Data;

struct _Enscribi_Recognition_Input_Data
{ 
    void *data;
    int w, h;
    int nrof_strokes;
    int offset[1];
}; 

struct _Enscribi_Recognition_Stroke_Data
{ 
    int nrof_points;
    int coord[1];
}; 

struct _Enscribi_Recognition_Output_Data
{ 
    void *data;
    int nrof_matches;
    float score[10];
    char match[50];
}; 

#endif
