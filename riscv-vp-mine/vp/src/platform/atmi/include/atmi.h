/*
  Copyright (C) 2006, 2009  INRIA

  This file is part of ATMI

  ATMI is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

  Author: Pierre Michaud <pmichaud@irisa.fr>
*/




#ifndef ATMI_H
#define ATMI_H

#include <gsl/gsl_spline.h>




/* ATMI parameters in SI units

If di is the interface thickness and ki the interface material thermal conductivity,
the equivalent thermal conductance is h1 = ki/di

If the heat-sink thermal resistance is R (in K/W), the effective heat
transfer coefficient is h2 = 1/(R*L^2) where L^2 is the base-plate area
*/
typedef struct {
  double z1; /* layer 1 thickness (m) */
  double d;  /* layer 2 thickness (m) */
  double k1; /* layer 1 thermal conductivity (W/mK) */
  double a1; /* layer 1 thermal diffusivity (m^2/s) */
  double k2; /* layer 2 thermal conductivity (W/mK) */
  double a2; /* layer 2 thermal diffusivity (m^2/s) */
  double h1; /* layer 1/layer 2 thermal conductance (W/m^2K) */
  double h2; /* layer 2/ambient thermal conductance (W/m^2K) */
  double L;  /* width (m) */
  double MAXR; /* not an ATMI parameter, for internal use */
} atmi_param;


void atmi_set_param(atmi_param *p,
		    double val_z1,
		    double val_d,
		    double val_k1,
		    double val_a1,
		    double val_k2,
		    double val_a2,
		    double val_h1,
		    double val_h2,
		    double val_L);

void atmi_fill_param(atmi_param *p, 
		     double celsiuszone, /* celsius */
		     double heatsink_resistance, /* K/W */
		     double heatsink_width, /* m */
		     double copper_thickness, /* m */
		     double bulk_silicon_thickness, /* m */
		     double interface_thickness, /* m */
		     double interface_thermal_cond /* W/mK */);

void atmi_print_param(atmi_param *p);


/* POINT source: temperature at distance r in function of time (response to step)
   Assumes infinite L 
 */
double atmi_point(atmi_param *p, 
		  double power, /* source power (W) */
		  double r, /* distance from the source (m) */
		  double t, /* time (s), used if steady=0 */
		  char steady);


/* One-dimensional heat conduction ( = disk of infinite radius)
   Assumes infinite L 
 */
double atmi_1D(atmi_param *p, 
               double q, /* power density (W/m^2) */
               double t, /* time (s), used if steady=0 */
               double steady);


/* Contribution from insulated walls at x=+-L/2 and y=+-L/2 (method of images)
   That's where parameter L is taken into account
*/
double atmi_images(atmi_param *p, 
		   double power, /* power (W) */
		   double t, /* time (s), used if steady=0 */
		   char steady);


/* DISK source: temperature at disk center in function of time (response to step)
   Assumes infinite L 
 */
double atmi_disk_center(atmi_param *p, 
			double q, /* power density (W/m^2) */
			double r, /* disk radius (m) */
			double t, /* time (s) */
			char steady);


/* RIGHT TRIANGLE BCA (right angle C): temperature at vertex B
   Assumes infinite L 
 */
double atmi_rtri(atmi_param *p, 
		 double q, /* power density (W/m^2) */
		 double a, /* side BC (m) */
		 double b, /* side CA (m) */
		 double t, /* time (s) */
		 char steady);


/* SQUARE source: temperature at square center
   Assumes infinite L 
 */
double atmi_square_center(atmi_param *p, 
			  double q, /* power density (W/m^2) */
			  double a, /* square side (m) */
			  double t, /* time (s) */
			  char steady);


/* RECTANGLE source: temperature at rectangle center
   Assumes infinite L 
 */
double atmi_rect_center(atmi_param *p, 
			double q, /* power density (W/m^2) */
			double a, /* rectangle width (m) */
			double b, /* rectangle height (m) */
			double t, /* time (s) */
			char steady);


/* RECTANGLE source: temperature at point (x,y)
   (point (0,0) is rectangle center)
   Assumes infinite L 
*/
double atmi_rectangle(atmi_param *p, 
		      double q, /* power density (W/m^2)*/
		      double a, /* rectangle width (m) */
		      double b, /* rectangle height (m) */
		      double x, 
		      double y, 
		      double t, /* time (s), used if steady=0 */
		      char steady);




/****************************************************************
           Steady-state temperature 
*****************************************************************/


#define ATMI_GRIDMAX 1024


typedef double atmi_grid [ATMI_GRIDMAX][ATMI_GRIDMAX];


/* Rectangle coordinates: (x1,y1) and (x2,y2) are diagonal corners
 */
