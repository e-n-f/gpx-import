/* gpx-import/src/main.c
 *
 * GPX file importer
 *
 * Copyright Daniel Silverstone <dsilvers@digital-scurf.org>
 *
 * Written for the OpenStreetMap project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; version 2 of the License
 * only.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <inttypes.h>
#include <errno.h>
#include <math.h>

#include "gpx.h"
#include "db.h"
#include "image.h"
#include "filename.h"
#include "interpolate.h"
#include "cache.h"

int
main(int argc, char **argv)
{
  if (argc < 2) {
    fprintf(stderr, "Usage: %s file.gpx ...\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  int i;
  for (i = 1; i < argc; i++) {
    INFO("GPX file %s", argv[i]);

    GPX *g;
    char *error = NULL;
    g = gpx_parse_file(argv[i], &error);
        
    if (g != NULL && error == NULL && g->goodpoints > 0) {
      INFO("GPX contained %d good point(s) and %d bad point(s)", g->goodpoints, g->badpoints);
      if (g->badpoints > 0) {
        INFO("%d missed <time>, %d had bad latitude, %d had bad longitude",
             g->missed_time, g->bad_lat, g->bad_long);
      }

      GPXTrackPoint *pt = g->points;
      GPXTrackPoint *prev = NULL;
      while (pt != NULL) {
        if (prev != NULL && pt->segment != prev->segment) {
          prev = NULL;
        }

        if (prev != NULL) {
          double lat1 = prev->latitude / 1000000000.0;
          double lon1 = prev->longitude / 1000000000.0;
          double lat2 = pt->latitude / 1000000000.0;
          double lon2 = pt->longitude / 1000000000.0;
          double rat = cos((lat1 + lat2) / 2 * M_PI / 180);

          double angle = atan2(lat2 - lat1, (lon2 - lon1) * rat);
          if (angle < 0) {
            angle += 2 * M_PI;
          }

          printf("%.7f,%.7f %.7f,%.7f 8:%d\n",
                  lat1, lon1, lat2, lon2,
                  (int) (angle * 256 / (2 * M_PI)));
        }

        prev = pt;
        pt = pt->next;
      }

      gpx_free(g);
    } else {
      if (error == NULL) {
        if (g->badpoints > 0) {
          INFO("%d missed <time>, %d had bad latitude, %d had bad longitude",
               g->missed_time, g->bad_lat, g->bad_long);
        }
        if (g->goodpoints > 0) {
          ERROR("Failure while parsing GPX");
        } else {
          ERROR("No good points found");
        }
      }
    }
  }
}
