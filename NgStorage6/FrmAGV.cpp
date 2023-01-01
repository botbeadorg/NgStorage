// ---------------------------------------------------------------------------

#include <fmx.h>
#pragma hdrstop

#include "FrmAGV.h"
#include "MainForm.h"
#include <FMX.Layouts.hpp>
#include <FMX.InertialMovement.hpp>
#include <System.Types.hpp>
#include "http_conn.h"
#include "warning_delete_ware_info.h"
#include "spec_packs.h"
#include "BulkTasksForm.h"
// ---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.fmx"
TFrame1 *Frame1;

__fastcall TFrame1::~TFrame1() {
	size_t i;
	size_t j;
	size_t q;
	TCircle* c = 0;
	for (i = 0; i < cols.Length; ++i) {
		cols[i]->DisposeOf();
	}
	for (i = 0; i < agv_buttons.Length; ++i) {
		agv_buttons[i]->DisposeOf();
	}
	for (std::vector<storloc*>::iterator it_sls = storlocs.begin(); storlocs.end() != it_sls;
	++it_sls) {
		delete *it_sls;
	}

	for (q = 0; q < M.Length; ++q) {
		for (i = 0; i < M[q].Length; ++i) {
			for (j = 0; j < M[q][i].Length; ++j) {
				if (M[q][i][j]) {
					c = (TCircle*)M[q][i][j];
					c->DisposeOf();
				}
			}
		}
	}

	// for (q = 0; q < M1.Length; ++q) {
	// for (i = 0; i < M1[q].Length; ++i) {
	// for (j = 0; j < M1[q][i].Length; ++j) {
	// if (M1[q][i][j]) {
	// c = (TCircle*)M1[q][i][j];
	// c->DisposeOf();
	// }
	// }
	// }
	// }
	DeleteCriticalSection(&cs_at_items);

	for (i = 0; i < slcts.Length; ++i) {
		slcts[i]->Parent->RemoveObject(slcts[i]);
		slcts[i]->DisposeOf();
	}
	for (i = 0; i < rects.Length; ++i) {
		rects[i]->Parent->RemoveObject(rects[i]);
		rects[i]->DisposeOf();
	}
	for (i = 0; i < layouts.Length; ++i) {
		layouts[i]->DisposeOf();
	}
	delete anic;
}

// ---------------------------------------------------------------------------
__fastcall TFrame1::TFrame1(TComponent* Owner) : TFrame(Owner) {
	size_t i;
	GlobalUseDX = 1;
	// GlobalUseGPUCanvas = 1;
	GlobalUseDXSoftware = 1;
	// max_w = 2048;
	// max_h = 1024;
	// max_w = 1024;
	// max_h = 768;
	max_w = 1600;
	max_h = 900;
	Parent = Form1;

	M = Form1->m.Copy();
	// layouts.set_length(M.Length);
	layouts.set_length(M.Length + 1);
	// scales.set_length(M.Length);
	scales.set_length(M.Length + 1);

	GridLayout1->Align = TAlignLayout::Left;
	GridLayout1->Width = 150;
	GridLayout1->ItemWidth = 150;
	GridLayout1->ItemHeight = 80;

	Align = TAlignLayout::Client;
	agvprms = Form1->agvprms.Copy();
	agv_colors = Form1->agv_colors.Copy();

	add_color_buttons();

	StringGrid1->Parent = GridPanelLayout1;
	StringGrid1->Align = TAlignLayout::Client;
	GridPanelLayout1->ControlCollection->operator[](0)->Control = StringGrid1;
	GridPanelLayout1->ControlCollection->operator[](0)->Row = 1;
	GridPanelLayout1->ControlCollection->operator[](0)->Column = 0;
	GridPanelLayout1->ControlCollection->operator[](0)->RowSpan = 1;
	GridPanelLayout1->ControlCollection->operator[](0)->ColumnSpan = 1;
	GridPanelLayout1->RowCollection->Items[0]->SizeStyle = TGridPanelLayout::TSizeStyle::Percent;
	GridPanelLayout1->RowCollection->Items[0]->Value = 92;
	GridPanelLayout1->RowCollection->Items[1]->SizeStyle = TGridPanelLayout::TSizeStyle::Percent;
	GridPanelLayout1->RowCollection->Items[1]->Value = 8;

	ScrollBox1->Parent = GridPanelLayout1;
	ScrollBox1->Align = TAlignLayout::Client;
	GridPanelLayout1->ControlCollection->operator[](1)->Control = ScrollBox1;
	GridPanelLayout1->ControlCollection->operator[](1)->Row = 0;
	GridPanelLayout1->ControlCollection->operator[](1)->Column = 0;
	GridPanelLayout1->ControlCollection->operator[](1)->RowSpan = 1;
	GridPanelLayout1->ControlCollection->operator[](1)->ColumnSpan = 1;

	Layout2->Parent = ScrollBox1;

	anic = new TAniCalculations(0);
	anic->Animation = 1;
	anic->AutoShowing = 1;
	anic->Averaging = 1;
	ScrollBox1->AniCalculations->Assign(anic);

	Layout2->SetBounds(0, 0, max_w, max_h);
	// scale = ScrollBox1->Width / max_w;
	scale = 0.75;
	// Layout2->Margins->Top = 20;
	// Layout2->Margins->Left = 20;
	Layout2->Scale->X = scale;
	Layout2->Scale->Y = scale;

	for (i = 0; i < layouts.Length - 1; ++i) {
		layouts[i] = (TLayout*)Layout2->Clone(0);
		layouts[i]->Tag = i;
		ScrollBox1->AddObject(layouts[i]);
		scales[i] = TPointF(scale, scale);
	}

	// Layout2->SetBounds(0, 0, 2*max_w + 100, 2*max_h + 100);
	Layout2->SetBounds(0, 0, max_w, max_h);
	// scale = ScrollBox1->Width / (2 * max_w + 100);
	// scale = ScrollBox1->Width / (max_w);
	// Layout2->Margins->Top = 20;
	// Layout2->Margins->Left = 20;
	scale = 0.85;
	Layout2->Scale->X = scale;
	Layout2->Scale->Y = scale;
	layouts[layouts.Length - 1] = (TLayout*)Layout2->Clone(0);
	layouts[layouts.Length - 1]->Tag = layouts.Length - 1;
	ScrollBox1->AddObject(layouts[layouts.Length - 1]);
	scales[layouts.Length - 1] = TPointF(scale, scale);
	draw_basis();
	build_column_plan();

	// size_t pp = DataModule1->query_available_storlocs();

	Layout2->Visible = false;

	for (i = 0; i < layouts.Length; ++i)
		if (i)
			layouts[i]->Visible = false;
	crt_layout = layouts[0];

	agvprms = Form1->agvprms.Copy();
	StringGrid1->AutoCalculateContentSize = true;
	StringGrid1->ReadOnly = 1;
	add_agv_title();

	if (Form1->real_count_agvs)
		ColorButton1Click(agv_buttons[0]);

	// Selection1->Visible = false;

	// m = Form1->m.Copy();
	// Layout2->Scale->X = 0.5;
	// Layout2->Scale->X = 1;
	// Layout2->Scale->Y = 1;
	// Layout2->Scale->Y = 0.5;

	InitializeCriticalSectionAndSpinCount(&cs_at_items, 0x400);
	SetEvent(Form1->draw_basis_end);
}

// ---------------------------------------------------------------------------
void __fastcall TFrame1::ColorButton1Click(TObject *Sender) {
	TCornerButton * b = (TCornerButton*)Sender;
	TImage * img = 0;
	TText * txt = 0;
	TFloatAnimation *a = 0;
	TFloatAnimation *d = 0;
	for (size_t i = 0; i < b->Children->Count; ++i) {
		if (__classid(TImage) == b->Children->operator[](i)->ClassType()) {
			img = (TImage *)b->Children->operator[](i);
			for (size_t j = 0; j < img->Children->Count; ++j) {
				if (__classid(TFloatAnimation) == img->Children->operator[](j)->ClassType()) {
					a = (TFloatAnimation *)(img->Children->operator[](j));
					for (size_t k = 0; k < fas.Length; ++k) {
						d = fas[k];
						if ((a != d) && d->Tag) {
							d->StopValue = ((TImage*)(d->Parent))->Position->Point.X + 10;
							// d->Enabled = true;
							d->Tag = 0;
							d->Start();
						}
					}
					if (a->Tag)
						return;
					a->StopValue = img->Position->Point.X - 10;
					// a->Enabled = true;
					a->Start();
					a->Tag = 1;
				}
			}
		}
		else if (__classid(TText) == b->Children->operator[](i)->ClassType()) {
			TText * txt1 = 0;
			txt = (TText *)b->Children->operator[](i);
			for (size_t j = 0; j < txts.Length; ++j) {
				txt1 = txts[j];
				if ((txt != txt1) && txt1->Tag) {
					txt1->TextSettings->FontColor = TAlphaColors::Aliceblue;
					txt1->Tag = 0;
				}
			}
			if (txt->Tag)
				return;
			txt->TextSettings->FontColor = TAlphaColors::Lightsalmon;
			txt->Tag = 1;
		}
	}

	agv_n *ta = (agv_n*)b->Tag;
	if (StringGrid1->Columns[0]->Header != _T("编号")) {
		int k = Form1->agvprms.Length;
		int i = k - cols.Length;
		float w = (float)(StringGrid1->Width - 20) / ((float)(agvprms.Length));
		if (i > 0) {
			cols.set_length(Form1->agvprms.Length);
			for (int j = 0; j < i; ++j) {
				TStringColumn * col = 0;
				col = new TStringColumn(0);
				StringGrid1->AddObject(col);
				cols[k + j] = col;
			}
		}
		else if (i < 0) {
			int q = cols.Length;
			for (; i < 0; ++i)
				cols[q + i]->Visible = false;
		}
		for (size_t d = 0; d < k; ++d) {
			cols[d]->Header = Form1->agvprms[d];
			cols[d]->Visible = 1;
			cols[d]->Width = w;
		}
		StringGrid1->Cells[0][0] = ta->id;
		StringGrid1->Cells[1][0] = ta->name;
		StringGrid1->Cells[2][0] = ta->addr_b;
		StringGrid1->Cells[3][0] = ta->id_warehouse;
		StringGrid1->Cells[4][0] = ta->storey;
		StringGrid1->Cells[5][0] = ta->battery;
		StringGrid1->Cells[6][0] = ta->No_cmd; // it's the running task id
	}
	else {
		StringGrid1->Cells[0][0] = ta->id;
		StringGrid1->Cells[1][0] = ta->name;
		StringGrid1->Cells[2][0] = ta->addr_b;
		StringGrid1->Cells[3][0] = ta->id_warehouse;
		StringGrid1->Cells[4][0] = ta->storey;
		StringGrid1->Cells[5][0] = ta->battery;
		StringGrid1->Cells[6][0] = ta->No_cmd; // it's the running task id
	}

}

