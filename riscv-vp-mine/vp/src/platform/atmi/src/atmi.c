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


#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <gsl/gsl_errno.h>
#include <gsl/gsl_integration.h>
#include <gsl/gsl_spline.h>
#include <gsl/gsl_fft_complex.h>
#include "/home/x/riscv-vp-mine/vp/src/platform/atmi/include/atmi.h"

#ifdef USE_OPENMP
#include <omp.h>
#define MAX_THREADS 4
#define NTHREADS MIN(MAX_THREADS,omp_get_num_procs())
#else
#define NTHREADS 1
#endif


#define ATMI_EPS 1e-3
#define USER_EPS 1e-3


/*
#define VERBOSE(command) command 
*/
#ifndef VERBOSE
#define VERBOSE(command) 
#endif


#define INTEGRATE(f,a,b,e) gsl_qags(f,a,b,e)

#define MIN(a,b) (((a)<(b))? a:b)
#define MAX(a,b) (((a)>(b))? a:b)


#ifndef M_PI
#define M_PI (4*atan(1.))
#endif

#define ASSERT(cond) if (!(cond)) {fprintf(stderr,"assert line %d\n",__LINE__); exit(1);}



/*****************************************************************************
  numerical integration routine from the GNU Scientific Library
******************************************************************************/

#define EPSABS 0
#define SUBINTSMAX 10000


static double gsl_qags(double (*f)(double), double a, double b, double eps)
{
  double ff (double x, void * params) {
    return f(x);
  }
  double result,error;
  gsl_integration_workspace *w;
  gsl_function F;
  w = gsl_integration_workspace_alloc(SUBINTSMAX);
  F.function = &ff;
  F.params = NULL;
  gsl_integration_qags(&F,a,b,EPSABS,eps,SUBINTSMAX,w,&result,&error); 
  gsl_integration_workspace_free(w);
  return result;
}



/***********************************************************************************
                  ATMI parameters
************************************************************************************/


#define CELSIUS_0 273.15 /* K */
#define AMU (1.674e-27) /* Kg */
#define BOLTZMANNK (1.38e-23) /* J/K */
#define SI_ATOMICMASS (28.0855 * AMU) /* Kg */
#define CU_ATOMICMASS (63.546 * AMU) /* Kg */
#define SI_DEBYETEMP (640.) /* K */
#define CU_DEBYETEMP (343.) /* K */
#define SI_K_20C (153.) /* W/mK */
#define CU_K (400.) /* W/mK */
#define SI_DENSITY (2330.) /* Kg/m^3 */
#define CU_DENSITY (8940.)
#define CU_SPECIFIC_HEAT (385.) /* J/KgK */



/* specific heat capacity (J/KgK) (Debye model) */
static double specific_heat_capacity(double atomicmass, double debyetemp, double temp)
{
  double y;
  double f(double x) {
    return pow(x,4)*exp(x)/pow(exp(x)-1,2);
  }
  y = debyetemp / temp;
  return 9 * (BOLTZMANNK/atomicmass) * pow(1/y,3) * INTEGRATE(f,0,y,ATMI_EPS);
}


/* Formula for Si thermal conductivity taken from 
   Leturcq, Dorkel, Napieralski, Lachiver,
   "A new approach to thermal analysis of power devices",
   IEEE Transactions on Electron Devices, vol. ED-34, No. 5, may 1987
*/
static double Si_thermal_conductivity(double temp_celsius)
{
  return SI_K_20C * pow((CELSIUS_0+temp_celsius)/(CELSIUS_0+20),-1.324);
}


static double Si_thermal_diffusivity(double temp_celsius)
{
  double Cp;
  Cp = specific_heat_capacity(SI_ATOMICMASS,SI_DEBYETEMP,CELSIUS_0+temp_celsius);
  return Si_thermal_conductivity(temp_celsius) / (Cp * SI_DENSITY);
}


static double Cu_thermal_conductivity(double temp_celsius)
{
  return CU_K;
}


static double Cu_thermal_diffusivity(double temp_celsius)
{
  double Cp;
  /* Cp = specific_heat_capacity(CU_ATOMICMASS,CU_DEBYETEMP,CELCIUS_0+temp_celsius); */
  Cp = CU_SPECIFIC_HEAT;
  return Cu_thermal_conductivity(temp_celsius) / (Cp * CU_DENSITY);
}




/*****************************************************************************
 Numerical inversion of Laplace transform taken from
 "algorithm 368: numerical inversion of Laplace transforms", H. Stehfest,
 Communications of the ACM 13(1) and 13(10), 1970
******************************************************************************/

/* should not be used to recover oscillating functions
   or functions with discontinuities 
   ==> ok for temperature response to step power
*/


#define N2 10


static double vi[N2];


static long long facto(int n)
{
  ASSERT(n>=0);
  if (n==0) return 1;
  return n*facto(n-1);
}


static long long bino(int n, int m)
{
  ASSERT(m>=0);
  ASSERT(n>=1);
  if (m==0) return 1;
  if (n==m) return 1;
  return bino(n-1,m) + bino(n-1,m-1);
}


static void fill_vi()
{
  int i,k;
  ASSERT((N2 & 1)==0);
  for (i=1; i<=N2; i++) {
    vi[i-1]=0;
    for (k=((int)((i+1)/2)); k<=MIN(i,N2/2); k++) {
      vi[i-1] += pow(k,1+N2/2)*bino(N2/2,k)*bino(2*k,k)*bino(k,i-k);
    }
    vi[i-1] *= pow(-1.,N2/2+i) / facto(N2/2);
  }
}



static double inverse_laplace(double (*f)(double), double t)
{
  int i;
  double ft;
  ASSERT(t>0);
  ft = 0;
  for (i=1;i<=N2;i++) {
    ft += vi[i-1] * f(log(2.)*i/t);
  }
  ft *= log(2.)/t;
  return(ft);
}



/*********************************************************************************
                Heat equation solution
**********************************************************************************/

#define IPHI 5.
#define IPSI 5.


static double e(atmi_param *p, double x2, double s)
{
  double y;
  y = p->h2 / p->k2;
  return exp(-2*x2*p->d) * (x2-y) / (x2+y);
}


static double f(atmi_param *p, double x, double x1, double s)
{
  double x2,y;
  if (s==0) {
    y = e(p,x,0);
    return (p->k1 * (x/p->h1 + (1+y) / ((1-y) * p->k2)));
  }
  x2 = sqrt(x*x + s/p->a2);
  y = e(p,x2,s);  
  return p->k1 * x1 * (1/p->h1 + (1+y)/(p->k2*x2*(1-y)));
}


static double g(atmi_param *p, double x, double x1, double s)
{
  double y;
  y = f(p,x,x1,s);
  return exp(-2*x1*p->z1) * (y-1)/(y+1);
}


static double t_h(atmi_param *p, double x, double t)
{
  double h(double s) {
    double x1,y;
    ASSERT(s!=0);
    x1 = sqrt(x*x + s/p->a1);
    y = g(p,x,x1,s);
    return y / (s * (1-y) * x1);
  }
  if (t==0) return 0;
  return inverse_laplace(h,t);
}


static double phi(atmi_param *p, double r)
{
  double f(double x) {
    double y;
    y = g(p,x,x,0);
    return y * j0(x*r) / (1-y);
  }
  return 1 + 2*r*INTEGRATE(f,0,IPHI/p->z1,ATMI_EPS);
}


static double t_phi(atmi_param *p, double r, double t)
{
  double f(double x) {
    return x * j0(x*r) * t_h(p,x,t);
  }  
  if (t==0) return (0);
  return erfc(r/sqrt(4*p->a1*t)) + 2*r*INTEGRATE(f,0,IPHI/p->z1,ATMI_EPS);
}


static double psi(atmi_param *p, double r)
{
  double f(double x) {
    double y;
    if (x==0) return (0.5*r/(1/g(p,0,0,0)-1));
    y = g(p,x,x,0);
    return j1(x*r) * y/(x*(1-y));
  }
  return r * (1 + 2*INTEGRATE(f,0,IPSI/p->z1,ATMI_EPS));
}


static double t_psi(atmi_param *p, double r, double t)
{
  double y;
  double f(double x) {
    return j1(x*r) * t_h(p,x,t);
  }  
  if (t==0) return (0);
  y = 2*sqrt(p->a1*t/M_PI)*(1-exp(-r*r/(4*p->a1*t))) + r*erfc(r/sqrt(4*p->a1*t));
  return y + 2*r*INTEGRATE(f,0,IPSI/p->z1,ATMI_EPS);
}



/* Temperature at distance r from a point source */
/* Temperature becomes infinite as r -> 0 */
double atmi_point(atmi_param *p, 
		  double power, /* source power (W) */
		  double r, /* distance from the source (m) */
		  double t, /* time (s), used if steady=0 */
		  char steady)
{
  ASSERT(r>0);
  if (steady) return power * phi(p,r)/(2*M_PI*p->k1*r);
  return power * t_phi(p,r,t)/(2*M_PI*p->k1*r);
}


