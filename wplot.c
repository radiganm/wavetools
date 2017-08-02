#include "wave.h"
#include "psheader.h"
#include "psplot.h"
#include "defaults.h"

#define MAXPT 1000

char *USAGE =

"USAGE: %s [options] [<wave-file>]\n"
"Creates a PostScript file containing the plot of the wave-file. If no\n"
"filename is given, the wave-file is read from stdin.\n"
"Options: -s<start time>, -e<end time>, -l<length> in s, ms or values\n"
"         -t scale with start time (normaly the scale begins with t = 0 s)\n"
"         -w<width> width of figure in mm or cm, -f<size> font size in pt\n" 
"         -o<output file>, -h print this message, -v verbose mode\n";

int main(int argc,char **argv)
{
  wavfile w;
  FILE *f;
  VAL v[MAXPT];
  LONGINT i,j,k,n,s,e,l;
  REAL sv,ev,lv,t,t0,tstep,x,y,xstep,width,height;
  int su=0,eu=0,lu=0,topt=0,fsize;
  char c;
  char str[XTICKS][10];

  setexitmsg(argv[0]);
  f=stdout; 
  width=DEFPLOTWIDTH; fsize=DEFFONTSIZE;
  while(1)
  {
    c=getopt(argc,argv,"s:e:l:tw:f:o:hv");
    if(stdopt(c,USAGE)) break;
    switch(c)
    {
      case 's': su=getparam(optarg,&sv,U_NU|U_SEC); break;
      case 'e': eu=getparam(optarg,&ev,U_NU|U_SEC); break;
      case 'l': lu=getparam(optarg,&lv,U_NU|U_SEC); break;
      case 't': topt=1; break;
      case 'w': getparam(optarg,&width,U_METER); break;
      case 'f': getparam(optarg,&x,U_NU|U_PT); fsize=rint(x); break;
      case 'o': f=fileopen(optarg,"wb",0);
    };
  };
  if(optind<argc-1) errexit(ERR_ARG);

  if(optind<argc)
    wavfin(&w,argv[optind]);
  else
    wavstdin(&w);

  s=0; e=w.n;
  if(su)
  {
    if(su==U_NU) s=rint(sv); else s=rint(sv*w.rate);
  };
  if(eu)
  {
    if(eu==U_NU) e=rint(ev); else e=rint(ev*w.rate);
  };
  if(su && eu && (lu || s>=e)) errexit(ERR_TIME);
  if(lu)
  {
    if(lu==U_NU) l=rint(lv); else l=rint(lv*w.rate);
    if(eu) s=e-l; else e=s+l; 
  };
  if(s<0 || e>w.n || s>=e) errexit(ERR_TIME);
  l=e-s;

  if(verbose)
    fprintf(stderr,
       "plot from pos. %ld to pos. %ld, time: %d:%05.2f s\n",
       s,e,l/(60*w.rate),((float)(l%(60*w.rate)))/(w.rate));
 
  k=l/MAXPT+1; n=l/k; 
  for(i=0;i<s;i++) wavbinget(&w);
  for(i=0;i<n;i++)
  {
    v[i]=wavget(&w); 
    for(j=1;j<k;j++) wavbinget(&w);
  };
  if(w.f==stdin) while(w.i<w.n) wavbinget(&w);

  t0 = topt ? ((REAL)s)/w.rate : 0;
  tstep = ((REAL)l)/(w.rate*XTICKS);
  for(i=0;i<XTICKS;i++)
  {
    t=1000*(t0+(1+i)*tstep);
    if(tstep<0.019 && fmod(t+0.05,1.)>0.1)
      sprintf(str[i],"%3.1lfms",t);
    else
      sprintf(str[i],"%ldms",(int)rint(t));
  };
  width*=PSUNIT; height=width*ASPRAT;
  
  fprintf(f,PS_SIZE,width,height,width,height);
  if(!fputs(PS_HEADER,f)) errexit(ERR_WRITE); 
  fprintf(f,PS_SCALE1,fsize);

  for(i=0;i<XTICKS;i++) fprintf(f,PS_XSCALE[i],str[i]);

  if(!fputs(PS_SCALE2,f)) errexit(ERR_WRITE);

  for(i=0;i<XTICKS;i++) fprintf(f,PS_XTICKS[i],str[i]);
 
  if(!fputs(PS_GRIDS,f)) errexit(ERR_WRITE);

  xstep=(XMAX-XMIN)/(REAL)(n-1);
  for(i=0;i<n;i++)
  {
    x=XMIN+i*xstep;
    y=YMID+v[i]*AMPL;
    if(i)
      fprintf(f,"%7.5f %7.5f L\n",x,y);
    else
      fprintf(f,"%7.5f %7.5f m\n",x,y);
  };
  
  if(!fputs(PS_END,f)) errexit(ERR_WRITE); 

  errexit(0);
}

