// ---------------------------------------------------------------------------

#include <fmx.h>
#pragma hdrstop

#include "MatrixForm.h"
// ---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.fmx"
TForm3 *Form3;

// ---------------------------------------------------------------------------
__fastcall TForm3::TForm3(TComponent* Owner) : TForm(Owner) {
	debut = true;
}

// ---------------------------------------------------------------------------
void __fastcall TForm3::FormShow(TObject *Sender) {
	if (debut) {

		fake_data();
		draw_plan2();
		debut = false;
	}
}

// ---------------------------------------------------------------------------
void __fastcall TForm3::draw_plan2() {
	size_t j, n, p, c, d;
	TRoundRect * r1;
	float dot_size_x, dot_size_y;

	RoundRect1->Stroke->Color = TAlphaColorRec::Null;
	RoundRect1->Position->DefaultValue = TPointF(0.0, 0.0);
	// RoundRect1->Position->Point = TPointF(0.0, 0.0);

	n = m.Length;
	p = m[0].Length;
	dot_size_x = (float)ClientWidth / (float)p;
	dot_size_y = (float)ClientHeight / (float)n;
	if (dot_size_x < dot_size_y) {
		RoundRect1->Size->Height = dot_size_x;
		RoundRect1->Size->Width = dot_size_x;
	}
	else {
		RoundRect1->Size->Height = dot_size_y;
		RoundRect1->Size->Width = dot_size_y;
	}

	for (c = 0; c < n; ++c) {
		for (d = 0; d < p; ++d) {
			if (m[c][d]) {
				r1 = (TRoundRect*)(RoundRect1->Clone(0));
				r1->Position->Point = TPointF(dot_size_x * d, dot_size_y * c);
				r1->OnMouseEnter = RoundRect1MouseEnter;
				r1->OnMouseLeave = RoundRect1MouseLeave;
				Layout1->AddObject(r1);
				m[c][d] = (uintptr_t)r1;
			}
		}
	}
	RoundRect1->Visible = false;
}

// ---------------------------------------------------------------------------
void __fastcall TForm3::FormClose(TObject *Sender, TCloseAction &Action) {
	Action = TCloseAction::caHide;
}
// ---------------------------------------------------------------------------

void __fastcall TForm3::repaint_plan() {
	size_t p, c, d, n;
	float x, y;
	TRoundRect *rrct;
	float dot_size_x, dot_size_y, r;

	n = m.Length;
	p = m[0].Length;
	dot_size_x = (float)ClientWidth / (float)p;
	dot_size_y = (float)ClientHeight / (float)n;

	x = y = 0;
	r = dot_size_x < dot_size_y ? dot_size_x : dot_size_y;
	// if (dot_size_x < dot_size_y) {
	// r = dot_size_x;
	// // y = (float)(dot_size_y - dot_size_x)/(float)2;
	// y = (float)(dot_size_y - dot_size_x);
	// }
	// else {
	// r = dot_size_y;
	// // x = (float)(dot_size_x - dot_size_y)/(float)2;
	// x = (float)(dot_size_x - dot_size_y);
	// }

	for (c = 0; c < n; ++c)
		for (d = 0; d < p; ++d)
			if (m[c][d]) {
				rrct = (TRoundRect*)(m[c][d]);
				rrct->Position->Point = TPointF(r * d, r * c);
				rrct->Size->Width = r;
				rrct->Size->Height = r;
				// if (x) {
				// rrct->Position->Point = TPointF(r * d, r * c + y);
				// }
				// else if (y) {
				// rrct->Position->Point = TPointF(r * d, r * c + x);
				// }

				// if(x){
				// rrct->Margins->Left = x;
				// rrct->Margins->Right = x;
				// }else if(y)
				// {
				// rrct->Margins->Top = y;
				// rrct->Margins->Bottom = y;
				// }
			}
}

// ---------------------------------------------------------------------------
void __fastcall TForm3::FormResize(TObject *Sender) {
	if (!debut) {
		repaint_plan();
	}
}
// ---------------------------------------------------------------------------

void __fastcall TForm3::RoundRect1MouseEnter(TObject *Sender) {
	// TRoundRect *rr = (TRoundRect*)Sender;
	// Popup1->TagString = PointToString(rr->ShapeRect.CenterPoint());
	Popup1->IsOpen = true;
}
// ---------------------------------------------------------------------------

void __fastcall TForm3::RoundRect1MouseLeave(TObject *Sender) {
	// TRoundRect *rr = (TRoundRect*)Sender;
	// Popup1->Position->Point = rr->ShapeRect.CenterPoint();
	Popup1->IsOpen = false;
}
// ---------------------------------------------------------------------------

void __fastcall TForm3::Popup1Popup(TObject *Sender) {
	// ((TPopup*)Sender)->Position->Point = StringToPoint(((TPopup*)Sender)->TagString);
	// ((TPopup*)Sender)->Size->Width = 100;
	// ((TPopup*)Sender)->Size->Height = 100;
}

// ---------------------------------------------------------------------------
void __fastcall TForm3::Popup1Paint(TObject *Sender, TCanvas *Canvas, const TRectF &ARect) {
	// TPointF tpf = StringToPoint(((TPopup*)Sender)->TagString);
	// ARect.SetLocation(tpf.X, tpf.Y);

	// ARect.Location = tpf;
	// ARect.Location = tpf;
	// ARect.SetWidth(100);
	// ARect.SetHeight(100);
	// ((TPopup*)Sender)->Position->Point = StringToPoint(((TPopup*)Sender)->TagString);
	// ((TPopup*)Sender)->Size->Width = 100;
	// ((TPopup*)Sender)->Size->Height = 100;

}

// ---------------------------------------------------------------------------
void __fastcall TForm3::Popup1Resized(TObject *Sender) {
	// if (((TPopup*)Sender)->Width > 100)
	// ((TPopup*)Sender)->Width = 100;
	// if (((TPopup*)Sender)->Height > 100)
	// ((TPopup*)Sender)->Height = 100;
	((TPopup*)Sender)->Height = height_popup + 20;
	((TPopup*)Sender)->Width = width_popup + 20;
}
// ---------------------------------------------------------------------------

void __fastcall TForm3::FormCreate(TObject *Sender) {
	// GlobalUseDXSoftware = 1;
	GlobalUseDirect2D = 1;
	// GlobalUseGPUCanvas   = 1;
	GlobalUseDX = 1;
	StringGrid1->Options = StringGrid1->Options >> TGridOption::Header;
	StringGrid1->RowHeight = 20;

	StringGrid1->Columns[0]->Width = 50;
	StringGrid1->Columns[1]->Width = 150;
	StyleBook1->LoadFromFile(Format(_T("%s%s%s"), ARRAYOFCONST((ExtractFilePath(ParamStr(0)),
		_T("MaterialPatternsBlue_Win"), _T(".Style")))));
	StyleBook = StyleBook1;
}
// ---------------------------------------------------------------------------

void __fastcall TForm3::fake_data() {
	size_t i;
	StringGrid1->RowCount = popup_params.Length;
	width_popup = StringGrid1->Columns[0]->Width + StringGrid1->Columns[1]->Width;
	height_popup = StringGrid1->RowCount * StringGrid1->RowHeight;
	for (i = 0; i < (size_t)(popup_params.Length); ++i) {
		StringGrid1->Row = i;
		// Cells[column][row]
		StringGrid1->Cells[0][StringGrid1->Row] = popup_params[i];
	}
}

// ---------------------------------------------------------------------------
