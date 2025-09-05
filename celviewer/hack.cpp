#include <vcl.h>
#include "hack.h"
//#include "USSR_std.h"
#include <io.h>
#include <fcntl.h>
#include "std_primitives.h"



AnsiString conv_to_hex(int num)
{
 int i;
 AnsiString dop="00000000";
 for(i=0;i<8;i++)
 {
  if(((num>>i)&0xf)<10)(dop.c_str())[7-i]='0'+((num>>i)&0xf);
  else (dop.c_str())[7-i]='A'+((num>>i)&0xf)-10;
 }
 return dop;
}

AnsiString get_ansi_intz(int num, int  max)
{
 int i;
 AnsiString d1=IntToStr(num),dm=IntToStr(max);
 for(i=0;i<dm.Length();i++)
  dm.c_str()[i]='0';
 for(i=0;i<d1.Length();i++)
  dm.c_str()[dm.Length()-d1.Length()+i]=d1.c_str()[i];
 return dm;
}

int big_test(int hand, unsigned int off, unsigned int fsize, hackstr *hack_pak)
{
int i,j,k;
unsigned int tmp,tsth[3],lastpos=0;
USSR_CSDinamic_Array<archdata> *archtmp;
archdata temp;
 archtmp=new USSR_CSDinamic_Array< archdata>;
 lseek(hand,off,SEEK_SET);
 _rtl_read(hand,&tmp,4);
 tmp=uint_to_bigend(tmp);
 if(tmp==0)goto losehack;
 if((((tmp)*3+4)*4)>fsize)goto losehack;
 _rtl_read(hand,tsth,12);
 if(tsth[0]!=0 || tsth[1]!=0 || tsth[2]!=0) goto losehack;

 temp.off=off;
 temp.size=(((tmp)*3+4)*4);
 temp.type=ARCH_BIG_HEAD;
 temp.head_size=0;
 temp.name=hack_pak->pfname+".bigh";
 temp.tolog="BIG file consist of "+IntToStr(tmp)+"files";
 archtmp->push(temp);
 for(i=0;i<tmp;i++)
 {
  k=_rtl_read(hand,tsth,12);
  if(k!=12)goto losehack;
  for(j=0;j<3;j++)tsth[j]=uint_to_bigend(tsth[j]);
  if(tsth[2]<lastpos)goto losehack;
  if((tsth[1]+tsth[2])>fsize)goto losehack;
  temp.off=tsth[2]+off;
  temp.size=tsth[1];
  temp.type=ARCH_BIG_ELEMENT;
  temp.head_size=0;
  temp.name=get_ansi_intz(i,tmp)+".big";
  temp.tolog="Extracted file from BIG archive, N"+IntToStr(i)+", header ID 0x"+conv_to_hex(tsth[0]);
  archtmp->push(temp);
 }

 hack_pak->maintype=HACK_MAINT_BIG;
 for(i=0;i<archtmp->size;i++)
 {
  hack_pak->arch.push(archtmp->operator [](i));
 }

 delete archtmp;
 return 1;

losehack:
 delete archtmp;
 return 0;
}

void save_action(int hand, archdata *hack_el, AnsiString dirr)
{
 int i=2048,handto,k,tmp;
 unsigned char buf[2048];
  if((lseek(hand,hack_el->off,SEEK_SET))==-1)return;
  if((handto=_rtl_creat((dirr+hack_el->name).c_str(),0))==-1)return;
  tmp=hack_el->size;
  while(i==2048)
  {
   if(tmp>2048)k=2048;
   else k=tmp;
   i=_rtl_read(hand,buf,k);
   _rtl_write(handto,buf,i);
   tmp-=i;
  }
  _rtl_close(handto);
}

hackstr hack_pak_explore;

void hack_reset()
{
  int i;
  for(i=0;i<hack_pak_explore.arch.size;i++)
  {
    if(hack_pak_explore.arch[i].arch!=NULL)
    {
     delete hack_pak_explore.arch[i].arch;
     hack_pak_explore.arch[i].arch=NULL;
    }
  }
  hack_pak_explore.arch.clear();
  hack_pak_explore.maintype=-1;
  hack_pak_explore.flags=0;
  hack_pak_explore.pfname="";
  hack_pak_explore.extructble=0;
}

int hack_inter_test(int hand, int off, int fsize,hackstr *hack_pak_explore)
{
  if(big_test(hand, off, fsize, hack_pak_explore)!=0)
   {hack_pak_explore->extructble=1;hack_pak_explore->maintype=HACK_MAINT_BIG; return 1;}
  return 0;
}

int hack_test_file(AnsiString ftotest)
{
  int fsize,hand,tmp,i,j;
  if( (hand=_rtl_open(ftotest.c_str(), O_RDONLY))==-1)return 1;
  fsize=lseek(hand,0,SEEK_END);
  lseek(hand,0,SEEK_SET);

  hack_pak_explore.arch.clear();
  hack_pak_explore.maintype=-1;
  hack_pak_explore.flags=0;
  hack_pak_explore.extructble=0;
  hack_pak_explore.pfname=ExtractFileName(ftotest);

  if(hack_inter_test(hand, 0, fsize, &hack_pak_explore)!=0){goto winhack;}

  _rtl_close(hand);
  return 0;
winhack:
  hack_pak_explore.flags|=1;
  _rtl_close(hand);
  return 1;
}

void hack_extract(AnsiString fname, hackstr *hack_pak)
{
 int i,hand;
 AnsiString newdirname;
 if(hack_pak->maintype==-1 || hack_pak->flags==0) return;

 if(hack_pak->extructble==0)return;

 if( (hand=_rtl_open(fname.c_str(), O_RDONLY))==-1)return;
 newdirname=ExtractFileDir(fname)+"\\ext_"+ExtractFileName(fname);
 CreateDir(newdirname);
 newdirname+="\\";
 for(i=0;i<hack_pak->arch.size;i++)
     save_action(hand, &hack_pak->arch[i],newdirname);
 _rtl_close(hand);
}
