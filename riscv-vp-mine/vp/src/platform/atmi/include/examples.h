
enum ColorMap {blackbody,rainbow,white};


struct rgb {
  unsigned char r,g,b;
};


double intersec(double x1, double x2, double xx1, double xx2);
void init_grid(atmi_grid g);
double read_grid(atmi_grid temp,int nx,int ny,double gridunit,double xmin,double ymin,double x,double y);
double gridmin(atmi_grid g, int nx, int ny);
double gridmax(atmi_grid g, int nx, int ny);
char myreadline(FILE *f);
int readint(FILE *f, char c);
double rgbtox(struct rgb p, enum ColorMap col);
void output_gridcolor(atmi_grid g, int nx, int ny, char minblack, enum ColorMap col);

