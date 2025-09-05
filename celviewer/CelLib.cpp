/***************************************************************/
//
//      Writed by Altmer (mail: max220291@list.ru, altmer@arts-union.ru)
//      август 2005 года
//
/***************************************************************/

#include "CelLib.h"
#include "std_primitives.h"
CelImage_3DO CelIm;
const int bits_per_pixel[]={-1,1,2,4,6,8,16,-1};


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
#include "StreamLib.h"
#include "cinepak_decode.h"
_3DOStream_inf _3dosinfo;
DSHeaderChunk *ch_shdr;
CinePakHeader *film_head;
CinePakFrame *film_frm;
void initStream()
{
   _3dosinfo.flag=0;
   _3dosinfo.playf=0;
   _3dosinfo.delay=0;
   _3dosinfo.frames=100;
   _3dosinfo.cntfrm=0;

}
void *tpoint;
void closeStream()
{
  _rtl_close(_3dosinfo.hand);
  if(_3dosinfo.flag==0)return;
  decode_cinepak_free(tpoint);
  _3dosinfo.flag=0;
  _3dosinfo.playf=0;
  _3dosinfo.delay=0;
  _3dosinfo.frames=100;
  _3dosinfo.cntfrm=0;

}


int getStream(char *file)
{
 int fsize,hand,tmp,i,j,fix,position=0;
  unsigned char *fbuf;
  unsigned int *fdata,temp;
  unsigned short *fshd;
  closeStream();
  if( (hand=_rtl_open(file, O_RDONLY))==-1)return 1;
   fsize=lseek(hand,0,SEEK_END);
   lseek(hand,0,SEEK_SET);
   //if(fsize>0x100000)return 2;
   fbuf=new unsigned char[10000];
   _rtl_read(hand,fbuf,10000);
   //_rtl_close(hand);
   fdata=(unsigned int*)fbuf;


   if(fdata[0]!=0x52444853 ) //SHDR
   {
    fix=2;
    goto out_point;
   }
   ch_shdr=(DSHeaderChunk *)fbuf;
   if(uint_to_bigend(ch_shdr->headerVersion)!=2 )
   {
     fix=3;
     goto out_point;
   }
   position=uint_to_bigend(ch_shdr->chunkSize);
   if(position>fsize)
    {
        fix=4;
        goto out_point;
    }
   temp=0;
retry_1:
    lseek(hand,position,SEEK_SET);
    _rtl_read(hand,&temp,4);
    if(temp!=0x4d4c4946) //FILM
    {
     _rtl_read(hand,&temp,4);
     position+=uint_to_bigend(temp);
     if(position>=fsize || temp==0)
      {
        fix=4;
        goto out_point;
      }
     goto retry_1;
    }

   lseek(hand,position,SEEK_SET);
   _rtl_read(hand,fbuf,0x100);
   film_head= (CinePakHeader *)fbuf;
   if(film_head->subChunkType!=0x52444846) //FHDR
   {
     fix=5;
     goto out_point;
   }
   if(film_head->version!=0 || film_head->cType!=0x64697663) //cvid
   {
     fix=6;     //uncnown type compression
     goto out_point;
   }

   CelIm.y_res=uint_to_bigend(film_head->height);
   CelIm.x_res=uint_to_bigend(film_head->width);
   CelIm.image_buf=new unsigned int[CelIm.y_res*CelIm.x_res];
   
   //tpoint=decode_cinepak_init();
   _3dosinfo.flag=1;
   _3dosinfo.hand=hand;
   _3dosinfo.fpos=position;//+uint_to_bigend(film_head->chunkSize);
   _3dosinfo.curr_pos=position;//+uint_to_bigend(film_head->chunkSize);
   _3dosinfo.file_size=fsize;
   _3dosinfo.ctype=0;
   _3dosinfo.frames=uint_to_bigend(film_head->count);
   _3dosinfo.cntfrm=0;
   _3dosinfo.playf=0;
   _3dosinfo.delay=0;

   tpoint=decode_cinepak_init();
   if(get_frame_from_Stream()!=0){fix=7;goto out_point;}


   delete []fbuf;
 return 0;
 out_point:
  _rtl_close(hand);
  delete []fbuf;
  return fix;
}