// ---------------------------------------------------------------------------
void __fastcall TFrame1::FloatAnimation2Finish(TObject *Sender) {
	TFloatAnimation * a = (TFloatAnimation*)Sender;
	a->Enabled = false;
}
// ---------------------------------------------------------------------------

void __fastcall TFrame1::add_color_buttons() {
	TColorButton* b = 0;
	TText * t = 0;
	TFloatAnimation *fa = 0;
	TImage * g = 0;
	Form1->real_count_agvs = DataModule1->query_agvs();

	if (Form1->real_count_agvs);
	else {
		ColorButton1->Visible = false;
		return;
	}

	// agv_buttons.set_length(agv_colors.Length);
	// txts.set_length(agv_colors.Length);
	// fas.set_length(agv_colors.Length);
	agv_buttons.set_length(Form1->real_count_agvs);
	txts.set_length(Form1->real_count_agvs);
	fas.set_length(Form1->real_count_agvs);
	for (size_t i = 0; i < agv_buttons.Length; ++i) {
		b = (TColorButton*)ColorButton1->Clone(0);
		b->Color = (unsigned)agv_colors[i];
		for (size_t j = 0; j < b->Children->Count; ++j) {
			if (__classid(TText) == b->Children->operator[](j)->ClassType()) {
				t = (TText *)b->Children->operator[](j);
				// t->Text = String(vehicle_cn_txt) + IntToStr((int)i + 1);
				t->Text = Form1->info_agvs[i].name;
				t->Tag = 0;
				txts[i] = t;
			}
			if (__classid(TImage) == b->Children->operator[](j)->ClassType()) {
				g = (TImage *)b->Children->operator[](j);
				for (size_t k = 0; k < g->Children->Count; ++k) {
					if (__classid(TFloatAnimation) == g->Children->operator[](k)->ClassType()) {
						fa = (TFloatAnimation *)(g->Children->operator[](k));
						fa->StartFromCurrent = true;
						fa->Duration = 0.15;
						fa->Tag = 0;
						fa->Enabled = true;
						fa->OnFinish = FloatAnimation2Finish;
						fas[i] = fa;
					}
				}
			}
		}

		b->OnClick = ColorButton1Click;
		// b->Tag = i;
		b->Tag = (int) & (Form1->info_agvs[i]);
		GridLayout1->AddObject(b);
		agv_buttons[i] = b;
	}
	ColorButton1->Visible = false;
}

// ---------------------------------------------------------------------------
void __fastcall TFrame1::layout_zoomin(TLayout *l) {
	// Layout2->Scale->X -= 0.1;
	// Layout2->Scale->Y -= 0.1;
	l->Scale->X -= 0.05;
	l->Scale->Y -= 0.05;
}

// ---------------------------------------------------------------------------
void __fastcall TFrame1::layout_zoomout(TLayout *l) {
	l->Scale->X += 0.05;
	l->Scale->Y += 0.05;
}

// ---------------------------------------------------------------------------
void __fastcall TFrame1::add_agv_title() {
	TStringColumn * col = 0;
	size_t i = 0;
	float w = (float)(StringGrid1->Width - 10) / ((float)(agvprms.Length));
	cols.set_length(agvprms.Length);
	for (i = 0; i < agvprms.Length; ++i) {
		col = new TStringColumn(0);
		col->Header = agvprms[i];
		col->Width = w;
		StringGrid1->AddObject(col);
		// StringGrid1->Cells[i][0] = agvprms[i];
		cols[i] = col;
	}
	// String x = StringGrid1->Columns[0]->Header;
}

// ---------------------------------------------------------------------------
void __fastcall TFrame1::add_schdl_title() {
	TColumn* c = 0;
	size_t i, j, k;
	float w;
	String titles[3] = {_T("库位选区1"), _T("<----交换---->"), _T("库位选区2")};
	w = (float)(StringGrid1->Width - 10) / (float)3;
	k = crt_layout->Tag;
	if (cols.Length >= 3) {

		for (i = 0; i < 3; ++i) {
			c = StringGrid1->ColumnByIndex(i);
			c->Width = w;
			c->Header = Format(_T("S%u%s"), ARRAYOFCONST((k, titles[i])));
			c->Visible = 1;
		}

		for (i = 3; i < cols.Length; ++i) {
			c = StringGrid1->ColumnByIndex(i);
			c->Visible = 0;
		}
	}
	else {
		j = cols.Length;
		cols.Length = 3;
		for (; j < cols.Length; ++j) {
			c = new TColumn(0);
			c->Header = Format(_T("S%u%s"), ARRAYOFCONST((k, titles[i])));
			c->Width = w;
			StringGrid1->AddObject(c);
			cols[j] = c;
		}
	}
}

void __fastcall TFrame1::add_slct_title() {
	size_t i, j;
	float w;
	TColumn* c = 0;
	String title = _T("库位选区");
	j = crt_layout->Tag;
	w = StringGrid1->Width - 10;
	if (cols.Length > 0) {
		for (i = 1; i < cols.Length; ++i) {
			c = StringGrid1->ColumnByIndex(i);
			c->Visible = 0;
		}

		c = StringGrid1->ColumnByIndex(0);
		c->Width = w;
		c->Header = Format(_T("S%u%s"), ARRAYOFCONST((j, title)));
	}
	else {
		cols.Length = 1;
		c = new TColumn(0);
		c->Header = Format(_T("S%u%s"), ARRAYOFCONST((j, title)));
		c->Width = w;
		StringGrid1->AddObject(c);
		cols[0] = c;
	}
}

