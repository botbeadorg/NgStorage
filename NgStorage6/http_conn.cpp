// ---------------------------------------------------------------------------

#pragma hdrstop

#include <process.h>
#include "common.h"
#include "http_conn.h"
#include "MainForm.h"
#include "task_list.h"
#include "FrmAGV.h"
#include "spec_packs.h"
// ---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma classgroup "FMX.Controls.TControl"
#pragma resource "*.dfm"
TDataModule1 *DataModule1;

// ---------------------------------------------------------------------------
__fastcall TDataModule1::TDataModule1(TComponent* Owner) : TDataModule(Owner) {
	id_client = UINT_MAX;
	timer_press->Enabled = false;
	count_circle_pressed = 0;
}
// ---------------------------------------------------------------------------

unsigned __stdcall client_id(void *p) {
	String myid;
	TStringList *prms_post = new TStringList();
#ifdef _DEBUG
	String myurl = Format(_T("http://%s:19998"), ARRAYOFCONST(((urls[0]))));
#else
	String myurl = Format(_T("http://%s:19998"), ARRAYOFCONST(((urls[1]))));
#endif
	prms_post->AddPair(_T("cmd"), _T("657308"));

	try {
		myid = DataModule1->request_common->Post(myurl, prms_post)->ContentAsString();
		TryStrToUInt(myid, DataModule1->id_client);
		Form1->Caption += Format(_T(" [客户端ID - %s]"), ARRAYOFCONST(((myid))));
	}
	catch (...) {
		Form1->Label1->Text = _T("连接错误");
	}
	delete prms_post;

	return 0;
}

// ---------------------------------------------------------------------------

void __fastcall TDataModule1::client_id() {
	unsigned id_thrd;
	CloseHandle((void *)_beginthreadex(0, 0, ::client_id, 0, 0, &id_thrd));
}

// ---------------------------------------------------------------------------

void __fastcall TDataModule1::timer_query_rtiTimer(TObject *Sender) {
	int i_r = rti_pending;
	size_t t_id;
	agvtask_view*p = 0;
	String r("");
	TStringList *prms_post = new TStringList();
#ifdef _DEBUG
	String myurl = Format(_T("http://%s:19998"), ARRAYOFCONST(((urls[0]))));
#else
	String myurl = Format(_T("http://%s:19998"), ARRAYOFCONST(((urls[1]))));
#endif
	prms_post->AddPair(_T("cmd"), _T("678000"));
	t_id = Form1->av_pending.pc.t_id;
	prms_post->AddPair(_T("id_ti"), String(t_id));
	try {
		r = DataModule1->request_query_rti->Post(myurl, prms_post)->ContentAsString();
		TryStrToInt(r, i_r);
		switch (i_r) {
		case rti_failure:
			SecureZeroMemory(&(Form1->av_pending), sizeof(Form1->av_pending));
			timer_query_rti->Enabled = false;
			SetEvent(event_nextagvtask);
			EnterCriticalSection(&(Frame1->cs_at_items));
			for (std::vector<agvtask_listitem>::reverse_iterator it = Frame1->at_items.rbegin();
			Frame1->at_items.rend() != it; ++it) {
				agvtask_listitem al = *it;
				if (al.v.pc.t_id == t_id) {
					al.txts_listitems[h_rti] = _T("失败");
					Form2->StringGrid1->Cells[h_rti][al.row_number] = _T("失败");
					break;
				}
			}
			LeaveCriticalSection(&(Frame1->cs_at_items));
			// ++Form2->myrowcount;

			break;
		case rti_success:
			SecureZeroMemory(&(Form1->av_pending), sizeof(Form1->av_pending));
			timer_query_rti->Enabled = false;
			SetEvent(event_nextagvtask);
			EnterCriticalSection(&(Frame1->cs_at_items));
			for (std::vector<agvtask_listitem>::reverse_iterator it = Frame1->at_items.rbegin();
			Frame1->at_items.rend() != it; ++it) {
				agvtask_listitem al = *it;
				if (al.v.pc.t_id == t_id) {
					al.txts_listitems[h_rti] = _T("成功");
					Form2->StringGrid1->Cells[h_rti][al.row_number] = _T("成功");
					break;
				}
			}
			LeaveCriticalSection(&(Frame1->cs_at_items));
			break;
		default: ;
		}
	}
	catch (...) {
		// Form1->Label1->Text = _T("连接错误");
		TThread::Queue(0, Classes::_di_TThreadProcedure(new TThrdProcRefLabelText(Form1->Label1,
			_T("连接错误"))));
	}
	delete prms_post;
}