int get_frame_from_Stream()
{
 unsigned char*fbuf;
 int temp,flag=0;
 if(_3dosinfo.flag==0)return 1;
    lseek(_3dosinfo.hand,_3dosinfo.curr_pos,SEEK_SET);
    _rtl_read(_3dosinfo.hand,&temp,4);
    _rtl_read(_3dosinfo.hand,&temp,4);
    _3dosinfo.curr_pos+=uint_to_bigend(temp);
 retry_1:
    if(_3dosinfo.curr_pos>=_3dosinfo.file_size)goto cicl;
    lseek(_3dosinfo.hand,_3dosinfo.curr_pos,SEEK_SET);
    _rtl_read(_3dosinfo.hand,&temp,4);
    if(temp!=0x4d4c4946) //FILM
    {
     _rtl_read(_3dosinfo.hand,&temp,4);
     _3dosinfo.curr_pos+=uint_to_bigend(temp);
     if(_3dosinfo.curr_pos>=_3dosinfo.file_size || temp==0)goto cicl;
     goto retry_1;
    }
 goto paint_point;

cicl:
    _3dosinfo.curr_pos=_3dosinfo.fpos;
retry_2:
    if(_3dosinfo.curr_pos>=_3dosinfo.file_size)return 2;
    lseek(_3dosinfo.hand,_3dosinfo.curr_pos,SEEK_SET);
    _rtl_read(_3dosinfo.hand,&temp,4);
    if(temp!=0x4d4c4946) //FILM
    {
     _rtl_read(_3dosinfo.hand,&temp,4);
     _3dosinfo.curr_pos+=uint_to_bigend(temp);
     if(_3dosinfo.curr_pos>=_3dosinfo.file_size || temp==0)return 2;
     goto retry_2;
    }

 paint_point:
  _rtl_read(_3dosinfo.hand,&temp,4);
  if(uint_to_bigend(temp)>1000000)return 3;
  fbuf=new unsigned char[uint_to_bigend(temp)+0x100];
  lseek(_3dosinfo.hand,_3dosinfo.curr_pos,SEEK_SET);
  _rtl_read(_3dosinfo.hand,fbuf,uint_to_bigend(temp));
  film_frm=(CinePakFrame *)fbuf;
  if(film_frm->subChunkType!=0x454d5246)
   {
    if(flag==1)
    {
     delete []fbuf;
     return 4;
    }
    flag=1;
    goto retry_1;
   }


  //film_frm->frameSize
  //if(film_frm->frameData[16]==0x10)
  //{
  //decode_cinepak_free(tpoint);
  //tpoint=decode_cinepak_init();
  //}

  _3dosinfo.delay=uint_to_bigend(film_frm->duration);

  temp=decode_cinepak(tpoint, film_frm->frameData, uint_to_bigend(film_frm->frameSize)-8, (unsigned char *)CelIm.image_buf, CelIm.x_res, CelIm.y_res, 32);
  //decode_cinepak_free(tpoint);

  delete []fbuf;
  return temp;

}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
int OpenIMAGE(char *file)
{
  int fsize,hand,tmp,i,j;
  unsigned char *fbuf;
  unsigned int *fdata;
  unsigned short *fshd;
  if( (hand=_rtl_open(file, O_RDONLY))==-1)return 1;
   fsize=lseek(hand,0,SEEK_END);
   lseek(hand,0,SEEK_SET);

   if(fsize>100000000)return 2;

   fbuf=new unsigned char[fsize+64];
   _rtl_read(hand,fbuf,fsize);
   _rtl_close(hand);

   fdata=(unsigned int*)fbuf;
   if(((unsigned int*)fbuf)[0]!=0x47414D49)return 3; /*IMAG*/
   //tmp=search_chank((unsigned int*)fbuf, fsize, 0x47414D49 /*IMAG*/);
   if(tmp==-1)
   {
      if(((unsigned int*)fbuf)[0]!=0x50504101){delete []fbuf;return 3;}
      if(((unsigned int*)fbuf)[1]!=0x4e524353){delete []fbuf;return 3;}
      CelIm.y_res=read_ushorte((unsigned char*)&(fbuf[12]));
      CelIm.x_res=read_ushorte((unsigned char*)&(fbuf[14]));
      fshd=(unsigned short*)&fbuf[88-64];
      tmp=84>>2;
      goto uzel_svedenia;
   }
   CelIm.x_res=read_uinte((unsigned char*)&(fdata[tmp+2]));
   CelIm.y_res=read_uinte((unsigned char*)&(fdata[tmp+3]));
   tmp=search_chank((unsigned int*)fbuf, fsize, 0x54414450 /*PDAT*/);
   if(tmp==-1) {delete []fbuf;return 3;}
   fshd=(unsigned short*)&fdata[tmp+2];

uzel_svedenia:
   if(CelIm.x_res*CelIm.y_res>0x1000000 || CelIm.x_res*CelIm.y_res*2>(fsize-tmp*4)){delete []fbuf;return 4;}
   CelIm.image_buf=new unsigned int[CelIm.x_res*CelIm.y_res];

   for(i=0;i<(CelIm.y_res);i++)
   for(j=0;j<(CelIm.x_res);j++)
   {
      if(i&1)
        CelIm.image_buf[(i)*CelIm.x_res+j]=ConvertPix_16Direct( read_ushorte((unsigned char*)(fshd+(i/2)*CelIm.x_res*2+j*2+1)));
      else
        CelIm.image_buf[(i)*CelIm.x_res+j]=ConvertPix_16Direct( read_ushorte((unsigned char*)(fshd+(i/2)*CelIm.x_res*2+j*2)));
   }

   delete []fbuf;
   return 0;


}

