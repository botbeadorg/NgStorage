// ---------------------------------------------------------------------------

#include <fmx.h>
#ifdef _WIN32
#include <tchar.h>
#endif
#pragma hdrstop
#include <System.StartUpCopy.hpp>
#include <boost/pool/singleton_pool.hpp>
#include "common.h"
// ---------------------------------------------------------------------------
USEFORM("spec_packs.cpp", Form5);
USEFORM("spec_packs1st.cpp", Form6);
USEFORM("warning_delete_ware_info.cpp", Form4);
USEFORM("task_list.cpp", Form2);
USEFORM("MatrixForm.cpp", Form3);
USEFORM("dm_db.cpp", DataModule3); /* TDataModule: File Type */
USEFORM("FrmAGV.cpp", Frame1); /* TFrame: File Type */
USEFORM("MainForm.cpp", Form1);
USEFORM("http_conn.cpp", DataModule1); /* TDataModule: File Type */
USEFORM("BulkTasksForm.cpp", Form7);
//---------------------------------------------------------------------------
typedef boost::singleton_pool < struct smlbuf_tag {
}, 256 > pool_smlbuf;

typedef boost::singleton_pool < struct bigbuf_tag {
}, LEN_SOCK_ADDR_BUF > pool_bigbuf;

void alloc_smlbuf(unsigned char* *p) {
	*p = (unsigned char *)pool_smlbuf::malloc();
	SecureZeroMemory(*p, 256);
}

void recycle_smlbuf(unsigned char *p) {
	pool_smlbuf::free(p);
}

void recycle_all_smlbuf() {
	pool_smlbuf::purge_memory();
}

void alloc_bigbuf(unsigned char* *p) {
	*p = (unsigned char *)pool_bigbuf::malloc();
	SecureZeroMemory(*p, LEN_SOCK_ADDR_BUF);
}

void recycle_bigbuf(unsigned char *p) {
	pool_bigbuf::free(p);
}

void recycle_all_bigbug() {
	pool_bigbuf::purge_memory();
}

// ---------------------------------------------------------------------------
extern "C" int FMXmain() {
	try {
		Application->Initialize();
		// CreateMutex(0, 0, params[UI]);
		// if (ERROR_ALREADY_EXISTS == GetLastError())
		// return 0;
		Application->CreateForm(__classid(TDataModule1), &DataModule1);
		Application->CreateForm(__classid(TDataModule3), &DataModule3);
		Application->CreateForm(__classid(TForm1), &Form1);
		Application->CreateForm(__classid(TForm2), &Form2);
		Application->CreateForm(__classid(TForm4), &Form4);
		Application->CreateForm(__classid(TForm5), &Form5);
		Application->CreateForm(__classid(TForm6), &Form6);
		Application->CreateForm(__classid(TForm7), &Form7);
		Application->Run();
	}
	catch (Exception &exception) {
		Application->ShowException(&exception);
	}
	catch (...) {
		try {
			throw Exception("");
		}
		catch (Exception &exception) {
			Application->ShowException(&exception);
		}
	}
	return 0;
}
// ---------------------------------------------------------------------------
