#include "wave.h"

char *USAGE =

"USAGE: %s [options] <wave-file> ...\n"
"Mixes several wave-files, - stands for stdin. The output file is as long\n"
"as the longest input file and normalized if necessary.\n"
"Options: -n force normalized output (set maximum peak to 1)\n"
"         -s scale n input files by multiplying each one with 1/n\n"
"         -m multiply the input files instead of adding them\n"
"         -o<output file>, -h print this message, -v verbose mode\n";

wavfile win,wout;
VAL *w=0;

int main(int argc,char **argv)
{
  char c;
  int norm=0,scale=0,mult=0;
  int i,m,k;
  VAL min,max;

  setexitmsg(argv[0]);
  wout.f=stdout;
  while(1)
  {
    c=getopt(argc,argv,"nsmo:hv");
    if(stdopt(c,USAGE)) break;
    switch(c)
    {
      case 'n': norm=1;  break;
      case 's': scale=1; break;
      case 'm': mult=1;  break;
      case 'o':
        wout.f=fileopen(optarg,"wb",0);
        break;
    };
  };

  m=argc-optind;
  if(!m) exit (ERR_SYNTAX);
  wout.n=0;

  for(k=optind;k<argc;k++)
  {
    if(argv[k][0]=='-')
      wavstdin(&win);
    else
      wavfin(&win,argv[k]);
    if(k==optind)
    {
      wout.rate=win.rate;
      wout.bits=win.bits;
      wout.bytes=win.bytes;
    } else {
      if(wout.rate!=win.rate)
      {
        fprintf(stderr,"%s: Incompatible sampling rates\n",PRG);
        errexit(1);
      };
    };
    if(win.n>wout.n)
    {
      if(!(w=realloc(w,win.n*sizeof(VAL)))) errexit(ERR_MEM);
      for(i=wout.n;i<win.n;i++) w[i]=mult;
      wout.n=win.n;
    };
    if(mult)
      for(i=0;i<win.n;i++) w[i]*=wavget(&win);
    else
      for(i=0;i<win.n;i++) w[i]+=wavget(&win);
  };
  if(scale || mult)
  {
    if(scale) for(i=0;i<wout.n;i++) w[i]/=m;
  } else {
    min=max=w[0];
    for(i=1;i<wout.n;i++)
    {
      if(w[i]<min) min=w[i];
      if(w[i]>max) max=w[i];
    };
    max = fabs(min) > fabs(max) ? fabs(min) : fabs(max);
    max/=MAXAMPL;
    if(max>1 || norm)
    {
      for(i=0;i<wout.n;i++) w[i]/=max;
    };
  };
  writeheader(&wout);
  for(i=0;i<wout.n;i++) wavput(&wout,w[i]);
  free(w);

  errexit(0);
}