int OpenCel(char *file)
{
 int fsize,hand,tmp,i,j;
 unsigned char *fbuf;
 unsigned int *fdata;
 unsigned short *fshd;
  if( (hand=_rtl_open(file, O_RDONLY))==-1)return 1;
   fsize=lseek(hand,0,SEEK_END);
   lseek(hand,0,SEEK_SET);

   if(fsize>100000000)return 2;

   fbuf=new unsigned char[fsize+64];
   _rtl_read(hand,fbuf,fsize);
   _rtl_close(hand);

   fdata=(unsigned int*)fbuf;

   tmp=0;
   if(((unsigned int*)fbuf)[0]!=0x20424343)return 3; /*CCB */
   //tmp=search_chank((unsigned int*)fbuf, fsize, 0x20424343 /*CCB */);
   //if(tmp==-1){delete []fbuf;return 3;}
   //tmp=0;
   
   for(j=0,i=3;i<((int)read_uinte((unsigned char*)&fdata[1+tmp])>>2)-2;i++,j++)
   {
    ((unsigned int*)&CelIm.ccb_chank)[j]=read_uinte((unsigned char*)&fdata[i+tmp]);
   }
   CelIm.x_res=(int)read_uinte((unsigned char*)&fdata[i+tmp]);
   CelIm.y_res=(int)read_uinte((unsigned char*)&fdata[i+tmp+1]);
   if(CelIm.x_res*CelIm.y_res>1000000){delete []fbuf; return 5;}

   tmp=search_chank((unsigned int*)fbuf, fsize, 0x54554C50 /*PLUT*/);
   if(tmp!=-1)
   {
    fshd=(unsigned short*)&fdata[tmp+3];
    for(j=0;j<32;j++)
    {
     (CelIm.plut_data)[j]=read_ushorte((unsigned char*)&fshd[j]);
    }
   }

   tmp=search_chank((unsigned int*)fbuf, fsize, 0x54414450 /*PDAT*/);
   if(tmp==-1){delete []fbuf;return 4;}
   CelIm.pdat_size=((read_uinte((unsigned char*)&fdata[1+tmp]))-8);
   if(CelIm.pdat_size>fsize-tmp*4){delete []fbuf;return 5;}
   
   CelIm.pdat_buf=new unsigned char[CelIm.pdat_size];

   if(CEL_FLAG_CCBPRE(CelIm.ccb_chank.flags)==0)
   {
    CelIm.ccb_chank.pre0=read_uinte((unsigned char*)&fdata[2+tmp]);
    tmp++;
    CelIm.pdat_size--;
    if(CEL_FLAG_PACKED(CelIm.ccb_chank.flags)==0)
    {
     CelIm.ccb_chank.pre1=read_uinte((unsigned char*)&fdata[2+tmp]);
     tmp++;
     CelIm.pdat_size--;
    }
   }
   for(j=0;j<(CelIm.pdat_size>>2);j++)
    {
     ((unsigned int*)CelIm.pdat_buf)[j]=fdata[tmp+2+j];
    }


   delete []fbuf;
   return 0;
}