// ---------------------------------------------------------------------------
void __fastcall TDataModule1::DataModuleCreate(TObject *Sender) {
	timer_query_rti->Enabled = false;
}

// ---------------------------------------------------------------------------
// query agvs registered
size_t __fastcall TDataModule1::query_agvs() {
	size_t m = 0;
	TStringList *prms_post = new TStringList();

#ifdef _DEBUG
	String myurl = Format(_T("http://%s:19998"), ARRAYOFCONST(((urls[0]))));
#else
	String myurl = Format(_T("http://%s:19998"), ARRAYOFCONST(((urls[1]))));
#endif
	prms_post->AddPair(_T("cmd"), _T("678003"));

	try {
		request_common->Post(myurl, prms_post)->ContentStream->Read(Form1->info_agvs,
			sizeof(agv_n) * Form1->count_agvs);
	}
	catch (...) {
		// Form1->Label1->Text = _T("连接错误");
		TThread::Queue(0, Classes::_di_TThreadProcedure(new TThrdProcRefLabelText(Form1->Label1,
			_T("连接错误"))));
	}

	for (size_t i = 0; i < Form1->count_agvs; ++i)
		if (Form1->info_agvs[i].addr_b)
			++m;

	delete prms_post;
	return m;
}
// ---------------------------------------------------------------------------

void __fastcall TDataModule1::agv_online(u_long addr_b) {

	TStringList *prms_post = new TStringList();
#ifdef _DEBUG
	String myurl = Format(_T("http://%s:19998"), ARRAYOFCONST(((urls[0]))));
#else
	String myurl = Format(_T("http://%s:19998"), ARRAYOFCONST(((urls[1]))));
#endif

	prms_post->AddPair(_T("cmd"), _T("678004"));
	prms_post->AddPair(_T("ip_agv"), UIntToStr((unsigned __int64)addr_b));
	try {
		request_common->Post(myurl, prms_post);
	}
	catch (...) {
		// Form1->Label1->Text = _T("连接错误");
		TThread::Queue(0, Classes::_di_TThreadProcedure(new TThrdProcRefLabelText(Form1->Label1,
			_T("连接错误"))));
	}
	delete prms_post;
}

// ---------------------------------------------------------------------------
void __fastcall TDataModule1::request_agv_onlineRequestCompleted(TObject * const Sender,
	IHTTPResponse * const AResponse) {
	state_agv_online sao;
	SecureZeroMemory(&sao, sizeof sao);
	AResponse->ContentStream->Read(&sao, sizeof sao);
	// if (sao.online) {
	// Frame1->StringGrid1->Cells[5][0] = _T("在线");
	// Frame1->StringGrid1->Cells[6][0] = sao.battery;
	// Frame1->StringGrid1->Cells[7][0] = sao.id_task;
	// }
	// else {
	// Frame1->StringGrid1->Cells[5][0] = _T("离线");
	// }
}