// ---------------------------------------------------------------------------
void __fastcall TFrame1::draw_basis() {
	size_t q, i, j, m;
	TCircle *c = 0;
	TText *t = 0;
	TText *ta = 0;
	TMySelection *s = 0;
	TRectangle *ra = 0;
	storloc *one_sl = 0;
	float k, l, w, f;
	TRectF r;
	TPointF p;
	std::vector<area>::iterator it_area;
	m = 0;
	slcts.set_length(2*(M.Length + 1));

	for (q = 0; q < M.Length; ++q) {
		k = (float)max_h / (float)M[q].Length;
		l = (float)max_w / (float)M[q][0].Length;
		w = k < l ? k : l;
		f = w - 7;
		Circle1->Width = f;
		Circle1->Height = f;

		for (i = 0; i < M[q].Length; ++i) {
			for (j = 0; j < M[q][i].Length; ++j) {
				if (M[q][i][j]) {
					c = (TCircle*)Circle1->Clone(0);
					p = TPointF(w * j, i * w);
					c->Position->Point = p;
					c->AutoCapture = 1;
					c->CanFocus = 1;

					for (it_area = Form1->cfp.areas.begin(); Form1->cfp.areas.end() != it_area;
					++it_area) {
						area aa = *it_area;
						if (aa.range[4] == q && (((aa.range[1] <= i) && (i <= aa.range[3])) &&
							((aa.range[0] <= j) && (j <= aa.range[2])))) {
							c->Fill->Color = aa.color_empty;

						}
					}
					layouts[q]->AddObject(c);
					M[q][i][j] = (unsigned)c;
				}
			}
		}

		s = new TMySelection(0);
		s->Parent = layouts[q];
		s->ParentBounds = 0;
		// s->Align =  TAlignLayout::Center;
		s->Position->Point = TPointF(30, 30);
		s->Size->Width = 500;
		s->Size->Height = 300;
		s->GripSize = 10;
		s->Color = TAlphaColors::Greenyellow;
		s->Tag = 1;
		s->Visible = 0;
		layouts[q]->AddObject(s);
		slcts[m++] = s;

		s = new TMySelection(0);
		s->Parent = layouts[q];
		s->ParentBounds = 0;
		// s->Align =  TAlignLayout::Center;
		s->Position->Point = TPointF(560, 30);
		s->Size->Width = 500;
		s->Size->Height = 300;
		s->GripSize = 10;
		s->Color = TAlphaColors::Orangered;
		s->Tag = -1;
		s->Visible = 0;
		layouts[q]->AddObject(s);
		slcts[m++] = s;
	}

	float y, x;
	y = x = 0;
	M1 = Form1->m.Copy();
	// for (q = 0; q < M1.Length; ++q) {
	// y += M1[q].Length;
	// x += M1[q][0].Length;
	// }
	y = M1[0].Length;
	x = M1[0][0].Length;
	// k = (float)(2 * max_h + 100) / (float)y;
	// l = (float)(2 * max_w + 100) / (float)x;
	// k = (float)(max_h) / (float)y;
	// l = (float)(max_w) / (float)(x*2);
	k = (float)(900) / (float)y;
	l = (float)(1600) / (float)(x * 2);
	w = k < l ? k : l;
	f = w - 4;
	Circle1->Width = f;
	Circle1->Height = f;

	// z or storey
	for (q = 0; q < M1.Length; ++q) {
		// row or y
		for (i = 0; i < M1[q].Length; ++i) {
			// column or x
			for (j = 0; j < M1[q][i].Length; ++j) {
				if (M1[q][i][j]) {
					// int v;
					// v = q ? (j + 36) : j;
					// v = j;
					c = (TCircle*)Circle1->Clone(0);
					// p = TPointF(w * j + q * (max_w + 100), w * i + q * (max_h + 100));
					// p = TPointF(w * j + q * (max_w + 100), w * i);
					// p = TPointF(w * j + q * (max_w), w * i);
					p = TPointF(w * j + q * (w * x), w * i);
					c->Position->Point = p;
					c->AutoCapture = 1;
					c->CanFocus = 1;
					one_sl = new storloc;
					SecureZeroMemory(one_sl, sizeof(storloc));

					one_sl->full = 0;
					for (it_area = Form1->cfp.areas.begin(); Form1->cfp.areas.end() != it_area;
					++it_area) {
						area aa = *it_area;
						// if (aa.range[4] == q && (((aa.range[1] <= i) && (i <= aa.range[3])) &&
						// ((aa.range[0] <= v) && (v <= aa.range[2])))) {
						// c->Fill->Color = aa.color_empty;
						// }
						if (aa.range[4] == q && (((aa.range[1] <= i) && (i <= aa.range[3])) &&
							((aa.range[0] <= j) && (j <= aa.range[2])))) {
							c->Fill->Color = aa.color_empty;
							// c->Fill->Color = TAlphaColors::White;
							one_sl->sn_flag[0] = aa.range[4] + 1;
							one_sl->sn_flag[1] = aa.No_area;
							// !0-,0+
							one_sl->sn_flag[2] = aa.base_point[0] ?
								(aa.base_point[0] - (i - aa.range[1])) : (i - aa.range[1]);
							one_sl->sn_flag[3] = aa.base_point[1] ?
								(aa.base_point[1] - (j - aa.range[0])) : (j - aa.range[0]);
							one_sl->txt =
								Sysutils::Format(_T("%sR%.*dC%.*d"),
								ARRAYOFCONST((aa.txt, 2, one_sl->sn_flag[2], 2,
								one_sl->sn_flag[3])));
							// // hide other points except A02R04C01 && A02R04C03 in area 2
							// if (aa.txt.operator != (_T("A02")));
							// else {
							// if (!(one_sl->txt.operator == (_T("A02R04C01")) ||
							// one_sl->txt.operator == (_T("A02R04C03"))))
							// c->Visible = 0;
							// }
							one_sl->color[0] = aa.color_empty;
							one_sl->color[1] = aa.color_occupied;
						}
					}
					one_sl->map_xyz[0] = i;
					one_sl->map_xyz[1] = j;
					one_sl->map_xyz[2] = q;
					one_sl->pnt_control = (size_t)c;
					c->Tag = (int)one_sl;
					bool inav = inavailable_storloc(one_sl);
					if (!inav)
						c->Visible = false;
					c->OnClick = Circle1Click;
					c->OnDblClick = Circle1DblClick;
					c->OnMouseDown = Circle1MouseDown;
					c->OnMouseUp = Circle1MouseUp;
					layouts[layouts.Length - 1]->AddObject(c);
					M1[q][i][j] = (unsigned)c;
				}
			}
		}
	}

	rects.set_length(Form1->cfp.len_sl_ary);
	for (i = 0; i < Form1->cfp.len_sl_ary; ++i) {
		specloc *one_sl = &(Form1->cfp.sl_ary[i]);
		ra = (TRectangle*)Rectangle1->Clone(0);
		// ra->Parent = layouts[layouts.Length - 1];
		if (one_sl->hittest)
			ra->OnClick = Rectangle1Click;
		one_sl->pnt_control = (size_t)ra;
		one_sl->signal = 0;
		ra->Tag = (int)one_sl;
		// ra->Width = f * (one_sl->range[2] - one_sl->range[0] + 1);
		// ra->Height = f * (one_sl->range[3] - one_sl->range[1] + 1);
		ra->Width = w * (one_sl->range[2] - one_sl->range[0] + 1) - 2;
		ra->Height = w * (one_sl->range[3] - one_sl->range[1] + 1) - 2;
		if (one_sl->range[4]) {
			p = TPointF(w * (one_sl->range[0] + 36), w * one_sl->range[1]);
		}
		else {
			p = TPointF(w * one_sl->range[0], w * one_sl->range[1]);
		}
		ra->Position->Point = p;
		ra->Fill->Color = one_sl->color[0];
		layouts[layouts.Length - 1]->AddObject(ra);
		rects[i] = ra;
	}

	Text2->Width = w;
	Text2->Height = w;
	Text2->TextSettings->FontColor = TAlphaColors::White;
	Text2->Text = 3;
	Text2->Parent = layouts[layouts.Length - 1];
	Text3->Width = w;
	Text3->Height = w;
	Text3->TextSettings->FontColor = TAlphaColors::White;
	Text3->TextSettings->Font->Size = 20;
	Text3->Text = 3;
	Text3->Parent = layouts[layouts.Length - 1];
	layouts[layouts.Length - 1]->AddObject(c);
	// txt_mycoords.set_length(num_corrds);
	txt_mycoords_2d.set_length(num_corrds);
	for (j = 0; j < num_corrds; ++j) {
		// t = (TText*)Text2->Clone(0);
		if (Form1->flags_coords[j].storey) {
			// p = TPointF(w * (Form1->flags_coords[j].x) + w * x - 2,
			// w * (Form1->flags_coords[j].y) - 3);
			// t->Position->Point = p;
			// t->Text = Form1->flags_coords[j].base;
			// layouts[layouts.Length - 1]->AddObject(t);
			// txt_mycoords[j] = t;
			if (Form1->flags_coords[j].axis) {
				// txt_mycoords_2d[j].set_length(Form1->flags_coords[j].);

				int u = Form1->flags_coords[j].base;

				// int kk = 0;
				if (u) {

					int fx = Form1->flags_coords[j].x;
					int fy = Form1->flags_coords[j].y;
					txt_mycoords_2d[j].set_length(u + 1);
					for (int e = 0; e < u + 1; ++e) {
						t = (TText*)Text2->Clone(0);
						p = TPointF(w * fx + w * x - 2, w * (fy + e) - 3);
						t->Position->Point = p;
						t->Text = Form1->flags_coords[j].base - e;
						t->TextSettings->FontColor = TAlphaColors::Springgreen;
						layouts[layouts.Length - 1]->AddObject(t);
						txt_mycoords_2d[j][e] = t;
					}
				}
				else {
					int fx = Form1->flags_coords[j].x;
					int fy = Form1->flags_coords[j].y;
					txt_mycoords_2d[j].set_length(Form1->flags_coords[j].len);
					for (int e = 0; e < Form1->flags_coords[j].len; ++e) {
						t = (TText*)Text2->Clone(0);
						p = TPointF(w * fx + w * x - 2, w * (fy + e) - 3);
						t->Position->Point = p;
						t->Text = Form1->flags_coords[j].base + e;
						t->TextSettings->FontColor = TAlphaColors::Springgreen;
						layouts[layouts.Length - 1]->AddObject(t);
						txt_mycoords_2d[j][e] = t;
					}
				}
			}
			else {

				int u = Form1->flags_coords[j].base;

				// int kk = 0;
				if (u) {
					int fx = Form1->flags_coords[j].x;
					int fy = Form1->flags_coords[j].y;
					txt_mycoords_2d[j].set_length(u + 1);

					// the flag of the area
					bool flagArea = false;

					for (int e = 0; e < u + 1; ++e) {
						t = (TText*)Text2->Clone(0);
						p = TPointF(w * (fx + e) + w * x - 2, w * (fy) - 3);
						t->Position->Point = p;
						if (!flagArea) {
							ta = (TText*)Text3->Clone(0);
							// ta->Position->Point.Y = t->Position->Point.Y;
							// ta->Position->Point.X = t->Position->Point.X-10;
							p.X -= 100;
							ta->Position->Point = p;
							ta->Width = 80;
							ta->Height = 80;
							ta->Text = _T("A03");
							ta->TagString = _T("0");
							ta->TextSettings->Font->Size = 30;
							ta->TextSettings->Font->Style =
								ta->TextSettings->Font->Style << TFontStyle::fsBold;
							ta->OnClick = Text3Click;
							layouts[layouts.Length - 1]->AddObject(ta);
							flagArea = 1;
						}
						t->Text = Form1->flags_coords[j].base - e;
						t->TextSettings->FontColor = TAlphaColors::Springgreen;

						// std::vector<area_storloc>::iterator it_a;
						// for (it_a = columns_plan.begin(); columns_plan.end()!= it_a; ++it_a) {
						// if((*it_a).area == Form1->flags_coords[j].area)
						// {
						// break;
						// }
						// }
						t->OnClick = Text2Click;
						t->TagString = _T("0");
						t->TextSettings->Font->Style =
							t->TextSettings->Font->Style << TFontStyle::fsBold;
                        t->Tag = 3;
						layouts[layouts.Length - 1]->AddObject(t);
						txt_mycoords_2d[j][e] = t;
					}
				}
				else {
					int fx = Form1->flags_coords[j].x;
					int fy = Form1->flags_coords[j].y;
					txt_mycoords_2d[j].set_length(Form1->flags_coords[j].len);
					for (int e = 0; e < Form1->flags_coords[j].len; ++e) {
						t = (TText*)Text2->Clone(0);
						p = TPointF(w * (fx + e) + w * x - 2, w * (fy) - 3);
						t->Position->Point = p;
						t->Text = Form1->flags_coords[j].base + e;
						t->TextSettings->FontColor = TAlphaColors::Springgreen;
						layouts[layouts.Length - 1]->AddObject(t);
						txt_mycoords_2d[j][e] = t;
					}
				}
			}
			// p = TPointF(f * (Form1->flags_coords[j].x), f * (Form1->flags_coords[j].y));
			// p = TPointF(w * (Form1->flags_coords[j].x) - 2, w * (Form1->flags_coords[j].y) - 3);
			// t->Position->Point = p;
			// t->Text = Form1->flags_coords[j].base;
			// t->TextSettings->FontColor = TAlphaColors::Springgreen;
			// layouts[layouts.Length - 1]->AddObject(t);
			// txt_mycoords[j] = t;
		}
		else {
			if (Form1->flags_coords[j].axis) {
				// txt_mycoords_2d[j].set_length(Form1->flags_coords[j].);

				int u = Form1->flags_coords[j].base;

				// int kk = 0;
				if (u) {

					int fx = Form1->flags_coords[j].x;
					int fy = Form1->flags_coords[j].y;
					txt_mycoords_2d[j].set_length(u + 1);
					for (int e = 0; e < u + 1; ++e) {
						t = (TText*)Text2->Clone(0);
						p = TPointF(w * fx - 2, w * (fy + e) - 3);
						t->Position->Point = p;
						t->Text = Form1->flags_coords[j].base - e;
						t->TextSettings->FontColor = TAlphaColors::Springgreen;
						layouts[layouts.Length - 1]->AddObject(t);
						txt_mycoords_2d[j][e] = t;
					}
				}
				else {
					int fx = Form1->flags_coords[j].x;
					int fy = Form1->flags_coords[j].y;
					txt_mycoords_2d[j].set_length(Form1->flags_coords[j].len);
					for (int e = 0; e < Form1->flags_coords[j].len; ++e) {
						t = (TText*)Text2->Clone(0);
						p = TPointF(w * fx - 2, w * (fy + e) - 3);
						t->Position->Point = p;
						t->Text = Form1->flags_coords[j].base + e;
						t->TextSettings->FontColor = TAlphaColors::Springgreen;
						layouts[layouts.Length - 1]->AddObject(t);
						txt_mycoords_2d[j][e] = t;
					}
				}
			}
			else {

				int u = Form1->flags_coords[j].base;

				// int kk = 0;
				if (u) {
					int fx = Form1->flags_coords[j].x;
					int fy = Form1->flags_coords[j].y;
					txt_mycoords_2d[j].set_length(u + 1);
					for (int e = 0; e < u + 1; ++e) {
						t = (TText*)Text2->Clone(0);
						p = TPointF(w * (fx + e) - 2, w * (fy) - 3);
						t->Position->Point = p;
						t->Text = Form1->flags_coords[j].base - e;
						if ((!(Form1->flags_coords[j].base - e)) && (19 == u)) {
							ta = (TText*)Text3->Clone(0);
							// ta->Position->Point.Y = t->Position->Point.Y;
							// ta->Position->Point.X = t->Position->Point.X-10;
							p.X += 30;
							p.X = ceil(p.X);
							ta->Position->Point = p;
							ta->Width = 80;
							ta->Height = 80;
							ta->Text = _T("A00");
							ta->OnClick = Text3Click;
							ta->TagString = _T("0");
							ta->TextSettings->Font->Size = 30;
							ta->TextSettings->Font->Style =
								ta->TextSettings->Font->Style << TFontStyle::fsBold;
							layouts[layouts.Length - 1]->AddObject(ta);
						}
						else if ((!(Form1->flags_coords[j].base - e)) && (9 == u)) {
							ta = (TText*)Text3->Clone(0);
							// ta->Position->Point.Y = t->Position->Point.Y;
							// ta->Position->Point.X = t->Position->Point.X-10;
							p.Y -= 50;
							p.X += 75;
							p.X = floor(p.X);
							ta->Position->Point = p;
							ta->Width = 80;
							ta->Height = 80;
							ta->Text = _T("A01");
							ta->TextSettings->Font->Size = 30;
							ta->TextSettings->Font->Style =
								ta->TextSettings->Font->Style << TFontStyle::fsBold;
							ta->TagString = _T("0");
							ta->OnClick = Text3Click;
							layouts[layouts.Length - 1]->AddObject(ta);
						}
						t->TagString = _T("0");
						t->TextSettings->FontColor = TAlphaColors::Springgreen;
						t->OnClick = Text2Click;
						t->TextSettings->Font->Style =
							t->TextSettings->Font->Style << TFontStyle::fsBold;
						if(u == 19){
                            t->Tag = 0;
						}else if(u == 9){
                            t->Tag = 1;
                        }
						layouts[layouts.Length - 1]->AddObject(t);
						txt_mycoords_2d[j][e] = t;
					}
				}
				else {
					int fx = Form1->flags_coords[j].x;
					int fy = Form1->flags_coords[j].y;
					txt_mycoords_2d[j].set_length(Form1->flags_coords[j].len);
					for (int e = 0; e < Form1->flags_coords[j].len; ++e) {
						t = (TText*)Text2->Clone(0);
						p = TPointF(w * (fx + e) - 2, w * (fy) - 3);
						t->Position->Point = p;
						t->Text = Form1->flags_coords[j].base + e;
						t->TextSettings->FontColor = TAlphaColors::Springgreen;
						layouts[layouts.Length - 1]->AddObject(t);
						txt_mycoords_2d[j][e] = t;
					}
				}
			}
			// p = TPointF(f * (Form1->flags_coords[j].x), f * (Form1->flags_coords[j].y));
			// p = TPointF(w * (Form1->flags_coords[j].x) - 2, w * (Form1->flags_coords[j].y) - 3);
			// t->Position->Point = p;
			// t->Text = Form1->flags_coords[j].base;
			// t->TextSettings->FontColor = TAlphaColors::Springgreen;
			// layouts[layouts.Length - 1]->AddObject(t);
			// txt_mycoords[j] = t;
		}
	}
	Text2->Visible = false;
	Text3->Visible = false;

	s = new TMySelection(0);
	s->Parent = layouts[layouts.Length - 1];
	s->ParentBounds = 0;
	// s->Align =  TAlignLayout::Center;
	s->Position->Point = TPointF(30, 30);
	s->Size->Width = 500;
	s->Size->Height = 300;
	s->GripSize = 10;
	s->Color = TAlphaColors::Greenyellow;
	s->Tag = 1;
	s->Visible = 0;
	layouts[layouts.Length - 1]->AddObject(s);
	slcts[m++] = s;

	s = new TMySelection(0);
	s->Parent = layouts[q];
	s->ParentBounds = 0;
	// s->Align =  TAlignLayout::Center;
	s->Position->Point = TPointF(560, 30);
	s->Size->Width = 500;
	s->Size->Height = 300;
	s->GripSize = 10;
	s->Color = TAlphaColors::Orangered;
	// s->Color = TAlphaColors::Null;
	s->Tag = -1;
	s->Visible = 0;
	layouts[layouts.Length - 1]->AddObject(s);
	slcts[m++] = s;

	fullview = layouts[layouts.Length - 1];
	Circle1->Visible = 0;
	Rectangle1->Visible = 0;
}
// ---------------------------------------------------------------------------