unsigned int ConvertPix_16UC(unsigned short color)
{
unsigned short col;
  if(CEL_PRE0_UNCODED(CelIm.ccb_chank.pre0)==0)
  {
   col=CelIm.plut_data[color&31];
  }
  else col=color;
  return ((col&0x1f)<<(3+16))|(((col>>5)&0x1f)<<(3+8))|(((col>>10)&0x1f)<<(3));
}

int dec_inc,dec_shift;
unsigned int get_bits(unsigned char *array, int num_of_bits)
{
 int count=0;
 unsigned int rez_num=0;
 unsigned char *p,dp;
 int dop;
 p=array;
 dec_inc=0;
 do{
 if(num_of_bits>=(count+8-dec_shift)){
        dp=p[0]<<dec_shift;
        dp=dp>>dec_shift;
        rez_num=(rez_num<<(8-dec_shift))|dp;
        p++; dec_inc++;
        count+=8-dec_shift;
        dec_shift=0;
 }
 else{

        dp=p[0]<<(unsigned char)dec_shift;
        dp=dp>>(8-(num_of_bits-count));
        rez_num=(rez_num<<(num_of_bits-count))|dp;
        dec_shift+=num_of_bits-count;
        break;
 }
 }while(1);
 return rez_num;
}


unsigned int ConvertPix_PLUT(unsigned int color, int flag)
{
 unsigned short col;
 if(flag==6) col=CelIm.plut_data[color&31];
 else if(flag==8)
 {
  if(CEL_PRE0_UNCODED(CelIm.ccb_chank.pre0))
  {
   col=((color&3)<<3)|((color&(7<<2))<<(3+2))|((color&(7<<5))<<(3+4));
  }
  else col=CelIm.plut_data[color&31];
 }
 else col=CelIm.plut_data[color];
  return ((col&0x1f)<<(3+16))|(((col>>5)&0x1f)<<(3+8))|(((col>>10)&0x1f)<<(3));
}



