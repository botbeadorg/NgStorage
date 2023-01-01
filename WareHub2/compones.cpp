// ---------------------------------------------------------------------------

#pragma hdrstop

#include "common.h"
#include "compones.h"

// ---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma classgroup "Vcl.Controls.TControl"
#pragma link "SQLiteUniProvider"
#pragma link "UniProvider"
#pragma link "DBAccess"
#pragma link "Uni"
#pragma link "MemDS"
#pragma link "DAScript"
#pragma link "UniScript"
#pragma link "SQLiteUniProvider"
#pragma resource "*.dfm"
TDataModule1 *DataModule1;

// ---------------------------------------------------------------------------
__fastcall TDataModule1::TDataModule1(TComponent* Owner) : TDataModule(Owner) {
	// u_int x = (u_int)NetHTTPRequest0;
}
// ---------------------------------------------------------------------------

void __fastcall TDataModule1::IdHTTPServer1CommandGet(TIdContext *AContext,
	TIdHTTPRequestInfo *ARequestInfo, TIdHTTPResponseInfo *AResponseInfo) {
	bool err;
	int err_code;
	u_int cmd;
	String tmp_str, err_str;

	if (hcPOST != ARequestInfo->CommandType)
		return;

	UI_CONVERT_ERR_RESP(ARequestInfo->Params->Values[_T("cmd")], cmd, -1,
		_T("POST parameter error: cmd"));

	AResponseInfo->FreeContentStream = true;
	if (657194 == cmd) { // task issue
		size_t tmp_id_ti;
		task_issue ti;
		String str_sn_loc;
		SecureZeroMemory(&ti, sizeof(ti));

		EnterCriticalSection(&cs_id_task);
		tmp_id_ti = gId_task_issue;
		++gId_task_issue;
		gId_task_issue = gId_task_issue ? gId_task_issue : 1;
		update_id_ti(tmp_id_ti);
		LeaveCriticalSection(&cs_id_task);

		ti.one_pc.t_id = tmp_id_ti;
		UI_CONVERT_ERR_RESP(ARequestInfo->Params->Values[_T("id_client")], ti.id_client, -1,
			_T("POST parameter error: id_client"));
		UI_CONVERT_ERR_RESP(ARequestInfo->Params->Values[_T("type_product")],
			ti.one_pc.type_product, -1, _T("POST parameter error: type_product"));
		UI_CONVERT_ERR_RESP(ARequestInfo->Params->Values[_T("id_warehouse")], ti.id_warehouse, -1,
			_T("POST parameter error: id_warehouse"));
		UI_CONVERT_ERR_RESP(ARequestInfo->Params->Values[_T("tsk")], ti.one_pc.tsk, -1,
			_T("POST parameter error: tsk"));
		UI_CONVERT_ERR_RESP(ARequestInfo->Params->Values[_T("st")], ti.st, -1,
			_T("POST parameter error: st"));

		str_sn_loc = ARequestInfo->Params->Values[_T("sn_loc")];
		txtLocToInt(ti.one_pc.sn_loc, str_sn_loc);

		UI_CONVERT_ERR_RESP(ARequestInfo->Params->Values[_T("color")], ti.color, -1,
			_T("POST parameter error: color"));
		ti.packo = ARequestInfo->Params->Values[_T("packo")];
		ti.spec = ARequestInfo->Params->Values[_T("spec")];
		ti.packi = ARequestInfo->Params->Values[_T("packi")];
		ti.company = ARequestInfo->Params->Values[_T("company")];

		ti.r_ti = rti_pending;

		EnterCriticalSection(&cs_tis);
		tis.push_back(ti);
		LeaveCriticalSection(&cs_tis);

		unsigned char *new_buf = 0;
		dx *new_dx = 0;

		BUILD_DX_BLOCK(new_buf, new_dx, 657194, 0, ti.st, tmp_id_ti, &(ti.one_pc),
			sizeof(ti.one_pc));

		EnterCriticalSection(&cs_send2tcpsrv);
		send(dispatcher, new_buf, new_dx->len_pack, 0);
		LeaveCriticalSection(&cs_send2tcpsrv);
		putback_smlbuf(new_buf);

		AResponseInfo->ContentType = _T("text/plain");
		AResponseInfo->CharSet = _T("utf-8");
		AResponseInfo->ContentText = UIntToStr((unsigned)tmp_id_ti);
		AResponseInfo->ResponseNo = 200;
	}
	else if (657308 == cmd) { // return client id
		u_int tmp_gid;
		AResponseInfo->ContentType = _T("text/plain");
		AResponseInfo->CharSet = _T("utf-8");

		EnterCriticalSection(&cs_id_client);
		tmp_gid = gId_client;
		++gId_client;
		LeaveCriticalSection(&cs_id_client);

		AResponseInfo->ContentText = UIntToStr((unsigned)tmp_gid);
		AResponseInfo->ResponseNo = 200;
	}
	else if (678000 == cmd) { // query task issure result
		size_t c = 0;
		size_t r = rti_pending;
		std::vector<task_issue>t_tis;

		UI_CONVERT_ERR_RESP(ARequestInfo->Params->Values[_T("id_ti")], c, -1,
			_T("POST parameter error: id_ti"));

		EnterCriticalSection(&cs_tis);
		t_tis.assign(tis.begin(), tis.end());
		LeaveCriticalSection(&cs_tis);

		for (std::vector<task_issue>::iterator it_ti = t_tis.begin(); t_tis.end() != it_ti; ++it_ti)
		{
			task_issue one_ti = *it_ti;
			if (one_ti.one_pc.t_id == c) {
				r = one_ti.r_ti;
			}
		}

		AResponseInfo->ContentType = _T("text/plain");
		AResponseInfo->CharSet = _T("utf-8");
		AResponseInfo->ContentText = UIntToStr((unsigned)r);
		AResponseInfo->ResponseNo = 200;
	}
	else if (678001 == cmd) { // read wp
		u_int id_client;
		size_t i;
		TMemoryStream *ms = 0;
		std::vector<wp_with_ip>::iterator it_wps;
		wp_with_ip wwi;
		UI_CONVERT_ERR_RESP(ARequestInfo->Params->Values[_T("id_client")], id_client, -1,
			_T("POST parameter error: id_client"));
		ms = new TMemoryStream();
		AResponseInfo->ContentType = _T("application/octet-stream");
		AResponseInfo->ResponseNo = 200;
		AResponseInfo->ContentStream = ms;
		SecureZeroMemory(&wwi, sizeof(wwi));
		ms->Seek32(0, soBeginning);

		EnterCriticalSection(&cs_rcr_wp);
		for (i = 0; i < rcr_wp.count_readers; ++i)
			if (rcr_wp.pst[i].id_client != id_client);
			else
				break;

		if (i != rcr_wp.count_readers) { // existing reader
			EnterCriticalSection(&cs_wps);
			if (wps.empty()) {

			}
			else {
				it_wps = wps.begin();
				wwi = *(it_wps + rcr_wp.pst[i].new_prev);
				ms->Write(&wwi, sizeof wwi);
			}
			LeaveCriticalSection(&cs_wps);
			++rcr_wp.pst[i].new_prev;
		}
		else { // new reader
			EnterCriticalSection(&cs_wps);
			if (wps.empty()) {
				// read nothing
				ms->Write(&wwi, sizeof wwi);
				rcr_wp.pst[rcr_wp.count_readers].new_prev = -1;
			}
			else {
				it_wps = wps.end();
				// read the last element
				wwi = *(--it_wps);
				ms->Write(&wwi, sizeof wwi);
				rcr_wp.pst[rcr_wp.count_readers].new_prev = wps.size() - 1;
			}
			LeaveCriticalSection(&cs_wps);
			rcr_wp.pst[rcr_wp.count_readers].id_client = id_client;
			++rcr_wp.count_readers;
		}
		LeaveCriticalSection(&cs_rcr_wp);
	}
	else if (678002 == cmd) { // read ge
		size_t c = 0;
		std::vector<agv>::iterator it_agv;
		quant_elec_agv qea[16];
		SecureZeroMemory(qea, sizeof qea);

		EnterCriticalSection(&cs_agvs);
		for (it_agv = agvs.begin(); agvs.end() != it_agv; ++it_agv) {
			qea[c].addr_b_agv = (*it_agv).addr_b;
			qea[c].quant = (*it_agv).battery;
			++c;
		}
		LeaveCriticalSection(&cs_agvs);
		TMemoryStream *ms = new TMemoryStream();
		AResponseInfo->ContentType = _T("application/octet-stream");
		ms->Write(qea, sizeof(qea));
		AResponseInfo->ContentStream = ms;
		AResponseInfo->ResponseNo = 200;
	}
	else if (678003 == cmd) { // query agvs registered
		size_t i = 0;
		std::vector<agv>::iterator it_agv;
		agv *info_agvs = new agv[quota_agvs];
		SecureZeroMemory(info_agvs, sizeof(agv)* quota_agvs);
		// EnterCriticalSection(&cs_agvs);
		EnterCriticalSection(&cs_agvs);
		for (it_agv = agvs.begin(); agvs.end() != it_agv; ++it_agv) {
			info_agvs[i] = *it_agv;
			++i;
		}
		// LeaveCriticalSection(&cs_agvs);
		LeaveCriticalSection(&cs_agvs);
		TMemoryStream *ms = new TMemoryStream();
		AResponseInfo->ContentType = _T("application/octet-stream");
		ms->Write(info_agvs, sizeof(agv)* quota_agvs);
		AResponseInfo->ContentStream = ms;
		AResponseInfo->ResponseNo = 200;
		delete[]info_agvs;
	}
	else if (678005 == cmd) { // query the fresh agv registered at runtime
		size_t i = 0;
		TMemoryStream *ms = 0;
		agv *info_agvs = new agv[quota_agvs];
		SecureZeroMemory(info_agvs, sizeof(agv)* quota_agvs);
		EnterCriticalSection(&cs_agvs);
		for (std::vector<agv>::iterator it_agv = agvs.begin(); agvs.end() != it_agv; ++it_agv) {
			info_agvs[i] = *it_agv;
			++i;
		}
		LeaveCriticalSection(&cs_agvs);
		ms = new TMemoryStream();
		AResponseInfo->ContentType = _T("application/octet-stream");
		ms->Write(info_agvs, sizeof(agv)* quota_agvs);
		AResponseInfo->ContentStream = ms;
		AResponseInfo->ResponseNo = 200;
		delete[]info_agvs;
	}
	else if (678006 == cmd) { // query the number of storlocs that holds wares
		size_t r = 0;
		result_storloc->SQL->Text = _T("select count(*) from ware_storloc");
		result_storloc->Execute();
		if (result_storloc->RecordCount)
			r = result_storloc->Fields->operator[](0)->AsInteger;
		AResponseInfo->ContentType = _T("text/plain");
		AResponseInfo->CharSet = _T("utf-8");
		AResponseInfo->ContentText = UIntToStr((unsigned)r);
		AResponseInfo->ResponseNo = 200;
	}
	else if (678007 == cmd) { // return the existing locations that have been taken up
		size_t np = 0;
		rep_storloc *rep = 0;
		TMemoryStream *ms = 0;
		UI_CONVERT_ERR_RESP(ARequestInfo->Params->Values[_T("number_points")], np, -1,
			_T("POST parameter error: number_points"));

		result_storloc->SQL->Text = _T("select * from ware_storloc");
		result_storloc->Execute();
		np = result_storloc->RecordCount;
		rep = new rep_storloc[np];
		SecureZeroMemory(rep, sizeof(rep_storloc) * np);

		result_storloc->First();
		for (size_t i = 0; i < np; ++i) {
			AnsiString name;
			rep[i].loc[0] = result_storloc->FieldByName(_T("warehouse"))->AsInteger;
			rep[i].loc[1] = result_storloc->FieldByName(_T("storey"))->AsInteger;
			rep[i].loc[2] = result_storloc->FieldByName(_T("area"))->AsInteger;
			rep[i].loc[3] = result_storloc->FieldByName(_T("row"))->AsInteger;
			rep[i].loc[4] = result_storloc->FieldByName(_T("column"))->AsInteger;
			rep[i].color = result_storloc->FieldByName(_T("color"))->AsLongWord;
			rep[i].st = result_storloc->FieldByName(_T("st"))->AsLongWord;
			name = result_storloc->FieldByName(_T("name_storloc"))->AsAnsiString;
			MoveMemory(rep[i].txt, name.c_str(), name.Length());
			result_storloc->Next();
		}

		AResponseInfo->ContentType = _T("application/octet-stream");
		ms = new TMemoryStream();
		ms->Write(rep, sizeof(rep_storloc)* np);
		AResponseInfo->ResponseNo = 200;
		AResponseInfo->ContentStream = ms;

		delete[]rep;
	}
	else if (678009 == cmd) { // delete stroloc after moving a stack
		String name_storloc = ARequestInfo->Params->Values[_T("name_storloc")];
		if (name_storloc.operator != (_T(""))) {
			noresult_storloc->SQL->Clear();
			noresult_storloc->SQL->Text =
				Sysutils::Format(_T("DELETE FROM ware_storloc where name_storloc = '%s';"),
				ARRAYOFCONST((name_storloc)));
			for (; sqlcn_storloc->InTransaction;);
			sqlcn_storloc->StartTransaction();
			try {
				noresult_storloc->Execute();
				sqlcn_storloc->Commit();
			}
			catch (...) {
				sqlcn_storloc->Rollback();
			}
		}
		AResponseInfo->ResponseNo = 200;
	}
	else if (678010 == cmd) { // status of tasks

		size_t bein;
		size_t i = 0;
		size_t p = 0;
		size_t count = StrToUInt(ARequestInfo->Params->Values[_T("count")]);
		TMemoryStream *ms = 0;
		running_status_taskissue *rst = new running_status_taskissue[count];
		SecureZeroMemory(rst, sizeof(running_status_taskissue)*count);
		String ids = ARequestInfo->Params->Values[_T("ids")];
		while (p = ids.Pos(_T(","))) {

			bein = 0;
			rst[i].t_id = StrToUInt(ids.SubString(1, p - 1));
			EnterCriticalSection(&cs_tis);
			for (std::vector<task_issue>::reverse_iterator rit = tis.rbegin(); tis.rend() != rit;
			++rit) {
				task_issue ti = *rit;
				if (ti.one_pc.t_id == rst[i].t_id) {
					bein = 1;
					rst[i].state_running = 1;
					break;
				}
			}
			LeaveCriticalSection(&cs_tis);

			if (!bein) {
				EnterCriticalSection(&cs_tis_over);
				for (std::vector<task_issue_over>::iterator it = tis_over.begin();
				tis_over.end() != it; ++it) {
					task_issue_over tio = *it;
					if (tio.ti.one_pc.t_id == rst[i].t_id) {
						bein = 2;
						rst[i].state_running = 2;
						rst[i].color = tio.color;
						break;
					}
				}
				LeaveCriticalSection(&cs_tis_over);
			}

			ids.Delete(1, p);
			++i;
		}
		ms = new TMemoryStream();
		AResponseInfo->ContentType = _T("application/octet-stream");
		ms->Write(rst, sizeof(running_status_taskissue)* count);
		AResponseInfo->ContentStream = ms;
		AResponseInfo->ResponseNo = 200;

		delete[]rst;
	}
	else if (657311 == cmd) { // query product info at a certein point
		size_t count_rcds = 0;
		TMemoryStream *ms = 0;
		rep_spec_pack rsp12;
		SecureZeroMemory(&rsp12, sizeof(rsp12));

		try {
			String name_storloc = ARequestInfo->Params->Values[_T("name_storloc")];

			result_storloc->SQL->Text =
				Sysutils::Format
				(_T("SELECT st,packo,spec,packi,company FROM ware_storloc WHERE name_storloc = '%s';"),
				ARRAYOFCONST((name_storloc)));
			result_storloc->Execute();

			count_rcds = result_storloc->RecordCount;
			if (1 == count_rcds) {
				unsigned st = result_storloc->FieldByName("st")->AsLongWord;
				AnsiString packo = result_storloc->FieldByName("packo")->AsString;
				AnsiString spec = result_storloc->FieldByName("spec")->AsString;
				AnsiString packi = result_storloc->FieldByName("packi")->AsAnsiString;
				AnsiString company = result_storloc->FieldByName("company")->AsAnsiString;
				MoveMemory(rsp12.packo, packo.c_str(), packo.Length());
				MoveMemory(rsp12.spec, spec.c_str(), spec.Length());
				MoveMemory(rsp12.packi, packi.c_str(), packi.Length());
				MoveMemory(rsp12.company, company.c_str(), company.Length());
				ms = new TMemoryStream();
				AResponseInfo->ContentType = _T("application/octet-stream");
				ms->Write(&rsp12, sizeof(rsp12));
				AResponseInfo->ContentStream = ms;
				AResponseInfo->ResponseNo = 200;
			}
		}
		catch (...) {
			printf("error when executing 657311\n");
		}
	}
	else if (657312 == cmd) {
		size_t count_rcds = 0;
		unsigned color_i;
		try {
			size_t storey = ARequestInfo->Params->Values[_T("storey")].ToInt();
			size_t area = ARequestInfo->Params->Values[_T("area")].ToInt();
			size_t row = ARequestInfo->Params->Values[_T("row")].ToInt();
			size_t column = ARequestInfo->Params->Values[_T("column")].ToInt();
			String name_storloc = ARequestInfo->Params->Values[_T("name_storloc")];
			String packo = ARequestInfo->Params->Values[_T("packo")];
			String spec = ARequestInfo->Params->Values[_T("spec")];
			String packi = ARequestInfo->Params->Values[_T("packi")];
			String company = ARequestInfo->Params->Values[_T("company")];
			String color = ARequestInfo->Params->Values[_T("color")];
			result_storloc->SQL->Clear();
			result_storloc->SQL->Add
				(Sysutils::Format(_T("SELECT * FROM ware_storloc WHERE name_storloc = '%s';"),
				ARRAYOFCONST((name_storloc))));

			TryStrToUInt(color, color_i);

			result_storloc->Execute();
			count_rcds = result_storloc->RecordCount;
			if (1 == count_rcds) {
				noresult_storloc->SQL->Clear();
				noresult_storloc->SQL->Text =
					Sysutils::Format(_T("DELETE FROM ware_storloc where name_storloc = '%s';"),
					ARRAYOFCONST((name_storloc)));
				for (; sqlcn_storloc->InTransaction;);
				sqlcn_storloc->StartTransaction();
				try {
					noresult_storloc->Execute();
					sqlcn_storloc->Commit();
				}
				catch (...) {
					sqlcn_storloc->Rollback();
				}
				noresult_storloc->SQL->Clear();
				noresult_storloc->SQL->Add(Sysutils::Format("INSERT INTO ware_storloc (storey, area, row, column, packo, spec, packi, company, color, name_storloc)\
					 VALUES(%u, %u, %u, %u, '%s', '%s', '%s', '%s', %u, '%s');",
					ARRAYOFCONST((storey, area, row, column, packo, spec, packi, company, color_i,
					name_storloc))));
				for (; sqlcn_storloc->InTransaction;);
				sqlcn_storloc->StartTransaction();
				try {
					noresult_storloc->Execute();
					sqlcn_storloc->Commit();
				}
				catch (...) {
					sqlcn_storloc->Rollback();
				}
			}
			else {
				noresult_storloc->SQL->Clear();
				noresult_storloc->SQL->Add(Sysutils::Format("INSERT INTO ware_storloc (storey, area, row, column, packo, spec, packi, company, color, name_storloc)\
					 VALUES(%u, %u, %u, %u, '%s', '%s', '%s', '%s', %u, '%s');",
					ARRAYOFCONST((storey, area, row, column, packo, spec, packi, company, color_i,
					name_storloc))));
				for (; sqlcn_storloc->InTransaction;);
				sqlcn_storloc->StartTransaction();
				try {
					noresult_storloc->Execute();
					sqlcn_storloc->Commit();
				}
				catch (...) {
					sqlcn_storloc->Rollback();
				}
			}
		}
		catch (...) {
			printf("error when executing 657312\n");
		}
		AResponseInfo->ResponseNo = 200;
	}

ERR_END:
	switch (err_code) {
	case -1:
		AResponseInfo->ContentText = err_str;
		break;
	default: ;
	}
}