/* temperature at the center of a disk */
/* power density = q , radius = r */
double atmi_disk_center(atmi_param *p, double q, double r, double t, char steady)
{
  if (steady) return ((q/p->k1) * psi(p,r));
  return ((q/p->k1)*t_psi(p,r,t));
}


static double disk_infinite(atmi_param *p, double q, double s)
{
  double ee,ff,gg,s1,s2;
  s2 = sqrt(s/p->a2);
  ee = (p->k2/p->h2) * s2;
  ee = exp(-2*p->d*s2) * (ee-1)/(ee+1);
  s1 = sqrt(s/p->a1);
  ff = s1*p->k1/p->h1 + (p->k1/p->k2)*sqrt(p->a2/p->a1)*(1+ee)/(1-ee);
  gg = exp(-2*p->z1*s1) * (ff-1)/(ff+1);
  return (q/(s*p->k1*s1)) * (1+gg)/(1-gg);
}

/* temperature generated by a disk of infinite radius (power density q) */
double atmi_1D(atmi_param *p, double q, double t, double steady)
{
  double f(double s) {
    return disk_infinite(p,q,s);
  }
  if (steady) {
    return q * (p->z1/p->k1 + 1/p->h1 + p->d/p->k2 + 1/p->h2);
  } else {
    return inverse_laplace(f,t);
  }
}


static char equal(double x1, double x2, double acc) 
{
  return ((fabs(x1-x2) <= fabs(x1*acc)) || (fabs(x1-x2) <= fabs(x2*acc)));
}





#define MAXL 20
#define MINL 0.001

#define PARAM(cond,mess) if (!(cond)) {fprintf(stderr,mess); exit(1);}

static void atmi_validate_param(atmi_param *p)
{
  double r,w,ww;
  PARAM(p->k1 > 0,"Parameter k1 should be > 0\n");
  PARAM(p->k2 > 0,"Parameter k2 should be > 0\n");
  PARAM(p->a1 > 0,"Parameter a1 should be > 0\n");
  PARAM(p->a2 > 0,"Parameter a2 should be > 0\n");
  PARAM(p->h1 > 0,"Parameter h1 should be > 0\n");
  PARAM(p->h2 > 0,"Parameter h2 should be > 0\n");
  PARAM(p->L < MAXL,"Are you sure you want L to be that large ?\n");
  PARAM(p->L > MINL,"Are you sure you want L to be that small ?\n");
  ww = atmi_1D(p,1,0,1);
  for (r=MINL; r<MAXL; r*=1.1) {
    w = atmi_disk_center(p,1,r,0,1);
    if (equal(w,ww,ATMI_EPS)) {
      p->MAXR = r;
      return;
    }
  }
  PARAM(0,"Are you sure these are realistic parameters ?\n");
}


#define NEGLHSR 1e-10

void atmi_fill_param(atmi_param *p, 
		     double celsiuszone, /* celsius */
		     double heatsink_resistance, /* K/W */
		     double heatsink_width, /* m */
		     double copper_thickness, /* m */
		     double bulk_silicon_thickness, /* m */
		     double interface_thickness, /* m */
		     double interface_thermal_cond /* W/mK */)
{
  fill_vi();
  gsl_set_error_handler_off();
  p->z1 = bulk_silicon_thickness;
  p->d =  copper_thickness;
  p->k1 = Si_thermal_conductivity(celsiuszone);
  p->a1 = Si_thermal_diffusivity(celsiuszone);
  p->k2 = Cu_thermal_conductivity(celsiuszone);
  p->a2 = Cu_thermal_diffusivity(celsiuszone);
  p->h1 = interface_thermal_cond / interface_thickness;
  p->L =  heatsink_width;
  p->h2 = 1./((heatsink_resistance+NEGLHSR) * pow(p->L,2));
  atmi_validate_param(p);
}


void atmi_set_param(atmi_param *p,
		    double val_z1,
		    double val_d,
		    double val_k1,
		    double val_a1,
		    double val_k2,
		    double val_a2,
		    double val_h1,
		    double val_h2,
		    double val_L)
{
  fill_vi();
  gsl_set_error_handler_off();
  p->z1 = val_z1;
  p->d = val_d;
  p->k1 = val_k1;
  p->a1 = val_a1;
  p->k2 = val_k2;
  p->a2 = val_a2;
  p->h1 = val_h1;
  p->h2 = val_h2;
  p->L = val_L;
  atmi_validate_param(p);
}


void atmi_print_param(atmi_param *p)
{
  fprintf(stderr,"%-45s : %.2e\n","Layer 1 thickness (m)",p->z1);
  fprintf(stderr,"%-45s : %.2f\n","Layer 1 thermal conductivity (W/mK)",p->k1);
  /*fprintf(stderr,"%-45s : %.2f\n","Layer 1 specific heat (J/KgK)",p->k1/(p->a1*SI_DENSITY));*/
  fprintf(stderr,"%-45s : %.2e\n","Layer 1 thermal diffusivity (m^2/s)",p->a1);
  fprintf(stderr,"%-45s : %.2e\n","Layer 2 thickness (m)",p->d);
  fprintf(stderr,"%-45s : %.2f\n","Layer 2 thermal conductivity (W/mK)",p->k2);
  /*fprintf(stderr,"%-45s : %.2f\n","Layer 2 specific heat (J/KgK)",p->k2/(p->a2*CU_DENSITY));*/
  fprintf(stderr,"%-45s : %.2e\n","Layer 2 thermal diffusivity (m^2/s)",p->a2);
  fprintf(stderr,"%-45s : %.2e\n","Interface thermal conductance (W/m^2K)",p->h1);
  fprintf(stderr,"%-45s : %.2f\n","Effective heat transfer coefficient (W/m^2K)",p->h2);
  fprintf(stderr,"%-45s : %.2e\n","Width (m)",p->L);
  fprintf(stderr,"%-45s : %f\n","Thermal resistance (K/W)",1./(p->h2*pow(p->L,2)));
  VERBOSE(fprintf(stderr,"%-45s = %.10f\n","m^2K/W",atmi_1D(p,1,0,1)));
  VERBOSE(fprintf(stderr,"No influence beyond %.4f meters\n",p->MAXR));
}



#define DWPOINTS 7

double atmi_images(atmi_param *p, double power, double t, char steady)
{
  double r,w,q;
  int i,j,howmany;
  double maxr = MIN(p->MAXR,(DWPOINTS+0.5)*p->L);
  int maxpt = maxr / p->L;

  double f(double x) {
    int i,j,howmany;
    double r,sum,y;
    sum = 0;
    for (i=1;i<=maxpt;i++) {
      for (j=0;j<=i;j++) {
        r = p->L*sqrt(i*i+j*j);
        if (r <= maxr) {
          howmany = ((i!=j)&&(j!=0))? 8 : 4;
          sum += howmany * j0(x*r);
        } else {
	  break;
	}
      }
    }
    if (steady) {
      y = g(p,x,x,0);
      sum *= y/(1-y);
    } else {
      sum *= x * t_h(p,x,t);
    }
    return sum;
  }

  if (p->L >= p->MAXR) return 0;

  ASSERT((maxr >= p->L) && (maxr < MAXL));
  w = 0;
  for (i=1;i<=maxpt;i++) {
    for (j=0;j<=i;j++) {
      r = p->L*sqrt(i*i+j*j);
      if (r <= maxr) {
        howmany = ((i!=j)&&(j!=0))? 8 : 4;
        if (steady) {
          w += howmany / r;
        } else {
          w += howmany * erfc(r/sqrt(4*p->a1*t)) / r;
        }
      }
    }
  }
  w *= power/(2*M_PI*p->k1);
  w += (power/(M_PI*p->k1)) * INTEGRATE(f,0,IPHI/p->z1,ATMI_EPS);
  if (maxr < p->MAXR) {
    q = power / (p->L*p->L);
    w += atmi_1D(p,q,t,steady) - atmi_disk_center(p,q,maxr,t,steady);
  }
  return w;
}



/* temperature at the vertex B of a right triangle BCA */
/* power density = q, perpendicular sides a=BC and b=CA */
double atmi_rtri(atmi_param *p, double q, double a, double b, double t, char steady) 
{
  double f(double x) {
    if (steady) return psi(p,a/cos(x));
    return t_psi(p,a/cos(x),t);
  }
  ASSERT((a>=0) && (b>=0));
  if ((fabs(a)<1E-20) || (fabs(b)<1E-20)) return (0);
  return ((q/(2*M_PI*p->k1)) * INTEGRATE(f,0,atan(b/a),ATMI_EPS));
}


/* temperature at the vertex of a rectangle */
/* power density = q, sides a and b */
static double wtri2(atmi_param *p, double q, double a, double b, double t, char steady)
{
  double s = 0;
#pragma omp parallel sections shared(p,q,a,b,t,steady) reduction(+: s) num_threads(MIN(2,NTHREADS))
  {
#pragma omp section
    {
      s += atmi_rtri(p,q,a,b,t,steady);
    }
#pragma omp section
    {
      s += atmi_rtri(p,q,b,a,t,steady);
    }
  }
  return s;
}


