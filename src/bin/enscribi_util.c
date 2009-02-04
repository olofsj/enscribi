#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <sys/stat.h>
#include "ekanji_util.h"
#include "config.h"

char *
ekanji_theme_find(const char *name)
{
  static char eet[PATH_MAX + 1];
  struct stat st;

  // For development..
  snprintf(eet, sizeof(eet),
           "../../data/themes/%s.edj", name);
           
  if (!stat(eet, &st))
    return eet;

  snprintf(eet, sizeof(eet),
           "%s/.e/apps/"PACKAGE"/""themes/%s.edj",
           getenv("HOME"), name);
           
  if (!stat(eet, &st))
    return eet;

  snprintf(eet, sizeof(eet), PACKAGE_DATA_DIR"/themes/%s.edj", name);

  if (!stat(eet, &st))
    return eet;

  return NULL;
}

