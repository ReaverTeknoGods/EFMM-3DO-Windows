object Form1: TForm1
  Left = 247
  Top = 110
  Width = 708
  Height = 618
  Caption = ' 3DOViewer'
  Color = clBtnFace
  Font.Charset = RUSSIAN_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  Icon.Data = {
    0000010001002020100000000000E80200001600000028000000200000004000
    0000010004000000000000020000000000000000000000000000000000000000
    0000000080000080000000808000800000008000800080800000C0C0C0008080
    80000000FF0000FF000000FFFF00FF000000FF00FF00FFFF0000FFFFFF008888
    8888888888888888888888880000888888888888888888888888888800008888
    8888888888888888888888880000888CCCCCCCCCCCCCCCCCCCCCCCCCCC00888C
    CCCCCCCCCCCCCCCCCCCCCCCCCC00888CCCFCCCFCCFCCCFCCFCCCFCFFFC00888C
    CCFCCCFCCFCCCFCCFCCCFCFFFC00888CCCFCCCFCCFCCCFCCFCCCFCFFFC00888C
    CCFFFFFCCFFFFCCCFCCCFCCCCC00888CCCFCCCFCCFCCCFCCFCCCFCCCCC00888C
    CCFCCCFCCFCCCFCCFCFCFCCCCC00888CCCFCCCFCCFCCCFCCFFCFFCCCCC00888C
    CCCFFFCCCFFFFCCCFCCCFCCCCC00888CCCCCCCCCCCCCCCCCCCCCCCCCCC00888C
    CCCCCCCCCC99999CCCCCCCCCCC00888CCCCCCCCC999999999CCCCCCCCC00888C
    CCCCCCC9F9CCCCC99FCCCCCCCC00888CCCCCCC999FCCCC99F99CCCCCCC00888C
    CCCCCC99CFCCF999F99CCCCCCC00888CCCCCC9FCCCFF999FCCF9CCCCCC00888C
    CCCCC99FCCC999CCCF99CCCCCC00888CCCCCC99CFF999FFFFC99CCCCCC00888C
    CCCCC99CC999FFCCCC99CCCCCC00888CCCCCC99C999FFFFCCC99CCCCCC00888C
    CCCCCC9999CFFFCFC99CCCCCCC00888CCCCCCC999FCCFCCF999CCCCCCC00888C
    CCCCCCC99FCCCCCF99CCCCCCCC00888CCCCCCCCCF9999999FCCCCCCCCC00000C
    CCCCCCCCCC99999CCCCCCCCCCC00000CCCCCCCCCCCCCCCCCCCCCCCCCCC000000
    0000000000000000000000000000000000000000000000000000000000000000
    000A0000000A0000000F00000003000000030000000300000003000000030000
    0003000000030000000300000003000000030000000300000003000000030000
    0003000000030000000300000003000000030000000300000003000000030000
    0003000000030000000300000003E0000003E0000003FFFFFFFFFFFFFFFF}
  Menu = MainMenu1
  OldCreateOrder = False
  PixelsPerInch = 96
  TextHeight = 13
  object Image1: TImage
    Left = 8
    Top = 8
    Width = 320
    Height = 240
  end
  object Memo1: TMemo
    Left = 8
    Top = 264
    Width = 385
    Height = 289
    Lines.Strings = (
      'If you find some bugs - you can write to the mail:'
      'max220291@list.ru')
    ScrollBars = ssBoth
    TabOrder = 0
    OnDblClick = Memo1DblClick
  end
  object FileListBox1: TFileListBox
    Left = 400
    Top = 240
    Width = 289
    Height = 313
    ItemHeight = 13
    TabOrder = 1
    OnChange = FileListBox1Change
  end
  object DirectoryListBox1: TDirectoryListBox
    Left = 400
    Top = 32
    Width = 289
    Height = 201
    ItemHeight = 16
    TabOrder = 2
    OnChange = DirectoryListBox1Change
  end
  object DriveComboBox1: TDriveComboBox
    Left = 400
    Top = 8
    Width = 289
    Height = 19
    TabOrder = 3
    OnChange = DriveComboBox1Change
  end
  object Button1: TButton
    Left = 344
    Top = 8
    Width = 41
    Height = 25
    Caption = '>'
    TabOrder = 4
    OnClick = Button1Click
  end
  object Button2: TButton
    Left = 344
    Top = 40
    Width = 41
    Height = 25
    Caption = '->'
    TabOrder = 5
    OnClick = Button2Click
  end
  object Button3: TButton
    Left = 344
    Top = 72
    Width = 41
    Height = 25
    Caption = 'X'
    TabOrder = 6
    OnClick = Button3Click
  end
  object Button4: TButton
    Left = 336
    Top = 104
    Width = 57
    Height = 25
    Caption = 'Extract'
    TabOrder = 7
    OnClick = Button4Click
  end
  object OpenDialog1: TOpenDialog
    Left = 368
    Top = 208
  end
  object MainMenu1: TMainMenu
    Left = 368
    Top = 176
    object Command1: TMenuItem
      Caption = 'Command'
      object ConvertCurrentDir1: TMenuItem
        Caption = 'Convert Current Dir'
        OnClick = ConvertCurrentDir1Click
      end
      object Save1: TMenuItem
        Caption = 'Save'
        OnClick = Save1Click
      end
      object N1: TMenuItem
        Caption = '-'
      end
      object SaveLog1: TMenuItem
        Caption = 'Save Log'
        OnClick = SaveLog1Click
      end
    end
    object About1: TMenuItem
      Caption = 'Options'
    end
    object Help1: TMenuItem
      Caption = 'Help'
      OnClick = Help1Click
    end
    object About2: TMenuItem
      Caption = 'About'
      OnClick = About2Click
    end
  end
  object SaveDialog1: TSaveDialog
    Left = 336
    Top = 176
  end
  object Timer1: TTimer
    Interval = 10
    OnTimer = Timer1Timer
    Left = 336
    Top = 208
  end
end
