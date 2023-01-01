// ---------------------------------------------------------------------------

#include <fmx.h>
#pragma hdrstop

#include "task_list.h"
// ---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.fmx"
TForm2 *Form2;

const String txt_gridhead[9] = {
	_T("�����·����"), _T("���"), _T("�յ�"), _T("��������"), _T("��Ʒ����"), _T("�ֿ��"), _T("ʱ���"), _T("�·����"),
		_T("ִ��״̬")};

// ---------------------------------------------------------------------------
__fastcall TForm2::TForm2(TComponent* Owner) : TForm(Owner) {
}

// ---------------------------------------------------------------------------
void __fastcall TForm2::FormCreate(TObject *Sender) {
	TResourceStream *rc_strm_style = 0;
	// rc_strm_style = new TResourceStream((int)HInstance, "Resource_Style", RT_RCDATA);
	rc_strm_style = new TResourceStream((int)HInstance, "Resource_1", RT_RCDATA);
	StyleBook1->LoadFromStream(rc_strm_style);
	StyleBook = StyleBook1;
	rc_strm_style->DisposeOf();

	for (size_t i = 0; i < StringGrid1->ColumnCount; ++i) {
		StringGrid1->Columns[i]->Header = txt_gridhead[i];
		// StringGrid1->Columns[i]->ReadOnly = 1;
		StringGrid1->Columns[i]->Width = 110;
	}
	// StringGrid1->Cells[0][0]= _T("ss");  // the left-top cell under the header
	myrowcount = 0;
	StringGrid1->ReadOnly = 1;
}

// ---------------------------------------------------------------------------
void __fastcall TForm2::StringGrid1ApplyStyleLookup(TObject *Sender) {
	// int a;
	// THeader *h;
	// THeaderItem *i;
	// if (StringGrid1->FindStyleResource("header", h)) {
	// h->Height = 30;
	// for (a = 0; a < h->Count - 1; ++a) {
	// i = h->Items[a];
	// //i->StyledSettings = i->StyledSettings >> TStyledSetting::FontColor;
	// i->StyledSettings = i->StyledSettings >> TStyledSetting::Size;
	// i->StyledSettings = i->StyledSettings >> TStyledSetting::Family;
	// i->StyledSettings.Clear();
	// //i->TextSettings->HorzAlign = TTextAlign::Center;
	// i->TextSettings->Font->Family = _T("Microsoft YaHei UI");
	// i->TextSettings->Font->Size = 14;
	// //i->TextSettings->FontColor = TAlphaColors::Lightskyblue;
	// }
	// }
}
// ---------------------------------------------------------------------------
