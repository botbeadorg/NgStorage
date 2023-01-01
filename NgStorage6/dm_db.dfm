object DataModule3: TDataModule3
  OldCreateOrder = False
  OnCreate = DataModuleCreate
  Height = 566
  Width = 971
  object MySQLUniProvider1: TMySQLUniProvider
    Left = 64
    Top = 40
  end
  object UniConnection1: TUniConnection
    Left = 64
    Top = 104
  end
  object UniSQL1: TUniSQL
    Connection = UniConnection1
    Left = 152
    Top = 104
  end
  object UniScript1: TUniScript
    Connection = UniConnection1
    Left = 304
    Top = 104
  end
  object UniConnection2: TUniConnection
    Left = 64
    Top = 176
  end
  object UniSQL2: TUniSQL
    Connection = UniConnection2
    Left = 144
    Top = 176
  end
  object UniQuery1: TUniQuery
    Connection = UniConnection1
    Left = 232
    Top = 104
  end
  object UniQuery2: TUniQuery
    Connection = UniConnection2
    Left = 232
    Top = 176
  end
  object SQLiteUniProvider1: TSQLiteUniProvider
    Left = 152
    Top = 488
  end
  object UniConnection3: TUniConnection
    Left = 264
    Top = 488
  end
  object UniScript2: TUniScript
    Connection = UniConnection2
    Left = 304
    Top = 176
  end
  object UniQuery3: TUniQuery
    Connection = UniConnection3
    Left = 360
    Top = 488
  end
  object UniSQL3: TUniSQL
    Connection = UniConnection3
    Left = 432
    Top = 488
  end
  object UniScript3: TUniScript
    Connection = UniConnection3
    Left = 520
    Top = 488
  end
  object Timer_reconnect: TTimer
    Interval = 5000
    OnTimer = Timer_reconnectTimer
    Left = 776
    Top = 64
  end
end
