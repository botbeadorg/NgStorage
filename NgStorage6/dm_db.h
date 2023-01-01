// ---------------------------------------------------------------------------

#ifndef dm_dbH
#define dm_dbH
// ---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include "DAScript.hpp"
#include "DBAccess.hpp"
#include "MemDS.hpp"
#include "MySQLUniProvider.hpp"
#include "SQLiteUniProvider.hpp"
#include "Uni.hpp"
#include "UniProvider.hpp"
#include "UniScript.hpp"
#include <Data.DB.hpp>
#include <FMX.Types.hpp>

// ---------------------------------------------------------------------------
class TDataModule3 : public TDataModule {
__published: // IDE-managed Components
	TMySQLUniProvider *MySQLUniProvider1;
	TUniConnection *UniConnection1;
	TUniSQL *UniSQL1;
	TUniScript *UniScript1;
	TUniConnection *UniConnection2;
	TUniSQL *UniSQL2;
	TUniQuery *UniQuery1;
	TUniQuery *UniQuery2;
	TSQLiteUniProvider *SQLiteUniProvider1;
	TUniConnection *UniConnection3;
	TUniScript *UniScript2;
	TUniQuery *UniQuery3;
	TUniSQL *UniSQL3;
	TUniScript *UniScript3;
	TTimer *Timer_reconnect;

	void __fastcall DataModuleCreate(TObject *Sender);
	void __fastcall Timer_reconnectTimer(TObject *Sender);

private: // User declarations
public: // User declarations
	__fastcall TDataModule3(TComponent* Owner);

	String home;
};

// ---------------------------------------------------------------------------
extern PACKAGE TDataModule3 *DataModule3;
// ---------------------------------------------------------------------------
#endif