void __fastcall TFrame1::draw_path(double color, loc_base b, loc_base e) {
}

// ---------------------------------------------------------------------------
void __fastcall TFrame1::Circle1Click(TObject *Sender) {
	const _TCHAR* head[4] = {_T("外包装"), _T("规格"), _T("内包装"), _T("公司")};
	TCircle *c = (TCircle*)Sender;
	storloc *s = (storloc*)c->Tag;
	rep_spec_pack rsp;
	stor_loc_display(Sender);
	if (StringGrid1->Columns[0]->Header != head[0]) {
		int k = 4;
		int i = k - cols.Length;
		float w = 200;
		if (i > 0) {
			cols.set_length(4);
			for (int j = 0; j < i; ++j) {
				TStringColumn * col = 0;
				col = new TStringColumn(0);
				StringGrid1->AddObject(col);
				cols[k + j] = col;
			}
		}
		else if (i < 0) {
			int q = cols.Length;
			for (; i < 0; ++i)
				cols[q + i]->Visible = false;
		}
		for (size_t d = 0; d < k; ++d) {
			cols[d]->Header = head[d];
			cols[d]->Visible = 1;
			cols[d]->Width = w;
		}

		// StringGrid1->Cells[0][0] = ta->id;
		// StringGrid1->Cells[1][0] = ta->name;
		// StringGrid1->Cells[2][0] = ta->addr_b;
		// StringGrid1->Cells[3][0] = ta->id_warehouse;
		// StringGrid1->Cells[4][0] = ta->storey;
		// StringGrid1->Cells[5][0] = ta->battery;
		// StringGrid1->Cells[6][0] = ta->No_cmd; // it's the running task id
		rsp = DataModule1->query_info_ware(s->txt);
		StringGrid1->Cells[0][0] = String((char *)rsp.packo);
		StringGrid1->Cells[1][0] = String((char *)rsp.spec);
		StringGrid1->Cells[2][0] = String((char *)rsp.packi);
		StringGrid1->Cells[3][0] = String((char *)rsp.company);
	}
	else {
		// StringGrid1->Cells[0][0] = ta->id;
		// StringGrid1->Cells[1][0] = ta->name;
		// StringGrid1->Cells[2][0] = ta->addr_b;
		// StringGrid1->Cells[3][0] = ta->id_warehouse;
		// StringGrid1->Cells[4][0] = ta->storey;
		// StringGrid1->Cells[5][0] = ta->battery;
		// StringGrid1->Cells[6][0] = ta->No_cmd; // it's the running task id
		rsp = DataModule1->query_info_ware(s->txt);
		StringGrid1->Cells[0][0] = String((char *)rsp.packo);
		StringGrid1->Cells[1][0] = String((char *)rsp.spec);
		StringGrid1->Cells[2][0] = String((char *)rsp.packi);
		StringGrid1->Cells[3][0] = String((char *)rsp.company);
	}
}

