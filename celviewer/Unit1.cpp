//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Unit1.h"
#include "CelLib.h"
#include "hack.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
AnsiString str_head="3DOResExplorer 0.0.2 ";
Graphics::TBitmap *points=new Graphics::TBitmap();
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
        : TForm(Owner)
{
  Image1->Canvas->Rectangle(-1,-1,Image1->Width+1,Image1->Height+1);
  CelIm.pdat_buf=NULL;
  CelIm.image_buf=NULL;
  initStream();
  Caption=str_head;
  Button4->Enabled=false;

}
//---------------------------------------------------------------------------
                        
void __fastcall TForm1::SaveLog1Click(TObject *Sender)
{
if(SaveDialog1->Execute())
{
 Memo1->Lines->SaveToFile(SaveDialog1->FileName);
}
}
//---------------------------------------------------------------------------

void __fastcall TForm1::DriveComboBox1Change(TObject *Sender)
{
DirectoryListBox1->Drive=DriveComboBox1->Drive;
DirectoryListBox1->Update();
FileListBox1->Drive=DriveComboBox1->Drive;
FileListBox1->Directory=DirectoryListBox1->Directory;
FileListBox1->Update();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::DirectoryListBox1Change(TObject *Sender)
{
FileListBox1->Drive=DriveComboBox1->Drive;
FileListBox1->Directory=DirectoryListBox1->Directory;
FileListBox1->Update();
}
//---------------------------------------------------------------------------

int __fastcall TForm1::pict_explorer(AnsiString src)
{
int i,j;
 CelStrClear();
 closeStream();
 i=OpenCel((src).c_str());
 if(i==0)
 {
  if(i!=0)goto finita;
  i=CEL_PRE0_BPP(CelIm.ccb_chank.pre0);
  if (bits_per_pixel[i]==-1){goto finita;}
  i=CelToRaw();
 }
 if(i!=0)
 {
    i=OpenIMAGE((src).c_str());
 }
 if(i!=0)
 {
    i=getStream((src).c_str());
 }

 

 if(i!=0){goto finita;}

 Image1->Picture->Bitmap->Width=CelIm.x_res;
 Image1->Picture->Bitmap->Height=CelIm.y_res;

 points->Width=CelIm.x_res;
 points->Height=CelIm.y_res;
 points->FreeImage();
 points->PixelFormat=pf32bit;

 Image1->Picture->Bitmap->FreeImage();
 Image1->Picture->Bitmap->PixelFormat=pf32bit;


 int  *colline;
 for(i=0;i<CelIm.y_res;i++)
 {
 colline=(int*)Image1->Picture->Bitmap->ScanLine[i];

 for(j=0;j<CelIm.x_res;j++)
  {
     int k=CelIm.image_buf[i*CelIm.x_res+j];
     if(_3dosinfo.flag!=0)colline[j]=k;
     else colline[j]=((k&255)<<16)|(k&0xff00)|((k>>16)&255)|(k&0xff000000);
  }
 }
 return 0;

finita:

 return 1;

}

void __fastcall TForm1::FileListBox1Change(TObject *Sender)
{
int i,j;
Caption = str_head + (FileListBox1->FileName);
Image1->Picture->Bitmap->Width=0;
Image1->Picture->Bitmap->Height=0;
Image1->Canvas->Rectangle(-1,-1,Image1->Width+1,Image1->Height+1);

if (FileListBox1->FileName=="") return;

 hack_reset();
 hack_test_file(FileListBox1->FileName);
 if(hack_pak_explore.extructble!=0)Button4->Enabled=true;
 else Button4->Enabled=false; 
 pict_explorer(FileListBox1->FileName.c_str());


}
//---------------------------------------------------------------------------

void __fastcall TForm1::Memo1DblClick(TObject *Sender)
{
Memo1->Clear();
}
//---------------------------------------------------------------------------


void __fastcall TForm1::Save1Click(TObject *Sender)
{
//Graphics::TBitmap *BMP = new Graphics::TBitmap;

Image1->Picture->Bitmap->Width=CelIm.x_res;
Image1->Picture->Bitmap->Height=CelIm.y_res;

Image1->Picture->Bitmap->SaveToFile(FileListBox1->FileName+".bmp");
Memo1->Lines->Add("Cel saved as BMP.");
//Image1->Width=320;
//Image1->Height=240;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ConvertCurrentDir1Click(TObject *Sender)
{
int ii;
int i,j;
AnsiString dstr="";

 for(ii=0;ii<FileListBox1->Items->Count;ii++)
 {
  dstr=(FileListBox1->Items->operator [](ii));
//************************************************8888

 i=pict_explorer((dstr).c_str());
 if(i!=0)goto finita;

 Image1->Picture->Bitmap->SaveToFile((dstr)+".bmp");
 Memo1->Lines->Add(FileListBox1->Items->operator [](ii)+" - Cel saved as BMP.");

finita:

 }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Help1Click(TObject *Sender)
{
Memo1->Lines->Add("->>>>>>Help.....");
Memo1->Lines->Add("See menu - all easy! (я знаю вы во всем сами с усами, зачем вы вообще нажали эту кнопочку? :)");
Memo1->Lines->Add("...and if you want clear log memo - do double click...");
Memo1->Lines->Add("...and before using this programm, unpak data from 3DO cd-rom (search UN-CDROM from Troosh or 3DOCommander from Mnemonic)...");
Memo1->Lines->Add("...and for STREAMS clic to (>) for frame change, (->) for play, (X) for stop...");
Memo1->Lines->Add("...is it all...");
}
//---------------------------------------------------------------------------

void __fastcall TForm1::About2Click(TObject *Sender)
{
Memo1->Lines->Add("");
Memo1->Lines->Add("->>>>>>About..... "+str_head);
Memo1->Lines->Add("Started write by Altmer (mail: max220291@list.ru) 23.08.2005");
Memo1->Lines->Add("Great thanks for help:");
Memo1->Lines->Add("    to Mnemonic (http://do-homebrew.narod.ru) - help with documentation for CEL format");
Memo1->Lines->Add("    to Tim Ferguson: http://www.csse.monash.edu.au/~timf/ - help with CINEPAK compression algorithm");
Memo1->Lines->Add("means:");
Memo1->Lines->Add(" + CEL partial support");
Memo1->Lines->Add(" + IMAG 16-bit support");
Memo1->Lines->Add(" + BANNERs support");
Memo1->Lines->Add(" + STREAM CPAK video support");
Memo1->Lines->Add(" + accessible - Save to BMP");
Memo1->Lines->Add("far & near planes:");
Memo1->Lines->Add(" - full support CEL format");
Memo1->Lines->Add(" - full support STREAM format");
Memo1->Lines->Add(" - support archeves of anim and others data");
Memo1->Lines->Add(" - to AVI converter");
Memo1->Lines->Add(" - include direct cd-rom & iso explorer");
Memo1->Lines->Add(" *** up to v.0.0.2: ");
Memo1->Lines->Add(" +BIG file extraction added ");


}
//---------------------------------------------------------------------------


void __fastcall TForm1::Button1Click(TObject *Sender)
{
 int j,i;
 

 if((i=get_frame_from_Stream())!=0)return;

 int  *colline;
 for(i=0;i<CelIm.y_res;i++)
 {
 colline=(int*)Image1->Picture->Bitmap->ScanLine[i];

 for(j=0;j<CelIm.x_res;j++)
  {
     int k=CelIm.image_buf[i*CelIm.x_res+j]&0xffffff;
     //k=((k&255)<<16)|(k&0xff00)|((k>>16)&255)|(k&0xff000000);
     colline[j]=k;
  }
 }
 Image1->Refresh();

}
//---------------------------------------------------------------------------


void __fastcall TForm1::Button2Click(TObject *Sender)
{
 _3dosinfo.playf=1;
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Timer1Timer(TObject *Sender)
{
 int i,j;

 if(_3dosinfo.playf!=0 && _3dosinfo.flag!=0)
 {
   Image1->Visible=false;
   _3dosinfo.delay--;
   if(_3dosinfo.delay>0){return;}
   if(_3dosinfo.cntfrm==_3dosinfo.frames)
   {
        _3dosinfo.playf=0;
        _3dosinfo.cntfrm=0;
        return;
   }
   _3dosinfo.cntfrm++;
   i=get_frame_from_Stream();

  
   int  *colline;
   for(i=0;i<CelIm.y_res;i++)
   {
   
   colline=(int*)points->ScanLine[i];

   for(j=0;j<CelIm.x_res;j++)
    {
       int k=CelIm.image_buf[i*CelIm.x_res+j]&0xffffff;

       colline[j]=k;
    }
   }

   Form1->Canvas->Draw(8,8,points);

 }
 else
 {
     Image1->Visible=true;
 }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button3Click(TObject *Sender)
{
  _3dosinfo.playf=0;
}
//---------------------------------------------------------------------------



void __fastcall TForm1::Button4Click(TObject *Sender)
{
 hack_extract(FileListBox1->FileName, &hack_pak_explore);
 DirectoryListBox1->Update();
}
//---------------------------------------------------------------------------

