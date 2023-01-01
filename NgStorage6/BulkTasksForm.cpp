// ---------------------------------------------------------------------------

#include <fmx.h>
#pragma hdrstop

#include "BulkTasksForm.h"
// ---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.fmx"
TForm7 *Form7;

// ---------------------------------------------------------------------------
__fastcall TForm7::TForm7(TComponent* Owner) : TForm(Owner) {
}

// ---------------------------------------------------------------------------
void __fastcall TForm7::FormCreate(TObject *Sender) {
	TResourceStream *rc_strm_style = 0;
	// rc_strm_style = new TResourceStream((int)HInstance, "Resource_Style", RT_RCDATA);
	rc_strm_style = new TResourceStream((int)HInstance, "Resource_1", RT_RCDATA);
	StyleBook1->LoadFromStream(rc_strm_style);
	StyleBook = StyleBook1;
	rc_strm_style->DisposeOf();

	// StringGrid2->AutoCalculateContentSize = true;
	// StringGrid1->AutoCalculateContentSize = 1;
	// SecureZeroMemory(txts, sizeof(txts));
	// SecureZeroMemory(checkboxes_n, sizeof(checkboxes_n));
	// SecureZeroMemory(checkboxes_s, sizeof(checkboxes_s));

	ScrollBox1->Align = TAlignLayout::Client;

	anic = new TAniCalculations(0);
	anic->Animation = 1;
	anic->AutoShowing = 1;
	anic->Averaging = 1;
	ScrollBox1->AniCalculations->Assign(anic);

	Layout1->Parent = ScrollBox1;
	Layout1->SetBounds(0, 0, Width, Height *2);

	// //CheckBox1->Parent =  Layout1;
	// txts_produce[0] = (TText *)(Text1);
	// txts_produce[0]->Text = _T("±±");
	// txts_produce[0]->Tag = N;
	// txts_produce[0]->Font->Size = 30;
	// txts_produce[0]->Position->Point = TPointF(50, 30);
	// //txts_produce[0]->Parent = Layout1;
	// Layout1->AddObject(txts_produce[0]);
	// txts_produce[0]->Visible = 1;
	//
	// txts_produce[1] = (TText *)(Text1);
	// txts_produce[1]->Text = _T("ÄÏ");
	// txts_produce[1]->Tag = S;

	// Text1->Visible = 0;
	ScrollBox1->Align = TAlignLayout::Client;


}

// ---------------------------------------------------------------------------
void __fastcall TForm7::FormDestroy(TObject *Sender) {
	delete anic;
}
// ---------------------------------------------------------------------------