// ---------------------------------------------------------------------------
__fastcall TMySelection::TMySelection(System::Classes::TComponent* AOwner) : TSelection(AOwner) {
}

// ---------------------------------------------------------------------------
__fastcall TMySelection::~TMySelection() {
}

// ---------------------------------------------------------------------------
void __fastcall TMySelection::DrawHandle(Fmx::Graphics::TCanvas* const Canvas,
	const Fmx::Objects::TSelection::TGrabHandle Handle, const System::Types::TRectF &Rect) {
	TBrush *Fill = 0;
	TStrokeBrush *Stroke = 0;

	Fill = new TBrush(TBrushKind::Solid, Color);
	Stroke = new TStrokeBrush(TBrushKind::Solid, TAlphaColors::Black);
	try {
		Canvas->FillEllipse(Rect, AbsoluteOpacity, Fill);
		Canvas->DrawEllipse(Rect, AbsoluteOpacity, Stroke);
	}
	catch (...) {
	}
	delete Fill;
	delete Stroke;
}
// ---------------------------------------------------------------------------

void __fastcall TFrame1::Rectangle1Click(TObject *Sender) {
	stor_loc_display(Sender);
}
// ---------------------------------------------------------------------------

void __fastcall TFrame1::stor_loc_display(TObject *Sender) {
	TRectangle *r = 0;
	TCircle *c = 0;
	specloc *spl = 0;
	storloc *stl = 0;
	int ctrl_type = __classid(TCircle) != Sender->ClassType() ? ctrl_specloc : ctrl_storloc;
	if (Form1->Edit1->TagObject) {
		if (Sender != Form1->Edit1->TagObject) {
			if (Form1->Edit2->TagObject) {
				int ctrl_type1 = __classid(TCircle) != Form1->Edit2->TagObject->ClassType() ?
					ctrl_specloc : ctrl_storloc;
				if (Sender != Form1->Edit2->TagObject) {
					if (ctrl_type1 != ctrl_storloc) {
						r = (TRectangle*)Form1->Edit2->TagObject;
						spl = (specloc*)r->Tag;
						// r->Fill->Color = spl->signal ? spl->color[1] : spl->color[0];
						TThread::Queue(0, Classes::_di_TThreadProcedure(new TThrdProcRefRectColor(r,
							spl->signal ? spl->color[1] : spl->color[0])));
					}
					else {
						c = (TCircle*)Form1->Edit2->TagObject;
						stl = (storloc*)c->Tag;
						// c->Fill->Color = stl->full ? stl->color[1] : stl->color[0];
						TThread::Queue(0,
							Classes::_di_TThreadProcedure(new TThrdProcRefCircleColor(c,
							stl->full ? stl->color[1] : stl->color[0])));
					}
					// Form1->Edit2->Tag = ctrl_type;
					// Form1->Edit2->TagObject = Sender;
					TThread::Queue(0,
						Classes::_di_TThreadProcedure(new TThrdProcRefEditTagChage(Form1->Edit2,
						ctrl_type, Sender)));
					if (ctrl_type != ctrl_storloc) {
						r = (TRectangle*)Sender;
						// r->Fill->Color = TAlphaColors::Red;
						TThread::Queue(0, Classes::_di_TThreadProcedure(new TThrdProcRefRectColor(r,
							TAlphaColors::Red)));
						spl = (specloc*)r->Tag;
						// Form1->Edit2->Text = spl->txt;
						TThread::Queue(0,
							Classes::_di_TThreadProcedure(new TThrdProcRefEditText(Form1->Edit2,
							spl->txt)));
					}
					else {
						c = (TCircle*)Sender;
						// c->Fill->Color = TAlphaColors::Red;
						TThread::Queue(0,
							Classes::_di_TThreadProcedure(new TThrdProcRefCircleColor(c,
							TAlphaColors::Red)));
						stl = (storloc*)c->Tag;
						// Form1->Edit2->Text = stl->txt;
						TThread::Queue(0,
							Classes::_di_TThreadProcedure(new TThrdProcRefEditText(Form1->Edit2,
							stl->txt)));
					}
				}
				else {
					if (ctrl_type1 != ctrl_storloc) {
						r = (TRectangle*)Form1->Edit2->TagObject;
						spl = (specloc*)r->Tag;
						// r->Fill->Color = spl->signal ? spl->color[1] : spl->color[0];
						TThread::Queue(0, Classes::_di_TThreadProcedure(new TThrdProcRefRectColor(r,
							spl->signal ? spl->color[1] : spl->color[0])));
					}
					else {
						c = (TCircle*)Form1->Edit2->TagObject;
						stl = (storloc*)c->Tag;
						// c->Fill->Color = stl->full ? stl->color[1] : stl->color[0];
						TThread::Queue(0,
							Classes::_di_TThreadProcedure(new TThrdProcRefCircleColor(c,
							stl->full ? stl->color[1] : stl->color[0])));
					}
					// Form1->Edit2->Tag = loc_controlOther;
					// Form1->Edit2->TagObject = 0;
					TThread::Queue(0,
						Classes::_di_TThreadProcedure(new TThrdProcRefEditTagChage(Form1->Edit2,
						loc_controlOther, 0)));
					// Form1->Edit2->Text = _T("");
					TThread::Queue(0,
						Classes::_di_TThreadProcedure(new TThrdProcRefEditText(Form1->Edit2,
						_T(""))));
				}
			}
			else {
				// Form1->Edit2->Tag = ctrl_type;
				// Form1->Edit2->TagObject = Sender;
				TThread::Queue(0,
					Classes::_di_TThreadProcedure(new TThrdProcRefEditTagChage(Form1->Edit2,
					ctrl_type, Sender)));
				if (ctrl_type != ctrl_storloc) {
					r = (TRectangle*)Sender;
					// r->Fill->Color = TAlphaColors::Red;
					TThread::Queue(0, Classes::_di_TThreadProcedure(new TThrdProcRefRectColor(r,
						TAlphaColors::Red)));
					spl = (specloc*)r->Tag;
					// Form1->Edit2->Text = spl->txt;
					TThread::Queue(0,
						Classes::_di_TThreadProcedure(new TThrdProcRefEditText(Form1->Edit2,
						spl->txt)));
				}
				else {
					c = (TCircle*)Sender;
					// c->Fill->Color = TAlphaColors::Red;
					TThread::Queue(0, Classes::_di_TThreadProcedure(new TThrdProcRefCircleColor(c,
						TAlphaColors::Red)));
					stl = (storloc*)c->Tag;
					// Form1->Edit2->Text = stl->txt;
					TThread::Queue(0,
						Classes::_di_TThreadProcedure(new TThrdProcRefEditText(Form1->Edit2,
						stl->txt)));
				}
			}
		}
		else {
			int ctrl_type1 = __classid(TCircle) != Form1->Edit1->TagObject->ClassType() ?
				ctrl_specloc : ctrl_storloc;
			if (ctrl_type1 != ctrl_storloc) {
				r = (TRectangle*)Form1->Edit1->TagObject;
				spl = (specloc*)r->Tag;
				// r->Fill->Color = spl->signal ? spl->color[1] : spl->color[0];
				TThread::Queue(0, Classes::_di_TThreadProcedure(new TThrdProcRefRectColor(r,
					spl->signal ? spl->color[1] : spl->color[0])));
			}
			else {
				c = (TCircle*)Form1->Edit1->TagObject;
				stl = (storloc*)c->Tag;
				// c->Fill->Color = stl->full ? stl->color[1] : stl->color[0];
				TThread::Queue(0, Classes::_di_TThreadProcedure(new TThrdProcRefCircleColor(c,
					stl->full ? stl->color[1] : stl->color[0])));
			}
			// Form1->Edit1->Tag = loc_controlOther;
			// Form1->Edit1->TagObject = 0;
			TThread::Queue(0, Classes::_di_TThreadProcedure
				(new TThrdProcRefEditTagChage(Form1->Edit1, loc_controlOther, 0)));
			// Form1->Edit1->Text = _T("");
			TThread::Queue(0, Classes::_di_TThreadProcedure(new TThrdProcRefEditText(Form1->Edit1,
				_T(""))));
		}
	}
	else {
		if (Form1->Edit2->TagObject) {
			if (Sender != Form1->Edit2->TagObject) {
				// Form1->Edit1->Tag = ctrl_type;
				// Form1->Edit1->TagObject = Sender;
				TThread::Queue(0,
					Classes::_di_TThreadProcedure(new TThrdProcRefEditTagChage(Form1->Edit1,
					ctrl_type, Sender)));
				if (ctrl_type != ctrl_storloc) {
					r = (TRectangle*)Sender;
					// r->Fill->Color = TAlphaColors::Red;
					TThread::Queue(0, Classes::_di_TThreadProcedure(new TThrdProcRefRectColor(r,
						TAlphaColors::Red)));
					spl = (specloc*)r->Tag;
					// Form1->Edit1->Text = spl->txt;
					TThread::Queue(0,
						Classes::_di_TThreadProcedure(new TThrdProcRefEditText(Form1->Edit1,
						spl->txt)));
				}
				else {
					c = (TCircle*)Sender;
					// c->Fill->Color = TAlphaColors::Red;
					TThread::Queue(0, Classes::_di_TThreadProcedure(new TThrdProcRefCircleColor(c,
						TAlphaColors::Red)));
					stl = (storloc*)c->Tag;
					// Form1->Edit1->Text = stl->txt;
					TThread::Queue(0,
						Classes::_di_TThreadProcedure(new TThrdProcRefEditText(Form1->Edit1,
						stl->txt)));
				}
			}
			else {
				// Form1->Edit1->Tag = ctrl_type;
				// Form1->Edit1->TagObject = Sender;
				TThread::Queue(0,
					Classes::_di_TThreadProcedure(new TThrdProcRefEditTagChage(Form1->Edit1,
					ctrl_type, Sender)));
				if (ctrl_type != ctrl_storloc) {
					r = (TRectangle*)Sender;
					spl = (specloc*)r->Tag;
					// Form1->Edit1->Text = spl->txt;
					TThread::Queue(0,
						Classes::_di_TThreadProcedure(new TThrdProcRefEditText(Form1->Edit1,
						spl->txt)));
				}
				else {
					c = (TCircle*)Sender;
					stl = (storloc*)c->Tag;
					// Form1->Edit1->Text = stl->txt;
					TThread::Queue(0,
						Classes::_di_TThreadProcedure(new TThrdProcRefEditText(Form1->Edit1,
						stl->txt)));
				}
				// Form1->Edit2->Tag = loc_controlOther;
				// Form1->Edit2->TagObject = 0;
				TThread::Queue(0,
					Classes::_di_TThreadProcedure(new TThrdProcRefEditTagChage(Form1->Edit2,
					loc_controlOther, 0)));
				// Form1->Edit2->Text = _T("");
				TThread::Queue(0,
					Classes::_di_TThreadProcedure(new TThrdProcRefEditText(Form1->Edit2, _T(""))));
			}
		}
		else {
			// Form1->Edit1->Tag = ctrl_type;
			// Form1->Edit1->TagObject = Sender;
			TThread::Queue(0, Classes::_di_TThreadProcedure
				(new TThrdProcRefEditTagChage(Form1->Edit1, ctrl_type, Sender)));
			if (ctrl_type != ctrl_storloc) {
				r = (TRectangle*)Sender;
				// r->Fill->Color = TAlphaColors::Red;
				TThread::Queue(0, Classes::_di_TThreadProcedure(new TThrdProcRefRectColor(r,
					TAlphaColors::Red)));
				spl = (specloc*)r->Tag;
				// Form1->Edit1->Text = spl->txt;
				TThread::Queue(0,
					Classes::_di_TThreadProcedure(new TThrdProcRefEditText(Form1->Edit1,
					spl->txt)));
			}
			else {
				c = (TCircle*)Sender;
				// c->Fill->Color = TAlphaColors::Red;
				TThread::Queue(0, Classes::_di_TThreadProcedure(new TThrdProcRefCircleColor(c,
					TAlphaColors::Red)));
				stl = (storloc*)c->Tag;
				// Form1->Edit1->Text = stl->txt;
				TThread::Queue(0,
					Classes::_di_TThreadProcedure(new TThrdProcRefEditText(Form1->Edit1,
					stl->txt)));
			}
		}
	}
}