// st INTEGER NOT NULL DEFAULT (datetime(CURRENT_TIMESTAMP, 'localtime')),\
// ---------------------------------------------------------------------------
void __fastcall TDataModule1::DataModuleCreate(TObject *Sender) {
	UNICN(UniConnection1, 1);
	UNICN(sqlitecn_agv, 0);
	UNICN(sqlcn_storloc, 0);
	UNICN(sqlitecn_ti, 0);
	result_storloc->FetchRows = 1000;
#ifndef _ATHOME
	// mysqlcn_finished_prod->ConnectString =
	// _T("Provider Name=MySQL;User ID=warehouse;Password=\"8jsjSS8B3jdYnOqu\";Data Source=192.168.200.10;Database=natergy;Port=12310;Login Prompt=False"
	// );
	// mysqlcn_finished_prod->SpecificOptions->Values[_T("Charset")] = _T("utf8mb4");
	// mysqlcn_finished_prod->SpecificOptions->Values[_T("UseUnicode")] = _T("True");
	// result_finished_prod->Connection = mysqlcn_finished_prod;
	// result_finished_prod->FetchRows = 1000;
	// result_finished_prod->Options->AutoPrepare = 1;
	// result_finished_prod->Options->FlatBuffers = 1;
	// mysqlcn_finished_prod->Connect();
#endif

	UniScript1->Connection = UniConnection1;
	UniScript1->SQL->Text = _T("CREATE TABLE IF NOT EXISTS agvs (\
ip INTEGER NOT NULL,\
storey INTEGER NOT NULL,\
id INTEGER NOT NULL,\
id_warehouse INTEGER NOT NULL DEFAULT 1,\
name TEXT NOT NULL,\
rowid_2th INTEGER PRIMARY KEY AUTOINCREMENT,\
UNIQUE (ip));CREATE TABLE IF NOT EXISTS ware_storloc (\
warehouse INTEGER NOT NULL DEFAULT 1,\
storey INTEGER NOT NULL DEFAULT 1,\
area INTEGER NOT NULL,\
row INTEGER NOT NULL,\
column INTEGER NOT NULL,\
st integer not null default (strftime('%s','now')),\
packo TEXT,\
spec TEXT,\
packi TEXT,\
company TEXT,\
color INTEGER,\
name_storloc TEXT,\
rowid_2th INTEGER PRIMARY KEY AUTOINCREMENT,\
UNIQUE (area, row, column));CREATE TABLE IF NOT EXISTS next_id_ti (\
next_id_ti UNSIGNED BIG INT NOT NULL,\
rowid_2th INTEGER PRIMARY KEY AUTOINCREMENT\
);CREATE TABLE IF NOT EXISTS prodsIO (\
st integer not null,\
packo TEXT,\
spec TEXT,\
packi TEXT,\
company TEXT,\
st1 integer not null default (strftime('%s','now')),\
rowid_2th INTEGER PRIMARY KEY AUTOINCREMENT\
);");
	// The serial number of Aera is globally unique in all the warehouses
	UniScript1->Execute();

	id_agv_max = 0;

}
// ---------------------------------------------------------------------------

