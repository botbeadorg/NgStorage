object DataModule1: TDataModule1
  OldCreateOrder = False
  OnCreate = DataModuleCreate
  OnDestroy = DataModuleDestroy
  Height = 437
  Width = 580
  object IdHTTPServer1: TIdHTTPServer
    Active = True
    Bindings = <
      item
        IP = '0.0.0.0'
        Port = 19998
      end>
    DefaultPort = 19998
    OnCommandGet = IdHTTPServer1CommandGet
    Left = 504
    Top = 368
  end
  object provider_sqlite: TSQLiteUniProvider
    Left = 504
    Top = 32
  end
  object sqlitecn_agv: TUniConnection
    Left = 32
    Top = 16
  end
  object result_agv: TUniQuery
    Connection = sqlitecn_agv
    Left = 112
    Top = 16
  end
  object noresult_agv: TUniSQL
    Connection = sqlitecn_agv
    Left = 192
    Top = 16
  end
  object UniConnection1: TUniConnection
    Left = 408
    Top = 32
  end
  object UniScript1: TUniScript
    Left = 408
    Top = 88
  end
  object sqlcn_storloc: TUniConnection
    Left = 32
    Top = 80
  end
  object result_storloc: TUniQuery
    Connection = sqlcn_storloc
    Left = 112
    Top = 80
  end
  object noresult_storloc: TUniSQL
    Connection = sqlcn_storloc
    Left = 192
    Top = 80
  end
  object script_storloc: TUniScript
    Connection = sqlcn_storloc
    Left = 280
    Top = 80
  end
  object sqlitecn_ti: TUniConnection
    Left = 32
    Top = 152
  end
  object result_ti: TUniQuery
    Connection = sqlitecn_ti
    Left = 112
    Top = 152
  end
  object noresult_ti: TUniSQL
    Connection = sqlitecn_ti
    Left = 192
    Top = 152
  end
end
