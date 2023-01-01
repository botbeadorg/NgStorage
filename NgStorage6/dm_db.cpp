// ---------------------------------------------------------------------------

#pragma hdrstop

#include "dm_db.h"
#include "MainForm.h"
// ---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma classgroup "FMX.Controls.TControl"
#pragma link "DAScript"
#pragma link "DBAccess"
#pragma link "MemDS"
#pragma link "MySQLUniProvider"
#pragma link "SQLiteUniProvider"
#pragma link "Uni"
#pragma link "UniProvider"
#pragma link "UniScript"
#pragma resource "*.dfm"
TDataModule3 *DataModule3;

extern uint32_t my_crc32(char *, size_t);
extern void alloc_smlbuf(unsigned char* *);
extern void recycle_smlbuf(unsigned char *);
extern void recycle_all_smlbuf();

// ---------------------------------------------------------------------------
__fastcall TDataModule3::TDataModule3(TComponent* Owner) : TDataModule(Owner) {
}

// ---------------------------------------------------------------------------
void __fastcall TDataModule3::DataModuleCreate(TObject *Sender) {
	Timer_reconnect->Enabled = false;
	// UniConnection1->ProviderName = _T("MySQL");
	// UniConnection1->Server = _T("localhost");
	// UniConnection1->Port = 3306;
	// UniConnection1->Username = _T("root");
	// UniConnection1->Password = _T("why1983316");
	// UniConnection1->LoginPrompt = false;
	// UniConnection1->SpecificOptions->Values[_T("Charset")] = _T("utf8mb4");
	// UniConnection1->SpecificOptions->Values[_T("UseUnicode")] = _T("True");
	// UniConnection1->Connect();
	//
	// UniScript1->Connection = UniConnection1;
	// UniScript1->SQL->Text =
	// _T("CREATE DATABASE IF NOT EXISTS ngstorage CHARACTER SET=utf8mb4 COLLATE=utf8mb4_general_ci;USE ngstorage;"
	// );
	// UniScript1->Execute();
	// UniConnection1->Database = _T("ngstorage");
	//
	// UniConnection2->ProviderName = _T("MySQL");
	// UniConnection2->Server = _T("192.168.1.100");
	// UniConnection2->Port = 3306;
	// UniConnection2->Username = _T("NIroot");
	// UniConnection2->Password = _T("Nty-Mg-2018*04*09");
	// UniConnection2->Database = _T("natergy");
	// UniConnection2->LoginPrompt = false;
	// UniConnection2->SpecificOptions->Values[_T("Charset")] = _T("utf8mb4");
	// UniConnection2->SpecificOptions->Values[_T("UseUnicode")] = _T("True");
	// UniConnection2->Connect();
	//
	// home = ExtractFilePath(ParamStr(0));
	// UniConnection3->ProviderName = _T("SQLite");
	// UniConnection3->SpecificOptions->Values[_T("Direct")] = _T("True");
	// UniConnection3->SpecificOptions->Values[_T("ForceCreateDatabase")] = _T("True");
	// UniConnection3->Database = home + _T("yhw.egarotSgN");
	// UniConnection3->Open();
	//
	// UniScript3->Connection = UniConnection3;
	// UniQuery3->Connection = UniConnection3;
	// UniSQL3->Connection = UniConnection3;
	// UniScript3->SQL->Clear();
	// UniScript3->SQL->Add(_T("CREATE TABLE IF NOT EXISTS tasks_issue (\
	// id_ti  INTEGER,\
	// s  TEXT,\
	// e  TEXT,\
	// type_task  INTEGER,\
	// type_produc  INTEGER,\
	// id_warehouse  INTEGER,\
	// r_ti  INTEGER,\
	// st  INTEGER,\
	// rowid_2th  INTEGER PRIMARY KEY AUTOINCREMENT NOT NULL,\
	// CONSTRAINT \"id_task_issue\" UNIQUE (id_ti));"));
	// UniScript3->SQL->Add(_T("CREATE INDEX IF NOT EXISTS indx_orn ON loc_ware (orn DESC);"));
	// UniScript3->SQL->Add(_T("CREATE INDEX IF NOT EXISTS indx_tn ON loc_ware (tn DESC);"));
	// UniScript3->SQL->Add(_T("CREATE INDEX IF NOT EXISTS indx_mn ON loc_ware (mn ASC);"));
	// UniScript3->Execute();

}

// ---------------------------------------------------------------------------
void __fastcall TDataModule3::Timer_reconnectTimer(TObject *Sender) {
	int v;
	unsigned t_id;
	unsigned char *tmp_buf = 0;
	dx *tmp_dx = 0;
	alloc_smlbuf(&tmp_buf);
	tmp_dx = (dx*)tmp_buf;
	tmp_dx->head[0] = tmp_dx->head[1] = tmp_dx->head[2] = UINT_MAX;
	tmp_dx->st = time(0);
	tmp_dx->cmd = 742577;
	tmp_dx->len_pack = sizeof(dx)+sizeof(uint32_t);
	tmp_dx->ip = Form1->my_addr_b;
	uint32_t crc32_test = my_crc32((char *)tmp_buf, sizeof(dx));
	*(uint32_t*)(tmp_buf +sizeof(dx)) = crc32_test;
	if (SOCKET_ERROR != send(Form1->mysock, (const char *)tmp_buf, tmp_dx->len_pack, 0));
	else {
		shutdown(Form1->mysock, SD_SEND);
		int nbytes = 0;
		unsigned char buf_base[LEN_SOCK_ADDR_BUF];
		memset(buf_base, 0, LEN_SOCK_ADDR_BUF);
		for (; nbytes = recv(Form1->mysock, (char *)buf_base, LEN_SOCK_ADDR_BUF, 0) > 0;);
		closesocket(Form1->mysock);

		v = 1;
		Form1->client_conn();
		CloseHandle((void *)_beginthreadex(0, 0, display_offline_status, &v, 0, &t_id));
	}
	recycle_smlbuf(tmp_buf);
}
// ---------------------------------------------------------------------------