void __fastcall TDataModule1::DataModuleDestroy(TObject *Sender) {
	sqlitecn_agv->Close();
	sqlcn_storloc->Close();
	UniConnection1->Close();

#ifndef _ATHOME
	// mysqlcn_finished_prod->Close();
#endif
}
// ---------------------------------------------------------------------------

void __fastcall TDataModule1::load_agvs() {
	size_t count_rcds = 0;
	result_agv->SQL->Text = _T("SELECT ip,storey,id,id_warehouse,name FROM agvs");
	result_agv->Execute();
	count_rcds = result_agv->RecordCount;
	result_agv->First();
	for (size_t i = 0; i < count_rcds; ++i) {
		agv a;
		AnsiString name_agv("");
		SecureZeroMemory(&a, sizeof(a));
		a.addr_b = result_agv->FieldByName("ip")->AsInteger;
		a.storey = result_agv->FieldByName("storey")->AsInteger;
		a.id = result_agv->FieldByName("id")->AsInteger;
		if (a.id > id_agv_max)
			id_agv_max = a.id;
		a.id_warehouse = result_agv->FieldByName("id_warehouse")->AsInteger;
		name_agv = result_agv->FieldByName("name")->AsAnsiString;
		MoveMemory(a.name, name_agv.c_str(), name_agv.Length());
		agvs.push_back(a);
		result_agv->Next();
	}
}
// ---------------------------------------------------------------------------

