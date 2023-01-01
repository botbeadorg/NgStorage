// ---------------------------------------------------------------------------

#include <fmx.h>
#pragma hdrstop

#include "warning_delete_ware_info.h"
// ---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.fmx"
TForm4 *Form4;

// ---------------------------------------------------------------------------
__fastcall TForm4::TForm4(TComponent* Owner) : TForm(Owner) {
}

// ---------------------------------------------------------------------------
void __fastcall TForm4::Button1Click(TObject *Sender) {
	ModalResult = mrNo;
}

// ---------------------------------------------------------------------------
void __fastcall TForm4::Button2Click(TObject *Sender) {
	ModalResult = mrYes;
}

// ---------------------------------------------------------------------------
void __fastcall TForm4::FormCreate(TObject *Sender) {
	TResourceStream *rc_strm_style = 0;
	// rc_strm_style = new TResourceStream((int)HInstance, "Resource_Style", RT_RCDATA);
	rc_strm_style = new TResourceStream((int)HInstance, "Resource_1", RT_RCDATA);
	StyleBook1->LoadFromStream(rc_strm_style);
	StyleBook = StyleBook1;
	rc_strm_style->DisposeOf();
}
// ---------------------------------------------------------------------------