typedef struct {
  double x1; 
  double y1;
  double x2;
  double y2;
} atmi_rect;


/* RECTANGLE sources: steady-state temperature at the center of each rectangle 
 */
void atmi_steady_rect(atmi_param *p, 
		      int nrect, /* number of rectangles */
		      atmi_rect rc[], /* rectangles coordinates */
		      double q[], /* power density in each rectangle */
		      double temperature[]);


/* Steady-state temperature in a rectangle subdivided into nx*ny square blocks
   (nx<=ATMI_GRIDMAX, ny<=ATMI_GRIDMAX)
*/
void atmi_steady_grid(atmi_param *p, 
		      int nx, /* number of blocks in the x direction */
		      int ny, /* number of blocks in the y direction */
		      double gridunit, /* block length (meters) */
		      atmi_grid q, /* power density */
		      atmi_grid temperature);

void atmi_grid_print(FILE *fd, atmi_grid g, int nx, int ny);

void atmi_grid_set(atmi_grid g,
		   double gridunit,
		   int *nx,
		   int *ny,
		   int nrect,
		   atmi_rect rc[],
		   double val[]);


/************************************************************************
  Transient temperature for a set of rectangle sources with
  time-varying power densities
*************************************************************************/



/* If you do not want event compression to distort the temperature curves, 
   You may undefine ATMI_COMPRESS_EVENTS, but convolutions will be much slower...
   Another solution is to take ATMI_COMPRESS_ACCURACY smaller
*/
#define ATMI_COMPRESS_EVENTS
#define ATMI_COMPRESS_ACCURACY 0.001


/* Warm-up time (seconds) */
#define ATMI_WARMUP_TIME 1000.

/* Maximum time (seconds) */
#define ATMI_MAX_TIME 10000.

#define ATMI_MAX_SAMP 200

/* Maximum number of rectangles*/
#define ATMI_MAX_RECT 50

/* Maximum number of sensors */
#define ATMI_MAX_SENSORS ATMI_MAX_RECT


#define ATMI_MEMSIZE 100000000



enum atmi_sources {unknown,point,disk,square,rectangle,rectangle0};


typedef struct {
  double val;
  long long m;
} atmi_event;


typedef struct {
  atmi_event *e;
  int nevents;
  atmi_event *last_e;
  int max_nevents;
} atmi_eventlist;


typedef struct {
  double w[ATMI_MAX_SAMP];
  double wsteady;
  int nmax;
  double t_propag;
  int MEMOSIZE;
  char *alreadycomp;
  double *wmem;
  gsl_interp_accel *acc;
  gsl_spline *spline;
} atmi_influence;


typedef struct {
  atmi_rect COORD;
  double XLEN;
  double YLEN;
  double AREA;
  double XPOS;
  double YPOS;
  atmi_eventlist pq;
  atmi_eventlist spq;
  int BIGSTEP;
  atmi_influence *autotempref;
  atmi_influence *tempref;
} atmi_source;


typedef struct {
  atmi_param PARAM;
  double TIMESTEP;
  int NRECT;
  int NSENS;
  int MEMOSIZE;
  int MAXEVENTS;
  atmi_source *rect;
  atmi_influence imatrix[ATMI_MAX_RECT][ATMI_MAX_SENSORS];
  atmi_influence ideltaw;
  double temperature[ATMI_MAX_SENSORS]; /* relative to ambient */
  int sensor[ATMI_MAX_SENSORS];
  long long m; 
  double t;  /* time in seconds */
} atmi_simulator;


typedef struct {
  int n;
  int s[ATMI_MAX_RECT];
} atmi_rectset;


/* A "sensor" is one of the rectangles, specified by its number.
   The temperature indicated by that sensor is the temperature at the 
   center of the rectangle
*/

void atmi_simulator_init(atmi_simulator *ts, 
			 const char *filename,
			 atmi_param *p,
			 double timestep,
			 int nrect, /* number of rectangle sources */
			 atmi_rect rc[], /* rectangles coordinates */
			 atmi_rectset *sensors,
			 double q[]); /* power density applied for ATMI_WARMUP_TIME */

void atmi_simulator_freemem(atmi_simulator *ts);

void atmi_simulator_step(atmi_simulator *ts, 
			 double q[] /* power density */);

void atmi_rectset_init(atmi_rectset *rs);

void atmi_rectset_add(atmi_rectset *rs, int rectnum);

void atmi_steady_throttle(atmi_param *param,
			  int nrect,
			  atmi_rect rc[],
			  atmi_rectset *sensors,
			  atmi_rectset domain[],
			  double wmax,
			  double q[]);

void atmi_print_response1(atmi_simulator *ts, atmi_influence *p);
void atmi_print_response2(atmi_simulator *ts, atmi_influence *p);


#endif
