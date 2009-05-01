#include <unistd.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <stdarg.h>
//#include <string.h>
//#include <sys/types.h>
//#include <sys/stat.h>
//#include <dirent.h>
//#include <utime.h>
//#include <errno.h>
//#include <limits.h>

#include <Eina.h>
#include <Ecore.h>
#include <zinnia.h>
#include "enscribi_recognizer_exe.h" 

#define READBUFSIZE 65536

static void *recognizer;
Ecore_Fd_Handler *_stdin_handler = NULL;

/*
 * Handles data from STDIN.
 *
 */
static int
_stdin_data(void *data, Ecore_Fd_Handler * fd_handler)
{
    int fd;
    ssize_t num = 0;
    void *buf = NULL;
    void *sdata = NULL;
    int i, s, p, pc;
    zinnia_character_t *character;
    zinnia_result_t *result;

    // Read data from STDIN
    fd = ecore_main_fd_handler_fd_get(fd_handler);
    buf = malloc(READBUFSIZE);
    num = read(fd, buf, READBUFSIZE);

    // convert internal stroke data to zinnia format
    Enscribi_Recognition_Input_Data *rec_data = buf;
    character = zinnia_character_new();
    zinnia_character_clear(character);
    zinnia_character_set_width(character, rec_data->w);
    zinnia_character_set_height(character, rec_data->h);

    for (s = 0; s < rec_data->nrof_strokes; s++) {
        Enscribi_Recognition_Stroke_Data *stroke_data;
        stroke_data = buf + rec_data->offset[s];
        for (p = 0; p < stroke_data->nrof_points; p++) {
            zinnia_character_add(character, s, stroke_data->coord[2*p], stroke_data->coord[2*p+1]);
        }
    }

    // classify stroke data and update matches
    result = zinnia_recognizer_classify(recognizer, character, 10);
    if (result == NULL) {
        fprintf(stderr, "%s\n", zinnia_recognizer_strerror(recognizer));
        return;
    }

    // Build return data
    //int ssize = sizeof(Enscribi_Recognition_Output_Data) + 10*(2*sizeof(int) + 5*sizeof(char));
    int ssize = sizeof(Enscribi_Recognition_Output_Data);
    Enscribi_Recognition_Output_Data *out_data = malloc(ssize);
    out_data->data = rec_data->data;
    out_data->nrof_matches = 10;
    for (i = 0; i < 10; i++) {
        out_data->score[i] = zinnia_result_score(result, i);
        char *str = zinnia_result_value(result, i);
        for (s = 0; s < 5; s++) {
            if (s < strlen(str))
                out_data->match[5*i+s] = str[s];
            else
                out_data->match[5*i+s] = '\0';
        }
    }

    // Return data
    write(STDOUT_FILENO, out_data, ssize);
    free(buf);
    zinnia_result_destroy(result);
    zinnia_character_destroy(character);

    return 1;
}


int
enscribi_recognizer_init()
{
    char * file;

    recognizer = zinnia_recognizer_new();
    if (zinnia_recognizer_open(recognizer, "/usr/lib/zinnia/model/tomoe/handwriting.model"))
        return 0;
    if (zinnia_recognizer_open(recognizer, "/usr/lib/zinnia/model/tomoe/handwriting-ja.model"))
        return 0;
    if (zinnia_recognizer_open(recognizer, "/usr/lib/zinnia/model/tomoe/handwriting-zh_CN.model"))
        return 0;

    fprintf(stderr, "ERROR: %s\n", zinnia_recognizer_strerror(recognizer));
    return 1;
}

int
main(int argc, char *argv[])
{

    ecore_init();

    enscribi_recognizer_init();

    _stdin_handler =
        ecore_main_fd_handler_add(STDIN_FILENO, ECORE_FD_READ, _stdin_data, NULL,
                NULL, NULL);

    ecore_main_loop_begin();
    ecore_shutdown();

    return 0;
}