void __fastcall TDataModule1::reg_agv(dx *one_pack) {
	bool bein = false;
	agv v_taken;
	std::vector<agv>::iterator b;
	SecureZeroMemory(&v_taken, sizeof(v_taken));
	EnterCriticalSection(&cs_agvs);
	for (b = agvs.begin(); agvs.end() != b; ++b)
		if (one_pack->ip == (*b).addr_b) {
			v_taken = (*b);
			bein = true;
			break;
		}
	LeaveCriticalSection(&cs_agvs);

	if (bein) {
		size_t new_storey = 0;
		if (657185 == one_pack->cmd)
			new_storey = 1;
		else if (657186 == one_pack->cmd)
			new_storey = 2;

		if (new_storey != v_taken.storey) {
			noresult_agv->SQL->Clear();
			noresult_agv->SQL->Text = Sysutils::Format("UPDATE agvs SET storey = %u WHERE ip = %u;",
				ARRAYOFCONST((new_storey, v_taken.addr_b)));
			for (; sqlitecn_agv->InTransaction;);
			sqlitecn_agv->StartTransaction();
			try {
				noresult_agv->Execute();
				sqlitecn_agv->Commit();
			}
			catch (...) {
				sqlitecn_agv->Rollback();
			}
			EnterCriticalSection(&cs_agvs);
			for (b = agvs.begin(); agvs.end() != b; ++b)
				if (v_taken.addr_b == (*b).addr_b) {
					(*b).storey = new_storey;
					break;
				}
			LeaveCriticalSection(&cs_agvs);
		}
	}
	else {
		agv v;
		AnsiString prefix_agv("AGV");
		SecureZeroMemory(&v, sizeof(v));
		v.battery = UINT_MAX;
		v.addr_b = one_pack->ip;
		// v.storey = (657185 == one_pack->cmd) ? 1 : 2;
		if (657185 == one_pack->cmd)
			v.storey = 1;
		else if (657186 == one_pack->cmd)
			v.storey = 2;
#ifdef _DEBUG
		printf("agv %u registered in storey %u\n", v.addr_b, v.storey);
#endif
		v.latest = one_pack->st;
		v.id = ++id_agv_max;
		prefix_agv.operator += (v.addr_b);
		MoveMemory(v.name, prefix_agv.c_str(), prefix_agv.Length());
		EnterCriticalSection(&cs_agvs);
		agvs.push_back(v);
		LeaveCriticalSection(&cs_agvs);

		noresult_agv->SQL->Clear();
		noresult_agv->SQL->Add
			(Sysutils::Format("INSERT INTO agvs (ip, storey, id, name) VALUES(%u, %u, %u, '%s');",
			ARRAYOFCONST((v.addr_b, v.storey, v.id, v.name))));
		for (; sqlitecn_agv->InTransaction;);
		sqlitecn_agv->StartTransaction();
		try {
			noresult_agv->Execute();
			sqlitecn_agv->Commit();
		}
		catch (...) {
			sqlitecn_agv->Rollback();
		}
	}
}

