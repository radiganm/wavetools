#include "wave.h"
#include "font8x8.h"
#ifdef _WATCOM_
  #include <graph.h>
  #include <conio.h>
  #define G640x480x16 18
  #define TEXT -1
  #define vga_getkey() getch()
  #define vga_setcolor(c) _setcolor(c)
  #define vga_drawpixel(x,y) _setpixel(x,y)
  #define vga_drawline(x1,y1,x2,y2) {_moveto(x1,y1);_lineto(x2,y2);}
  #define vga_init()
  #define vga_setflipchar(t)
  #define vga_setmode(s) _setvideomode(s)
  #define vga_disabledriverreport()
  #define vga_clear() _clearscreen(0)
#else
  #include <vga.h>
#endif

#define VGAMODE  G640x480x16
#define XMAX     640
#define YMAX     480
#define FONT     gl_font8x8
#define CHRBYT   8
#define CHRYL    8
#define CHRXL    8
#define HEADLINE 0
#define WINX0    (5*CHRXL)
#define WINX1    (XMAX-4*CHRXL)
#define WINY0    (HEADLINE+4*CHRYL)
#define WINY1    (2*YMAX/3)
#define WINYM    (0.5*(WINY0+WINY1))
#define WINXL    (WINX1-WINX0)
#define WINYL    (WINY1-WINY0)
#define BOXX0    WINX0
#define BOXX1    WINX1
#define BOXY0    (WINY1+5*CHRYL)
#define BOXY1    (YMAX-2*CHRYL)
#define BOXYM    (0.5*(BOXY0+BOXY1))
#define BOXXL    (BOXX1-BOXX0)
#define BOXYL    (BOXY1-BOXY0)
#define STEP     0.7

#define WHITE 15
#define LGRAY 7
#define DGRAY 8
#define BLACK 0

#ifdef _WATCOM_
  #define UP    (72+256)
  #define DOWN  (80+256)
  #define LEFT  (75+256)
  #define RIGHT (77+256)
  #define INS   (82+256)
  #define DEL   (83+256)
  #define HOME  (71+256)
  #define END   (79+256)
  #define PGUP  (73+256)
  #define PGDN  (81+256)
  #define QUIT  'q'
#else
  #define UP    (65+256)
  #define DOWN  (66+256)
  #define LEFT  (68+256)
  #define RIGHT (67+256)
  #define INS   (50+256)
  #define DEL   (51+256)
  #define HOME  (49+256)
  #define END   (52+256)
  #define PGUP  (53+256)
  #define PGDN  (54+256)
  #define QUIT  'q'
#endif

#define MAXRES 16
int maxres=MAXRES;
REAL res[MAXRES+1]  = {0.001,0.002,0.005,0.01,0.02,0.05,0.1,0.2,0.5,1,2,5,10,0};


char *USAGE = 

"USAGE: %s [options] [<wave-file>]\n"
"Shows the graph of a wave-file in graphic mode. If no filename is given,\n"
"the wave-file is read from stdin.\n"
"Options: -h: print this message, -v: verbose mode\n";

char *STDIN = "-standard-input-";

wavfile w;
VAL *val;
REAL wlen;
char *nam;

void writestr(int x,int y,BYTE *s)
{
  int i,j,k,m;
  BYTE *p;

  m=1<<CHRXL;
  for(i=0;s[i] && i<80;i++)
  {
    p=FONT+s[i]*CHRBYT;
    for(j=0;j<CHRYL;j++)
      for(k=0;k<CHRXL;k++)
        if(p[j] & (m>>k)) vga_drawpixel(x+i*CHRXL+k,y+j);
  };
}

void writeheadline()
{
  char s[81];
  int x;
  
  sprintf(s,
    "%s: time %d:%05.2f s (%lu values), sampling %lu Hz, %u bits",
    nam,w.n/(60*w.rate),((float)(w.n%(60*w.rate)))/(w.rate),
    w.n,w.rate,w.bits);
  x=CHRXL*strlen(s); x = (XMAX-x)/2; if(x<0) x=0;
  writestr(x,0,s); 
}

void drawgraph(REAL t0,REAL r)
{
  int i,p,q;
  VAL x,y,s,t;
  char lab[16];

  vga_setcolor(DGRAY);
  for(i=1;i<=9;i+=2)
  {
    p=rint(WINXL*0.1*i);
    vga_drawline(WINX0+p,WINY0,WINX0+p,WINY1);
  };
  for(i=1;i<=19;i+=2)
  {
    p=rint(WINYL*0.05*i);
    vga_drawline(WINX0,WINY0+p,WINX1,WINY0+p);
  };
  vga_setcolor(LGRAY);
  s=r/10;
  for(i=0;i<=10;i+=2)
  {
    p=rint(WINXL*0.1*i);
    vga_drawline(WINX0+p,WINY0,WINX0+p,WINY1);
    t=t0+s*i;
    if(wlen<1)
    {
      t*=1000; 
      if(r<0.01)
        sprintf(lab,"%5.1lfms",t);
      else 
        sprintf(lab,"%ldms",(int)rint(t));
    } else {
      if(r<0.01)
        sprintf(lab,"%6.4lfs",t);
      else if(r<0.1)
        sprintf(lab,"%5.3lfs",t);
      else
        sprintf(lab,"%4.2lfs",t);
    };
    p-=4*strlen(lab);
    writestr(WINX0+p,WINY1+CHRYL,lab);
  };
  for(i=0;i<=20;i+=2)
  {
    p=rint(WINYL*0.05*i);
    vga_drawline(WINX0,WINY0+p,WINX1,WINY0+p);
    sprintf(lab,"%+3.1f",(VAL)1-0.1*i);
    writestr(WINX0-38,WINY0+p-3,lab);
  };
  vga_setcolor(WHITE);
  t=t0;
  p=rint(-0.5*WINYL*val[(int)rint(t*w.rate)]+WINYM);
  for(i=1;i<=WINXL;i++)
  {
    t=t0+i*r/((REAL)WINXL);
    q=rint(-0.5*WINYL*val[(int)rint(t*w.rate)]+WINYM);
    vga_drawline(WINX0+i-1,p,WINX0+i,q);
    p=q;
  };
}