int CelToRaw()
{
 int i,j,k,str_pos,dop;
 unsigned char *point;
 unsigned short pixcol;
 unsigned int tmp,tmp2,bpp;
 CelIm.image_buf=new unsigned int [CelIm.x_res*CelIm.y_res];

 for(i=0;i<CelIm.x_res*CelIm.y_res;i++)CelIm.image_buf[i]=0xff000000;

 bpp=bits_per_pixel[CEL_PRE0_BPP(CelIm.ccb_chank.pre0)];
 switch(CEL_PRE0_BPP(CelIm.ccb_chank.pre0))
 {
   case 1:
   case 2:
   case 3:
   case 4:
   case 5:
        if(CEL_FLAG_PACKED(CelIm.ccb_chank.flags))
        {
           str_pos=0;
           for(i=0;i<CelIm.y_res;i++)
           {
             k=str_pos;
             if(bpp==8){
             str_pos+=((read_ushorte(&CelIm.pdat_buf[k])+2)<<2);
             k+=2; }
             else {
             str_pos+=((CelIm.pdat_buf[k]+2)<<2);
             k++;}
             j=0;
             dec_shift=0;
             while(j<CelIm.x_res)
             {
               tmp=get_bits(&CelIm.pdat_buf[k],8);
               k+=dec_inc;
               switch((tmp>>6)&0x3)
               {
                case 0:
                        goto end_str8l;
                break;
                case 1:
                        for(dop=0;dop<((tmp&0x3f)+1) && j<CelIm.x_res;dop++)
                         {
                          CelIm.image_buf[i*CelIm.x_res+j]=ConvertPix_PLUT(get_bits(&CelIm.pdat_buf[k],bpp),bpp);
                          k+=dec_inc;
                          j++;
                         }
                break;
                case 3:
                        tmp2=get_bits(&CelIm.pdat_buf[k],bpp);
                        k+=dec_inc;
                        for(dop=0;dop<(tmp&0x3f)+1 && j<CelIm.x_res;dop++)
                         {
                          CelIm.image_buf[i*CelIm.x_res+j]=ConvertPix_PLUT(tmp2,bpp);
                          j++;
                         }

                break;
                case 2:
                        j+=(tmp&0x3f)+1;
                break;
               }
             }
end_str8l:
           }
        }
        else
        {

         for(i=0;i<CelIm.y_res;i++)
         {
          k=((CelIm.x_res*bpp+31))>>5;
          k=(k<<2)*i;
          dec_shift=0;
          for(j=0;j<CelIm.x_res;j++)
          {
            CelIm.image_buf[i*CelIm.x_res+j]=ConvertPix_PLUT(get_bits(&CelIm.pdat_buf[k],bpp),bpp);
            k+=dec_inc;
          }
         }
        }
   break;
   case 6:
        if(CEL_FLAG_PACKED(CelIm.ccb_chank.flags))
        {
           str_pos=0;
           for(i=0;i<CelIm.y_res;i++)
           {
             k=str_pos;
             str_pos+=((read_ushorte(&CelIm.pdat_buf[k])+2)<<2);
             k+=2;
             j=0;
             while(j<CelIm.x_res)
             {
               switch((CelIm.pdat_buf[k]>>6)&0x3)
               {
                case 0:
                        k++;
                        goto end_str;
                break;
                case 1:
                        for(dop=0;dop<((CelIm.pdat_buf[k]&0x3f)+1) && j<CelIm.x_res;dop++)
                         {
                          CelIm.image_buf[i*CelIm.x_res+j]=ConvertPix_16UC(read_ushorte(&CelIm.pdat_buf[k+1+(dop<<1)]));
                          j++;
                         }
                        k+=(((CelIm.pdat_buf[k]&0x3f)+1)<<1)+1;
                break;
                case 3:
                        for(dop=0;dop<(CelIm.pdat_buf[k]&0x3f)+1 && j<CelIm.x_res;dop++)
                         {
                          CelIm.image_buf[i*CelIm.x_res+j]=ConvertPix_16UC(read_ushorte(&CelIm.pdat_buf[k+1]));
                          j++;
                         }
                        k+=3;
                break;
                case 2:
                        j+=(CelIm.pdat_buf[k]&0x3f)+1;
                        k++;
                break;
               }
             }
end_str:
           }
        }
        else
        {
         for(i=0;i<CelIm.y_res;i++)
          for(j=0;j<CelIm.x_res;j++)
            CelIm.image_buf[i*CelIm.x_res+j]=ConvertPix_16UC(read_ushorte(&CelIm.pdat_buf[(i*((CelIm.x_res+1)&0xfffffffe)+j)<<1]));
        }
   break;
   default:
        return 1;
 };

 return 0;
}

void CelStrClear()
{
 if(CelIm.pdat_buf!=NULL)delete []CelIm.pdat_buf;
 CelIm.pdat_buf=NULL;
 if(CelIm.image_buf!=NULL)delete []CelIm.image_buf;
 CelIm.image_buf=NULL;
}