#include "wave.h"

char *USAGE =

"USAGE: %s [options] [<wave-file>|+<pause>] ...\n"
"Concatenates several wave-files (*.wav) and performs conversions of\n"
"sample rates and sample depths if necessary. If no filename or - is\n"
"given, the wave-file is read from stdin.\n"
"<pause> insert silence, in s or ms (eg. +500ms)\n"
"Options: -r<sample rate> in Hz, kHz or f (f=11025 Hz)\n"
"         -b<sample depth> sample resolution in bits\n"
"         -s<speed> factor for replay (100%% is normal speed)\n"
"         -o<output file>, -h print this message, -v verbose mode\n";

#define WMAX 16

wavfile wav[WMAX];
REAL p[WMAX];
wavfile ref;

void catfile(wavfile *w)
{
  LONG m,j,l;
  REAL p0,p1,s;
  VAL x,y,z;

  if(w->bytes==ref.bytes && w->rate==ref.rate)
  {
    while(w->i<w->n)
      wavbinput(&ref,wavbinget(w));
  }
  else
  {
    m=rint((REAL)w->n*(((REAL)ref.rate)/w->rate));
    s=((REAL)w->rate)/ref.rate;
    x=0.;
    if(w->rate>=ref.rate)
    {
      for(j=0;j<m;j++)
      {
        p0=s*j;
        p1=p0+s;
        if(p0>=w->i)
        {
          x=wavget(w);
        };
        z=(w->i-p0)*x;
        while(p1>=w->i && w->i<w->n)
        {
          x=wavget(w);
          z+=x;
        };
        z-=(w->i-p1)*x;
	z/=s;
        wavput(&ref,z); 
      };
    } else {
      p1=0.;
      for(j=0;j<m;j++)
      {
        p1+=s;
        if(p1>w->i) 
        {
          y=x;
          x=w->i==w->n ? 0. : wavget(w);
        };
        z=x+(y-x)*(w->i-p1);
        wavput(&ref,z);
      };
    };
  };
}

int main(int argc,char **argv)
{
  int i,n;
  LONG j;
  REAL x,speed=1.;
  int rate=0,bits=0,def=0;
  int u,c;

  setexitmsg(argv[0]);
  ref.f=stdout;
  for(i=0;i<WMAX;i++) wav[i].f=NULL;
  while(1)
  {
    c=getopt(argc,argv,"r:b:s:o:hv");
    if(stdopt(c,USAGE)) break;
    switch(c)
    {
      case 'r':
        x=getfreq(optarg);
        if(x<0)
        {
          fprintf(stderr,"%s: Invalid sampling rate\n",PRG);
          errexit(1);
        };
        rate=1;
        ref.rate=rint(x);
        break;
      case 'b':
        u=getparam(optarg,&x,U_NU);
        if(x<1 || x>32)
        {
          fprintf(stderr,"%s: Invalid sampling depth\n",PRG);
          errexit(1);
        };
        bits=1;
        ref.bits=rint(x);
        ref.bytes=(ref.bits+7)/8;
        break;
      case 's':
        getparam(optarg,&speed,U_NU|U_GAIN); 
        if(speed<=0) errexit(ERR_PARAM);
        break;
      case 'o':
        ref.f=fileopen(optarg,"wb",0);
        break;
    };
  };
  
  n=argc-optind;
  if(n==0)
  {
    n=1;
    wav[0].f=stdin;
    readheader(wav);
  } else {
    for(i=0;i<n;i++)
    {
      if(argv[optind+i][0]=='+')
      {
        if((x=gettime(argv[optind+i]))<0) errexit(ERR_TIME);
        wav[i].f=NULL;
        p[i]=x;
      } else if(argv[optind+i][0]=='-') {
        wavstdin(&(wav[i]));
      } else {
        wavfin(&(wav[i]),argv[optind+i]);
      };
    };
  };
  
  for(i=0;i<n && !def;i++)
  {
    if(wav[i].f==NULL) continue;
    if(!rate) 
    {
      ref.rate=wav[i].rate;
      rate=1;
    };
    if(!bits) 
    {
      ref.bits=wav[i].bits;
      ref.bytes=wav[i].bits>8?2:1;
      bits=1;
    };
    def=rate&&bits;
  };
  if(!def)
  {
    fprintf(stderr,"%s: Unspecified output-format\n",PRG);
    errexit(ERR_PREV);
  };

  ref.n=ref.i=0;
  for(i=0;i<n;i++)
  {
    if(wav[i].f==NULL)
    {
      ref.n+=(wav[i].n=rint(p[i]*ref.rate));
    } else {
      wav[i].rate=rint(wav[i].rate*speed);
      ref.n+=rint((REAL)wav[i].n*(((REAL)ref.rate)/wav[i].rate));
    };
  };

  writeheader(&ref);
  for(i=0;i<n;i++)
  {
    if(wav[i].f==NULL)
    {
      for(j=0;j<wav[i].n;j++) wavput(&ref,0);
    } else {
      catfile(&(wav[i]));
    };
  };

  errexit(0);
}
    

