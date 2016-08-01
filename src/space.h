/*******************************************************************************
 * This file is part of SWIFT.
 * Copyright (c) 2012 Pedro Gonnet (pedro.gonnet@durham.ac.uk)
 *                    Matthieu Schaller (matthieu.schaller@durham.ac.uk)
 *               2015 Peter W. Draper (p.w.draper@durham.ac.uk)
 *               2016 John A. Regan (john.a.regan@durham.ac.uk)
 *                    Tom Theuns (tom.theuns@durham.ac.uk)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/
#ifndef SWIFT_SPACE_H
#define SWIFT_SPACE_H

/* Config parameters. */
#include "../config.h"

/* Some standard headers. */
#include <stddef.h>

/* Includes. */
#include "cell.h"
#include "lock.h"
#include "parser.h"
#include "part.h"
#include "space.h"

/* Some constants. */
#define space_maxdepth 10
#define space_cellallocchunk 1000
#define space_splitsize_default 400
#define space_maxsize_default 8000000
#define space_subsize_default 8000000
#define space_stretch 1.10f
#define space_maxreldx 0.25f

/* Split size. */
extern int space_splitsize;
extern int space_maxsize;
extern int space_subsize;

/* Map shift vector to sortlist. */
extern const int sortlistID[27];

/* Entry in a list of sorted indices. */
struct entry {
  float d;
  int i;
};

/* The space in which the cells reside. */
struct space {

  /* Spatial extent. */
  double dim[3];

  /* Cell widths. */
  double width[3], iwidth[3];

  /* The minimum and maximum cutoff radii. */
  double h_max, cell_min;

  /* Current maximum displacement for particles. */
  float dx_max;

  /* Number of cells. */
  int nr_cells, tot_cells;

  /* Space dimensions in number of cells. */
  int maxdepth, cdim[3];

  /* The (level 0) cells themselves. */
  struct cell *cells;

  /* Buffer of unused cells. */
  struct cell *cells_new;

  /* The particle data (cells have pointers to this). */
  struct part *parts;
  struct xpart *xparts;
  struct gpart *gparts;

  /* The total number of parts in the space. */
  size_t nr_parts, size_parts;
  size_t nr_gparts, size_gparts;

  /* Is the space periodic? */
  int periodic;

  /* Are we doing gravity? */
  int gravity;

  /* General-purpose lock for this space. */
  swift_lock_type lock;

  /* Number of queues in the system. */
  int nr_queues;

  /* The associated engine. */
  struct engine *e;

  /* Buffers for parts that we will receive from foreign cells. */
  struct part *parts_foreign;
  size_t nr_parts_foreign, size_parts_foreign;
  struct gpart *gparts_foreign;
  size_t nr_gparts_foreign, size_gparts_foreign;
};

/* Interval stack necessary for parallel particle sorting. */
struct qstack {
  volatile ptrdiff_t i, j;
  volatile int min, max;
  volatile int ready;
};
struct parallel_sort {
  struct part *parts;
  struct gpart *gparts;
  struct xpart *xparts;
  int *ind;
  struct qstack *stack;
  unsigned int stack_size;
  volatile unsigned int first, last, waiting;
};
extern struct parallel_sort space_sort_struct;

/* function prototypes. */
void space_parts_sort(struct space *s, int *ind, size_t N, int min, int max,
                      int verbose);
void space_gparts_sort(struct space *s, int *ind, size_t N, int min, int max,
                       int verbose);
struct cell *space_getcell(struct space *s);
int space_getsid(struct space *s, struct cell **ci, struct cell **cj,
                 double *shift);
void space_init(struct space *s, const struct swift_params *params,
                double dim[3], struct part *parts, struct gpart *gparts,
                size_t Npart, size_t Ngpart, int periodic, int gravity,
                int verbose, int dry_run);
void space_map_cells_pre(struct space *s, int full,
                         void (*fun)(struct cell *c, void *data), void *data);
void space_map_parts(struct space *s,
                     void (*fun)(struct part *p, struct cell *c, void *data),
                     void *data);
void space_map_parts_xparts(struct space *s,
                            void (*fun)(struct part *p, struct xpart *xp,
                                        struct cell *c));
void space_map_cells_post(struct space *s, int full,
                          void (*fun)(struct cell *c, void *data), void *data);
void space_rebuild(struct space *s, double h_max, int verbose);
void space_recycle(struct space *s, struct cell *c);
void space_split(struct space *s, struct cell *cells, int verbose);
void space_do_split(struct space *s, struct cell *c);
void space_do_parts_sort();
void space_do_gparts_sort();
void space_link_cleanup(struct space *s);
void space_clean(struct space *s);

#endif /* SWIFT_SPACE_H */