/* temperature at the center of a square */
/* power density = q, side = a */
double atmi_square_center(atmi_param *p, double q, double a, double t, char steady)
{
  return(8*atmi_rtri(p,q,a/2,a/2,t,steady));
}


/* temperature at the center of a rectangle */
/* power density = q, sides a and b */
double atmi_rect_center(atmi_param *p, double q, double a, double b, double t, char steady)
{
  return(4*wtri2(p,q,a/2,b/2,t,steady));
}


static double sign(double x)
{
  return ((x>=0)? 1 : -1);
}


static double wtri(atmi_param *p, double q, double a, double b1, double b2, double t, double steady)
{
  double f(double x) {
    if (steady) return psi(p,a/cos(x));
    return t_psi(p,a/cos(x),t);
  }
  ASSERT(a>=0);
  if (fabs(a)<1E-20) return 0;
  return ((q/(2*M_PI*p->k1)) * INTEGRATE(f,atan(b1/a),atan(b2/a),ATMI_EPS));
}


/* Rectangle source */
/* Temperature at point (x,y) (point (0,0) is rectangle center)*/
double atmi_rectangle(atmi_param *p, 
		      double q, /* power density (W/m^2)*/
		      double a, /* rectangle width (m) */
		      double b, /* rectangle height (m) */
		      double x, 
		      double y, 
		      double t, /* time (s), used if steady=0 */
		      char steady) 
{
  double v = 0;
#pragma omp parallel sections shared(p,q,a,b,x,y,t,steady) reduction(+: v) num_threads(MIN(4,NTHREADS))
  {
#pragma omp section
    {
      v += sign(b/2-y) * wtri(p,q,fabs(b/2-y),x-a/2,x+a/2,t,steady);
    }
#pragma omp section
    {
      v += sign(a/2+x) * wtri(p,q,fabs(a/2+x),y-b/2,y+b/2,t,steady);
    }
#pragma omp section
    {
      v += sign(b/2+y) * wtri(p,q,fabs(b/2+y),-a/2-x,a/2-x,t,steady);
    }
#pragma omp section
    {
      v += sign(a/2-x) * wtri(p,q,fabs(a/2-x),-b/2-y,b/2-y,t,steady);
    }
  }
  return v;
} 




/************************************************************************************
  "atmi_steady_rect" computes steady-state temperature for a coarse power density map
   consisting of rectangle sources
   (gives temperature at the center of each rectangle)
*************************************************************************************/



void atmi_steady_rect(atmi_param *param, 
		      int nrect, 
		      atmi_rect rc[],
		      double q[],
		      double temperature[])
{
  int i,j;
  double p;
  double a,b,x,y,dw;
  p = 0;
  for (i=0; i<nrect; i++) {
    p += q[i] * fabs(rc[i].x2-rc[i].x1) * fabs(rc[i].y2-rc[i].y1);
  }
  dw = atmi_images(param,p,0,1);
  for (i=0; i<nrect; i++) {
    temperature[i] = dw;
    for (j=0; j<nrect; j++) { 
      a = fabs(rc[j].x2-rc[j].x1);
      b = fabs(rc[j].y2-rc[j].y1);
      x = (rc[i].x1 + rc[i].x2 - rc[j].x1 - rc[j].x2) / 2;
      y = (rc[i].y1 + rc[i].y2 - rc[j].y1 - rc[j].y2) / 2;
      temperature[i] += atmi_rectangle(param,q[j],a,b,x,y,0,1);
    }
  }
}



/***************************************************************************************
  "atmi_steady_grid" computes steady-state temperature for a detailed power density map
****************************************************************************************/


#define MEMOMAX (ATMI_GRIDMAX*(ATMI_GRIDMAX+1)/2)
#define POINTAPPROX 10

#define USE_INTERPOL
#define INTERPOL_POINTS 1000
#define INTERPOL_MAXDIST 0.1



static void dft2(double data[],
		 int nx, /* number of columns */
		 int ny /* number of rows */,
		 gsl_fft_complex_wavetable *wvx,
		 gsl_fft_complex_wavetable *wvy,
		 gsl_fft_complex_workspace *wkx,
		 gsl_fft_complex_workspace *wky,
		 int dir)
{
  int i;
  for (i=0; i<ny; i++) {
    if (dir>=0) {
      gsl_fft_complex_forward (&data[i*2*nx],1,nx,wvx,wkx);
    } else {
      gsl_fft_complex_inverse (&data[i*2*nx],1,nx,wvx,wkx);
    }
  }
  for (i=0; i<nx; i++) {
    if (dir>=0) {
      gsl_fft_complex_forward (&data[i*2],nx,ny,wvy,wky);
    } else {
      gsl_fft_complex_inverse (&data[i*2],nx,ny,wvy,wky);
    }
  }
}


static void cmul(double a[2],double b[2],double c[2])
{
  c[0] = a[0]*b[0] - a[1]*b[1];
  c[1] = a[0]*b[1] + a[1]*b[0];
}


void atmi_steady_grid(atmi_param *param, 
		      int nx,
		      int ny,
		      double gridunit,
		      atmi_grid q,
		      atmi_grid temperature)
{
  const int nx2 = 2*nx;
  const int ny2 = 2*ny;
  char memo[MEMOMAX];
  double memoval[MEMOMAX];
#ifdef USE_INTERPOL
  gsl_interp_accel *acc;
  gsl_spline *spline;
  double x[INTERPOL_POINTS], y[INTERPOL_POINTS];
#endif
  int i,j,k,mi,ii,jj;
  double dw1,sqx,r,sqa;
  double *q2,*w2,*t2;
  gsl_fft_complex_wavetable *wvx, *wvy;
  gsl_fft_complex_workspace *wkx, *wky;
  q2 = (double *) malloc(2*nx2*ny2 * sizeof(double));
  w2 = (double *) malloc(2*nx2*ny2 * sizeof(double));
  t2 = (double *) malloc(2*nx2*ny2 * sizeof(double));
  wvx = gsl_fft_complex_wavetable_alloc(nx2);
  wvy = gsl_fft_complex_wavetable_alloc(ny2);
  wkx = gsl_fft_complex_workspace_alloc(nx2);
  wky = gsl_fft_complex_workspace_alloc(ny2);

  ASSERT((nx > 0) && (nx <= ATMI_GRIDMAX));
  ASSERT((ny > 0) && (ny <= ATMI_GRIDMAX));
  sqx = gridunit;
  sqa = sqx * sqx;
  dw1 = atmi_images(param,1.,0,1);
  for (i=0; i<ATMI_GRIDMAX; i++) {
    for (j=0; j<ATMI_GRIDMAX; j++) {
      temperature[i][j] = 0;
    }
  }
  for (i=0; i<MEMOMAX; i++) {
    memo[i] = 0;
  }
  /* fill q2 with power density */
  for (i=0; i<nx2; i++) {
    for (j=0; j<ny2; j++) {
      k = 2*(nx2*j+i);
      if ((i<nx) && (j<ny)) {
	q2[k] = q[i][j];
      } else {
	q2[k] = 0;
      }
      q2[k+1] = 0;
    }
  }

#ifdef USE_INTERPOL
  acc = gsl_interp_accel_alloc();
  spline = gsl_spline_alloc(gsl_interp_cspline,INTERPOL_POINTS);
#pragma omp parallel for schedule(static,100) private(i) shared(x,y,param,sqa) num_threads(NTHREADS)
  for (i=0; i<INTERPOL_POINTS; i++) {
    x[i] = ((i+1) * INTERPOL_MAXDIST) / INTERPOL_POINTS;
    y[i] = atmi_point(param,sqa,x[i],0,1);
  }
  gsl_spline_init(spline,x,y,INTERPOL_POINTS);
#endif

  /* fill w2 with influence coefficient */
  for (i=0; i<nx; i++) {
    for (j=0; j<ny; j++) {
      k = 2*(nx2*j+i);
      mi = (j<=i)? j+i*(i+1)/2 : i+j*(j+1)/2;
      if (!memo[mi]) {
	r = sqx * sqrt(i*i+j*j);
	if (r > (POINTAPPROX*sqx)) {
#ifdef USE_INTERPOL
	  ASSERT(r < INTERPOL_MAXDIST);
	  memoval[mi] = sqa*dw1 + gsl_spline_eval(spline,r,acc);;
#else
	  memoval[mi] = sqa*dw1 + atmi_point(param,sqa,r,0,1);
#endif
	} else {
	  memoval[mi] = sqa*dw1 + atmi_rectangle(param,1.,sqx,sqx,i*sqx,j*sqx,0,1);
	}
	memo[mi] = 1;
      }
      w2[k] = memoval[mi];
      w2[k+1] = 0;
    }
  }
    
  /* fill the rest of w2 so that cyclic convolution gives what we want */
  for (i=0; i<nx2; i++) {
    for (j=0; j<ny2; j++) {
      k = 2*(nx2*j+i);
      if ((i>=nx) || (j>=ny)) {
	ii = (i<nx)? i : nx2-i;
	jj = (j<ny)? j : ny2-j;
	ASSERT(ii<=nx);
	ASSERT(jj<=ny);
	if ((ii<nx) && (jj<ny)) {
	  w2[k] = w2[2*(nx2*jj+ii)];
	} else {
	  w2[k] = 0;
	}
	w2[k+1] = 0;
      }
    }
  }

  /* compute DFTs */
  dft2(q2,nx2,ny2,wvx,wvy,wkx,wky,1);
  dft2(w2,nx2,ny2,wvx,wvy,wkx,wky,1);

  /* multiply DFTs */
  for (i=0; i<nx2; i++) {
    for (j=0; j<ny2; j++) {
      k = 2*(nx2*j+i);
      cmul(&q2[k],&w2[k],&t2[k]);
    }
  }

  /* inverse DFT */
  dft2(t2,nx2,ny2,wvx,wvy,wkx,wky,-1);

  for (i=0; i<nx; i++) {
    for (j=0; j<ny; j++) {
      k = 2*(nx2*j+i);
      temperature[i][j] = t2[k];
    }
  }

  free(q2);
  free(w2);
  free(t2);
  gsl_fft_complex_wavetable_free(wvx);
  gsl_fft_complex_wavetable_free(wvy);
  gsl_fft_complex_workspace_free(wkx);
  gsl_fft_complex_workspace_free(wky);
#ifdef USE_INTERPOL
  gsl_spline_free(spline);
  gsl_interp_accel_free(acc);
#endif
}



