// ---------------------------------------------------------------------------

#include <fmx.h>
#pragma hdrstop

#include "spec_packs1st.h"
#include "MainForm.h"

// ---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.fmx"
TForm6 *Form6;

// ---------------------------------------------------------------------------
__fastcall TForm6::TForm6(TComponent* Owner) : TForm(Owner) {
}

// ---------------------------------------------------------------------------
void __fastcall TForm6::FormCreate(TObject *Sender) {
	TResourceStream *rc_strm_style = 0;
	// rc_strm_style = new TResourceStream((int)HInstance, "Resource_Style", RT_RCDATA);
	rc_strm_style = new TResourceStream((int)HInstance, "Resource_1", RT_RCDATA);
	StyleBook1->LoadFromStream(rc_strm_style);
	StyleBook = StyleBook1;
	rc_strm_style->DisposeOf();

	for (size_t i = 0; i < Form1->records_sp.num[0]; ++i) {
		ComboBox1->Items->Add(Form1->records_sp.arys[0][i]);
	}
	for (size_t i = 0; i < Form1->records_sp.num[1]; ++i) {
		ComboBox2->Items->Add(Form1->records_sp.arys[1][i]);
	}
	for (size_t i = 0; i < Form1->records_sp.num[2]; ++i) {
		ComboBox3->Items->Add(Form1->records_sp.arys[2][i]);
	}
	for (size_t i = 0; i < Form1->records_sp.num[3]; ++i) {
		ComboBox4->Items->Add(Form1->records_sp.arys[3][i]);
	}

	rspwc = new rep_spec_pack_with_color;
	SecureZeroMemory(rspwc, sizeof(rep_spec_pack_with_color));

	Tag = (int)rspwc;
}

// ---------------------------------------------------------------------------
void __fastcall TForm6::FormDestroy(TObject *Sender) {
	delete rspwc;
}

// ---------------------------------------------------------------------------
void __fastcall TForm6::Button1Click(TObject *Sender) {
	if (!(ComboBox1->ItemIndex && ComboBox2->ItemIndex && ComboBox3->ItemIndex &&
		ComboBox4->ItemIndex && ColorComboBox1->Color)) {
		Label1->Text = _T("请检查！你使用了无效值！");
		return;
	}

	SecureZeroMemory(rspwc, sizeof(rep_spec_pack_with_color));
	AnsiString t_str = ComboBox1->Items->operator[](ComboBox1->ItemIndex);
	MoveMemory(rspwc->rep_sp.packo, t_str.c_str(), t_str.Length());
	t_str = ComboBox2->Items->operator[](ComboBox2->ItemIndex);
	MoveMemory(rspwc->rep_sp.spec, t_str.c_str(), t_str.Length());
	t_str = ComboBox3->Items->operator[](ComboBox3->ItemIndex);
	MoveMemory(rspwc->rep_sp.packi, t_str.c_str(), t_str.Length());
	t_str = ComboBox4->Items->operator[](ComboBox4->ItemIndex);
	MoveMemory(rspwc->rep_sp.company, t_str.c_str(), t_str.Length());
	rspwc->color = ColorComboBox1->Color;
	ModalResult = mrYes;
}
// ---------------------------------------------------------------------------

unsigned __fastcall TForm6::product_small_or_big() {
	unsigned r = 0;
	int i = ComboBox1->ItemIndex;
	if (i > 1) {
		r = (i - 1) < Form1->records_sp.divider[packageo] ? 1 : 2;
	}
	return r;
}

// ---------------------------------------------------------------------------
