//---------------------------------------------------------------------------

#ifndef http_srvH
#define http_srvH
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <IdBaseComponent.hpp>
#include <IdComponent.hpp>
#include <IdCustomHTTPServer.hpp>
#include <IdCustomTCPServer.hpp>
#include <IdHTTPServer.hpp>
//---------------------------------------------------------------------------
class TDataModule2 : public TDataModule
{
__published:	// IDE-managed Components
private:	// User declarations
public:		// User declarations
	__fastcall TDataModule2(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TDataModule2 *DataModule2;
//---------------------------------------------------------------------------
#endif
