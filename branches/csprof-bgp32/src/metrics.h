// -*-Mode: C++;-*- // technically C99
// $Id: metrics.h 1269 2008-04-29 20:16:06Z eraxxon $

#ifndef CSPROF_METRICS_H
#define CSPROF_METRICS_H

#include <sys/types.h>

#include "hpcfile_csprof.h"

/* flags for metrics */

// tallent: I have moved flags into hpcfile_csprof.h.  The flags don't
// really belong there but:
// 1) metrics.c uses hpcfile_csprof_data_t to implement metrics
//    info, which already confuses boundaries
// 2) metric info needs to exist in a library so csprof (hpcrun),
//    xcsprof (hpcprof) and hpcfile can use it.  hpcfile at least
//    satisfies this.

/* prototypes */

/* Does what you expect. */
int csprof_get_max_metrics();

/* Set the maximum number of metrics that can be tracked by the library
   simultaneously.  This function can only be called for effect once. */
int csprof_set_max_metrics(int max_metrics);

/* Return the number of metrics being recorded. */
int csprof_num_recorded_metrics();

/* Request a new metric id.  Returns the new metric id on success, -1
   on failure (that is, you are already tracking the maximum number of
   metrics possible). */
int csprof_new_metric();

/* Associate `name' with `metric_id' for later processing.  The sample
   period defaults to `1' for this metric. */
void csprof_set_metric_info(int metric_id, char *name, uint64_t flags);

/* A more detailed version of `csprof_set_metric_info'. */
void csprof_set_metric_info_and_period(int metric_id, char *name,
				       uint64_t flags, size_t period);

/* Record `value' for `metric_id'. */
void csprof_record_metric(int metric_id, size_t value);

/* Useful only for csprof itself. */
hpcfile_csprof_data_t *csprof_get_metric_data();

#endif