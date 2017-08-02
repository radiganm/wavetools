#include "wave.h"

char *USAGE =

"USAGE: %s [options] [<wave-file>]\n"
"Cuts a section out of a wave-file. If no wave-file is specified, it\n"
"is read from stdin.\n"
"Options: -s<start time>, -e<end time>, -l<length> in s, ms or values\n"
"         -o<output file>, -h print this message, -v verbose mode\n";

int main(int argc,char **argv)
{
  wavfile w,o;
  LONGINT s,e,l;
  REAL sv,ev,lv;
  int su=0,eu=0,lu=0;
  char c;

  setexitmsg(argv[0]);
  o.f=stdout;
  while(1)
  {
    c=getopt(argc,argv,"s:e:l:o:hv");
    if(stdopt(c,USAGE)) break;
    switch(c)
    {
      case 's': su=getparam(optarg,&sv,U_NU|U_SEC); break;
      case 'e': eu=getparam(optarg,&ev,U_NU|U_SEC); break;
      case 'l': lu=getparam(optarg,&lv,U_NU|U_SEC); break;
      case 'o': o.f=fileopen(optarg,"wb",0);
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

  if(verbose) 
    fprintf(stderr,
       "source has %lu values, "
       "cut from pos. %ld to pos. %ld, time: %d:%05.2f s\n",
       w.n,s,e,(e-s)/(60*w.rate),((float)((e-s)%(60*w.rate)))/(w.rate));

  o.bits=w.bits;
  o.bytes=w.bytes;
  o.rate=w.rate;
  o.n=e-s;
  writeheader(&o);

  while(w.i<s) wavbinget(&w);
  while(o.i<o.n) wavbinput(&o,wavbinget(&w));

  errexit(0);
};