// ---------------------------------------------------------------------------
void __fastcall TDataModule1::timer_fresh_agvTimer(TObject *Sender) {
	size_t n = 0;
	size_t m = Form1->count_agvs;
	TStringList *prms_post = new TStringList();
	agv_n *temp_agvs = new agv_n[m];
#ifdef _DEBUG
	String myurl = Format(_T("http://%s:19998"), ARRAYOFCONST(((urls[0]))));
#else
	String myurl = Format(_T("http://%s:19998"), ARRAYOFCONST(((urls[1]))));
#endif
	prms_post->AddPair(_T("cmd"), _T("678005"));
	try {
		request_common->Post(myurl, prms_post)->ContentStream->Read(temp_agvs, sizeof(agv_n) * m);
	}
	catch (...) {
		// Form1->Label1->Text = _T("连接错误");
		TThread::Queue(0, Classes::_di_TThreadProcedure(new TThrdProcRefLabelText(Form1->Label1,
			_T("连接错误"))));
	}

	for (size_t i = 0; i < Form1->count_agvs; ++i)
		if (temp_agvs[i].addr_b)
			++n;

	for (size_t i = 0; i < n; ++i) {
		size_t a_ip = temp_agvs[i].addr_b;
		size_t j;
		for (j = 0; j < Form1->real_count_agvs; ++j)
			if (Form1->info_agvs[j].addr_b == a_ip)
				break;
		if (j != Form1->real_count_agvs) {
			Form1->info_agvs[j] = temp_agvs[i];
		}
		else {
			TColorButton* b = 0;
			TText * t = 0;
			TFloatAnimation *fa = 0;
			TImage * g = 0;
			Form1->info_agvs[Form1->real_count_agvs] = temp_agvs[i];
			Form1->info_agvs[Form1->real_count_agvs].id_warehouse = Form1->id_warehouse;
			++Form1->real_count_agvs;

			Frame1->agv_buttons.set_length(Form1->real_count_agvs);
			Frame1->txts.set_length(Form1->real_count_agvs);
			Frame1->fas.set_length(Form1->real_count_agvs);
			b = (TColorButton*)Frame1->ColorButton1->Clone(0);
			m = Form1->real_count_agvs - 1;
			b->Color = (unsigned)Form1->agv_colors[m];
			for (size_t j = 0; j < b->Children->Count; ++j) {
				if (__classid(TText) == b->Children->operator[](j)->ClassType()) {
					t = (TText *)b->Children->operator[](j);
					t->Text = Form1->info_agvs[m].name;
					t->Tag = 0;
					Frame1->txts[m] = t;
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
							fa->OnFinish = Frame1->FloatAnimation2Finish;
							Frame1->fas[m] = fa;
						}
					}
				}
			}

			b->OnClick = Frame1->ColorButton1Click;
			b->Tag = (int) & (Form1->info_agvs[m]);
			Frame1->GridLayout1->AddObject(b);
			Frame1->agv_buttons[m] = b;
			b->Visible = True;
		}
	}

	if (Frame1->StringGrid1->Columns[0]->Header == _T("编号")) {
		String num = Frame1->StringGrid1->Cells[0][0];
		if (num.operator != (_T(""))) {
			int id_agv = StrToInt(num);
			agv_n an;
			SecureZeroMemory(&an, sizeof(an));
			for (size_t i = 0; i < Form1->real_count_agvs; ++i) {
				if (Form1->info_agvs[i].id == id_agv) {
					Frame1->StringGrid1->Cells[5][0] = Form1->info_agvs[i].battery;
					Frame1->StringGrid1->Cells[6][0] = Form1->info_agvs[i].No_cmd;
					break;
				}
			}
		}
	}

	delete[]temp_agvs;
	delete prms_post;
}
// ---------------------------------------------------------------------------

size_t __fastcall TDataModule1::query_available_storlocs() {
	size_t r = 0;
	size_t n = 0;
	size_t color = TAlphaColors::Sandybrown;
	rep_storloc *rst;
	String c;
	TStringList *prms_post = new TStringList();
#ifdef _DEBUG
	String myurl = Format(_T("http://%s:19998"), ARRAYOFCONST(((urls[0]))));
#else
	String myurl = Format(_T("http://%s:19998"), ARRAYOFCONST(((urls[1]))));
#endif
	prms_post->AddPair(_T("cmd"), _T("678006"));
	try {
		c = request_common->Post(myurl, prms_post)->ContentAsString();
		r = c.ToInt();
		prms_post->Clear();
		prms_post->AddPair(_T("cmd"), _T("678007"));
		prms_post->AddPair(_T("number_points"), c);
		n = r + Form1->count_agvs; // 可能有比查询时更多的点数
		rst = new rep_storloc[n];
		SecureZeroMemory(rst, sizeof(rep_storloc) * n);
		request_common->Post(myurl, prms_post)->ContentStream->Read(rst, sizeof(rep_storloc) * n);

		for (size_t i = 0; i < n; ++i) {
			if (rst[i].st) {
				size_t x, y, z;
				for (z = 0; z < Frame1->M1.Length; ++z) {
					for (y = 0; y < Frame1->M1[z].Length; ++y) {
						for (x = 0; x < Frame1->M1[z][y].Length; ++x) {
							if (Frame1->M1[z][y][x]) {
								TCircle *crl = (TCircle*)Frame1->M1[z][y][x];
								storloc *sl = (storloc*)crl->Tag;
								if (sl->txt.operator == (String((const char *)rst[i].txt))) {
									sl->color[0] = sl->color[1] = color;
									// crl->Fill->Color = sl->color[0];
									TThread::Queue(0,
										Classes::_di_TThreadProcedure
										(new TThrdProcRefCircleColor(crl, sl->color[0])));
									break;
								}
							}
						}
					}
				}
			}
		}

		delete[]rst;
	}
	catch (...) {
		// Form1->Label1->Text = _T("连接错误");
		TThread::Queue(0, Classes::_di_TThreadProcedure(new TThrdProcRefLabelText(Form1->Label1,
			_T("连接错误"))));
	}
	delete prms_post;
	return r;
}