void drawbox(REAL t0,REAL r)
{
  int i,p,q,c0,c1;
  VAL x,y,s,t;

  if(w.n<2*BOXXL+4) return;
  vga_setcolor(LGRAY);
  vga_drawline(BOXX0,BOXY0,BOXX0,BOXY1);
  vga_drawline(BOXX1,BOXY0,BOXX1,BOXY1);
  vga_drawline(BOXX0,BOXYM,BOXX1,BOXYM);
  vga_setcolor(DGRAY);
  s=0.4*(BOXY1-BOXY0);
  c0=rint(BOXXL*t0/wlen);
  c1=rint(BOXXL*(t0+r)/wlen);
  vga_setcolor(DGRAY);
  for(i=0;i<=BOXXL;i++)
  {
    if(i==c0) vga_setcolor(WHITE);
    p=(i*(w.n-4))/(BOXXL+1);
    vga_drawpixel(BOXX0+i,BOXYM-s*val[(int)p]);
    vga_drawpixel(BOXX0+i,BOXYM-s*val[(int)p+1]);
    vga_drawpixel(BOXX0+i,BOXYM-s*val[(int)p+2]);
    vga_drawpixel(BOXX0+i,BOXYM-s*val[(int)p+3]);
    if(i==c1) vga_setcolor(DGRAY);
  };
  vga_setcolor(WHITE);
  vga_drawline(BOXX0+c0,BOXYM+s,BOXX0+c0,BOXYM-s);
  vga_drawline(BOXX0+c1,BOXYM+s,BOXX0+c1,BOXYM-s);
}    
    

int getkey()
{
  int c=0;
  
#ifdef _WATCOM_  
  c=vga_getkey();
  if(c==0) c=vga_getkey()+256;
  return c;
#else
  while(c==0) c=vga_getkey();
  if(c!=27) return c;
  if((c=vga_getkey())!=91) return c;
  return vga_getkey()+256;
#endif
}

void domenu(int actres)
{
  int key;
  int r=actres,r0=-1;
  REAL t=0,t0=-1;
  
  while(1)
  {
    writeheadline();
    drawgraph(t,res[r]);
    drawbox(t,res[r]);
    r0=r; t0=t;
    while(r==r0 && t==t0)
    {
      key=getkey();
      if(key==QUIT) return;
      switch(key)
      {
        case UP   : r = r==0 ? 0 : r-1;
                    t=rint(5*t/res[r])*0.2*res[r]; break;
        case DOWN : r = r==maxres ? maxres : r+1; 
                    if(wlen<t+res[r]) t=wlen-res[r];
                    break;
        case LEFT : t=floor(5*(t-res[r]*STEP)/res[r])*0.2*res[r]; 
                    if(t<0) t=0; break;
        case RIGHT: t=ceil(5*(t+res[r]*STEP)/res[r])*0.2*res[r]; 
                    if(t+res[r]>wlen) t=wlen-res[r]; break;
        case HOME : t=0; break;
        case END  : t=wlen-res[r]; break;
        case PGUP : r = r<=3 ? 0 : r-3;
                    t=rint(5*t/res[r0])*0.2*res[r]; break;
        case PGDN : r = r>=maxres-3 ? maxres : r+3; 
                    if(wlen<t+res[r]) t=wlen-res[r];
      };
    };
    vga_clear();
  }; 
}

int main(int argc,char **argv)
{
  int i,j,x,y,n;
  char c;
  REAL t;
  int actres,key;

  setexitmsg(argv[0]);

  while(1)
  {
    c=getopt(argc,argv,"hv");
    if(stdopt(c,USAGE)) break;
  };
  if(argc==optind)
  {
    nam=STDIN;
    wavstdin(&w);
  } else {
    nam=argv[optind];
    wavfin(&w,nam);
  };

  if(!(val=malloc(w.n*sizeof(VAL)))) errexit(ERR_MEM);
  for(i=0;i<w.n;i++) val[i]=wavget(&w);

  wlen=w.n/((REAL)w.rate);

  for(maxres=0;maxres<MAXRES;maxres++)
    if(res[maxres]>=wlen) break;
  res[maxres]=wlen;
  t=0; actres=maxres;

  if(!verbose) vga_disabledriverreport();
  vga_init();
  vga_setflipchar('t');
  vga_setmode(VGAMODE);

  domenu(actres);

  vga_setmode(TEXT);
  free(val);

  errexit(0);
}