void atmi_grid_print(FILE *fd, atmi_grid g, int nx, int ny)
{
  int i,j;
  for (j=ny-1; j>=0; j--) {
    for (i=0; i<nx; i++) {
      fprintf(fd,"%.8e ",g[i][j]);
    }
    fprintf(fd,"\n");
  }
}


static void fswap(double *x, double *y) 
{
  double temp;
  temp = *x;
  *x = *y;
  *y = temp;
}


static double intersec(double x1, double x2, double xx1, double xx2)
{
  double y1,y2;
  if (x2<x1) fswap(&x1,&x2);
  if (xx2<xx1) fswap(&xx1,&xx2);
  y1 = (xx1 > x1)? xx1 : x1;
  y2 = (xx2 < x2)? xx2 : x2;
  if (y2>y1) {
    return y2-y1;
  } else {
    return 0;
  }
}


void atmi_grid_set(atmi_grid g,
		   double gridunit,
		   int *nx,
		   int *ny,
		   int nrect,
		   atmi_rect rc[],
		   double val[])
{
  int i,j,k;
  double interx,intery;
  double xmin,xmax,ymin,ymax;

  if (nrect<=0) {
    fprintf(stderr,"atmi_grid_set: number of recatngles must be > 0\n");
    ASSERT(0);
  }
  if (nrect > ATMI_MAX_RECT) {
    fprintf(stderr,"atmi_grid_set: number of rectangles must be < %d\n",ATMI_MAX_RECT);
    ASSERT(0);
  }
  if (gridunit<=0) {
    fprintf(stderr,"atmi_grid_set: grid unit must be > 0\n");
    ASSERT(0);
  }

  xmin = MIN(rc[0].x1,rc[0].x2);
  xmax = MAX(rc[0].x1,rc[0].x2);
  ymin = MIN(rc[0].y1,rc[0].y2);
  ymax = MAX(rc[0].y1,rc[0].y2);
  for (k=1; k<nrect; k++) {
    xmin = (rc[k].x1 < xmin) ? rc[k].x1 : xmin;
    xmin = (rc[k].x2 < xmin) ? rc[k].x2 : xmin;
    xmax = (rc[k].x1 > xmax) ? rc[k].x1 : xmax;
    xmax = (rc[k].x2 > xmax) ? rc[k].x2 : xmax;
    ymin = (rc[k].y1 < ymin) ? rc[k].y1 : ymin;
    ymin = (rc[k].y2 < ymin) ? rc[k].y2 : ymin;
    ymax = (rc[k].y1 > ymax) ? rc[k].y1 : ymax;
    ymax = (rc[k].y2 > ymax) ? rc[k].y2 : ymax;
  }
  *nx = (xmax-xmin) / gridunit;
  *ny = (ymax-ymin) / gridunit;
  if (((*nx) <= 0) || ((*ny) <= 0)) {
    fprintf(stderr,"atmi_grid_set: rectangles not valid\n");
    ASSERT(0);
  }
  if (((*nx) > ATMI_GRIDMAX) || ((*ny) > ATMI_GRIDMAX)) {
    fprintf(stderr,"atmi_grid_set: grid is not large enough\n");
    ASSERT(0);
  }

  for (i=0; i<ATMI_GRIDMAX; i++) {
    for (j=0; j<ATMI_GRIDMAX; j++) {
      g[i][j] = 0;
    }
  }
  for (k=0; k<nrect; k++) {
    for (i=0; i<ATMI_GRIDMAX; i++) {
      interx = intersec(i*gridunit,(i+1)*gridunit,rc[k].x1,rc[k].x2);
      for (j=0; j<ATMI_GRIDMAX; j++) {
	intery = intersec(j*gridunit,(j+1)*gridunit,rc[k].y1,rc[k].y2);
	g[i][j] += val[k] * interx * intery;
      }
    }
  }
  for (i=0; i<ATMI_GRIDMAX; i++) {
    for (j=0; j<ATMI_GRIDMAX; j++) {
      g[i][j] = g[i][j] / (gridunit*gridunit);
    }
  }
}


/************************************************************************************
  thermal simulator
*************************************************************************************/


#define COMPRESSFREQ 30

/* these two parameters impact convolution speed and accuracy */
#define DIFFLAT 0.001
#define BTSRATIO 0.1

#define BTSMIN 1

#define SAMPLEFREQ 2


typedef atmi_influence InfluenceArray [ATMI_MAX_SENSORS];
typedef double DoubleArray [ATMI_MAX_SENSORS];


static double sampletime(atmi_simulator *p, int n)
{
  return p->TIMESTEP * pow(2,(double)n/SAMPLEFREQ);
}


static void init_Influence(atmi_influence *p, int memosize)
{
  int i;
  for (i=0; i<ATMI_MAX_SAMP; i++) {
    p->w[i] = 0;
  }
  p->wsteady = 0;
  p->nmax = 0;
  p->t_propag = 0;
  p->MEMOSIZE = memosize;
  p->alreadycomp = (char*) malloc(memosize*sizeof(char));
  ASSERT(p->alreadycomp);
  p->wmem = (double*) malloc(memosize*sizeof(double));
  ASSERT(p->wmem);
  for (i=0; i<memosize; i++) {
    p->alreadycomp[i] = 0;
    p->wmem[i] = 0;
  }
}


static void init_interpol(atmi_simulator *ts, atmi_influence *p)
{
  int n;
  double x[ATMI_MAX_SAMP+1], y[ATMI_MAX_SAMP+1];
  p->acc = gsl_interp_accel_alloc();
  p->spline = gsl_spline_alloc(gsl_interp_cspline,p->nmax+1);
  x[0] = 0;
  y[0] = 0;
  for (n=1; n<(p->nmax+1); n++) {
    x[n] = sampletime(ts,n-1);
    y[n] = p->w[n-1];
  }
  gsl_spline_init(p->spline,x,y,p->nmax+1);
}


static void init_EventList(atmi_simulator *ts, atmi_eventlist *p, char use)
{
  int n;
  p->nevents = 0;
  p->last_e = NULL;
  if (use) {
    p->e = (atmi_event *) malloc(ts->MAXEVENTS * sizeof(atmi_event));
    ASSERT(p->e);
    for (n=0; n<ts->MAXEVENTS; n++) {
      p->e[n].m = -1;
      p->e[n].val = 0;
    }
    p->max_nevents = COMPRESSFREQ;
  } else {
    p->e = NULL;
    p->max_nevents = 0;
  }
}



static void new_event(atmi_simulator *ts, atmi_eventlist *p, long long m, double val) 
{
  if (!p->e) return;
  ASSERT(p->nevents < ts->MAXEVENTS);
  p->last_e = & p->e[p->nevents];
  p->nevents++;
  p->last_e->m = m;
  p->last_e->val = val;
}

#if 0
static void print_events(atmi_simulator *ts, atmi_eventlist *p)
{
  int n;
  ASSERT(p->e);
  ASSERT(p->nevents >= 2);
  printf("%20.10f %20.10f\n",(double) 0,p->e[1].val);
  for (n=2; n < p->nevents; n++) {
    printf("%20.10f %20.10f\n",((double)(p->e[n].m-p->e[1].m)-0.1) * ts->TIMESTEP,p->e[n-1].val);
    printf("%20.10f %20.10f\n",((double)(p->e[n].m-p->e[1].m)) * ts->TIMESTEP,p->e[n].val);
  }
}
#endif