// ---------------------------------------------------------------------------
void __fastcall TDataModule1::DataModuleDestroy(TObject *Sender) {
	//
}

// ---------------------------------------------------------------------------
void __fastcall TDataModule1::timer_query_elecTimer(TObject *Sender) {
	TStringList *prms_post = new TStringList();
#ifdef _DEBUG
	String myurl = Format(_T("http://%s:19998"), ARRAYOFCONST(((urls[0]))));
#else
	String myurl = Format(_T("http://%s:19998"), ARRAYOFCONST(((urls[1]))));
#endif
	prms_post->AddPair(_T("cmd"), _T("678008"));
}

// ---------------------------------------------------------------------------
void __fastcall TDataModule1::timer_request_running_statusTimer(TObject *Sender) {
	size_t count_tasks_running = 0;
	String ids(_T(""));
	running_status_taskissue *rst = 0;
	TStringList *prms_post = new TStringList();
#ifdef _DEBUG
	String myurl = Format(_T("http://%s:19998"), ARRAYOFCONST(((urls[0]))));
#else
	String myurl = Format(_T("http://%s:19998"), ARRAYOFCONST(((urls[1]))));
#endif
	prms_post->AddPair(_T("cmd"), _T("678010"));
	EnterCriticalSection(&(Frame1->cs_at_items));
	for (std::vector<agvtask_listitem>::reverse_iterator it = Frame1->at_items.rbegin();
	Frame1->at_items.rend() != it; ++it) {
		agvtask_listitem al = *it;
		if (2 != al.state_running) {
			ids += al.v.pc.t_id;
			ids += _T(',');
			++count_tasks_running;
		}
		else
			break;
	}
	LeaveCriticalSection(&(Frame1->cs_at_items));

	if (ids.operator == (_T("")))
		return;

	prms_post->AddPair(_T("ids"), ids);
	prms_post->AddPair(_T("count"), String(count_tasks_running));
	rst = new running_status_taskissue[count_tasks_running];
	try {
		request_running_status->Post(myurl, prms_post)->ContentStream->Read(rst,
			sizeof(running_status_taskissue) * count_tasks_running);
		for (size_t i = 0; i < count_tasks_running; ++i) {
			EnterCriticalSection(&(Frame1->cs_at_items));

			for (std::vector<agvtask_listitem>::reverse_iterator it = Frame1->at_items.rbegin();
			Frame1->at_items.rend() != it; ++it) {
				agvtask_listitem al = *it;

				if ((*it).v.pc.t_id == rst[i].t_id) {
					if (0 == rst[i].state_running) {
						(*it).state_running = 0;
						(*it).txts_listitems[h_state_running] = _T("未决");
						Form2->StringGrid1->Cells[h_state_running][(*it).row_number] = _T("未决");
					}
					else if (1 == rst[i].state_running) {
						(*it).state_running = 1;
						(*it).txts_listitems[h_state_running] = _T("运行中");
						Form2->StringGrid1->Cells[h_state_running][(*it).row_number] = _T("运行中");
					}
					else if (2 == rst[i].state_running) {
                        int tsk_once =  ((*it)).v.pc.tsk;
						((*it)).state_running = 2;
						((*it)).txts_listitems[h_state_running] = _T("完成");
						Form2->StringGrid1->Cells[h_state_running][(*it).row_number] = _T("完成");
						if (I1 == ((*it)).v.pc.tsk || I2 == ((*it)).v.pc.tsk) {
							storloc * sl = (storloc*)(((*it)).v.e.c->Tag);
							sl->color[0] = sl->color[1] = rst[i].color;
							// (*al).v.e.c->Fill->Color = sl->color[0];
							TThread::Queue(0,
								Classes::_di_TThreadProcedure
								(new TThrdProcRefCircleColor((*it).v.e.c, sl->color[0])));
						}
						// else if (U1 == (*it).v.pc.tsk || U2 == (*it).v.pc.tsk) {
						// storloc * sl = (storloc*)((*it).v.s.c->Tag);
						// sl->color[0] = sl->color[1] = TAlphaColors::White;
						// // (*it).v.s.c->Fill->Color = sl->color[0];
						// TThread::Queue(0,
						// Classes::_di_TThreadProcedure
						// (new TThrdProcRefCircleColor((*it).v.s.c, sl->color[0])));
						// }

						else if (X1 == ((*it)).v.pc.tsk || X2 == ((*it)).v.pc.tsk ||
							X12 == ((*it)).v.pc.tsk || X21 == ((*it)).v.pc.tsk) {
							storloc * sl = (storloc*)(((*it)).v.s.c->Tag);
							sl->color[0] = sl->color[1] = TAlphaColors::White;
							// (*al).v.s.c->Fill->Color = sl->color[0];
							TThread::Queue(0,
								Classes::_di_TThreadProcedure
								(new TThrdProcRefCircleColor((*it).v.s.c, sl->color[0])));
							sl = (storloc*)(((*it)).v.e.c->Tag);
							sl->color[0] = sl->color[1] = rst[i].color;
							// (*al).v.e.c->Fill->Color = sl->color[0];
							TThread::Queue(0,
								Classes::_di_TThreadProcedure
								(new TThrdProcRefCircleColor((*it).v.e.c, sl->color[0])));
						}
						// (*it).v.l->Visible = 0;
						// Frame1->fullview->RemoveObject((*it).v.l);
						// (*it).v.l->DisposeOf();
						TThread::Queue(0,
							Classes::_di_TThreadProcedure
							(new TThrdProcRefRemoveLine(Frame1->fullview, (*it).v.l)));
					}

					break;
				}
			}

			LeaveCriticalSection(&(Frame1->cs_at_items));
		}
	}
	catch (...) {
		// Form1->Label1->Text = _T("连接错误");
		TThread::Queue(0, Classes::_di_TThreadProcedure(new TThrdProcRefLabelText(Form1->Label1,
			_T("连接错误"))));
	}

	delete[]rst;
	delete prms_post;
}

