object DataModule1: TDataModule1
  OldCreateOrder = False
  OnCreate = DataModuleCreate
  OnDestroy = DataModuleDestroy
  Height = 488
  Width = 999
  object NetHTTPClient1: TNetHTTPClient
    Asynchronous = False
    ConnectionTimeout = 60000
    ResponseTimeout = 60000
    HandleRedirects = True
    AllowCookies = True
    UserAgent = 'Embarcadero URI Client/1.0'
    Left = 64
    Top = 48
  end
  object request_task_issue: TNetHTTPRequest
    Asynchronous = False
    ConnectionTimeout = 60000
    ResponseTimeout = 60000
    Client = NetHTTPClient1
    Left = 208
    Top = 48
  end
  object request_common: TNetHTTPRequest
    Asynchronous = False
    ConnectionTimeout = 60000
    ResponseTimeout = 60000
    Client = NetHTTPClient1
    Left = 360
    Top = 48
  end
  object request_query_rti: TNetHTTPRequest
    Asynchronous = False
    ConnectionTimeout = 60000
    ResponseTimeout = 60000
    Client = NetHTTPClient1
    Left = 208
    Top = 120
  end
  object request_query_wp: TNetHTTPRequest
    Asynchronous = False
    ConnectionTimeout = 60000
    ResponseTimeout = 60000
    Client = NetHTTPClient1
    Left = 208
    Top = 192
  end
  object request_query_elec: TNetHTTPRequest
    Asynchronous = False
    ConnectionTimeout = 60000
    ResponseTimeout = 60000
    Client = NetHTTPClient1
    Left = 209
    Top = 264
  end
  object timer_query_rti: TTimer
    OnTimer = timer_query_rtiTimer
    Left = 408
    Top = 168
  end
  object timer_query_wp: TTimer
    Left = 408
    Top = 224
  end
  object timer_query_elec: TTimer
    Interval = 180000
    OnTimer = timer_query_elecTimer
    Left = 408
    Top = 288
  end
  object request_agv_online: TNetHTTPRequest
    Asynchronous = True
    ConnectionTimeout = 60000
    ResponseTimeout = 60000
    Client = NetHTTPClient1
    OnRequestCompleted = request_agv_onlineRequestCompleted
    Left = 360
    Top = 104
  end
  object request_fresh_agv: TNetHTTPRequest
    Asynchronous = False
    ConnectionTimeout = 60000
    ResponseTimeout = 60000
    Client = NetHTTPClient1
    Left = 624
    Top = 48
  end
  object timer_fresh_agv: TTimer
    Interval = 3000
    OnTimer = timer_fresh_agvTimer
    Left = 624
    Top = 104
  end
  object request_erase_storloc: TNetHTTPRequest
    Asynchronous = False
    ConnectionTimeout = 60000
    ResponseTimeout = 60000
    Client = NetHTTPClient1
    Left = 624
    Top = 288
  end
  object request_running_status: TNetHTTPRequest
    Asynchronous = False
    ConnectionTimeout = 60000
    ResponseTimeout = 60000
    Client = NetHTTPClient1
    Left = 440
    Top = 432
  end
  object timer_request_running_status: TTimer
    Interval = 5000
    OnTimer = timer_request_running_statusTimer
    Left = 600
    Top = 432
  end
  object request_info_storloc: TNetHTTPRequest
    Asynchronous = False
    ConnectionTimeout = 60000
    ResponseTimeout = 60000
    Client = NetHTTPClient1
    Left = 168
    Top = 432
  end
  object timer_task_auto_checker: TTimer
    Enabled = False
    Interval = 5000
    Left = 864
    Top = 48
  end
  object timer_press: TTimer
    Enabled = False
    Interval = 250
    OnTimer = timer_pressTimer
    Left = 824
    Top = 432
  end
  object request_change_storloc: TNetHTTPRequest
    Asynchronous = False
    ConnectionTimeout = 60000
    ResponseTimeout = 60000
    Client = NetHTTPClient1
    Left = 760
    Top = 288
  end
end
