//---------------------------------------------------------------------------

#ifndef Unit1H
#define Unit1H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Dialogs.hpp>
#include <ExtCtrls.hpp>
#include <Grids.hpp>
#include <Outline.hpp>
#include <FileCtrl.hpp>
#include <Menus.hpp>
#include <MPlayer.hpp>
//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE-managed Components
        TOpenDialog *OpenDialog1;
        TMemo *Memo1;
        TImage *Image1;
        TFileListBox *FileListBox1;
        TDirectoryListBox *DirectoryListBox1;
        TDriveComboBox *DriveComboBox1;
        TMainMenu *MainMenu1;
        TMenuItem *Command1;
        TMenuItem *About1;
        TMenuItem *ConvertCurrentDir1;
        TMenuItem *About2;
        TMenuItem *SaveLog1;
        TSaveDialog *SaveDialog1;
        TMenuItem *N1;
        TMenuItem *Save1;
        TMenuItem *Help1;
        TButton *Button1;
        TButton *Button2;
        TTimer *Timer1;
        TButton *Button3;
        TButton *Button4;
        void __fastcall SaveLog1Click(TObject *Sender);
        void __fastcall DriveComboBox1Change(TObject *Sender);
        void __fastcall DirectoryListBox1Change(TObject *Sender);
        void __fastcall FileListBox1Change(TObject *Sender);
        void __fastcall Memo1DblClick(TObject *Sender);
        void __fastcall Save1Click(TObject *Sender);
        void __fastcall ConvertCurrentDir1Click(TObject *Sender);
        void __fastcall Help1Click(TObject *Sender);
        void __fastcall About2Click(TObject *Sender);
        void __fastcall Button1Click(TObject *Sender);
        void __fastcall Button2Click(TObject *Sender);
        void __fastcall Timer1Timer(TObject *Sender);
        void __fastcall Button3Click(TObject *Sender);
        void __fastcall Button4Click(TObject *Sender);
        
private:	// User declarations
public:		// User declarations
        __fastcall TForm1(TComponent* Owner);
        int __fastcall pict_explorer(AnsiString src);
};
//---------------------------------------------------------------------------
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
