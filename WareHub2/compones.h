// ---------------------------------------------------------------------------

#ifndef componesH
#define componesH
// ---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <IdBaseComponent.hpp>
#include <IdComponent.hpp>
#include <IdCustomHTTPServer.hpp>
#include <IdCustomTCPServer.hpp>
#include <IdHTTPServer.hpp>
#include <IdContext.hpp>
#include "SQLiteUniProvider.hpp"
#include "UniProvider.hpp"
#include "DBAccess.hpp"
#include "Uni.hpp"
#include <Data.DB.hpp>
#include "MemDS.hpp"
#include "DAScript.hpp"
#include "UniScript.hpp"

#define UNICN(unicn, fc)\
do {\
	(unicn)->ProviderName = _T("SQLite");\
	if (fc)\
		(unicn)->SpecificOptions->Values[_T("ForceCreateDatabase")] = _T("True");\
	(unicn)->SpecificOptions->Values[_T("Direct")] = _T("True");\
	(unicn)->Database = path_db;\
	(unicn)->Connect();\
}\
while (0)

// ---------------------------------------------------------------------------
class TDataModule1 : public TDataModule {
__published: // IDE-managed Components
	TIdHTTPServer *IdHTTPServer1;
	TSQLiteUniProvider *provider_sqlite;
	TUniConnection *sqlitecn_agv;
	TUniQuery *result_agv;
	TUniSQL *noresult_agv;
	TUniConnection *UniConnection1;
	TUniScript *UniScript1;
	TUniConnection *sqlcn_storloc;
	TUniQuery *result_storloc;
	TUniSQL *noresult_storloc;
	TUniScript *script_storloc;
	TUniConnection *sqlitecn_ti;
	TUniQuery *result_ti;
	TUniSQL *noresult_ti;

	void __fastcall IdHTTPServer1CommandGet(TIdContext *AContext, TIdHTTPRequestInfo *ARequestInfo,
		TIdHTTPResponseInfo *AResponseInfo);
	void __fastcall DataModuleCreate(TObject *Sender);
	void __fastcall DataModuleDestroy(TObject *Sender);

private: // User declarations
public: // User declarations
	__fastcall TDataModule1(TComponent* Owner);
	void __fastcall load_agvs();
	void __fastcall reg_agv(dx *);
	// see https://blog.csdn.net/cb168/article/details/17962051
	// void __fastcall myRequestCompleted(TObject* const , const Net::Httpclient::_di_IHTTPResponse);
	unsigned __fastcall next_id_ti();
	void __fastcall update_id_ti(unsigned);

	size_t id_agv_max;

};

// ---------------------------------------------------------------------------
extern PACKAGE TDataModule1 *DataModule1;
// ---------------------------------------------------------------------------
#endif
