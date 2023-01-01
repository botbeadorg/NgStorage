// ---------------------------------------------------------------------------

#ifndef http_connH
#define http_connH
// ---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <System.Net.HttpClient.hpp>
#include <System.Net.HttpClientComponent.hpp>
#include <System.Net.URLClient.hpp>
#include <FMX.Types.hpp>

// ---------------------------------------------------------------------------
class TDataModule1 : public TDataModule {
__published: // IDE-managed Components
	TNetHTTPClient *NetHTTPClient1;
	TNetHTTPRequest *request_task_issue;
	TNetHTTPRequest *request_common;
	TNetHTTPRequest *request_query_rti;
	TNetHTTPRequest *request_query_wp;
	TNetHTTPRequest *request_query_elec;
	TTimer *timer_query_rti;
	TTimer *timer_query_wp;
	TTimer *timer_query_elec;
	TNetHTTPRequest *request_agv_online;
	TNetHTTPRequest *request_fresh_agv;
	TTimer *timer_fresh_agv;
	TNetHTTPRequest *request_erase_storloc;
	TNetHTTPRequest *request_running_status;
	TTimer *timer_request_running_status;
	TNetHTTPRequest *request_info_storloc;
	TTimer *timer_task_auto_checker;
	TTimer *timer_press;
	TNetHTTPRequest *request_change_storloc;

	void __fastcall timer_query_rtiTimer(TObject *Sender);
	void __fastcall DataModuleCreate(TObject *Sender);
	void __fastcall request_agv_onlineRequestCompleted(TObject * const Sender,
		IHTTPResponse * const AResponse);
	void __fastcall timer_fresh_agvTimer(TObject *Sender);
	void __fastcall DataModuleDestroy(TObject *Sender);
	void __fastcall timer_query_elecTimer(TObject *Sender);
	void __fastcall timer_request_running_statusTimer(TObject *Sender);
	void __fastcall timer_pressTimer(TObject *Sender);

private: // User declarations
public: // User declarations
	__fastcall TDataModule1(TComponent* Owner);

	void __fastcall client_id();
	size_t __fastcall query_agvs();
	void __fastcall agv_online(u_long);
	size_t __fastcall query_available_storlocs();
	rep_spec_pack __fastcall query_info_ware(String);

	u_int id_client;

	size_t count_circle_pressed;
};

// ---------------------------------------------------------------------------
extern PACKAGE TDataModule1 *DataModule1;
// ---------------------------------------------------------------------------
#endif
