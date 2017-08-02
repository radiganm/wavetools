#include "wave.h"
#include "defaults.h"

char *USAGE =

"USAGE %s [options] <frequency> <duration>\n"
"Generates a wave-file of a periodic function (default: sin).\n"
"<frequency> basic frequency in Hz or kHz\n"
"<duration> length of wave-file in s or ms\n"
"Options: -s<sample rate> in Hz, kHz or f (11025 Hz) (default " DEFRATESTR ")\n"
"         -b<sample depth> sample resolution in bits (default " DEFBITSSTR ")\n"
"         -a<amplitude> max. amplitude (default 1)\n"
"         -p<phase> phase in %%, rad or deg (default 0)\n"
"         -r rectangular wave, -t triangular wave, -w saw-tooth wave, -n noise\n"
"         -i<width> rectangular impulses, width in s, ms or %% of period\n"
"         -o<output file>, -h print this message, -v verbose mode\n";

wavfile w;
REAL pulsewidth;

VAL fct_sin(VAL x)
{
  return sin(2*PI*x);
}

VAL fct_rect(VAL x)
{
  return x>0.5 ? 1. : -1.;
}

VAL fct_tri(VAL x)
{
  if(x<0.25) return 4*x;
  if(x<0.75) return 2-4*x;
  return 4*x-4;
}

VAL fct_saw(VAL x)
{
  return x>0.5 ? 2*x-2 : 2*x;
}

VAL fct_noise(VAL x)
{
  return (VAL)(random()%20000)*0.0001-1.;
};

VAL fct_imp(VAL x)
{
  return x>1-pulsewidth ? 1. : 0.;
}


int main(int argc,char **argv)
{
  int i,n,iu=0;
  REAL t,s,x,p=0,a=MAXAMPL,pw=0.;
  REAL l=0,f=0;
  VAL (*fct)(VAL);
  char c;

  setexitmsg(argv[0]);
  w.f=stdout;
  w.rate=DEFRATE;
  w.bits=DEFBITS;
  w.bytes=DEFBYTES;
  fct=&fct_sin;
  
  while(1)
  {
    c=getopt(argc,argv,"s:b:o:p:a:rtwni:hv");
    if(stdopt(c,USAGE)) break;
    switch(c)
    {
      case 's':
        x=getfreq(optarg);
        if(x<0)
        {
          fprintf(stderr,"%s: Invalid sampling rate\n",PRG);
          errexit(1);
        };
        w.rate=rint(x);
        break;
      case 'b':
        getparam(optarg,&x,U_NU);
        if(x<1 || x>32)
        {
          fprintf(stderr,"%s: Invalid sampling depth\n",PRG);
          errexit(1);
        };
        w.bits=rint(x);
        w.bytes=(w.bits+7)/8;
        break;
      case 'o':
        w.f=fileopen(optarg,"wb",0);
        break;
      case 'p':
        getparam(optarg,&p,U_NU|U_GAIN|U_PHASE);
        break;
      case 'a':
        getparam(optarg,&a,U_NU|U_GAIN);
        if(a<0 || a>1) 
        {
          fprintf(stderr,"%s: Invalid amplitude\n",PRG);
          errexit(1);
        };
        break;
      case 'r': fct=&fct_rect;  break;
      case 't': fct=&fct_tri;   break;
      case 'w': fct=&fct_saw;   break;
      case 'n': fct=&fct_noise; break;
      case 'i':
        iu=getparam(optarg,&pulsewidth,U_NU|U_GAIN|U_SEC); 
        fct=&fct_imp;
        break;
    };
  };
  
  if(argc-optind!=2) errexit(ERR_SYNTAX);
  for(i=optind;i<argc;i++)
  {
    n=getparam(argv[i],&x,U_HZ|U_SEC);
    if(n==U_HZ) f=x; else l=x;
  };
  if(f<=0 || l<=0) errexit(ERR_SYNTAX);
  if(iu==U_SEC) pulsewidth*=f;

  w.n=rint(w.rate*l);
  writeheader(&w);

  s=f/w.rate;
  for(i=0;i<w.n;i++)
  {
    wavput(&w,a*((*fct)(fmod(i*s+p,1.))));
  }; 

  errexit(0);
}
    