// ---------------------------------------------------------------------------
void __fastcall TFrame1::agvtask_lineto(agvtask_view *av) {
	int ctrl_type1 = loc_controlOther, ctrl_type2 = loc_controlOther;
	size_t task_type = AGVTASKother;
	TPoint a, b;

	if (Form1->get_agvtask_type(&task_type)) {
		SecureZeroMemory(av, sizeof(*av));
		Form1->init_agvtask_view(av);
		if (CHRG != task_type) {
			ctrl_type1 = Form1->Edit1->Tag;
			ctrl_type2 = Form1->Edit2->Tag;

			if (!(Form1->Edit1->TagObject && Form1->Edit2->TagObject))
				return;

			av->pc.tsk = task_type;
			av->pc.t_id = Form1->number_cmd();

			av->s.type = ctrl_type1;
			if (ctrl_specloc == ctrl_type1) {
				av->s.r = (TRectangle*)Form1->Edit1->TagObject;
				specloc *spl = (specloc*)av->s.r->Tag;
				SecureZeroMemory(&(av->pc.sn_loc[0]), sizeof(av->pc.sn_loc[0]));
				MoveMemory(&(av->pc.sn_loc[0]), spl->sn_flag, sizeof(spl->sn_flag));
			}
			else {
				av->s.c = (TCircle*)Form1->Edit1->TagObject;
				storloc *stlc = (storloc*)av->s.c->Tag;
				MoveMemory(&(av->pc.sn_loc[0]), stlc->sn_flag, sizeof(stlc->sn_flag));
			}
			av->e.type = ctrl_type2;
			if (ctrl_specloc == ctrl_type2) {
				av->e.r = (TRectangle*)Form1->Edit2->TagObject;
				specloc *spl = (specloc*)av->e.r->Tag;
				SecureZeroMemory(&(av->pc.sn_loc[1]), sizeof(av->pc.sn_loc[1]));
				MoveMemory(&(av->pc.sn_loc[1]), spl->sn_flag, sizeof(spl->sn_flag));
			}
			else {
				av->e.c = (TCircle*)Form1->Edit2->TagObject;
				storloc *stlc = (storloc*)av->e.c->Tag;
				MoveMemory(&(av->pc.sn_loc[1]), stlc->sn_flag, sizeof(stlc->sn_flag));
			}

			a = ctrl_type1 == ctrl_specloc ? rect_center((TRectangle*)Form1->Edit1->TagObject) :
				circle_center((TCircle*)Form1->Edit1->TagObject);
			b = ctrl_type2 == ctrl_specloc ? rect_center((TRectangle*)Form1->Edit2->TagObject) :
				circle_center((TCircle*)Form1->Edit2->TagObject);

			av->l = (TLine*)Line1->Clone(0);
			av->l->Visible = 1;
			av->l->Stroke->Dash = TStrokeDash::Dash;

			if (b.x < a.x) {
				if (b.y > a.y) {
					av->l->Position->Point = a;
					av->l->Height = a.x - b.x;
					av->l->Width = b.y - a.y;
					av->l->RotationCenter->Point = TPoint(0, 0);
					av->l->RotationAngle = 90;
				}
				else
					av->l->BoundsRect = TRect(b, a);
			}
			else {
				if (a.y > b.y) {
					TPoint c(b.x, a.y + (a.y - b.y));
					av->l->BoundsRect = TRect(a, c);
					av->l->RotationCenter->Point = TPoint(0, 0);
					av->l->RotationAngle = -90;
					av->l->Width = a.y - b.y;
					av->l->Height = b.x - a.x;
				}
				else
					av->l->BoundsRect = TRect(a, b);
			}

			// av->l->Parent = layouts[layouts.Length - 1];
			// layouts[layouts.Length - 1]->AddObject(av->l);
			// fullview->AddObject(av->l);
			TThread::Queue(0, Classes::_di_TThreadProcedure(new TThrdProcRefAddLine(fullview,
				av->l)));
		}
		else {

		}
	}
}
// ---------------------------------------------------------------------------