static void init_Source(atmi_simulator *ts, atmi_source *s, atmi_rect rcoord, char use_fe, char use_se)
{
  s->COORD = rcoord;
  s->XLEN = fabs(s->COORD.x2 - s->COORD.x1); /* rectangle width */
  s->YLEN = fabs(s->COORD.y2 - s->COORD.y1); /* rectangle height */
  s->AREA = s->XLEN * s->YLEN;
  s->XPOS = (s->COORD.x1 + s->COORD.x2) / 2; /* rectangle center coordinates */
  s->YPOS = (s->COORD.y1 + s->COORD.y2) / 2; 
  init_EventList(ts,&s->pq,use_fe);
  init_EventList(ts,&s->spq,use_se);
  s->BIGSTEP = 100./ts->TIMESTEP;
  s->autotempref = NULL;
  s->tempref = NULL;
}



static char equalx(double x1, double x2) 
{
  return ((fabs(x1-x2) <= fabs(x1*USER_EPS)) || (fabs(x1-x2) <= fabs(x2*USER_EPS)) || (fabs(x1-x2) < 1e-20));
}



static double dist(atmi_source *si, atmi_source *sj)
{
  return sqrt(pow(sj->XPOS-si->XPOS,2)+pow(sj->YPOS-si->YPOS,2));
}



void atmi_print_response1(atmi_simulator *ts, atmi_influence *p)
{
  int n;
  for (n=0; n<p->nmax; n++) {
    printf("%20.10f %30.20f\n",sampletime(ts,n),p->w[n]);
  }
}


static char curve_increasing(double x1, double x2, double xref, double acc)
{
  if (x2 >= x1) {
    return 1;
  } else if (fabs(x2-x1) < (xref*acc)) {
    return 1;
  } else {
    return 0;
  }
}



static void check_stepresponse(atmi_simulator *ts, atmi_influence *p, double acc)
{
  int n;
  double w = 0;
  char ok;
  for (n=0; n<p->nmax; n++) {
    ok = curve_increasing(w,p->w[n],p->wsteady,acc);
    if (!ok) {
      fprintf(stderr,"WARNING n=%2d: (%e) %e (%e) \n",n,p->w[n-1],p->w[n],p->w[n+1]);
    }
    w = p->w[n];
  }
}


static char is_inside(atmi_source *si, atmi_source *sj)
{
  double x0 = si->XPOS - si->XLEN/2;
  double x1 = si->XPOS + si->XLEN/2;
  double y0 = si->YPOS - si->YLEN/2;
  double y1 = si->YPOS + si->YLEN/2;
  return (x0<=sj->XPOS) && (sj->XPOS<=x1) && (y0<=sj->YPOS) && (sj->YPOS<=y1);
}


#define UNITQ 1.

static void init_imatrix(atmi_simulator *ts, atmi_influence *p, atmi_influence *pp, int i, int j)
{
  int n;
  double wsteady,t,prevt;
  enum atmi_sources stype;
  char steady;
  atmi_source *si,*sj;
  ASSERT(i<ts->NRECT);
  ASSERT(j<ts->NSENS);
  si = &ts->rect[i];
  sj = &ts->rect[ts->sensor[j]];

  init_Influence(p,ts->MEMOSIZE);

  if (pp) {
    p->nmax = pp->nmax;
    p->wsteady = pp->wsteady;
    p->t_propag = pp->t_propag;
    for (n=0; n<ATMI_MAX_SAMP; n++) {
      p->w[n] = pp->w[n];
    }
    init_interpol(ts,p);
    return;
  }

  wsteady = atmi_rectangle(&ts->PARAM,UNITQ,si->XLEN,si->YLEN,sj->XPOS-si->XPOS,sj->YPOS-si->YPOS,0,1);
  stype = unknown;

  if (equalx(si->XPOS,sj->XPOS) && equalx(si->YPOS,sj->YPOS)) {
    /* self influence */
    p->wsteady = atmi_disk_center(&ts->PARAM,UNITQ,sqrt(si->AREA/M_PI),0,1); 
    if (equal(wsteady,p->wsteady,USER_EPS)) {
      stype = disk;
      VERBOSE(fprintf(stderr,"Influence %d->%d: approximate source as a disk\n",i,ts->sensor[j]));
    } else  {
      p->wsteady = atmi_square_center(&ts->PARAM,UNITQ,sqrt(si->AREA),0,1);
      if (equal(wsteady,p->wsteady,USER_EPS)) {
	stype = square;
	VERBOSE(fprintf(stderr,"Influence %d->%d: approximate source as a square\n",i,ts->sensor[j]));
      } else {
	stype = rectangle0;
	p->wsteady = wsteady;
	VERBOSE(fprintf(stderr,"Influence %d->%d: rectangle\n",i,ts->sensor[j]));
      }
    }
  } else if (!is_inside(si,sj)) {
    p->wsteady = atmi_point(&ts->PARAM,UNITQ*si->AREA,dist(si,sj),0,1);
    if (equal(wsteady,p->wsteady,USER_EPS)) {
      stype = point;
      VERBOSE(fprintf(stderr,"Influence %d->%d: approximate source as a point\n",i,ts->sensor[j]));
    }
  }

  if (stype == unknown) {
    stype = rectangle;
    p->wsteady = wsteady;
    VERBOSE(fprintf(stderr,"Influence %d->%d: rectangle\n",i,ts->sensor[j]));
  }

  /* compute transient response to step power */
  p->t_propag = -1.;
  steady = 0;
  t = 0;
  for (n=0; n<ATMI_MAX_SAMP; n++) {
    prevt = t;
    t = sampletime(ts,n);
    if (t > ATMI_MAX_TIME) {
      p->nmax = n;
      break;
    }
    if (steady) {
      p->w[n] = p->wsteady;
    } else {
      switch (stype) {
      case point:
	p->w[n] = atmi_point(&ts->PARAM,UNITQ*si->AREA,dist(si,sj),t,0);
	break;
      case square:
	p->w[n] = atmi_square_center(&ts->PARAM,UNITQ,sqrt(si->AREA),t,0);
	break;
      case disk:
	p->w[n] = atmi_disk_center(&ts->PARAM,UNITQ,sqrt(si->AREA/M_PI),t,0);
	break;
      case rectangle:
	p->w[n] = wsteady = atmi_rectangle(&ts->PARAM,UNITQ,si->XLEN,si->YLEN,sj->XPOS-si->XPOS,sj->YPOS-si->YPOS,t,0);
	break;
      case rectangle0:
	p->w[n] = atmi_rect_center(&ts->PARAM,UNITQ,si->XLEN,si->YLEN,t,0);
	break;
      default: ASSERT(0);
      }
      steady = (fabs(p->w[n]-p->wsteady) < (USER_EPS*p->wsteady));
      if ((p->t_propag < 0) && (p->w[n] >= (DIFFLAT*p->wsteady))) {
	p->t_propag = prevt;
      } 
    }
    VERBOSE(fprintf(stderr,"%d ",n);fflush(stderr));
    p->nmax = n;
  }
  VERBOSE(fprintf(stderr,"\n"));
  VERBOSE(fprintf(stderr,"Diffusion latency: %.10f\n",p->t_propag));

  /* add the contribution from finite heat-sink width */
  ASSERT(p->nmax == ts->ideltaw.nmax);
  for (n=0; n<p->nmax; n++) {
    p->w[n] += UNITQ * si->AREA * ts->ideltaw.w[n]; /* ideltaw in K/W */
  }
  p->wsteady += UNITQ * si->AREA * ts->ideltaw.wsteady;

  if (dist(si,sj) < ts->PARAM.MAXR) {
    check_stepresponse(ts,p,USER_EPS);
  }

  init_interpol(ts,p);
}




static void init_ideltaw(atmi_simulator *ts, atmi_influence *ideltaw)
{
  char steady;
  int n;
  double t,prevt;
  atmi_influence *p;

  p = &ts->ideltaw;
  init_Influence(p,ts->MEMOSIZE);

  if (ideltaw) {
    p->nmax = ideltaw->nmax;
    p->wsteady = ideltaw->wsteady;
    p->t_propag = ideltaw->t_propag;
    for (n=0; n<ATMI_MAX_SAMP; n++) {
      p->w[n] = ideltaw->w[n];
    }
    init_interpol(ts,p);
    return;
  }

  p->wsteady = atmi_images(&ts->PARAM,1.,0,1); /* in K/W */
  steady = 0;
  p->t_propag = -1.;
  VERBOSE(fprintf(stderr,"Influence of image sources\n"));
  t = 0;
  for (n=0; n<ATMI_MAX_SAMP; n++) {
    prevt = t;
    t = sampletime(ts,n);
    if (t > ATMI_MAX_TIME) {
      p->nmax = n;
      break;
    }
    if (steady) {
      p->w[n] = p->wsteady;
    } else {
      p->w[n] = atmi_images(&ts->PARAM,1.,t,0);
      steady = (fabs(p->w[n]-p->wsteady) < (USER_EPS*p->wsteady));
      if ((p->t_propag < 0) && (p->w[n] >= (DIFFLAT*p->wsteady))) {
	p->t_propag = prevt;
      } 
    }
    VERBOSE(fprintf(stderr,"%d ",n);fflush(stderr));
    p->nmax = n;
  }
  VERBOSE(fprintf(stderr,"\n"));
  VERBOSE(fprintf(stderr,"Diffusion latency: %.10f\n",p->t_propag));

  init_interpol(ts,p);
}