// ---------------------------------------------------------------------------
unsigned __fastcall TDataModule1::next_id_ti() {
	result_ti->SQL->Text = _T("SELECT next_id_ti FROM next_id_ti;");
	result_ti->Execute();
	if (result_ti->RecordCount) {
		result_ti->First();
		gId_task_issue = result_ti->FieldByName(_T("next_id_ti"))->AsLongWord;
	}
	else {
		gId_task_issue = 1;
		noresult_ti->SQL->Clear();
		noresult_ti->SQL->Text = _T("INSERT INTO next_id_ti (next_id_ti) VALUES(1);");
		for (; sqlitecn_ti->InTransaction;);
		sqlitecn_ti->StartTransaction();
		try {
			noresult_ti->Execute();
			sqlitecn_ti->Commit();
		}
		catch (...) {
			sqlitecn_ti->Rollback();
		}
	}
}
// ---------------------------------------------------------------------------

void __fastcall TDataModule1::update_id_ti(unsigned new_id) {
	noresult_ti->SQL->Clear();
	noresult_ti->SQL->Text = Sysutils::Format("UPDATE next_id_ti SET next_id_ti = %u;",
		ARRAYOFCONST((new_id)));
	for (; sqlitecn_ti->InTransaction;);
	sqlitecn_ti->StartTransaction();
	try {
		noresult_ti->Execute();
		sqlitecn_ti->Commit();
	}
	catch (...) {
		sqlitecn_ti->Rollback();
	}
}
// ---------------------------------------------------------------------------