TPoint __fastcall TFrame1::rect_center(TRectangle *r) {
	TRect rct(r->BoundsRect.left, r->BoundsRect.top, r->BoundsRect.right, r->BoundsRect.bottom);
	return CenterPoint(rct);
}

// ---------------------------------------------------------------------------
TPoint __fastcall TFrame1::circle_center(TCircle *c) {
	TRect rct(c->BoundsRect.left, c->BoundsRect.top, c->BoundsRect.right, c->BoundsRect.bottom);
	return CenterPoint(rct);
}
// ---------------------------------------------------------------------------

size_t __fastcall TFrame1::control_type(TObject *obj) {
	return __classid(TCircle) != obj->ClassType() ? ctrl_specloc : ctrl_storloc;
}
// ---------------------------------------------------------------------------

void __fastcall TFrame1::restore_task_end_point(agvtask_view *av) {
	specloc *spl = 0;
	storloc *stlc = 0;
	TRectangle *r = 0;
	TCircle *c = 0;
	if (ctrl_specloc == av->s.type) {
		spl = (specloc*)av->s.r->Tag;
		if (spl->signal) {
			// av->s.r->Fill->Color = spl->color[1];
			TThread::Queue(0, Classes::_di_TThreadProcedure(new TThrdProcRefRectColor(av->s.r,
				spl->color[1])));
		}
		else {
			// av->s.r->Fill->Color = spl->color[0];
			TThread::Queue(0, Classes::_di_TThreadProcedure(new TThrdProcRefRectColor(av->s.r,
				spl->color[0])));
		}
	}
	else if (ctrl_storloc == av->s.type) {
		stlc = (storloc*)av->s.c->Tag;
		if (stlc->full) {
			// av->s.c->Fill->Color = stlc->color[1];
			TThread::Queue(0, Classes::_di_TThreadProcedure(new TThrdProcRefCircleColor(av->s.c,
				stlc->color[1])));
		}
		else {
			// av->s.c->Fill->Color = stlc->color[0];
			TThread::Queue(0, Classes::_di_TThreadProcedure(new TThrdProcRefCircleColor(av->s.c,
				stlc->color[0])));
		}
	}
	if (ctrl_specloc == av->e.type) {
		spl = (specloc*)av->e.r->Tag;
		if (spl->signal) {
			// av->e.r->Fill->Color = spl->color[1];
			TThread::Queue(0, Classes::_di_TThreadProcedure(new TThrdProcRefRectColor(av->e.r,
				spl->color[1])));
		}
		else {
			// av->e.r->Fill->Color = spl->color[0];
			TThread::Queue(0, Classes::_di_TThreadProcedure(new TThrdProcRefRectColor(av->e.r,
				spl->color[0])));
		}
	}
	else if (ctrl_storloc == av->e.type) {
		stlc = (storloc*)av->e.c->Tag;
		if (stlc->full) {
			// av->e.c->Fill->Color = stlc->color[1];
			TThread::Queue(0, Classes::_di_TThreadProcedure(new TThrdProcRefCircleColor(av->e.c,
				stlc->color[1])));
		}
		else {
			// av->e.c->Fill->Color = stlc->color[0];
			TThread::Queue(0, Classes::_di_TThreadProcedure(new TThrdProcRefCircleColor(av->e.c,
				stlc->color[0])));
		}
	}

	// Form1->Edit1->Tag = loc_controlOther;
	// Form1->Edit1->TagObject = 0;
	TThread::Queue(0, Classes::_di_TThreadProcedure(new TThrdProcRefEditTagChage(Form1->Edit1,
		loc_controlOther, 0)));
	// Form1->Edit1->Text = _T("");
	TThread::Queue(0, Classes::_di_TThreadProcedure(new TThrdProcRefEditText(Form1->Edit1,
		_T(""))));
	// Form1->Edit2->Tag = loc_controlOther;
	// Form1->Edit2->TagObject = 0;
	TThread::Queue(0, Classes::_di_TThreadProcedure(new TThrdProcRefEditTagChage(Form1->Edit2,
		loc_controlOther, 0)));
	// Form1->Edit2->Text = _T("");
	TThread::Queue(0, Classes::_di_TThreadProcedure(new TThrdProcRefEditText(Form1->Edit2,
		_T(""))));
}
// ---------------------------------------------------------------------------

void __fastcall TFrame1::wipe_line(agvtask_view *av) {
	thrd_ua = new ThreadUpdateAgview(av);
	thrd_ua->WaitFor();
	thrd_ua->DisposeOf();
}

// ---------------------------------------------------------------------------
__fastcall ThreadUpdateAgview::ThreadUpdateAgview(agvtask_view *agview)
	: TThread(false), agview(agview) {
}

// ---------------------------------------------------------------------------
__fastcall ThreadUpdateAgview::ThreadUpdateAgview() {

}

// ---------------------------------------------------------------------------
__fastcall ThreadUpdateAgview::~ThreadUpdateAgview() {
}

// ---------------------------------------------------------------------------
void __fastcall ThreadUpdateAgview::Execute() {
	Synchronize(update_agvtask_view);
}

// ---------------------------------------------------------------------------
void __fastcall ThreadUpdateAgview::update_agvtask_view() {
	// lyt->Canvas->BeginScene();
	agview->l->Visible = 0;

	// agview->l->Parent->RemoveObject(agview->l);
	// Calling RemoveObject is equivalent to setting Parent to nil.
	// agview->l->Parent = 0;
	// lyt->Repaint();
	// lyt->Canvas->EndScene();
	Frame1->fullview->RemoveObject(agview->l);
	// agview->l->Parent = 0;
	agview->l->DisposeOf();
	agview->l = 0;
	Frame1->fullview->Repaint();
}

// ---------------------------------------------------------------------------
void __fastcall TFrame1::Circle1DblClick(TObject *Sender) {

	if (mrYes != Form4->ShowModal())
		return;
	TCircle * c = (TCircle*)Sender;
	storloc *sl = (storloc*)c->Tag;
	TStringList *prms_post = new TStringList();
#ifdef _DEBUG
	String myurl = Format(_T("http://%s:19998"), ARRAYOFCONST(((urls[0]))));
#else
	String myurl = Format(_T("http://%s:19998"), ARRAYOFCONST(((urls[1]))));
#endif
	prms_post->AddPair(_T("cmd"), _T("678009"));
	prms_post->AddPair(_T("name_storloc"), sl->txt);
	if (TAlphaColors::White == sl->color[0])
		return;

	try {
		DataModule1->request_erase_storloc->Post(myurl, prms_post);
		sl->color[0] = sl->color[1] = TAlphaColors::White;
		// c->Fill->Color = sl->color[0];
		TThread::Queue(0, Classes::_di_TThreadProcedure(new TThrdProcRefCircleColor(c,
			sl->color[0])));
	}
	catch (...) {
		// Form1->Label1->Text = _T("连接错误");
		TThread::Queue(0, Classes::_di_TThreadProcedure(new TThrdProcRefLabelText(Form1->Label1,
			_T("连接错误"))));
	}
}