static void normalize(double *xlen, double *ylen, double *x, double *y)
{
  *x = fabs(*x);
  *y = fabs(*y);
  if (equalx(*xlen,*ylen)) {
    if (*x < *y) {
      fswap(xlen,ylen);
      fswap(x,y);
    }
  } else if (*xlen < *ylen) {
    fswap(xlen,ylen);
    fswap(x,y);
  }
}


static char same_influence(atmi_source *si1,atmi_source *sj1,atmi_source *si2,atmi_source *sj2)
{
  double x1,y1,x2,y2,xlen1,ylen1,xlen2,ylen2;
  xlen1 = si1->XLEN;
  ylen1 = si1->YLEN;
  x1 = si1->XPOS - sj1->XPOS;
  y1 = si1->YPOS - sj1->YPOS;
  normalize(&xlen1,&ylen1,&x1,&y1);
  xlen2 = si2->XLEN;
  ylen2 = si2->YLEN;
  x2 = si2->XPOS - sj2->XPOS;
  y2 = si2->YPOS - sj2->YPOS;
  normalize(&xlen2,&ylen2,&x2,&y2);
  return (equalx(xlen1,xlen2) && equalx(ylen1,ylen2) && equalx(x1,x2) && equalx(y1,y2));
}



static double interpolw(atmi_simulator *ts, atmi_influence *p, long long m)
{
  double res,t;
  ASSERT((m*ts->TIMESTEP) < ATMI_MAX_TIME);
  if (m==0) return 0;
  ASSERT(m > 0);
  if ((m < p->MEMOSIZE) && (p->alreadycomp[m])) {
    return p->wmem[m];
  }
  t = m * ts->TIMESTEP;
  res = gsl_spline_eval(p->spline,t,p->acc);
  if (m < p->MEMOSIZE) {
    p->wmem[m] = res;
    p->alreadycomp[m] = 1;
  }
  return res;
}


void atmi_print_response2(atmi_simulator *ts, atmi_influence *p)
{
  long long m;
  double dm,maxt;
  maxt = sampletime(ts,p->nmax-1);
  dm = 1;
  for (m=0; m<(maxt/ts->TIMESTEP); m+=dm) {
    printf("%20.10f %30.20f\n",m*ts->TIMESTEP,interpolw(ts,p,m));
    dm *= 1.01;
  }
}


/* Convolution */
static double tempcontrib(atmi_simulator *ts,
			  atmi_eventlist *s, 
			  atmi_influence *p, 
			  long long m)
{
  int n;
  double w;
  ASSERT(s->e);
  if (s->nevents==0) return 0;
  w = s->e[0].val * interpolw(ts,p,m-s->e[0].m);
  for (n=1; n < s->nevents; n++) {
    w += (s->e[n].val-s->e[n-1].val) * interpolw(ts,p,m-s->e[n].m);
  }
  return w;
}


static double compute_energy(atmi_eventlist *p, long long m)
{
  int n;
  double energy = 0;
  ASSERT(p->nevents > 0);
  for (n=0; n<(p->nevents-1); n++) {
    ASSERT(p->e[n].m < p->e[n+1].m);
    energy += p->e[n].val * (p->e[n+1].m-p->e[n].m);
  }
  ASSERT(p->e[p->nevents-1].m < m);
  energy += p->e[p->nevents-1].val * (m - p->e[p->nevents-1].m);
  return energy;
}


static double reldiff(double x, double y)
{
  if (fabs(x)<fabs(y)) return fabs((y-x)/y);
  if (fabs(y)<fabs(x)) return fabs((y-x)/x);
  return 0;
}



static void compress_events(atmi_simulator *ts, atmi_eventlist *p, atmi_influence *s, long long m, double acc)
{
  int n,nn;
  double sum,dw,newdw,val;
  long long startm;
  double energy;
  char merge;
  if (!p->e) return;
  ASSERT(s);
  if (p->nevents > p->max_nevents) {
    energy = compute_energy(p,m);
    nn = 0;
    startm = p->e[0].m;
    sum = p->e[0].val * (p->e[1].m - p->e[0].m);
    val = p->e[0].val;
    dw = (interpolw(ts,s,m-p->e[0].m)-interpolw(ts,s,m-p->e[1].m)) / (p->e[1].m-p->e[0].m);
    ASSERT(p->nevents > 2);
    for (n=1; n<(p->nevents-1); n++) {
      newdw = (interpolw(ts,s,m-p->e[n].m)-interpolw(ts,s,m-p->e[n+1].m)) / (p->e[n+1].m-p->e[n].m);
      merge = (((m-p->e[n].m)*ts->TIMESTEP) > s->t_propag);
      merge &= (newdw >= dw);
      merge &= ((reldiff(dw,newdw) * reldiff(val,p->e[n].val)) <= acc);
      if (merge && (n<(p->nevents-2))) {
	/* merge event */
	sum += p->e[n].val * (p->e[n+1].m - p->e[n].m);
      } else {
	/* finalize event */
	ASSERT(nn < n);
	p->e[nn].m = startm;
	p->e[nn].val = sum / (p->e[n].m - startm);
	nn++;
	if (n<(p->nevents-2)) {
	  /* start new event */
	  startm = p->e[n].m;
	  dw = newdw;
	  sum = p->e[n].val * (p->e[n+1].m - p->e[n].m);
	  val = p->e[n].val;
	} else {
	  /* loop exit */
	  ASSERT(n==(p->nevents-2));
	  ASSERT(nn <= n);
	  p->e[nn] = p->e[n];
	  nn++;
	}
      }

    }
    ASSERT(nn <= (p->nevents-1));
    p->e[nn] = p->e[p->nevents-1];
    nn++;
    p->nevents = nn;
    p->max_nevents = nn + COMPRESSFREQ;
    ASSERT(equal(energy,compute_energy(p,m),1e-12)); /* compression preserves energy */
  }
}


static void update_temperature(atmi_simulator *p) 
{
  int i,j;

#ifdef ATMI_COMPRESS_EVENTS
  /* for accelerating convolutions */
  for (i=0; i<p->NRECT; i++) {
    compress_events(p,&p->rect[i].pq,p->rect[i].autotempref,p->m,ATMI_COMPRESS_ACCURACY);
    compress_events(p,&p->rect[i].spq,p->rect[i].tempref,p->m,ATMI_COMPRESS_ACCURACY);
  }
#endif

  /* compute temperature */
#pragma omp parallel for if(p->NSENS>1) schedule(dynamic,1) private(i,j) shared(p) num_threads(MIN(p->NSENS,NTHREADS))
  for (j=0; j<p->NSENS; j++) {
    p->temperature[j] = 0;
    for (i=0; i<p->NRECT; i++) {
      if (is_inside(&p->rect[i],&p->rect[p->sensor[j]])) {
	p->temperature[j] += tempcontrib(p,&p->rect[i].pq,&p->imatrix[i][j],p->m);
      } else {
	p->temperature[j] += tempcontrib(p,&p->rect[i].spq,&p->imatrix[i][j],p->m);
      }
    }
  }
}



static void aggregate_events(atmi_source *p)
{
  long long m0,m1,m2;
  double v0,v1;
  if (!p->spq.e) return;
  if (p->spq.nevents < 3) return;
  m0 = p->spq.e[p->spq.nevents-3].m;
  m1 = p->spq.e[p->spq.nevents-2].m;
  m2 = p->spq.e[p->spq.nevents-1].m;
  v0 = p->spq.e[p->spq.nevents-3].val;
  v1 = p->spq.e[p->spq.nevents-2].val;
  if ((m2-m0) < p->BIGSTEP) {
    /* merge events */
    ASSERT(m0 < m2);
    p->spq.e[p->spq.nevents-3].val = (v0*(m1-m0) + v1*(m2-m1)) / (m2-m0);
    p->spq.e[p->spq.nevents-2] = p->spq.e[p->spq.nevents-1];
    p->spq.nevents--;
  }
}


static void set_event_start(atmi_simulator *ts, atmi_source *p, double val, long long m)
{
  /* update slow events */
  new_event(ts,&p->spq,m,val);
  aggregate_events(p);

  /* update fast events */
  new_event(ts,&p->pq,m,val);
}


