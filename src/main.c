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

#include "gpx.h"
#include "db.h"
#include "image.h"
#include "filename.h"
#include "interpolate.h"
#include "cache.h"

int
main(int argc, char **argv)
{
  DBJob j;
  j.error = NULL;
  DBJob *job = &j;
  GPX *g;
  g = gpx_parse_file(argv[1], &(job->error));
      
  if (g != NULL && job->error == NULL && g->goodpoints > 0) {
    INFO("GPX contained %d good point(s) and %d bad point(s)", g->goodpoints, g->badpoints);
    if (g->badpoints > 0) {
      INFO("%d missed <time>, %d had bad latitude, %d had bad longitude",
           g->missed_time, g->bad_lat, g->bad_long);
    }

    GPXTrackPoint *pt = g->points;
    while (pt != NULL) {
      printf("%.6f,%.6f %d\n",
              pt->latitude / 1000000000.0,
              pt->longitude / 1000000000.0,
              pt->segment);

      pt = pt->next;
    }
  } else {
    if (job->error == NULL) {
      if (g->badpoints > 0) {
        INFO("%d missed <time>, %d had bad latitude, %d had bad longitude",
             g->missed_time, g->bad_lat, g->bad_long);
      }
      if (g->goodpoints > 0) {
        job->error = strdup("XML failure while parsing GPX data");
        ERROR("Failure while parsing GPX");
      } else {
        job->error = strdup("Unable to find any good GPX track points");
        ERROR("No good points found");
      }
    }
  }
}