// ---------------------------------------------------------------------------
void __fastcall TFrame1::build_column_plan() {

	size_t q, i, j;
	TCircle *c = 0;
	storloc *sl = 0;
	// z or storey
	for (q = 0; q < M1.Length; ++q) {
		// row or y
		for (i = 0; i < M1[q].Length; ++i) {
			// column or x
			for (j = 0; j < M1[q][i].Length; ++j) {
				if (M1[q][i][j]) {
					c = (TCircle*)(M1[q][i][j]);
					sl = (storloc*)(c->Tag);
					if (!inavailable_storloc(sl))
						continue;
					if (2 != sl->sn_flag[1]) {
						std::vector<area_storloc>::iterator it_area;
						for (it_area = columns_plan.begin(); columns_plan.end() != it_area;
						++it_area) {
							if ((*it_area).area == sl->sn_flag[1])
								break;
						}
						if (it_area == columns_plan.end()) {
							area_storloc as;
							as.area = sl->sn_flag[1];
							storlco_wareinfo sw;
							SecureZeroMemory(&sw, sizeof(sw));
							sw.in_task = 0;
							sw.r = sl->sn_flag[2];
							sw.sn.map_xyz[0] = sl->map_xyz[0];
							sw.sn.map_xyz[1] = sl->map_xyz[1];
							sw.sn.map_xyz[2] = sl->map_xyz[2];
							sw.sn.sn_flag[0] = sl->sn_flag[0];
							sw.sn.sn_flag[1] = sl->sn_flag[1];
							sw.sn.sn_flag[2] = sl->sn_flag[2];
							sw.sn.sn_flag[3] = sl->sn_flag[3];
							AnsiString txt = AnsiString(sl->txt);
							MoveMemory(sw.sn.txt, txt.c_str(), txt.Length());

							column_storloc cs;
							cs.in = cs.out = 0;
							cs.storey = sl->sn_flag[0];
							cs.area = sl->sn_flag[1];
							cs.c = sl->sn_flag[3];
							cs.rows.push_back(sw);
							as.columns.push_back(cs);
							columns_plan.push_back(as);
						}
						else {
							std::vector<column_storloc>::iterator it_c;
							for (it_c = (*it_area).columns.begin();
							(*it_area).columns.end() != it_c; ++it_c) {
								if ((*it_c).c == sl->sn_flag[3])
									break;
							}
							if (it_c == (*it_area).columns.end()) {
								storlco_wareinfo sw;
								SecureZeroMemory(&sw, sizeof(sw));
								sw.in_task = 0;
								sw.r = sl->sn_flag[2];
								sw.sn.map_xyz[0] = sl->map_xyz[0];
								sw.sn.map_xyz[1] = sl->map_xyz[1];
								sw.sn.map_xyz[2] = sl->map_xyz[2];
								sw.sn.sn_flag[0] = sl->sn_flag[0];
								sw.sn.sn_flag[1] = sl->sn_flag[1];
								sw.sn.sn_flag[2] = sl->sn_flag[2];
								sw.sn.sn_flag[3] = sl->sn_flag[3];
								SecureZeroMemory(&sw.rsp, sizeof(sw.rsp));
								AnsiString txt = AnsiString(sl->txt);
								MoveMemory(sw.sn.txt, txt.c_str(), txt.Length());

								column_storloc cs;
								cs.in = cs.out = 0;
								cs.storey = sl->sn_flag[0];
								cs.area = sl->sn_flag[1];
								cs.c = sl->sn_flag[3];
								cs.rows.push_back(sw);
								(*it_area).columns.push_back(cs);
							}
							else {
								storlco_wareinfo sw;
								SecureZeroMemory(&sw, sizeof(sw));
								sw.in_task = 0;
								sw.r = sl->sn_flag[2];
								sw.sn.map_xyz[0] = sl->map_xyz[0];
								sw.sn.map_xyz[1] = sl->map_xyz[1];
								sw.sn.map_xyz[2] = sl->map_xyz[2];
								sw.sn.sn_flag[0] = sl->sn_flag[0];
								sw.sn.sn_flag[1] = sl->sn_flag[1];
								sw.sn.sn_flag[2] = sl->sn_flag[2];
								sw.sn.sn_flag[3] = sl->sn_flag[3];
								SecureZeroMemory(&sw.rsp, sizeof(sw.rsp));
								AnsiString txt = AnsiString(sl->txt);
								MoveMemory(sw.sn.txt, txt.c_str(), txt.Length());

								(*it_c).rows.push_back(sw);
							}
						}
					}
				}
			}
		}
	}

	for (std::vector<area_storloc>::iterator it_a = columns_plan.begin();
	it_a != columns_plan.end(); ++it_a) {
		for (std::vector<column_storloc>::iterator it_c = (*it_a).columns.begin();
		(*it_a).columns.end() != it_c; it_c++) {
			std::sort((*it_c).rows.begin(), (*it_c).rows.end(), rowASC);
		}
		// std::sort((*it_a).columns.begin(), (*it_a).columns.end(), columnDESC);
		std::sort((*it_a).columns.begin(), (*it_a).columns.end(), columnASC);
	}
	std::sort(columns_plan.begin(), columns_plan.end(), areaASC);

#ifdef _DEBUG
	std::vector<area_storloc>::iterator it_a;
	for (it_a = columns_plan.begin(); it_a != columns_plan.end(); ++it_a) {
		AnsiString No_a((*it_a).area);
		OutputDebugStringA(No_a.c_str());
		std::vector<column_storloc>::iterator it_c;
		for (it_c = (*it_a).columns.begin(); (*it_a).columns.end() != it_c; it_c++) {
			AnsiString No_c((*it_c).c);
			OutputDebugStringA(No_c.c_str());
			std::vector<storlco_wareinfo>::iterator it_r;
			for (it_r = (*it_c).rows.begin(); it_r != (*it_c).rows.end(); ++it_r) {
				OutputDebugStringA((*it_r).sn.txt);
			}
		}
	}
#endif
}

// ---------------------------------------------------------------------------
void __fastcall TFrame1::Circle1MouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift,
	float X, float Y) {
	TCircle *c = (TCircle*)Sender;
	storloc *s = (storloc*)(c->Tag);
	SecureZeroMemory(&half_sl, sizeof(half_sl));
	half_sl.c = c;
	half_sl.txt.operator += (s->txt);
	MoveMemory(half_sl.sn_flag, s->sn_flag, sizeof(s->sn_flag));
	DataModule1->timer_press->Enabled = true;
	DataModule1->count_circle_pressed = 0;
}
// ---------------------------------------------------------------------------

void __fastcall TFrame1::Circle1MouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift,
	float X, float Y) {
	DataModule1->timer_press->Enabled = false;
	if (DataModule1->count_circle_pressed > 4) {
		Form5->Button1->Text = _T("修改");
		Form5->Label1->Text = _T("");
		if (mrYes == Form5->ShowModal()) {

			TStringList *prms_post = new TStringList();
#ifdef _DEBUG
			String myurl = Format(_T("http://%s:19998"), ARRAYOFCONST(((urls[0]))));
#else
			String myurl = Format(_T("http://%s:19998"), ARRAYOFCONST(((urls[1]))));
#endif
			prms_post->AddPair(_T("cmd"), _T("657312"));
			prms_post->AddPair(_T("storey"), String(half_sl.sn_flag[0]));
			prms_post->AddPair(_T("area"), String(half_sl.sn_flag[1]));
			prms_post->AddPair(_T("row"), String(half_sl.sn_flag[2]));
			prms_post->AddPair(_T("column"), String(half_sl.sn_flag[3]));
			prms_post->AddPair(_T("name_storloc"), half_sl.txt);

			prms_post->AddPair(_T("packo"), String(Form5->rspwc->rep_sp.packo));
			prms_post->AddPair(_T("spec"), String(Form5->rspwc->rep_sp.spec));
			prms_post->AddPair(_T("packi"), String(Form5->rspwc->rep_sp.packi));
			prms_post->AddPair(_T("company"), String(Form5->rspwc->rep_sp.company));
			prms_post->AddPair(_T("color"), String(Form5->rspwc->color));
			try {
				DataModule1->request_change_storloc->Post(myurl, prms_post);
				TCircle *c = half_sl.c;
				storloc *s = (storloc*)(c->Tag);
				s->color[0] = s->color[1] = Form5->rspwc->color;
				// c->Fill->Color = s->color[0];
				TThread::Queue(0, Classes::_di_TThreadProcedure(new TThrdProcRefCircleColor(c,
					s->color[0])));
			}
			catch (...) {
				// Form1->Label1->Text = _T("连接错误");
				TThread::Queue(0,
					Classes::_di_TThreadProcedure(new TThrdProcRefLabelText(Form1->Label1,
					_T("连接错误"))));
			}
			delete prms_post;
		}
	}
	DataModule1->count_circle_pressed = 0;
}

// ---------------------------------------------------------------------------
void __fastcall TFrame1::ScrollBox1DblClick(TObject *Sender) {
	// Form7->ShowModal();
}
// ---------------------------------------------------------------------------

void __fastcall TFrame1::ScrollBox1MouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift,
	float X, float Y) {
	if (TMouseButton::mbRight == Button) {
		// POINT p;
		// p.x = X;
		// p.y = Y;
		// ClientToScreen((HWND)(Form1->Handle), &p);
		// //float y2 = ClientToScreen((HWND )(Form1->Handle), Y);
		// //PopupMenu1->Popup(p.x, p.y);
		// // TPointF pf;
		// // TPointF pf2;
		// // pf.X = X;
		// // pf.Y = Y;
		// // // pf2 = LocalToScreen(pf);
		// // // pf2 = ScreenToLocal(pf);
		// // // pf2 = AbsoluteToLocal(pf);
		// // PopupMenu1->Popup(X, Y);
		//
		// TPointF pf_sb1;
		// pf_sb1 = ScrollBox1->ClipRect.TopLeft();
		// pf_sb1.X += X;
		// pf_sb1.Y += Y;
		// pf_sb1 = LocalToScreen(pf_sb1);
		// // PopupMenu1->Popup(pf_sb1.X, pf_sb1.Y);
		// //PopupMenu1->Popup(X, Y);
		TPointF pf;
		pf.X = X;
		pf.Y = Y;
		pf = LocalToScreen(pf);
		// pf = LocalToAbsolute(pf);
		PopupMenu1->Popup(pf.X, pf.Y);
	}
}

// ---------------------------------------------------------------------------
void __fastcall TFrame1::MenuItem1Click(TObject *Sender) {
	Form7->ShowModal();
}

// ---------------------------------------------------------------------------
void __fastcall TFrame1::Text3Click(TObject *Sender) {
	//
	// TText *t = (TText*)Sender;
	// if (t->TagString != _T("0")) {
	// t->TagString = _T("0");
	// t->TextSettings->FontColor = TAlphaColors::Springgreen;
	// }
	// else {
	// t->TagString = _T("1");
	// t->TextSettings->FontColor = TAlphaColors::Tomato;
	// }
}

// ---------------------------------------------------------------------------
void __fastcall TFrame1::Text2Click(TObject *Sender) {
	TText *t = (TText*)Sender;
	if (t->TagString != _T("0")) {
		t->TagString = _T("0");
		t->TextSettings->FontColor = TAlphaColors::Springgreen;
	}
	else {
		t->TagString = _T("1");
		t->TextSettings->FontColor = TAlphaColors::Tomato;
	}
    OutputDebugStringA(AnsiString(t->Tag).c_str());
}
// ---------------------------------------------------------------------------