static void apply_power_density(atmi_simulator *p, double q[])
{
  int i;
  for (i=0; i<p->NRECT; i++) {
    set_event_start(p,&p->rect[i],q[i],p->m);
  }
}


static void write_influence(FILE *atmifile, atmi_influence *p)
{
  int n = 0;
  n += fwrite(&p->nmax,sizeof(p->nmax),1,atmifile);
  n += fwrite(&p->wsteady,sizeof(p->wsteady),1,atmifile);
  n += fwrite(p->w,sizeof(p->w[0]),ATMI_MAX_SAMP,atmifile);
  n += fwrite(&p->t_propag,sizeof(p->t_propag),1,atmifile);
  ASSERT(n==(ATMI_MAX_SAMP+3));
}


static void read_influence(FILE *atmifile, atmi_influence *p)
{
  int n = 0;
  n += fread(&p->nmax,sizeof(p->nmax),1,atmifile);
  n += fread(&p->wsteady,sizeof(p->wsteady),1,atmifile);
  n += fread(p->w,sizeof(p->w[0]),ATMI_MAX_SAMP,atmifile);
  n += fread(&p->t_propag,sizeof(p->t_propag),1,atmifile);
  ASSERT(n==(ATMI_MAX_SAMP+3));
}


static char read_atmifile(atmi_simulator *p, 
			  const char *filename,
			  atmi_influence *ideltaw,
			  atmi_influence imatrix[ATMI_MAX_RECT][ATMI_MAX_SENSORS])
{
  FILE *atmifile;
  char ok;
  int i,j,n;
  atmi_param param;
  atmi_rect rect;
  double x;
  if (!filename) {
    return 0;
  }
  atmifile = fopen(filename,"r");
  ok = (atmifile != NULL);

  if (atmifile) {
    ok &= fread(&param.z1,sizeof(param.z1),1,atmifile);
    ok &= fread(&param.d,sizeof(param.d),1,atmifile);
    ok &= fread(&param.k1,sizeof(param.k1),1,atmifile);
    ok &= fread(&param.a1,sizeof(param.a1),1,atmifile);
    ok &= fread(&param.k2,sizeof(param.k2),1,atmifile);
    ok &= fread(&param.a2,sizeof(param.a2),1,atmifile);
    ok &= fread(&param.h1,sizeof(param.h1),1,atmifile);
    ok &= fread(&param.h2,sizeof(param.h2),1,atmifile);
    ok &= fread(&param.L,sizeof(param.L),1,atmifile);
    ok &= (param.z1==p->PARAM.z1);
    ok &= (param.d==p->PARAM.d);
    ok &= (param.k1==p->PARAM.k1);
    ok &= (param.a1==p->PARAM.a1);
    ok &= (param.k2==p->PARAM.k2);
    ok &= (param.a2==p->PARAM.a2);
    ok &= (param.h1==p->PARAM.h1);
    ok &= (param.h2==p->PARAM.h2);
    ok &= (param.L==p->PARAM.L);
    ok &= fread(&x,sizeof(x),1,atmifile);
    ok &= (p->TIMESTEP == x);
    ok &= fread(&n,sizeof(n),1,atmifile);
    ok &= (p->NRECT == n);
    ok &= fread(&n,sizeof(n),1,atmifile);
    ok &= (p->NSENS == n);
    for (i=0; i<p->NRECT; i++) {
      ok &= fread(&rect.x1,sizeof(rect.x1),1,atmifile);
      ok &= fread(&rect.y1,sizeof(rect.y1),1,atmifile);
      ok &= fread(&rect.x2,sizeof(rect.x2),1,atmifile);
      ok &= fread(&rect.y2,sizeof(rect.y2),1,atmifile);
      ok &= (rect.x1==p->rect[i].COORD.x1);
      ok &= (rect.y1==p->rect[i].COORD.y1);
      ok &= (rect.x2==p->rect[i].COORD.x2);
      ok &= (rect.y2==p->rect[i].COORD.y2);
    }
    if (!ok) {
      fprintf(stderr,"File %s does not match\n",filename);
    } else {
      fprintf(stderr,"File %s matches\n",filename);
      read_influence(atmifile,ideltaw);
      for (i=0; i<p->NRECT; i++) {
	for (j=0; j<p->NSENS; j++) {
	  read_influence(atmifile,&imatrix[i][j]);
	}
      }
    }
    fclose(atmifile);
  } else {
    fprintf(stderr,"File %s does not exist\n",filename);
  }
  return ok;
}



static void create_atmifile(atmi_simulator *p, const char *filename)
{
  FILE *atmifile;
  int i,j;
  if (!filename) {
    return;
  }
  atmifile = fopen(filename,"r");
  if (atmifile) {
    fprintf(stderr,"File %s already exists, not overwritten\n",filename);
    fclose(atmifile);
    return;
  }
  ASSERT(!atmifile);
  fprintf(stderr,"Creating file %s\n",filename);
  atmifile = fopen(filename,"w");
  ASSERT(atmifile);
  fwrite(&p->PARAM.z1,sizeof(p->PARAM.z1),1,atmifile);
  fwrite(&p->PARAM.d,sizeof(p->PARAM.d),1,atmifile);
  fwrite(&p->PARAM.k1,sizeof(p->PARAM.k1),1,atmifile);
  fwrite(&p->PARAM.a1,sizeof(p->PARAM.a1),1,atmifile);
  fwrite(&p->PARAM.k2,sizeof(p->PARAM.k2),1,atmifile);
  fwrite(&p->PARAM.a2,sizeof(p->PARAM.a2),1,atmifile);
  fwrite(&p->PARAM.h1,sizeof(p->PARAM.h1),1,atmifile);
  fwrite(&p->PARAM.h2,sizeof(p->PARAM.h2),1,atmifile);
  fwrite(&p->PARAM.L,sizeof(p->PARAM.L),1,atmifile);
  fwrite(&p->TIMESTEP,sizeof(p->TIMESTEP),1,atmifile);
  fwrite(&p->NRECT,sizeof(p->NRECT),1,atmifile);
  fwrite(&p->NSENS,sizeof(p->NSENS),1,atmifile);
  for (i=0; i<p->NRECT; i++) {
    fwrite(&p->rect[i].COORD.x1,sizeof(p->rect[i].COORD.x1),1,atmifile);
    fwrite(&p->rect[i].COORD.y1,sizeof(p->rect[i].COORD.y1),1,atmifile);
    fwrite(&p->rect[i].COORD.x2,sizeof(p->rect[i].COORD.x2),1,atmifile);
    fwrite(&p->rect[i].COORD.y2,sizeof(p->rect[i].COORD.y2),1,atmifile);
  }
  write_influence(atmifile,&p->ideltaw);
  for (i=0; i<p->NRECT; i++) {
    for (j=0; j<p->NSENS; j++) {
      write_influence(atmifile,&p->imatrix[i][j]);
    }
  }
  fclose(atmifile);
}



static void atmi_rectset_check(atmi_rectset *rs, int nrect)
{
  int i;
  if (rs->n > nrect) {
    fprintf(stderr,"too many rectangles in set\n");
    ASSERT(0);
  }
  for (i=0; i<rs->n; i++) {
    if ((rs->s[i]<0) || (rs->s[i] >= nrect)) {
      fprintf(stderr,"%d is not a valid rectangle number\n",rs->s[i]);
      ASSERT(0);
    }
  }
}



