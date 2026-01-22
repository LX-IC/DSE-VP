
/*
Miscellaneous functions used in examples
*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "/home/x/riscv-vp-mine/vp/src/platform/atmi/include/atmi.h"
#include "/home/x/riscv-vp-mine/vp/src/platform/atmi/include/examples.h"


#define ASSERT(cond) if (!(cond)) {fprintf(stderr,"assert line %d\n",__LINE__); exit(1);}


static void fswap(double *x, double *y) 
{
  double temp;
  temp = *x;
  *x = *y;
  *y = temp;
}


double intersec(double x1, double x2, double xx1, double xx2)
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


void init_grid(atmi_grid g)
{
  int i,j;
  for (i=0; i<ATMI_GRIDMAX; i++) {
    for (j=0; j<ATMI_GRIDMAX; j++) {
      g[i][j] = 0;
    }
  }
}


double read_grid(atmi_grid temp, 
		 int nx,
		 int ny,
		 double gridunit,
		 double xmin,
		 double ymin,
		 double x,
		 double y)
{
  int i,j;
  i = (x-xmin)/gridunit;
  j = (y-ymin)/gridunit;
  ASSERT(i<nx);
  ASSERT(j<ny);
  return temp[i][j];
}


static struct rgb xtorgb_blackbody(double x)
{
  struct rgb p;
  ASSERT((x>=0) && (x<=1));
  if (x < (1./3)) {
    p.r = 255 * 3 * x;
    p.g = p.b = 0;
  } else if (x < (2./3)) {
    p.r = 255;
    p.g = 255 * 3 * (x-1./3);
    p.b = 0;
  } else {
    p.r = p.g = 255;
    p.b = 255 * 3 * (x-2./3);
  }
  return p;
}


static struct rgb xtorgb_rainbow(double x)
{
  struct rgb p;
  ASSERT((x>=0) && (x<=1));
  if (x < (1./3)) {
    p.r = 0;
    p.g = 255 * 3 * x;
    p.b = 255;
  } else if (x < (2./3)) {
    p.r = 255 * 3 * (x-1./3);
    p.g = 255;
    p.b = 255 * 3 * (2./3-x);
  } else {
    p.r = 255;
    p.g = 255 * 3 * (1-x);
    p.b = 0;
  }
  return p;
}


static struct rgb xtorgb_white(double x)
{
  struct rgb p;
  ASSERT((x>=0) && (x<=1));
  p.r = 255 * x;
  p.g = 255 * x;
  p.b = 255 * x;
  return p;
}



static int similarity(struct rgb p1,struct rgb p2)
{
  int s;
  s = abs((int)(p1.r)-(int)(p2.r));
  s += abs((int)(p1.g)-(int)(p2.g));
  s += abs((int)(p1.b)-(int)(p2.b));;
  return s;
}


double rgbtox(struct rgb p, enum ColorMap col)
{
  double x,y;
  struct rgb q;
  int minsim = 3*256;
  x = 0;
  for (y=0; y<=1; y+=0.001) {
    switch (col) {
    case blackbody: q = xtorgb_blackbody(y); break; 
    case rainbow:   q = xtorgb_rainbow(y); break; 
    case white:     q = xtorgb_white(y); break; 
    default: ASSERT(0);
    }
    if (similarity(q,p) < minsim) {
      minsim = similarity(q,p);
      x = y;
    }
  }
  return x;
}


char myreadline(FILE *f)
{
  char c;
  int i = 0;
  do {
    fscanf(f,"%c",&c);
    i++;
    ASSERT(i<1000000);
  } while ((c!=EOF) && (c!='\n'));
  return c;
}


int readint(FILE *f, char c)
{
  int m,n,a;
  fscanf(f,"%d ",&m);
  if ((c>='0') && (c<='9')) {
    a = m;
    n = c - '0';
    while (a > 0) {
      a = a/10;
      n *= 10;
    }
    ASSERT(m < n);
    n += m;
  } else {
    n = m;
  }
  return n;
}


double gridmin(atmi_grid g, int nx, int ny)
{
  int i,j;
  double gmin = g[0][0];
  for (i=0; i<nx; i++) {
    for (j=0; j<ny; j++) {
      if (g[i][j] < gmin) {
        gmin = g[i][j];
      }
    }
  }
  return gmin;
}


double gridmax(atmi_grid g, int nx, int ny)
{
  int i,j;
  double gmax = g[0][0];
  for (i=0; i<nx; i++) {
    for (j=0; j<ny; j++) {
      if (g[i][j] > gmax) {
        gmax = g[i][j];
      }
    }
  }
  return gmax;
}



void output_gridcolor(atmi_grid g, int nx, int ny, char minblack, enum ColorMap col)
{
  int i,j;
  struct rgb p;
  double maxval,minval,x;
  maxval = gridmax(g,nx,ny);
  minval = (minblack)? gridmin(g,nx,ny) : 0;
  ASSERT(maxval > minval);
  printf("P6 %d %d %d\n",nx,ny,255);
  for (j=0; j<ny; j++) {
    for (i=0; i<nx; i++) {
      x = (g[i][j]-minval)/(1e-12+maxval-minval);
      switch (col) {
      case blackbody: p = xtorgb_blackbody(x); break;
      case rainbow:   p = xtorgb_rainbow(x); break;
      case white:     p = xtorgb_white(x); break;
      default: ASSERT(0);
      }
      printf("%c%c%c",p.r,p.g,p.b);
    }
  }
}


