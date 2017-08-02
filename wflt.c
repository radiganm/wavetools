#include "wave.h"

char *USAGE =

"USAGE: %s [options] [<wave-file>]\n"
"Performs several transformations on a wave-file. If no filename is\n"
"specified, input is read from stdin. Parameters -m, -l, -h -b and -w can\n"
"be defined as time, frequency and number of values. Transformations are\n"
"applied in the following order (omega=2*pi*f):\n"
"Options: -m<width> mean value filter (y[n]=1/w*(x[n-w/2] + ... + x[n+w/2]))\n"
"         -l<frequency> low pass filter (dy/dt=omega*(x-y))\n"
"         -f<frequency> high pass filter (omega*y=dx/dt-dy/dt)\n"
"         -b<frequency> band pass filter (d^2/dt^2*y+4pi*w*dy/dt+omega^2*y=x)\n"
"         -w<frequency> band width (only with option -b, default f/2\n"
"         -g<gain> amplify signal (y=g*x)\n"
"         -a<bias> add bias (y=x+b), -c center output (avg(y)=0)\n"
"         -n normalize output (min(y)=-1, max(y)=1)\n"
"         -r reverse output (satanic mode), -i invert output (y[n]=-x[n])\n"
"         -o<output file>, -h print this message, -v verbose mode\n";

int main(int argc,char **argv)
{
  wavfile w,o;
  LONGINT m,p,p0;
  LONG i,j;
  VAL s,x0,y0,v;
  REAL mv,l,h,f,d,g=1.,b=0.,min,mid,max;
  int mu=0,lu=0,hu=0,fu=0,du=0,gu=0,bu=0,norm=0,center=0,rev=0,inv=0;
  char c;
  VAL *x,*mx;

  setexitmsg(argv[0]);
  o.f=stdout;
  while(1)
  {
    c=getopt(argc,argv,"m:l:f:b:w:g:a:cnrio:hv");
    if(stdopt(c,USAGE)) break;
    switch(c)
    {
      case 'm': mu=getparam(optarg,&mv,U_NU|U_SEC|U_HZ); break;
      case 'l': lu=getparam(optarg,&l,U_NU|U_SEC|U_HZ); break;
      case 'f': hu=getparam(optarg,&h,U_NU|U_SEC|U_HZ); break;
      case 'b': fu=getparam(optarg,&f,U_NU|U_SEC|U_HZ); break;
      case 'w': du=getparam(optarg,&d,U_NU|U_SEC|U_HZ); break;
      case 'g': gu=getparam(optarg,&g,U_NU|U_GAIN); break;
      case 'a': bu=getparam(optarg,&b,U_NU|U_GAIN); break;
      case 'c': center=1; break;
      case 'n': norm=1;   break;
      case 'r': rev=1;    break;
      case 'i': inv=1;    break;
      case 'o': o.f=fileopen(optarg,"wb",0);
    };
  };

  if(gu || bu)
  {
    if(norm)
    {
      fprintf(stderr,
              "%s: option -n cannot e used together with -g or -b\n",PRG);
      errexit(ERR_PREV);
    };
    if(fabs(b)>1) errexit(ERR_PARAM);
  };
  if(optind<argc-1) errexit(ERR_ARG);
  
  if(optind<argc)
    wavfin(&w,argv[optind]);
  else
    wavstdin(&w);

  if(!(x=malloc(w.n*sizeof(VAL)))) errexit(ERR_MEM);

  for(i=0;i<w.n;i++) x[i]=wavget(&w);

  if(mu)
  {
    if(mu==U_SEC) mv*=w.rate;
    if(mu==U_HZ) mv=w.rate/mv;
    m=rint(mv);
    if(m<=1) mu=0;
  };

  if(mu)
  {
    if(!(mx=malloc(m*sizeof(VAL)))) errexit(ERR_MEM);
    p0=m/2;
    for(j=0;j<m;j++) mx[j]=0.;
    for(j=0;j<p0;j++) mx[j]=x[j];
    p=p0;
    for(i=0;i<w.n;i++)
    {
      mx[p]=p0<w.n ? x[p0] : 0.;
      s=0.;
      for(j=0;j<m;j++) s+=mx[j];
      x[i]=s/m;
      p++;p0++;
      p%=m;
    };
    free(mx);
  };

  if(lu)
  {
    if(lu==U_SEC) l=1./l;
    if(lu==U_NU) l=w.rate/l;
    l*=0.25*2*PI/w.rate;
    y0=0.;x0=0.;
    for(i=0;i<w.n;i++)
    {
      y0+=l*(0.75*x0+0.25*x[i]-y0);
      y0+=l*(0.50*x0+0.50*x[i]-y0);
      y0+=l*(0.25*x0+0.75*x[i]-y0);
      y0+=l*(             x[i]-y0);
      x0=x[i];
      x[i]=y0;
    };
  };

  if(hu)
  {
    if(hu==U_SEC) h=1./h;
    if(hu==U_NU) h=w.rate/h;
    h*=0.25*2*PI/w.rate;
    y0=0.;x0=0.;
    for(i=0;i<w.n;i++)
    {
      s=0.25*(x[i]-x0);
      y0+=s-h*y0;
      y0+=s-h*y0;
      y0+=s-h*y0;
      y0+=s-h*y0;
      x0=x[i];
      x[i]=y0;
    };
  };

  if(fu)
  {
    if(fu==U_SEC) f=1./f;
    if(fu==U_NU) f=w.rate/f;
    switch(du)
    {
      case 0: d=f/2; break;
      case U_SEC: d=1./d; break;
      case U_NU: d=w.rate/d;
    };
    if(f<0.5001*d)
    {
      fprintf(stderr,"%s: illegal band width\n",PRG);
      errexit(1);
    };
    f=0.25*2*PI*f/w.rate;
    d=0.25*4*PI*d/w.rate;
    s=d*sqrt(f*f-0.0625*d*d);
    f=f*f;
    y0=x0=v=0.;
    for(i=0;i<w.n;i++)
    {
      v+=-f*y0-d*v+0.75*x0+0.25*x[i]; y0+=v;
      v+=-f*y0-d*v+0.50*x0+0.50*x[i]; y0+=v;
      v+=-f*y0-d*v+0.25*x0+0.75*x[i]; y0+=v;
      v+=-f*y0-d*v+             x[i]; y0+=v;
      x0=x[i];
      x[i]=s*y0;
    };
  };
     

  if(norm|center)
  {  
    min=max=mid=x[0];
    for(i=1;i<w.n;i++)
    {
      if(x[i]<min) min=x[i];
      if(x[i]>max) max=x[i];
      mid+=x[i];
    };
    mid/=w.n;
    if(center)
    {
      b=-mid;
      if(norm) g=MAXAMPL/(max-mid>mid-min ? max-mid : mid-min);
    } else {
      b=-(max+min)/2.;
      g=MAXAMPL*2/(max-min);
    };
  };

  if(inv)
  {
    g=-g; b=-b;
  };
  
  if(gu||bu||norm||inv)
  {
    for(i=0;i<w.n;i++) x[i]=g*(x[i]+b);
  };

  o.bits=w.bits;
  o.bytes=w.bytes;
  o.rate=w.rate;
  o.n=w.n;
  writeheader(&o);

  if(rev)
  {
    while(o.i<o.n) wavput(&o,x[o.n-(o.i+1)]);
  } else {
    while(o.i<o.n) wavput(&o,x[o.i]);
  };

  free(x);

  errexit(0);
};