void atmi_simulator_init(atmi_simulator *p, 
			 const char *filename,
			 atmi_param *param,
			 double timestep,
			 int nrect,
			 atmi_rect rc[],
			 atmi_rectset *sensors,
			 double q[])
{
  int i,j,ii,jj;
  char similar,readfromfile;
  double w;
  atmi_influence ideltaw;
  InfluenceArray *imatrix;
  char use_fe[ATMI_MAX_RECT];
  char use_se[ATMI_MAX_RECT];

  imatrix = (InfluenceArray *) malloc(ATMI_MAX_RECT*sizeof(InfluenceArray));
  ASSERT(imatrix);

  p->PARAM = *param;
  p->TIMESTEP = timestep;
  p->NRECT = nrect;
  p->NSENS = sensors->n;

  if (nrect<=0) {
    fprintf(stderr,"atmi_simulator_init: number of rectangles must be > 0\n");
    ASSERT(0);
  }
  if (nrect > ATMI_MAX_RECT) {
    fprintf(stderr,"atmi_simulator_init: number of rectangles must be < %d\n",ATMI_MAX_RECT);
    ASSERT(0);
  }

  if (sensors->n<=0) {
    fprintf(stderr,"atmi_simulator_init: number of sensors must > 0\n");
    ASSERT(0);
  }

  atmi_rectset_check(sensors,nrect);

  for (i=0; i<sensors->n; i++) {
    ASSERT(sensors->s[i] >= 0);
    ASSERT(sensors->s[i] < nrect);
    /* fprintf(stderr,"sensor %d in rectangle %d\n",i,sensors->s[i]); */
  }
  p->MEMOSIZE = (ATMI_MEMSIZE/2) / (8*nrect*(2+sensors->n));
  p->MAXEVENTS = (ATMI_MEMSIZE/2) / (16*2*nrect);
  VERBOSE(fprintf(stderr,"MEMOSIZE: %d\n",p->MEMOSIZE));
  VERBOSE(fprintf(stderr,"MAXEVENTS: %d\n",p->MAXEVENTS));

  p->rect = (atmi_source *) malloc(nrect * sizeof(atmi_source));
  ASSERT(p->rect);

  for (i=0; i<nrect; i++) {
    use_fe[i] = 0;
    use_se[i] = 0;
    init_Source(p,&p->rect[i],rc[i],0,0);
  }

  for (j=0; j<sensors->n; j++) {
    p->temperature[j] = 0;
    p->sensor[j] = sensors->s[j];
  }

  /* determine which kind of event is useful for each source */
  for (j=0; j<p->NSENS; j++) {
    for (i=0; i<p->NRECT; i++) {
      if (is_inside(&p->rect[i],&p->rect[p->sensor[j]])) {
	/* use fast events */
	use_fe[i] = 1;
      } else {
	/* use slow events */
	use_se[i] = 1;
      }
    }
  }

  for (i=0; i<nrect; i++) {
    ASSERT(use_fe[i] || use_se[i]);
    init_Source(p,&p->rect[i],rc[i],use_fe[i],use_se[i]);
  }

  readfromfile = read_atmifile(p,filename,&ideltaw,imatrix);

  if (!readfromfile) {
    fprintf(stderr,"Computing power-step responses. May take some time ...\n");
  }

  if (readfromfile) {
    init_ideltaw(p,&ideltaw);
  } else {
    init_ideltaw(p,NULL);
  }

  for (i=0; i<nrect; i++) {
    for (j=0; j<sensors->n; j++) {
      similar = 0;
      ii = 0;
      jj = 0;
      while ((ii<i) || (jj<j)) {
	ASSERT(ii<=i);
	if (same_influence(&p->rect[ii],&p->rect[p->sensor[jj]],&p->rect[i],&p->rect[p->sensor[j]])) {
	  similar = 1;
	  break;
	}
	jj++;
	if (jj==sensors->n) {
	  ii++;
	  jj = 0;
	}
      }
      if (similar) {
	init_imatrix(p,&p->imatrix[i][j],&p->imatrix[ii][jj],i,j);
	VERBOSE(fprintf(stderr,"Influence %d->%d = Influence %d->%d\n",i,p->sensor[j],ii,p->sensor[jj]));
      } else {
	if (readfromfile) {
	  init_imatrix(p,&p->imatrix[i][j],&imatrix[i][j],i,j);
	} else {
	  init_imatrix(p,&p->imatrix[i][j],NULL,i,j);
	}
      }

      if (is_inside(&p->rect[i],&p->rect[p->sensor[j]])) {
	p->rect[i].autotempref = &p->imatrix[i][j];
      } else {
	if ((BTSRATIO*p->imatrix[i][j].t_propag) < (p->rect[i].BIGSTEP*p->TIMESTEP)) {
	  p->rect[i].BIGSTEP = MAX(BTSMIN,BTSRATIO * p->imatrix[i][j].t_propag / p->TIMESTEP);
	  p->rect[i].tempref = &p->imatrix[i][j];
	}
      }
    }
  }

  create_atmifile(p,filename);

  for (i=0; i<nrect; i++) {
    VERBOSE(fprintf(stderr,"Rect %2d: BTS = %4d x %e\n",i,p->rect[i].BIGSTEP,p->TIMESTEP));
  }

  /* compute steady state temperature corresponding to warm-up power density */ 
  for (j=0; j<sensors->n; j++) {
    w = 0;
    for (i=0; i<nrect; i++) {
      w += q[i] * p->imatrix[i][j].wsteady;
    }
    /* fprintf(stderr,"Initial temp rect %d = %f\n",p->sensor[j],w); */
  }

  p->m = 0;
  /* warm-up system */
  apply_power_density(p,q);
  p->m = (long long) (ATMI_WARMUP_TIME / p->TIMESTEP);
  update_temperature(p);
  p->t = 0;

  free(imatrix);
}


static void freemem_influence(atmi_influence *p)
{
  ASSERT(p->wmem!=NULL);
  free(p->alreadycomp);
  free(p->wmem);
  gsl_spline_free(p->spline);
  gsl_interp_accel_free(p->acc);
  p->alreadycomp = NULL;
  p->wmem = NULL;
  p->spline = NULL;
  p->acc = NULL;
}


void atmi_simulator_freemem(atmi_simulator *p)
{
  int i,j;
  for (i=0; i<p->NRECT; i++) {
    free(p->rect[i].pq.e);
    free(p->rect[i].spq.e);
    p->rect[i].pq.e = NULL;
    p->rect[i].spq.e = NULL;
  }
  freemem_influence(&p->ideltaw);
  for (i=0; i<p->NRECT; i++) {
    for (j=0; j<p->NSENS; j++) {
      freemem_influence(&p->imatrix[i][j]);
    }
  }
  free(p->rect);
  p->rect = NULL;
}


void atmi_simulator_step(atmi_simulator *p, double q[])
{
  p->t += p->TIMESTEP;
  apply_power_density(p,q); 
  /* this was power density between t-TIMESTEP and t */
  p->m++;   /* prepare the next time step */
  update_temperature(p);
  /* now p->temperature gives temperature at time p->t */
}


void atmi_rectset_init(atmi_rectset *rs)
{
  int i;
  rs->n = 0;
  for (i=0; i<ATMI_MAX_RECT; i++) {
    rs->s[i] = 0;
  }
}


void atmi_rectset_add(atmi_rectset *rs, int rectnum)
{
  int i;
  if ((rs->n<0) || (rs->n>=ATMI_MAX_RECT)) {
    fprintf(stderr,"atmi_rectset_add: set not initialized or too big\n");
    ASSERT(0);
  }
  for (i=0; i<rs->n; i++) {
    if (rs->s[i]==rectnum) {
      fprintf(stderr,"atmi_rectset_add: rectangle %d appears twice in the set\n",rectnum);
      ASSERT(0);
    }
  }
  rs->s[rs->n] = rectnum;
  rs->n++;
}



#define THROTTLERATIO (1-USER_EPS)
#define MAXITER (1000000/USER_EPS)

void atmi_steady_throttle(atmi_param *param,
			  int nrect,
			  atmi_rect rc[],
			  atmi_rectset *sensors,
			  atmi_rectset domain[],
			  double wmax,
			  double q[])
{
  int i,j,k,maxj;
  double dw,dx,dy,maxw,w;
  DoubleArray *wij;
  long long n = 0;

  if (wmax<=0) {
    fprintf(stderr,"atmi_steady_throttle: wmax must be > 0\n");
    ASSERT(0);
  }

  for (j=0; j<sensors->n; j++) {
    atmi_rectset_check(&domain[j],nrect);
  }

  /* compute influence matrix wij ==> celsius increase at sensor j per w/m^2 in rect i */
  wij = (DoubleArray *) malloc(nrect*sizeof(DoubleArray));
  ASSERT(wij);
  for (i=0; i<nrect; i++) {
    dw = atmi_images(param,1.0*(rc[i].x2-rc[i].x1)*(rc[i].y2-rc[i].y1),0,1);
    for (j=0; j<sensors->n; j++) {
      k = sensors->s[j];
      dx = 0.5 * (rc[k].x1+rc[k].x2 - rc[i].x1-rc[i].x2);
      dy = 0.5 * (rc[k].y1+rc[k].y2 - rc[i].y1-rc[i].y2);
      wij[i][j] = dw + atmi_rectangle(param,1.0,rc[i].x2-rc[i].x1,rc[i].y2-rc[i].y1,dx,dy,0,1);
    }
  }

  while (1) {
    /* search max temp */
    maxw = 0;
    maxj = -1;
    for (j=0; j<sensors->n; j++) {
      if (domain[j].n<=0) continue;
      w = 0;
      for (i=0; i<nrect; i++) {
        w += q[i] * wij[i][j];
      }
      if (w > maxw) {
        maxw = w;
        maxj = j;
      }
    }
    if (maxj<0) {
      fprintf(stderr,"atmi_steady_throttle: domains are empty\n");
      ASSERT(0);
    }
    ASSERT(maxj<sensors->n);
    if (maxw<=wmax) {
      break;
    } else {
      /* hottest sensor throttles associated domain */
      for (i=0; i<domain[maxj].n; i++) {
	q[domain[maxj].s[i]] *= THROTTLERATIO;
      }
    }
    n++;
    if (n>MAXITER) {
      fprintf(stderr,"atmi_steady_throttle: convergence to solution is very slow\n");
      ASSERT(0);
    }
  }

  free(wij);
}