// ---------------------------------------------------------------------------
rep_spec_pack __fastcall TDataModule1::query_info_ware(String name_storloc) {
	TStringList *prms_post = new TStringList();
	rep_spec_pack rsp;
	RtlSecureZeroMemory(&rsp, sizeof(rsp));
#ifdef _DEBUG
	String myurl = Format(_T("http://%s:19998"), ARRAYOFCONST(((urls[0]))));
#else
	String myurl = Format(_T("http://%s:19998"), ARRAYOFCONST(((urls[1]))));
#endif
	if (name_storloc.operator == (_T("")))
		goto END;

	prms_post->AddPair(_T("cmd"), _T("657311"));
	prms_post->AddPair(_T("name_storloc"), name_storloc);
	try {
		request_info_storloc->Post(myurl, prms_post)->ContentStream->Read(&rsp, sizeof(rsp));
	}
	catch (...) {
		// Form1->Label1->Text = _T("获取库位上产品信息失败");
		TThread::Queue(0, Classes::_di_TThreadProcedure(new TThrdProcRefLabelText(Form1->Label1,
			_T("获取库位上产品信息失败"))));
	}
END:
	delete prms_post;
	return rsp;
}

void __fastcall TDataModule1::timer_pressTimer(TObject *Sender) {
	++count_circle_pressed;
	// if(count_circle_pressed > 8){
	// count_circle_pressed = 0;
	// timer_press->Enabled = 0;
	// Form5->Button1->Text = _T("修改");
	// if(mrYes == Form5->ShowModal()){
	// }
	// }
}
// ---------------------------------------------------------------------------
