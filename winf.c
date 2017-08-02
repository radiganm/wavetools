#include "wave.h"

char *USAGE = 

"USAGE: %s [options] [<wave-file>] ...\n"
"Displays header information of wave-files. If no filename or - is given,\n"
"the wave-file is read from stdin.\n"
"Options: -s show file statistics\n"
"         -h print this message, -v verbose mode\n";

char *STDIN = "-standard-input-";

void showstat(wavfile *w)
{
  VAL x,y=0.,z=0.,step;
  VAL min=1.,max=-1.,minstep=2.,maxstep=-2.;
  REAL s,mid=0.,sdev=0.,midstep=0.;
  LONG zero=0,extr=0;
  REAL fzero,fextr;

  while(w->i<w->n)
  {
    x=wavget(w);
    step=x-y;
    if(min>x) min=x;
    if(max<x) max=x;
    mid+=x;
    sdev+=x*x;
    if(minstep>step && w->i>1) minstep=step;
    if(maxstep<step && w->i>1) maxstep=step;
    midstep+=fabs(step);
    if((y<0. && x>=0.) || (y>0. && x<=0.)) zero++;
    if((z<y && y>x) || (z>y && y<x)) extr++;
    if(x!=y) z=y;
    y=x;
  };
  mid/=w->n;
  midstep/=w->n;
  sdev/=w->n;
  sdev=sqrt(fabs(mid*mid-sdev));
  s=w->n/(REAL)w->rate;
  fzero=zero/s;
  fextr=extr/s;

  printf("  Amplitude (x[n])  : min = %+7.5f  mid = %+7.5f  max = %+7.5f\n",
         min,mid,max);
  printf("  Step (x[n+1]-x[n]): min = %+7.5f  mid = %+7.5f  max = %+7.5f\n",
         minstep,midstep,maxstep);
  printf("  Zero Values       : n =%8lu  n/sec =%8.1f\n",zero,fzero);
  printf("  Extreme Values    : n =%8lu  n/sec =%8.1f\n",extr,fextr);
  printf("  Standard Deviation (Volume): s = %7.5f =%5.1f %%\n",
         sdev,sdev*100.*sqrt(2));
}

int main(int argc,char **argv)
{
  int i,n;
  wavfile w;
  char c;
  int stat=0;
  char *nam;
  LONG j; 

  setexitmsg(argv[0]);
  while(1)
  {
    c=getopt(argc,argv,"shv");
    if(stdopt(c,USAGE)) break;
    if(c=='s') stat=1;
  };
  n=argc-optind;
  if(n==0) n=1;
  for(i=0;i<n;i++)
  {
    if(argc-optind==0 || argv[optind+i][0]=='-')
    {
      nam=STDIN;
      wavstdin(&w);
    } else {
      nam=argv[optind+i];
      wavfin(&w,nam);
    };
    printf(
      "%-20s: time%2d:%05.2f s (%7lu values), sampling%6lu Hz%3u bits\n",
      nam,w.n/(60*w.rate),((float)(w.n%(60*w.rate)))/(w.rate),
      w.n,w.rate,w.bits);
    if(stat) 
    {
      showstat(&w);
    } else {
      if(nam==STDIN && i!=n-1)
      {
        while(w.i<w.n) wavbinget(&w);
      };
    };
  };
  errexit(0);
};
                                    
