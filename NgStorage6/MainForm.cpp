// ---------------------------------------------------------------------------

#include <fmx.h>
#pragma hdrstop

#include <System.IOUtils.hpp>
#include <System.DateUtils.hpp>
#include "common.h"
#include <process.h>
#include <algorithm>
#include "MainForm.h"
#include "FrmAGV.h"
#include "http_conn.h"
#include "task_list.h"
#include "spec_packs1st.h"
// ---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.fmx"
TForm1 *Form1;

extern const size_t content_header;

extern uint32_t my_crc32(char *, size_t);
extern void *get_in_addr(struct sockaddr *);
extern void get_in_addr4(u_long *, struct sockaddr *);
extern void alloc_smlbuf(unsigned char* *);
extern void recycle_smlbuf(unsigned char *);
extern void recycle_all_smlbuf();

unsigned __stdcall recv_func(void *);
unsigned __stdcall calc_wp_func(void *);

bool idle_agv(size_t *i, struct agv a[]) {
	size_t j;
	for (j = 0; j < LEN_AGVS; ++j) {
		if (a[j].addr_b);
		else {
			*i = j;
			break;
		}
	}
	if (j == LEN_AGVS) {
		return false;
	}
	return true;
}

#define PARAM_CONVERT(str, value) \
do {\
	if (!TryStrToInt(ARequestInfo->Params->Values[(str)], (value))) { \
		AResponseInfo->ContentText = Format(_T("REQUEST PARAM ERROR @%s"), ARRAYOFCONST(((str))));\
		return;\
	}\
}\
while (0)

// ---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner) : TForm(Owner) {
	unsigned t_id;
	thrd_wp_stop = false;
	SecureZeroMemory(agvs, sizeof(agvs));
	InitializeCriticalSectionAndSpinCount(&cs_wps, 0x400);
	InitializeCriticalSectionAndSpinCount(&cs_agviews, 0x400);
	InitializeConditionVariable(&cv_wps);
	InitializeCriticalSectionAndSpinCount(&cs_tis, 0x400);
	thrd_wp = _beginthreadex(0, 0, calc_wp_func, 0, 0, &t_id);
}

// ---------------------------------------------------------------------------
__fastcall TForm1::~TForm1() {
	delete[]flags_coords;
	thrd_wp_stop = true;
	WakeConditionVariable(&cv_wps);
	WaitForSingleObject((void *)thrd_wp, INFINITE);
	CloseHandle((void *)thrd_wp);
	CloseHandle((void *)draw_basis_end);
	DeleteCriticalSection(&cs_wps);
	DeleteCriticalSection(&cs_agviews);
	DeleteCriticalSection(&cs_tis);
	while (!wps.empty()) {
		unsigned char *p = (unsigned char *)wps.front();
		recycle_smlbuf(p);
		wps.pop();
	}
}

// ---------------------------------------------------------------------------
unsigned __stdcall calc_wp_func(void *prm) {
	bool tobedeleted = 0;
	waypoint *p = 0;
	agvtask_view *av = 0;
	storloc *st = 0;
	specloc *sp = 0;
	for (; ;) {
		EnterCriticalSection(&(Form1->cs_wps));
		while (!Form1->thrd_wp_stop && Form1->wps.empty()) {
			SleepConditionVariableCS(&Form1->cv_wps, &Form1->cs_wps, INFINITE);
		}
		if (Form1->thrd_wp_stop) {
			LeaveCriticalSection(&(Form1->cs_wps));
			break;
		}
		p = Form1->wps.front();
		Form1->wps.pop();
		LeaveCriticalSection(&(Form1->cs_wps));

		tobedeleted = 0;
		std::vector<agvtask_view*>::iterator it;
		std::vector<agvtask_view*>::iterator it2;
		EnterCriticalSection(&(Form1->cs_agviews));
		for (it = Form1->q_agvt.begin(); Form1->q_agvt.end() != it; ++it) {
			LeaveCriticalSection(&(Form1->cs_agviews));
			av = *it;
			if (av->pc.t_id == p->t_id) {
				if (I1 == av->pc.tsk) {
					// if (av->indx_s1 == -1)
					// av->status1[++av->indx_s1] = p->state;
					// else {
					// if (p->state != av->status1[av->indx_s1]) {
					// av->status1[++(av->indx_s1)] = p->state;
					// if (agvTASKE == av->status1[av->indx_s1]) {
					// Frame1->wipe_line(av);
					// it2 = it;
					// tobedeleted = 1;
					//
					// }
					// else if (agvUNLOAD == av->status1[av->indx_s1]) {
					// st = (storloc*)av->e.c->Tag;
					// st->full = 1;
					// av->e.c->Fill->Color = st->color[1];
					// }
					// else if (agvLOAD == av->status1[av->indx_s1]) {
					// sp = (specloc*)av->s.r->Tag;
					// sp->signal = 0;
					// av->s.r->Fill->Color = sp->color[0];
					// }
					// }
					// }
					if (agvTASKE == p->state) {
						if (2 == p->act_load) {
							st = (storloc*)av->e.c->Tag;
							st->full = 1;
							av->e.c->Fill->Color = st->color[1];
						}
						Frame1->wipe_line(av);
						it2 = it;
						tobedeleted = 1;
					}
					else {
						if (1 == p->act_load) {
							sp = (specloc*)av->s.r->Tag;
							sp->signal = 0;
							av->s.r->Fill->Color = sp->color[0];
						}
						else if (2 == p->act_load) {
							st = (storloc*)av->e.c->Tag;
							st->full = 1;
							av->e.c->Fill->Color = st->color[1];
						}
					}
				}
				else if (U1 == av->pc.tsk) {
					// if (av->indx_s1 == -1)
					// av->status1[++av->indx_s1] = p->state;
					// else {
					// if (p->state != av->status1[av->indx_s1]) {
					// av->status1[++av->indx_s1] = p->state;
					// if (agvTASKE == av->status1[av->indx_s1]) {
					// Frame1->wipe_line(av);
					// it2 = it;
					// tobedeleted = 1;
					// }
					// else if (agvUNLOAD == av->status1[av->indx_s1]) {
					// st = (storloc*)av->e.c->Tag;
					// st->full = 1;
					// av->e.c->Fill->Color = st->color[1];
					// }
					// else if (agvLOAD == av->status1[av->indx_s1]) {
					// st = (storloc*)av->s.c->Tag;
					// st->full = 0;
					// av->s.c->Fill->Color = st->color[0];
					// }
					// }
					// }

					if (agvTASKE == p->state) {
						if (2 == p->act_load) {
							st = (storloc*)av->e.c->Tag;
							st->full = 1;
							av->e.c->Fill->Color = st->color[1];
						}
						Frame1->wipe_line(av);
						it2 = it;
						tobedeleted = 1;
					}
					else {
						if (1 == p->act_load) {
							st = (storloc*)av->s.c->Tag;
							st->full = 0;
							av->s.c->Fill->Color = st->color[0];
						}
						else if (2 == p->act_load) {
							st = (storloc*)av->e.c->Tag;
							st->full = 1;
							av->e.c->Fill->Color = st->color[1];
						}
					}
				}
				else if (X1 == av->pc.tsk || X2 == av->pc.tsk) {
					// if (-1 == av->indx_s1)
					// av->status1[++av->indx_s1] = p->state;
					// else {
					// if (p->state != av->status1[av->indx_s1]) {
					// av->status1[++av->indx_s1] = p->state;
					// if (agvTASKE == av->status1[av->indx_s1]) {
					// Frame1->wipe_line(av);
					// it2 = it;
					// tobedeleted = 1;
					// }
					// else if (agvUNLOAD == av->status1[av->indx_s1]) {
					// st = (storloc*)av->e.c->Tag;
					// st->full = 1;
					// av->e.c->Fill->Color = st->color[1];
					// }
					// else if (agvLOAD == av->status1[av->indx_s1]) {
					// st = (storloc*)av->s.c->Tag;
					// st->full = 0;
					// av->s.c->Fill->Color = st->color[0];
					// }
					// }
					// }

					if (agvTASKE == p->state) {
						if (2 == p->act_load) {
							st = (storloc*)av->e.c->Tag;
							st->full = 1;
							av->e.c->Fill->Color = st->color[1];
						}
						Frame1->wipe_line(av);
						it2 = it;
						tobedeleted = 1;
					}
					else {
						if (1 == p->act_load) {
							st = (storloc*)av->s.c->Tag;
							st->full = 0;
							av->s.c->Fill->Color = st->color[0];
						}
						else if (2 == p->act_load) {
							st = (storloc*)av->e.c->Tag;
							st->full = 1;
							av->e.c->Fill->Color = st->color[1];
						}
					}
				}
				else if (CHRG == av->pc.tsk) {
					if (-1 == av->indx_s1)
						av->status1[++av->indx_s1] = p->state;
					else {
						av->status1[++av->indx_s1] = p->state;
						if (agvTASKE == av->status1[av->indx_s1]) {
							it2 = it;
							tobedeleted = 1;
						}
						else if (agvCHRG == av->status1[av->indx_s1]) {
							sp = (specloc*)av->e.r->Tag;
							sp->signal = 1;
							av->e.r->Fill->Color = sp->color[1];
						}
						else if (agvCHRGFULL == av->status1[av->indx_s1]) {
							sp = (specloc*)av->e.r->Tag;
							sp->signal = 0;
							av->e.r->Fill->Color = sp->color[0];
						}
					}
				}
				else if (I2 == av->pc.tsk) {
					if (agvTASKE == p->state) {
						if (2 == p->act_load) {
							st = (storloc*)av->e.c->Tag;
							st->full = 1;
							av->e.c->Fill->Color = st->color[1];
						}
						Frame1->wipe_line(av);
						it2 = it;
						tobedeleted = 1;
					}
					else {
						if (1 == p->act_load) {
							sp = (specloc*)av->s.r->Tag;
							sp->signal = 0;
							av->s.r->Fill->Color = sp->color[0];
						}
						else if (2 == p->act_load) {
							st = (storloc*)av->e.c->Tag;
							st->full = 1;
							av->e.c->Fill->Color = st->color[1];
						}
					}
				}
				else if (U2 == av->pc.tsk) {
					if (agvTASKE == p->state) {
						if (2 == p->act_load) {
							st = (storloc*)av->e.c->Tag;
							st->full = 1;
							av->e.c->Fill->Color = st->color[1];
						}
						Frame1->wipe_line(av);
						it2 = it;
						tobedeleted = 1;
					}
					else {
						if (1 == p->act_load) {
							st = (storloc*)av->s.c->Tag;
							st->full = 0;
							av->s.c->Fill->Color = st->color[0];
						}
						else if (2 == p->act_load) {
							st = (storloc*)av->e.c->Tag;
							st->full = 1;
							av->e.c->Fill->Color = st->color[1];
						}
					}
				}
			}
			EnterCriticalSection(&(Form1->cs_agviews));
		}
		LeaveCriticalSection(&(Form1->cs_agviews));

		if (tobedeleted) {
			EnterCriticalSection(&(Form1->cs_agviews));
			Form1->q_agvt.erase(it2);
			LeaveCriticalSection(&(Form1->cs_agviews));
			recycle_smlbuf((unsigned char *)(*it2));
		}

		recycle_smlbuf((unsigned char *)p);
	}

	return 0;
}

// ---------------------------------------------------------------------------
unsigned __stdcall recv_func(void *prm) {
	int nbytes = 0;
	size_t offset = 0;
	size_t capacity = 0;

	unsigned char buf_base[LEN_SOCK_ADDR_BUF];
	unsigned char *buf = 0;
	SecureZeroMemory(buf_base, LEN_SOCK_ADDR_BUF);
	for (; ;) {
		buf = buf_base + offset;
		capacity = LEN_SOCK_ADDR_BUF - offset;
		if ((nbytes = recv(Form1->mysock, (char *)buf, capacity, 0)) <= 0) {
			shutdown(Form1->mysock, SD_SEND);
			unsigned char buf_base2[LEN_SOCK_ADDR_BUF];
			memset(buf_base2, 0, LEN_SOCK_ADDR_BUF);
			for (; nbytes = recv(Form1->mysock, (char *)buf_base2, LEN_SOCK_ADDR_BUF, 0) > 0;);
			closesocket(Form1->mysock);
			break;
		}
		else {
			size_t len_taken = nbytes + offset;
			size_t len_pack = 0;
		BUF_PARSE:
			if (len_taken < LEN_DX) {
				offset = len_taken;
				continue;
			}
			else {
				if (((UINT_MAX == *(size_t*)(buf_base)) && (UINT_MAX == *(size_t*)
					(buf_base +sizeof(size_t))) && (UINT_MAX == *(size_t*)(buf_base +
					2 * sizeof(size_t))))) {
					dx *adx = 0;
					len_pack = *(size_t*)(buf_base +sizeof(size_t) * 3);
					if (len_pack > len_taken) {
						offset = len_taken;
						continue;
					}

					adx = (dx*)buf_base;
					if (657184 == adx->cmd) {
						for (size_t j = 0; j < LEN_AGVS; ++j) {
							if (adx->ip == Form1->agvs[j].addr_b) {
								SecureZeroMemory(&(Form1->agvs[j]), sizeof(agv));
								break;
							}
						}
					}
					else if (657185 == adx->cmd) {
						size_t j = 0;
						if (idle_agv(&j, Form1->agvs)) {
							Form1->agvs[j].addr_b = adx->ip;
							Form1->agvs[j].storey = 1;
							++Form1->agvs[j].No_cmd;
						}
					}
					else if (657186 == adx->cmd) {
						size_t j = 0;
						if (idle_agv(&j, Form1->agvs)) {
							Form1->agvs[j].addr_b = adx->ip;
							Form1->agvs[j].storey = 2;
							++Form1->agvs[j].No_cmd;
						}
					}
					else if (657201 == adx->cmd) {
						for (size_t j = 0; j < LEN_AGVS; ++j) {
							if (adx->ip == Form1->agvs[j].addr_b) {
								size_t len_wps = len_pack -sizeof(dx)-sizeof(uint32_t);
								MoveMemory(&(Form1->agvs[j].wps), buf_base +sizeof(dx), len_wps);
								++Form1->agvs[j].No_cmd;
							}
						}
					}
					else if (657202 == adx->cmd) {
						size_t j = 0;
						unsigned char *p = 0;
						for (j = 0; j < LEN_AGVS; ++j) {
							if (adx->ip == Form1->agvs[j].addr_b)
								break;
						}
						alloc_smlbuf(&p);
						waypoint *p1 = (waypoint*)p;
						*p1 = *(waypoint*)(buf_base +sizeof(dx));
						for (std::vector<agvtask_view*>::iterator it_b = Form1->q_agvt.begin();
						Form1->q_agvt.end() < it_b; ++it_b) {
							agvtask_view *tmp_av = *it_b;
							if ((tmp_av)->pc.t_id == p1->t_id) {
								if ((tmp_av->pc.tsk == I1) || (tmp_av->pc.tsk == U1) ||
									(tmp_av->pc.tsk == X1) || (tmp_av->pc.tsk == X2) ||
									(tmp_av->pc.tsk == CHRG)) {
									if (tmp_av->indx_agv1 != -1);
									else
										tmp_av->indx_agv1 = j;
								}
								else if ((tmp_av->pc.tsk == I2) || (tmp_av->pc.tsk == U2) ||
									(tmp_av->pc.tsk == X12) || (tmp_av->pc.tsk == X21)) {
									if (-1 == tmp_av->indx_agv1 && -1 == tmp_av->indx_s1) {
										tmp_av->indx_agv1 = j;
									}
									else if (-1 != tmp_av->indx_s1 && -1 == tmp_av->indx_agv2) {
										tmp_av->indx_agv2 = j;
									}
								}
							}
						}

						EnterCriticalSection(&Form1->cs_wps);
						Form1->wps.push(p1);
						LeaveCriticalSection(&Form1->cs_wps);
						WakeConditionVariable(&Form1->cv_wps);
					}
					else if (657203 == adx->cmd) {
						for (size_t j = 0; j < LEN_AGVS; ++j) {
							if (adx->ip == Form1->agvs[j].addr_b) {
								Form1->agvs[j].battery = *(size_t*)(buf_base +sizeof(dx));
								++Form1->agvs[j].No_cmd;
							}
						}
					}
					else if (757172 == adx->cmd) {
						Form1->No_cmd_base = adx->No;
						Form1->No_cmd_crt = Form1->No_cmd_base;
						Form1->No_cmd_limit = Form1->No_cmd_base + RANGE;
					}
					else if (657301 == adx->cmd) {

					}

					len_taken -= len_pack;
					MoveMemory(buf_base, buf_base + len_pack, len_taken);
					goto BUF_PARSE;
				}
				else {
					size_t mod = 3 * sizeof(size_t);
					size_t num_mod = len_taken / mod;
					size_t rem = len_taken % mod;
					size_t k;
					buf = buf_base;
					for (k = 0; k < num_mod; (++k), (buf += mod))
						if ((UINT_MAX == *(size_t*)buf) &&
							(UINT_MAX == *(size_t*)(buf +sizeof(size_t))) &&
							(UINT_MAX == *(size_t*)(buf + 2 * sizeof(size_t))))
							break;
					if (k != num_mod) {
						MoveMemory(buf_base, buf, len_taken -= ((size_t)buf - (size_t)buf_base));
						goto BUF_PARSE;
					}
					else {
						if (rem) {
							MoveMemory(buf_base, buf, rem);
							offset = rem;
						}
						else
							offset = 0;
					}
				}
			}
		}
	}

	return 0;
}

// ---------------------------------------------------------------------------
bool __fastcall TForm1::idle_agv(size_t * i) {
	size_t j;
	for (j = 0; j < LEN_AGVS; ++j) {
		if (agvs[j].addr_b);
		else {
			*i = j;
			break;
		}
	}
	if (j == LEN_AGVS) {
		return false;
	}
	return true;
}

// ---------------------------------------------------------------------------
void __fastcall TForm1::FormCreate(TObject * Sender) {
	unsigned t_id;
	size_t j;
	TResourceStream *rc_strm_style = 0;
	// size_t nm;
	// GlobalUseGPUCanvas = 1;
	use_winsock(2, 2);
	GlobalUseDX = 1;
	GlobalUseDXSoftware = 1;
	Quality = TCanvasQuality::ccHighPerformance;
	home = Ioutils::TPath::GetDirectoryName(ParamStr(0));
	conf = home + PathDelim + String(_T("NgStorage")) + String(_T(".ini"));
	spec_package = home + PathDelim + String(_T("SpecPackage")) + String(_T(".ini"));
	exes_status = (TIntegerSet)0;
	exes_paths.Length = UI;
	SecureZeroMemory(launch_infos, sizeof launch_infos);
	for (size_t i = tcpSrv; i < UI; ++i) {
		launch_infos[i].cbSize = sizeof launch_infos[i];
		launch_infos[i].fMask =
			SEE_MASK_NOASYNC | SEE_MASK_FLAG_NO_UI | SEE_MASK_UNICODE | SEE_MASK_NO_CONSOLE |
			SEE_MASK_NOCLOSEPROCESS;
		launch_infos[i].hwnd = 0;
		launch_infos[i].lpVerb = _T("open");
		launch_infos[i].lpDirectory = 0;
		launch_infos[i].nShow = SW_HIDE;
		launch_infos[i].lpParameters = params[i];
	}
	// boot(home);
	if (FileExists(conf))
		parse_config();
	else {
		conf_template();
		parse_config();
	}

	SecureZeroMemory(&records_sp, sizeof(records_sp));
	if (FileExists(spec_package))
		parse_spec_package();
	else {
		spec_package_template();
		parse_spec_package();
	}

	build_matrix();
	// add_menuitems();
	draw_basis_end = CreateEvent(0, 0, 0, 0);
	btn_context_init();

	// nm = m.Length;
	// storey_forms.set_length(nm);
	// for (size_t i = 0; i < nm; ++i) {
	// storey_forms[i] = new TForm3(0);
	// storey_forms[i]->Tag = i;
	// storey_forms[i]->Caption = String(_T("≤„")) + IntToStr((int)(i + 1));
	// // storey_forms[i]->m.Copy(m.operator [](i));
	// // storey_forms[i]->m = m[i];
	// // storey_forms[i]->m.Copy(m.operator [](i));
	// storey_forms[i]->m = m.operator[](i).Copy();
	// storey_forms[i]->popup_params = popup_params.Copy();
	// }

	// MaterialPatternsBlue_Win
	// UbuntuClearFantasy_Win
	/*
	 StyleBook1->LoadFromFile(Format(_T("%s%s%s"), ARRAYOFCONST((ExtractFilePath(ParamStr(0)),
	 _T("UbuntuClearFantasy_Win"), _T(".Style")))));
	 StyleBook = StyleBook1;
	 */
	// rc_strm_style = new TResourceStream((int)HInstance, "Resource_Style", RT_RCDATA);
	rc_strm_style = new TResourceStream((int)HInstance, "Resource_1", RT_RCDATA);
	// rc_strm_style = new TResourceStream((int)HInstance, "Resource_2", RT_RCDATA);
	StyleBook1->LoadFromStream(rc_strm_style);
	StyleBook = StyleBook1;
	rc_strm_style->DisposeOf();

	info_agvs = new agv_n[count_agvs];
	SecureZeroMemory(info_agvs, sizeof(agv_n) * count_agvs);

	Frame1 = new TFrame1(0);
	DataModule1->query_available_storlocs();

	j = buttons_storey.Length;
	for (size_t i = 0; i < j; ++i) {
		buttons_storey[i]->Visible = 0;
	}
	storey_click(buttons_storey[j - 1]);
	ComboBox1->CanFocus = 0;
	// ComboBox2->CanFocus = 0;
	Edit2->CanFocus = 0;
	Edit1->CanFocus = 0;
	Edit2->Tag = 0;
	Edit2->TagObject = 0;
	Edit1->Tag = 0;
	Edit1->TagObject = 0;
	SpeedButton1->Enabled = 0;

	Label1->TagString = _T("connect status");
	// client_conn();
	// thrd_rcv = _beginthreadex(0, 0, recv_func, 0, 0, &t_id);
	event_nextagvtask = CreateEvent(0, 0, 1, 0);
	DataModule1->client_id();
}
// ---------------------------------------------------------------------------

void __fastcall TForm1::boot(String root) {
	String exe_name;
	TStringDynArray exes;

	exes = TDirectory::GetFiles(root, _T("*.exe"), TSearchOption::soAllDirectories);
	for (size_t i = 0; i < (size_t)exes.Length; ++i) {
		exe_name = Ioutils::TPath::GetFileName(exes[i]);
		for (size_t j = tcpSrv; j < UI; ++j) {
			if (exe_names[j] == exe_name) {
				exes_status.operator << (j);
				exes_paths[j] = exes[i];
			}
		}
	}

	if (0x1f != exes_status.ToInt()) {
		String parent;
		parent = TDirectory::GetParent(root);
		if (String(_T("")) != parent)
			boot(parent);
	}
	else {
		CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
		for (size_t i = tcpSrv; i < UI; ++i) {
			launch_infos[i].lpFile = exes_paths[i].c_str();
			ShellExecuteEx(&launch_infos[i]);
		}
		return;
	}
}

// ---------------------------------------------------------------------------
void __fastcall TForm1::parse_config() {
	int num_storey;
	int i, j, k, h, l;
	TStringList *sl = 0;
	TIniFile *conf_ini = 0;
	String b, c, t;
	pack_loc_base plb;
	pack_artery art;
	try {
		conf_ini = new TIniFile(conf);
		sl = new TStringList;
		id_warehouse = conf_ini->ReadInteger(_T("LocInfo"), _T("id_warehouse"), 1);
		num_storey = conf_ini->ReadInteger(_T("LocInfo"), _T("num_storey"), 2);
		for (i = 0; i < num_storey; ++i) {
			b = IntToStr((int)i) + conffix[storey_x];
			plb.lb[x] = conf_ini->ReadInteger(_T("LocInfo"), b, 0);

			b = IntToStr((int)i) + conffix[storey_y];
			plb.lb[y] = conf_ini->ReadInteger(_T("LocInfo"), b, 0);

			plb.lb[storey] = i;
			cfp.storeys.push_back(plb);

			b = IntToStr((int)i) + conffix[storey_artery_y];
			j = conf_ini->ReadInteger(_T("LocInfo"), b, 0);
			for (k = 0; k < j; ++k) {
				c = b + IntToStr((int)k);
				t = conf_ini->ReadString(_T("LocInfo"), c, _T(""));
				l = 0;
				while (h = t.Pos(_T(","))) {
					art.a[l] = StrToInt(t.SubString(1, h - 1));
					t.Delete(1, h);
					++l;
				}
				art.a[l] = StrToInt(t);
				art.a[storey_artery] = i;
				art.a[axis] = axisY;
				cfp.arteries.push_back(art);
			}

			b = IntToStr((int)i) + conffix[storey_artery_x];
			j = conf_ini->ReadInteger(_T("LocInfo"), b, 0);
			for (k = 0; k < j; ++k) {
				c = b + IntToStr((int)k);
				t = conf_ini->ReadString(_T("LocInfo"), c, _T(""));
				l = 0;
				while (h = t.Pos(_T(","))) {
					art.a[l] = StrToInt(t.SubString(1, h - 1));
					t.Delete(1, h);
					++l;
				}
				art.a[l] = StrToInt(t);
				art.a[storey_artery] = i;
				art.a[axis] = axisX;
				cfp.arteries.push_back(art);
			}
		}

		num_storey = conf_ini->ReadInteger(_T("Area"), _T("num_areas"), 6);
		for (i = 0; i < num_storey; ++i) {
			area a_area;
			SecureZeroMemory(&a_area, sizeof(a_area));
			b = IntToStr((int)i) + area_params[area_range];
			c = conf_ini->ReadString(_T("Area"), b, _T(""));
			l = 0;
			while (h = c.Pos(_T(","))) {
				a_area.range[l] = StrToInt(c.SubString(1, h - 1));
				c.Delete(1, h);
				++l;
			}
			a_area.range[l] = StrToInt(c);
			b = IntToStr((int)i) + area_params[area_light];
			a_area.color_empty = conf_ini->ReadFloat(_T("Area"), b, TAlphaColors::White);
			b = IntToStr((int)i) + area_params[area_dark];
			a_area.color_occupied = conf_ini->ReadFloat(_T("Area"), b, TAlphaColors::White);
			b = IntToStr((int)i) + area_params[area_storey];
			a_area.range[storey_artery] = conf_ini->ReadFloat(_T("Area"), b, 0);
			b = IntToStr((int)i) + area_params[area_txt];
			a_area.txt = conf_ini->ReadString(_T("Area"), b, _T(""));
			a_area.No_area = a_area.txt.SubString(2, a_area.txt.Length() - 1).ToInt();
			b = IntToStr((int)i) + area_params[area_base_point];
			c = conf_ini->ReadString(_T("Area"), b, _T(""));
			l = c.Pos(_T("C"));
			a_area.base_point[0] = StrToInt(c.SubString(2, l - 2));
			a_area.base_point[1] = StrToInt(c.SubString(l + 1, c.Length() - l));
			cfp.areas.push_back(a_area);
		}

		num_storey = conf_ini->ReadInteger(_T("SpecLoc"), _T("num_specloc"), 20);
		cfp.sl_ary = new specloc[num_storey];
		cfp.len_sl_ary = num_storey;
		for (i = 0; i < num_storey; ++i) {
			b = IntToStr((int)i) + specloc_params[sl_type];
			cfp.sl_ary[i].sn_flag[0] = conf_ini->ReadInteger(_T("SpecLoc"), b, 0);
			b = IntToStr((int)i) + specloc_params[sl_sn];
			cfp.sl_ary[i].sn_flag[1] = conf_ini->ReadInteger(_T("SpecLoc"), b, 0);
			b = IntToStr((int)i) + specloc_params[sl_txt];
			cfp.sl_ary[i].txt = conf_ini->ReadString(_T("SpecLoc"), b, _T(""));
			b = IntToStr((int)i) + specloc_params[sl_range];
			c = conf_ini->ReadString(_T("SpecLoc"), b, _T(""));
			l = 0;
			while (h = c.Pos(_T(","))) {
				cfp.sl_ary[i].range[l] = StrToInt(c.SubString(1, h - 1));
				c.Delete(1, h);
				++l;
			}
			cfp.sl_ary[i].range[l] = StrToInt(c);
			b = IntToStr((int)i) + specloc_params[sl_storey];
			cfp.sl_ary[i].range[storey_artery] = conf_ini->ReadInteger(_T("SpecLoc"), b, 0);
			b = IntToStr((int)i) + specloc_params[sl_color0];
			cfp.sl_ary[i].color[0] = conf_ini->ReadFloat(_T("SpecLoc"), b, 0);
			b = IntToStr((int)i) + specloc_params[sl_color1];
			cfp.sl_ary[i].color[1] = conf_ini->ReadFloat(_T("SpecLoc"), b, 0);
			b = IntToStr((int)i) + specloc_params[hittest];
			cfp.sl_ary[i].hittest = conf_ini->ReadInteger(_T("SpecLoc"), b, 0);
		}

		// std::sort(cfp.storeys.begin(), cfp.storeys.end(), storey_sort);

		l = conf_ini->ReadInteger(_T("CoordS"), _T("NumsOrgs"), 8);
		num_corrds = l;
		flags_coords = new coords[l];
		SecureZeroMemory(flags_coords, l * sizeof(coords));
		for (i = 0; i < l; ++i) {
			b = IntToStr((int)i) + coords_suffix[0];
			flags_coords[i].x = conf_ini->ReadInteger(_T("CoordS"), b, 0);
			b = IntToStr((int)i) + coords_suffix[1];
			flags_coords[i].y = conf_ini->ReadInteger(_T("CoordS"), b, 0);
			b = IntToStr((int)i) + coords_suffix[2];
			flags_coords[i].axis = conf_ini->ReadInteger(_T("CoordS"), b, 0);
			b = IntToStr((int)i) + coords_suffix[3];
			flags_coords[i].base = conf_ini->ReadInteger(_T("CoordS"), b, 0);
			b = IntToStr((int)i) + coords_suffix[4];
			flags_coords[i].len = conf_ini->ReadInteger(_T("CoordS"), b, 0);
			b = IntToStr((int)i) + coords_suffix[5];
			flags_coords[i].storey = conf_ini->ReadInteger(_T("CoordS"), b, 0);
			b = IntToStr((int)i) + coords_suffix[6];
            flags_coords[i].area = conf_ini->ReadInteger(_T("CoordS"), b, -1);
		}

		l = conf_ini->ReadInteger(_T("DotPopup"), _T("ParamCount"), 8);
		popup_params.set_length(l);
		for (i = 0; i < l; ++i) {
			t = String(_T("DPP")) + IntToStr((int)i);
			c = conf_ini->ReadString(_T("DotPopup"), t, _T(""));
			popup_params[i] = c;
		}

		l = conf_ini->ReadInteger(_T("AGVPrms"), _T("ParamCount"), 10);
		agvprms.set_length(l);
		for (i = 0; i < l; ++i) {
			t = String(_T("AGVp")) + IntToStr((int)i);
			c = conf_ini->ReadString(_T("AGVPrms"), t, _T(""));
			agvprms[i] = c;
		}

		count_agvs = conf_ini->ReadInteger(_T("AGVPrms"), _T("AGVnumber"), 9);
		agv_colors.set_length(count_agvs);
		for (i = 0; i < count_agvs; ++i) {
			t = String(_T("agvColor")) + IntToStr((int)i);
			agv_colors[i] = conf_ini->ReadFloat(_T("AGVPrms"), t, 0);
		}

		sl->Clear();
		conf_ini->ReadSectionValues(_T("DefaultColor"), sl);
		for (i = 0; i < sl->Count; ++i) {
			t = sl->operator[](i);
			j = t.Pos(_T("="));
			c = t.SubString(j + 1, t.Length() - j);
			cfp.used_colors.push_back(StrToUInt(c));
		}

		sl->Clear();
		conf_ini->ReadSectionValues(_T("AvailColor"), sl);
		for (i = 0; i < sl->Count; ++i) {
			t = sl->operator[](i);
			j = t.Pos(_T("="));
			c = t.SubString(j + 1, t.Length() - j);
			cfp.all_colors.push_back(StrToUInt(c));
		}

	}
	catch (Exception* e) {
		// ShowMessage(_T("Configuration Parse Error"));
	}
	delete sl;
	delete conf_ini;
	conf_ini = 0;
	sl = 0;

#ifdef _DEBUG
	std::vector<pack_loc_base>::iterator it_plb;
	for (it_plb = cfp.storeys.begin(); it_plb != cfp.storeys.end(); it_plb++) {
		plb = *it_plb;
		t = plb.lb[x];
		t += ",";
		t += plb.lb[y];
		t += ",";
		t += plb.lb[storey];
		OutputDebugString(t.c_str());
	}

	std::vector<pack_artery>::iterator it_pa;
	for (it_pa = cfp.arteries.begin(); it_pa != cfp.arteries.end(); ++it_pa) {
		art = *it_pa;
		t = art.a[bx];
		t += ",";
		t += art.a[by];
		t += ",";
		t += art.a[ex];
		t += ",";
		t += art.a[ey];
		t += ",";
		t += IntToStr((int)art.a[storey_artery]);
		OutputDebugString(t.c_str());
	}

#endif
}

// ---------------------------------------------------------------------------
void __fastcall TForm1::conf_template() {
	TIniFile *conf_ini = new TIniFile(conf);
	conf_ini->WriteInteger(_T("LocInfo"), _T("id_warehouse"), 1);
	conf_ini->WriteInteger(_T("LocInfo"), _T("num_storey"), 2);
	conf_ini->WriteInteger(_T("LocInfo"), _T("0storey_x"), 36);
	conf_ini->WriteInteger(_T("LocInfo"), _T("0storey_y"), 28);
	// 0-none,1,2..
	conf_ini->WriteInteger(_T("LocInfo"), _T("0storey_artery_y"), 7);
	// conf_ini->WriteString(_T("LocInfo"), _T("0storey_artery_y0"), _T("0,0,35,3"));
	conf_ini->WriteString(_T("LocInfo"), _T("0storey_artery_y0"), _T("0,0,35,4"));
	// conf_ini->WriteString(_T("LocInfo"), _T("0storey_artery_y1"), _T("0,4,19,9"));
	// conf_ini->WriteString(_T("LocInfo"), _T("0storey_artery_y1"), _T("0,5,23,9"));
	conf_ini->WriteString(_T("LocInfo"), _T("0storey_artery_y1"), _T("0,5,15,9"));
	conf_ini->WriteString(_T("LocInfo"), _T("0storey_artery_y2"), _T("20,5,23,9"));
	conf_ini->WriteString(_T("LocInfo"), _T("0storey_artery_y3"), _T("34,4,35,9"));
	conf_ini->WriteString(_T("LocInfo"), _T("0storey_artery_y4"), _T("0,10,35,13"));
	conf_ini->WriteString(_T("LocInfo"), _T("0storey_artery_y5"), _T("16,14,17,14"));
	conf_ini->WriteString(_T("LocInfo"), _T("0storey_artery_y6"), _T("25,14,27,14"));
	conf_ini->WriteInteger(_T("LocInfo"), _T("0storey_artery_x"), 4);
	conf_ini->WriteString(_T("LocInfo"), _T("0storey_artery_x0"), _T("0,14,15,27"));
	conf_ini->WriteString(_T("LocInfo"), _T("0storey_artery_x1"), _T("16,24,17,27"));
	// conf_ini->WriteString(_T("LocInfo"), _T("0storey_artery_x2"), _T("18,14,18,25"));
	conf_ini->WriteString(_T("LocInfo"), _T("0storey_artery_x2"), _T("24,14,24,27"));
	conf_ini->WriteString(_T("LocInfo"), _T("0storey_artery_x3"), _T("25,24,27,27"));
	conf_ini->WriteInteger(_T("LocInfo"), _T("1storey_x"), 36);
	conf_ini->WriteInteger(_T("LocInfo"), _T("1storey_y"), 28);
	conf_ini->WriteInteger(_T("LocInfo"), _T("1storey_artery_y"), 3);
	conf_ini->WriteString(_T("LocInfo"), _T("1storey_artery_y0"), _T("11,20,11,22"));
	conf_ini->WriteString(_T("LocInfo"), _T("1storey_artery_y1"), _T("12,20,12,21"));
	conf_ini->WriteString(_T("LocInfo"), _T("1storey_artery_y2"), _T("13,20,13,20"));
	conf_ini->WriteInteger(_T("LocInfo"), _T("1storey_artery_x"), 4);
	conf_ini->WriteString(_T("LocInfo"), _T("1storey_artery_x0"), _T("0,0,35,19"));
	conf_ini->WriteString(_T("LocInfo"), _T("1storey_artery_x1"), _T("22,20,24,20"));
	conf_ini->WriteString(_T("LocInfo"), _T("1storey_artery_x2"), _T("0,20,10,27"));
	conf_ini->WriteString(_T("LocInfo"), _T("1storey_artery_x3"), _T("33,20,33,20"));

	conf_ini->WriteFloat(_T("DefaultColor"), _T("Bad"), 0xff000000);
	conf_ini->WriteFloat(_T("DefaultColor"), _T("lift"), 0xff32cd32);

	conf_ini->WriteFloat(_T("Area"), _T("num_areas"), 6);
	conf_ini->WriteString(_T("Area"), _T("0area_range"), _T("16,14,35,27"));
	// conf_ini->WriteFloat(_T("Area"), _T("0area_light"), 0xff90ee90);
	// conf_ini->WriteFloat(_T("Area"), _T("0area_dark"), 0xffdaa520);
	conf_ini->WriteFloat(_T("Area"), _T("0area_storey"), 0x00);
	conf_ini->WriteString(_T("Area"), _T("0area_base_point"), _T("R13C19"));
	conf_ini->WriteString(_T("Area"), _T("0area_txt"), _T("A00"));
	// conf_ini->WriteString(_T("Area"), _T("1area_range"), _T("24,4,33,9"));
	conf_ini->WriteString(_T("Area"), _T("1area_range"), _T("24,5,33,9"));
	// conf_ini->WriteFloat(_T("Area"), _T("1area_light"), 0xff00ffff);
	// conf_ini->WriteFloat(_T("Area"), _T("1area_dark"), 0xffcd853f);
	conf_ini->WriteFloat(_T("Area"), _T("1area_storey"), 0x00);
	conf_ini->WriteString(_T("Area"), _T("1area_base_point"), _T("R00C09"));
	conf_ini->WriteString(_T("Area"), _T("1area_txt"), _T("A01"));
	conf_ini->WriteString(_T("Area"), _T("2area_range"), _T("16,5,19,9"));
	conf_ini->WriteFloat(_T("Area"), _T("2area_storey"), 0x00);
	conf_ini->WriteString(_T("Area"), _T("2area_base_point"), _T("R00C03"));
	conf_ini->WriteString(_T("Area"), _T("2area_txt"), _T("A02"));
	// conf_ini->WriteString(_T("Area"), _T("2area_range"), _T("20,4,23,9"));
	// conf_ini->WriteString(_T("Area"), _T("2area_range"), _T("20,5,23,9"));
	// conf_ini->WriteFloat(_T("Area"), _T("2area_light"), 0xffffebcd);
	// conf_ini->WriteFloat(_T("Area"), _T("2area_dark"), 0xff0000ff);
	// conf_ini->WriteFloat(_T("Area"), _T("2area_storey"), 0x00);
	// conf_ini->WriteString(_T("Area"), _T("2area_base_point"), _T("R00C03"));
	// conf_ini->WriteString(_T("Area"), _T("2area_txt"), _T("A02"));
	// x+36
	// conf_ini->WriteString(_T("Area"), _T("3area_range"), _T("47,20,71,27"));
	conf_ini->WriteString(_T("Area"), _T("3area_range"), _T("11,20,35,27"));
	// conf_ini->WriteFloat(_T("Area"), _T("2area_light"), 0xffff69b4);
	// conf_ini->WriteFloat(_T("Area"), _T("2area_dark"), 0xff00008b);
	conf_ini->WriteFloat(_T("Area"), _T("3area_storey"), 0x01);
	conf_ini->WriteString(_T("Area"), _T("3area_base_point"), _T("R07C24"));
	conf_ini->WriteString(_T("Area"), _T("3area_txt"), _T("A03"));
	conf_ini->WriteString(_T("Area"), _T("4area_range"), _T("16,15,17,23"));
	// conf_ini->WriteFloat(_T("Area"), _T("3area_light"), 0xff90ee90);
	// conf_ini->WriteFloat(_T("Area"), _T("3area_dark"), 0xffdaa520);
	conf_ini->WriteFloat(_T("Area"), _T("4area_storey"), 0x00);
	conf_ini->WriteString(_T("Area"), _T("4area_base_point"), _T("R00C19"));
	conf_ini->WriteString(_T("Area"), _T("4area_txt"), _T("A00"));
	conf_ini->WriteString(_T("Area"), _T("5area_range"), _T("25,15,27,23"));
	// conf_ini->WriteFloat(_T("Area"), _T("4area_light"), 0xff90ee90);
	// conf_ini->WriteFloat(_T("Area"), _T("4area_dark"), 0xffdaa520);
	conf_ini->WriteFloat(_T("Area"), _T("5area_storey"), 0x00);
	conf_ini->WriteString(_T("Area"), _T("5area_base_point"), _T("R00C10"));
	conf_ini->WriteString(_T("Area"), _T("5area_txt"), _T("A00"));

	conf_ini->WriteInteger(_T("SpecLoc"), _T("num_specloc"), 18);
	conf_ini->WriteInteger(_T("SpecLoc"), _T("0sl_type"), 100001);
	conf_ini->WriteInteger(_T("SpecLoc"), _T("0sl_sn"), 0x00);
	conf_ini->WriteString(_T("SpecLoc"), _T("0sl_txt"), _T("D00"));
	conf_ini->WriteString(_T("SpecLoc"), _T("0sl_range"), _T("2,11,2,12"));
	conf_ini->WriteInteger(_T("SpecLoc"), _T("0sl_storey"), 0x00);
	conf_ini->WriteFloat(_T("SpecLoc"), _T("0sl_color0"), 0xfff0fff0);
	conf_ini->WriteFloat(_T("SpecLoc"), _T("0sl_color1"), 0xff696969);
	conf_ini->WriteInteger(_T("SpecLoc"), _T("0hittest"), 0x1);
	conf_ini->WriteInteger(_T("SpecLoc"), _T("1sl_type"), 100002);
	conf_ini->WriteInteger(_T("SpecLoc"), _T("1sl_sn"), 0x01);
	conf_ini->WriteString(_T("SpecLoc"), _T("1sl_txt"), _T("F01"));
	conf_ini->WriteString(_T("SpecLoc"), _T("1sl_range"), _T("6,11,6,12"));
	conf_ini->WriteInteger(_T("SpecLoc"), _T("1sl_storey"), 0x00);
	conf_ini->WriteFloat(_T("SpecLoc"), _T("1sl_color0"), 0xff2e8b57);
	conf_ini->WriteFloat(_T("SpecLoc"), _T("1sl_color1"), 0xffff4500);
	conf_ini->WriteInteger(_T("SpecLoc"), _T("2sl_type"), 100002);
	conf_ini->WriteInteger(_T("SpecLoc"), _T("2sl_sn"), 0x00);
	conf_ini->WriteString(_T("SpecLoc"), _T("2sl_txt"), _T("F00"));
	conf_ini->WriteString(_T("SpecLoc"), _T("2sl_range"), _T("12,11,12,11"));
	conf_ini->WriteInteger(_T("SpecLoc"), _T("2sl_storey"), 0x00);
	conf_ini->WriteFloat(_T("SpecLoc"), _T("2sl_color0"), 0xff2e8b57);
	conf_ini->WriteFloat(_T("SpecLoc"), _T("2sl_color1"), 0xffff4500);
	conf_ini->WriteInteger(_T("SpecLoc"), _T("3sl_type"), 100002);
	conf_ini->WriteInteger(_T("SpecLoc"), _T("3sl_sn"), 0x02);
	conf_ini->WriteString(_T("SpecLoc"), _T("3sl_txt"), _T("F02"));
	conf_ini->WriteString(_T("SpecLoc"), _T("3sl_range"), _T("12,12,12,12"));
	conf_ini->WriteInteger(_T("SpecLoc"), _T("3sl_storey"), 0x00);
	conf_ini->WriteFloat(_T("SpecLoc"), _T("3sl_color0"), 0xff2e8b57);
	conf_ini->WriteFloat(_T("SpecLoc"), _T("3sl_color1"), 0xffff4500);
	conf_ini->WriteInteger(_T("SpecLoc"), _T("4sl_type"), 100004);
	conf_ini->WriteInteger(_T("SpecLoc"), _T("4sl_sn"), 0x01);
	conf_ini->WriteString(_T("SpecLoc"), _T("4sl_txt"), _T("L01"));
	conf_ini->WriteString(_T("SpecLoc"), _T("4sl_range"), _T("25,14,27,14"));
	conf_ini->WriteInteger(_T("SpecLoc"), _T("4sl_storey"), 0x00);
	conf_ini->WriteFloat(_T("SpecLoc"), _T("4sl_color0"), 0xff7b68ee);
	conf_ini->WriteFloat(_T("SpecLoc"), _T("4sl_color1"), 0xff1e90ff);
	conf_ini->WriteInteger(_T("SpecLoc"), _T("4hittest"), 0x1);
	conf_ini->WriteInteger(_T("SpecLoc"), _T("5sl_type"), 100000);
	conf_ini->WriteInteger(_T("SpecLoc"), _T("5sl_sn"), 0x00);
	conf_ini->WriteString(_T("SpecLoc"), _T("5sl_txt"), _T("B00"));
	conf_ini->WriteString(_T("SpecLoc"), _T("5sl_range"), _T("26,13,26,13"));
	conf_ini->WriteInteger(_T("SpecLoc"), _T("5sl_storey"), 0x00);
	conf_ini->WriteFloat(_T("SpecLoc"), _T("5sl_color0"), 0x0);
	conf_ini->WriteFloat(_T("SpecLoc"), _T("5sl_color1"), 0x0);
	conf_ini->WriteInteger(_T("SpecLoc"), _T("6sl_type"), 100006);
	conf_ini->WriteInteger(_T("SpecLoc"), _T("6sl_sn"), 0x01);
	conf_ini->WriteString(_T("SpecLoc"), _T("6sl_txt"), _T("P01"));
	conf_ini->WriteString(_T("SpecLoc"), _T("6sl_range"), _T("26,12,26,12"));
	conf_ini->WriteInteger(_T("SpecLoc"), _T("6sl_storey"), 0x00);
	conf_ini->WriteFloat(_T("SpecLoc"), _T("6sl_color0"), 0x0);
	conf_ini->WriteFloat(_T("SpecLoc"), _T("6sl_color1"), 0x0);
	conf_ini->WriteInteger(_T("SpecLoc"), _T("7sl_type"), 100006);
	conf_ini->WriteInteger(_T("SpecLoc"), _T("7sl_sn"), 0x00);
	conf_ini->WriteString(_T("SpecLoc"), _T("7sl_txt"), _T("P00"));
	conf_ini->WriteString(_T("SpecLoc"), _T("7sl_range"), _T("26,11,26,11"));
	conf_ini->WriteInteger(_T("SpecLoc"), _T("7sl_storey"), 0x00);
	conf_ini->WriteFloat(_T("SpecLoc"), _T("7sl_color0"), 0x0);
	conf_ini->WriteFloat(_T("SpecLoc"), _T("7sl_color1"), 0x0);
	conf_ini->WriteInteger(_T("SpecLoc"), _T("8sl_type"), 100005);
	conf_ini->WriteInteger(_T("SpecLoc"), _T("8sl_sn"), 0x00);
	conf_ini->WriteString(_T("SpecLoc"), _T("8sl_txt"), _T("O00"));
	conf_ini->WriteString(_T("SpecLoc"), _T("8sl_range"), _T("29,11,29,11"));
	conf_ini->WriteInteger(_T("SpecLoc"), _T("8sl_storey"), 0x00);
	conf_ini->WriteFloat(_T("SpecLoc"), _T("8sl_color0"), 0x0);
	conf_ini->WriteFloat(_T("SpecLoc"), _T("8sl_color1"), 0x0);
	conf_ini->WriteInteger(_T("SpecLoc"), _T("9sl_type"), 100005);
	conf_ini->WriteInteger(_T("SpecLoc"), _T("9sl_sn"), 0x01);
	conf_ini->WriteString(_T("SpecLoc"), _T("9sl_txt"), _T("O01"));
	conf_ini->WriteString(_T("SpecLoc"), _T("9sl_range"), _T("29,12,29,12"));
	conf_ini->WriteInteger(_T("SpecLoc"), _T("9sl_storey"), 0x00);
	conf_ini->WriteFloat(_T("SpecLoc"), _T("9sl_color0"), 0x0);
	conf_ini->WriteFloat(_T("SpecLoc"), _T("9sl_color1"), 0x0);
	// sl_type°™°™the value in speclocflag enumeration
	conf_ini->WriteInteger(_T("SpecLoc"), _T("10sl_type"), 100003);
	// the sn of the specloc in the sl_type
	conf_ini->WriteInteger(_T("SpecLoc"), _T("10sl_sn"), 0x00);
	// the name of the specloc
	conf_ini->WriteString(_T("SpecLoc"), _T("10sl_txt"), _T("G00"));
	// the range of the specloc
	conf_ini->WriteString(_T("SpecLoc"), _T("10sl_range"), _T("35,8,35,8"));
	// the storey of the specloc
	conf_ini->WriteInteger(_T("SpecLoc"), _T("10sl_storey"), 0x00);
	// color
	conf_ini->WriteFloat(_T("SpecLoc"), _T("10sl_color0"), 0xffffff00);
	conf_ini->WriteFloat(_T("SpecLoc"), _T("10sl_color1"), 0xffff4500);
	// can be clicked
	conf_ini->WriteInteger(_T("SpecLoc"), _T("10hittest"), 0x1);
	conf_ini->WriteInteger(_T("SpecLoc"), _T("11sl_type"), 100009);
	conf_ini->WriteInteger(_T("SpecLoc"), _T("11sl_sn"), 0x00);
	conf_ini->WriteString(_T("SpecLoc"), _T("11sl_txt"), _T("W00"));
	conf_ini->WriteString(_T("SpecLoc"), _T("11sl_range"), _T("20,5,23,9"));
	conf_ini->WriteInteger(_T("SpecLoc"), _T("11sl_storey"), 0x00);
	// conf_ini->WriteFloat(_T("SpecLoc"), _T("11sl_color0"), 0xffffffff);
	conf_ini->WriteFloat(_T("SpecLoc"), _T("11sl_color0"), 0xff708090);
	conf_ini->WriteFloat(_T("SpecLoc"), _T("11sl_color1"), 0xff708090);
	// conf_ini->WriteInteger(_T("SpecLoc"), _T("11hittest"), 0x1);
	// conf_ini->WriteInteger(_T("SpecLoc"), _T("12sl_type"), 100007);
	// conf_ini->WriteInteger(_T("SpecLoc"), _T("12sl_sn"), 0x01);
	// conf_ini->WriteString(_T("SpecLoc"), _T("12sl_txt"), _T("Q01"));
	// conf_ini->WriteString(_T("SpecLoc"), _T("12sl_range"), _T("21,3,21,3"));
	// conf_ini->WriteInteger(_T("SpecLoc"), _T("12sl_storey"), 0x00);
	// conf_ini->WriteFloat(_T("SpecLoc"), _T("12sl_color0"), 0xffffffff);
	// conf_ini->WriteFloat(_T("SpecLoc"), _T("12sl_color1"), 0xff708090);
	// conf_ini->WriteInteger(_T("SpecLoc"), _T("13sl_type"), 100007);
	// conf_ini->WriteInteger(_T("SpecLoc"), _T("13sl_sn"), 0x02);
	// conf_ini->WriteString(_T("SpecLoc"), _T("13sl_txt"), _T("Q02"));
	// conf_ini->WriteString(_T("SpecLoc"), _T("13sl_range"), _T("22,3,22,3"));
	// conf_ini->WriteInteger(_T("SpecLoc"), _T("13sl_storey"), 0x00);
	// conf_ini->WriteFloat(_T("SpecLoc"), _T("13sl_color0"), 0xffffffff);
	// conf_ini->WriteFloat(_T("SpecLoc"), _T("13sl_color1"), 0xff708090);
	// conf_ini->WriteInteger(_T("SpecLoc"), _T("14sl_type"), 100007);
	// conf_ini->WriteInteger(_T("SpecLoc"), _T("14sl_sn"), 0x03);
	// conf_ini->WriteString(_T("SpecLoc"), _T("14sl_txt"), _T("Q03"));
	// conf_ini->WriteString(_T("SpecLoc"), _T("14sl_range"), _T("23,3,23,3"));
	// conf_ini->WriteInteger(_T("SpecLoc"), _T("14sl_storey"), 0x00);
	// conf_ini->WriteFloat(_T("SpecLoc"), _T("14sl_color0"), 0xffffffff);
	// conf_ini->WriteFloat(_T("SpecLoc"), _T("14sl_color1"), 0xff708090);
	conf_ini->WriteInteger(_T("SpecLoc"), _T("12sl_type"), 100004);
	conf_ini->WriteInteger(_T("SpecLoc"), _T("12sl_sn"), 0x01);
	conf_ini->WriteString(_T("SpecLoc"), _T("12sl_txt"), _T("L01"));
	conf_ini->WriteString(_T("SpecLoc"), _T("12sl_range"), _T("22,16,24,16"));
	conf_ini->WriteInteger(_T("SpecLoc"), _T("12sl_storey"), 0x01);
	conf_ini->WriteFloat(_T("SpecLoc"), _T("12sl_color0"), 0xff7b68ee);
	conf_ini->WriteFloat(_T("SpecLoc"), _T("12sl_color1"), 0xff1e90ff);
	conf_ini->WriteInteger(_T("SpecLoc"), _T("12hittest"), 0x1);
	conf_ini->WriteInteger(_T("SpecLoc"), _T("13sl_type"), 100000);
	conf_ini->WriteInteger(_T("SpecLoc"), _T("13sl_sn"), 0x02);
	conf_ini->WriteString(_T("SpecLoc"), _T("13sl_txt"), _T("B02"));
	conf_ini->WriteString(_T("SpecLoc"), _T("13sl_range"), _T("23,17,23,17"));
	conf_ini->WriteInteger(_T("SpecLoc"), _T("13sl_storey"), 0x01);
	conf_ini->WriteFloat(_T("SpecLoc"), _T("13sl_color0"), 0x0);
	conf_ini->WriteFloat(_T("SpecLoc"), _T("13sl_color1"), 0x0);
	conf_ini->WriteInteger(_T("SpecLoc"), _T("14sl_type"), 100006);
	conf_ini->WriteInteger(_T("SpecLoc"), _T("14sl_sn"), 0x02);
	conf_ini->WriteString(_T("SpecLoc"), _T("14sl_txt"), _T("P02"));
	conf_ini->WriteString(_T("SpecLoc"), _T("14sl_range"), _T("23,18,23,18"));
	conf_ini->WriteInteger(_T("SpecLoc"), _T("14sl_storey"), 0x01);
	conf_ini->WriteFloat(_T("SpecLoc"), _T("14sl_color0"), 0x0);
	conf_ini->WriteFloat(_T("SpecLoc"), _T("14sl_color1"), 0x0);
	conf_ini->WriteInteger(_T("SpecLoc"), _T("15sl_type"), 100003);
	conf_ini->WriteInteger(_T("SpecLoc"), _T("15sl_sn"), 0x02);
	conf_ini->WriteString(_T("SpecLoc"), _T("15sl_txt"), _T("G02"));
	conf_ini->WriteString(_T("SpecLoc"), _T("15sl_range"), _T("27,16,27,16"));
	conf_ini->WriteInteger(_T("SpecLoc"), _T("15sl_storey"), 0x01);
	conf_ini->WriteFloat(_T("SpecLoc"), _T("15sl_color0"), 0xffffff00);
	conf_ini->WriteFloat(_T("SpecLoc"), _T("15sl_color1"), 0xffff4500);
	conf_ini->WriteInteger(_T("SpecLoc"), _T("15hittest"), 0x1);
	conf_ini->WriteInteger(_T("SpecLoc"), _T("16sl_type"), 100005);
	conf_ini->WriteInteger(_T("SpecLoc"), _T("16sl_sn"), 0x02);
	conf_ini->WriteString(_T("SpecLoc"), _T("16sl_txt"), _T("O02"));
	conf_ini->WriteString(_T("SpecLoc"), _T("16sl_range"), _T("31,18,31,18"));
	conf_ini->WriteInteger(_T("SpecLoc"), _T("16sl_storey"), 0x01);
	conf_ini->WriteFloat(_T("SpecLoc"), _T("16sl_color0"), 0x0);
	conf_ini->WriteFloat(_T("SpecLoc"), _T("16sl_color1"), 0x0);
	// conf_ini->WriteInteger(_T("SpecLoc"), _T("17sl_type"), 100007);
	// conf_ini->WriteInteger(_T("SpecLoc"), _T("17sl_sn"), 0x01);
	// conf_ini->WriteString(_T("SpecLoc"), _T("17sl_txt"), _T("Q01"));
	// conf_ini->WriteString(_T("SpecLoc"), _T("17sl_range"), _T("17,3,17,3"));
	// conf_ini->WriteInteger(_T("SpecLoc"), _T("17sl_storey"), 0x00);
	// conf_ini->WriteFloat(_T("SpecLoc"), _T("17sl_color0"), 0xffffffff);
	// conf_ini->WriteFloat(_T("SpecLoc"), _T("17sl_color1"), 0xff708090);
	// conf_ini->WriteInteger(_T("SpecLoc"), _T("18sl_type"), 100007);
	// conf_ini->WriteInteger(_T("SpecLoc"), _T("18sl_sn"), 0x02);
	// conf_ini->WriteString(_T("SpecLoc"), _T("18sl_txt"), _T("Q02"));
	// conf_ini->WriteString(_T("SpecLoc"), _T("18sl_range"), _T("18,3,18,3"));
	// conf_ini->WriteInteger(_T("SpecLoc"), _T("18sl_storey"), 0x00);
	// conf_ini->WriteFloat(_T("SpecLoc"), _T("18sl_color0"), 0xffffffff);
	// conf_ini->WriteFloat(_T("SpecLoc"), _T("18sl_color1"), 0xff708090);
	// conf_ini->WriteInteger(_T("SpecLoc"), _T("19sl_type"), 100007);
	// conf_ini->WriteInteger(_T("SpecLoc"), _T("19sl_sn"), 0x03);
	// conf_ini->WriteString(_T("SpecLoc"), _T("19sl_txt"), _T("Q03"));
	// conf_ini->WriteString(_T("SpecLoc"), _T("19sl_range"), _T("19,3,19,3"));
	// conf_ini->WriteInteger(_T("SpecLoc"), _T("19sl_storey"), 0x00);
	// conf_ini->WriteFloat(_T("SpecLoc"), _T("19sl_color0"), 0xffffffff);
	// conf_ini->WriteFloat(_T("SpecLoc"), _T("19sl_color1"), 0xff708090);
	// conf_ini->WriteInteger(_T("SpecLoc"), _T("20sl_type"), 100007);
	// conf_ini->WriteInteger(_T("SpecLoc"), _T("20sl_sn"), 0x00);
	// conf_ini->WriteString(_T("SpecLoc"), _T("20sl_txt"), _T("Q00"));
	// conf_ini->WriteString(_T("SpecLoc"), _T("20sl_range"), _T("16,3,16,3"));
	// conf_ini->WriteInteger(_T("SpecLoc"), _T("20sl_storey"), 0x00);
	// conf_ini->WriteFloat(_T("SpecLoc"), _T("20sl_color0"), 0xffffffff);
	// conf_ini->WriteFloat(_T("SpecLoc"), _T("20sl_color1"), 0xff708090);
	conf_ini->WriteInteger(_T("SpecLoc"), _T("17sl_type"), 100001);
	conf_ini->WriteInteger(_T("SpecLoc"), _T("17sl_sn"), 0x01);
	conf_ini->WriteString(_T("SpecLoc"), _T("17sl_txt"), _T("D01"));
	conf_ini->WriteString(_T("SpecLoc"), _T("17sl_range"), _T("2,9,2,9"));
	conf_ini->WriteInteger(_T("SpecLoc"), _T("17sl_storey"), 0x00);
	conf_ini->WriteFloat(_T("SpecLoc"), _T("17sl_color0"), 0xfff0fff0);
	conf_ini->WriteFloat(_T("SpecLoc"), _T("17sl_color1"), 0xff696969);
	conf_ini->WriteInteger(_T("SpecLoc"), _T("17hittest"), 0x1);
	conf_ini->WriteInteger(_T("CoordS"), _T("NumsOrgs"), 0x08);
	conf_ini->WriteInteger(_T("CoordS"), _T("0x"), 16);
	conf_ini->WriteInteger(_T("CoordS"), _T("0y"), 10);
	conf_ini->WriteInteger(_T("CoordS"), _T("0axis"), 0); // x÷·
	conf_ini->WriteInteger(_T("CoordS"), _T("0base"), 3); // length
    conf_ini->WriteInteger(_T("CoordS"), _T("0area"), 2);
	conf_ini->WriteInteger(_T("CoordS"), _T("1x"), 15);
	conf_ini->WriteInteger(_T("CoordS"), _T("1y"), 5);
	conf_ini->WriteInteger(_T("CoordS"), _T("1axis"), 1); // y÷·
	conf_ini->WriteInteger(_T("CoordS"), _T("1base"), 0);
	conf_ini->WriteInteger(_T("CoordS"), _T("1len"), 5);
	conf_ini->WriteInteger(_T("CoordS"), _T("2x"), 16);
	conf_ini->WriteInteger(_T("CoordS"), _T("2y"), 13);
	conf_ini->WriteInteger(_T("CoordS"), _T("2axis"), 0);
	conf_ini->WriteInteger(_T("CoordS"), _T("2base"), 19);
    conf_ini->WriteInteger(_T("CoordS"), _T("2area"), 0);
	conf_ini->WriteInteger(_T("CoordS"), _T("3x"), 15);
	conf_ini->WriteInteger(_T("CoordS"), _T("3y"), 14);
	conf_ini->WriteInteger(_T("CoordS"), _T("3axis"), 1);
	conf_ini->WriteInteger(_T("CoordS"), _T("3base"), 13);
	conf_ini->WriteInteger(_T("CoordS"), _T("4x"), 24);
	conf_ini->WriteInteger(_T("CoordS"), _T("4y"), 10);
	conf_ini->WriteInteger(_T("CoordS"), _T("4axis"), 0);
	conf_ini->WriteInteger(_T("CoordS"), _T("4base"), 9);
	conf_ini->WriteInteger(_T("CoordS"), _T("4area"), 1);
	conf_ini->WriteInteger(_T("CoordS"), _T("5x"), 34);
	conf_ini->WriteInteger(_T("CoordS"), _T("5y"), 5);
	conf_ini->WriteInteger(_T("CoordS"), _T("5axis"), 1);
	conf_ini->WriteInteger(_T("CoordS"), _T("5base"), 0);
	conf_ini->WriteInteger(_T("CoordS"), _T("5len"), 5);
	conf_ini->WriteInteger(_T("CoordS"), _T("6x"), 11);
	conf_ini->WriteInteger(_T("CoordS"), _T("6y"), 19);
	conf_ini->WriteInteger(_T("CoordS"), _T("6axis"), 0);
	conf_ini->WriteInteger(_T("CoordS"), _T("6base"), 24);
	conf_ini->WriteInteger(_T("CoordS"), _T("6storey"), 1); // storey 2
    conf_ini->WriteInteger(_T("CoordS"), _T("6area"), 3);
	conf_ini->WriteInteger(_T("CoordS"), _T("7x"), 10);
	conf_ini->WriteInteger(_T("CoordS"), _T("7y"), 20);
	conf_ini->WriteInteger(_T("CoordS"), _T("7axis"), 1);
	conf_ini->WriteInteger(_T("CoordS"), _T("7base"), 7);
	conf_ini->WriteInteger(_T("CoordS"), _T("7storey"), 1);

	conf_ini->WriteInteger(_T("DotPopup"), _T("ParamCount"), 5);
	conf_ini->WriteString(_T("DotPopup"), _T("DPP0"), _T("πÊ∏Ò"));
	conf_ini->WriteString(_T("DotPopup"), _T("DPP1"), _T("≈˙∫≈(π˙ƒ⁄)"));
	conf_ini->WriteString(_T("DotPopup"), _T("DPP2"), _T("≈˙∫≈(π˙Õ‚)"));
	conf_ini->WriteString(_T("DotPopup"), _T("DPP3"), _T("◊‹∂‚ ˝"));
	conf_ini->WriteString(_T("DotPopup"), _T("DPP4"), _T("∂‚–Ú∫≈"));

	conf_ini->WriteInteger(_T("AGVPrms"), _T("ParamCount"), 7);
	conf_ini->WriteString(_T("AGVPrms"), _T("AGVp0"), _T("±‡∫≈"));
	conf_ini->WriteString(_T("AGVPrms"), _T("AGVp1"), _T("√˚◊÷"));
	conf_ini->WriteString(_T("AGVPrms"), _T("AGVp2"), _T("IPµÿ÷∑"));
	conf_ini->WriteString(_T("AGVPrms"), _T("AGVp3"), _T("≤÷ø‚"));
	conf_ini->WriteString(_T("AGVPrms"), _T("AGVp4"), _T("¬•≤„"));
	conf_ini->WriteString(_T("AGVPrms"), _T("AGVp5"), _T("µÁ¡ø"));
	conf_ini->WriteString(_T("AGVPrms"), _T("AGVp6"), _T("µ±«∞»ŒŒÒ±‡∫≈"));
	// conf_ini->WriteFloat(_T("AGVPrms"), _T("AGVnumber"), 16);
	conf_ini->WriteFloat(_T("AGVPrms"), _T("AGVnumber"), 9);
	conf_ini->WriteFloat(_T("AGVPrms"), _T("agvColor0"), 0xff8a2be2);
	conf_ini->WriteFloat(_T("AGVPrms"), _T("agvColor1"), 0xff5f9ea0);
	conf_ini->WriteFloat(_T("AGVPrms"), _T("agvColor2"), 0xff6495ed);
	conf_ini->WriteFloat(_T("AGVPrms"), _T("agvColor3"), 0xff008000);
	conf_ini->WriteFloat(_T("AGVPrms"), _T("agvColor4"), 0xffd2691e);
	conf_ini->WriteFloat(_T("AGVPrms"), _T("agvColor5"), 0xffff7f50);
	conf_ini->WriteFloat(_T("AGVPrms"), _T("agvColor6"), 0xff8fbc8f);
	conf_ini->WriteFloat(_T("AGVPrms"), _T("agvColor7"), 0xffdc143c);
	conf_ini->WriteFloat(_T("AGVPrms"), _T("agvColor8"), 0xffffd700);
	// conf_ini->WriteFloat(_T("AGVPrms"), _T("agvColor9"), 0xffdaa520);
	// conf_ini->WriteFloat(_T("AGVPrms"), _T("agvColor10"), 0xffff69b4);
	// conf_ini->WriteFloat(_T("AGVPrms"), _T("agvColor11"), 0xff778899);
	// conf_ini->WriteFloat(_T("AGVPrms"), _T("agvColor12"), 0xffba55d3);
	// conf_ini->WriteFloat(_T("AGVPrms"), _T("agvColor13"), 0xff7b68ee);
	// conf_ini->WriteFloat(_T("AGVPrms"), _T("agvColor14"), 0xffc71585);
	// conf_ini->WriteFloat(_T("AGVPrms"), _T("agvColor15"), 0xff9acd32);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Antiquewhite"), 0xfffaebd7);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Aqua"), 0xff00ffff);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Aquamarine"), 0xff7fffd4);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Beige"), 0xfff5f5dc);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Bisque"), 0xffffe4c4);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Blanchedalmond"), 0xffffebcd);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Blue"), 0xff0000ff);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Brown"), 0xffa52a2a);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Burlywood"), 0xffdeb887);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Chartreuse"), 0xff7fff00);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Chocolate"), 0xffd2691e);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Coral"), 0xffff7f50);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Cornsilk"), 0xfffff8dc);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Crimson"), 0xffdc143c);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Cyan"), 0xff00ffff);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Darkblue"), 0xff00008b);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Darkcyan"), 0xff008b8b);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Darkgoldenrod"), 0xffb8860b);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Darkgray"), 0xffa9a9a9);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Darkgreen"), 0xff006400);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Darkgrey"), 0xffa9a9a9);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Darkkhaki"), 0xffbdb76b);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Darkmagenta"), 0xff8b008b);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Darkolivegreen"), 0xff556b2f);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Darkorange"), 0xffff8c00);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Darkorchid"), 0xff9932cc);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Darkred"), 0xff8b0000);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Darksalmon"), 0xffe9967a);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Darkseagreen"), 0xff8fbc8f);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Darkslateblue"), 0xff483d8b);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Darkslategray"), 0xff2f4f4f);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Darkslategrey"), 0xff2f4f4f);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Darkturquoise"), 0xff00ced1);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Darkviolet"), 0xff9400d3);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Deeppink"), 0xffff1493);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Deepskyblue"), 0xff00bfff);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Dimgray"), 0xff696969);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Dimgrey"), 0xff696969);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Firebrick"), 0xffb22222);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Forestgreen"), 0xff228b22);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Fuchsia"), 0xffff00ff);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Gainsboro"), 0xffdcdcdc);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Gold"), 0xffffd700);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Goldenrod"), 0xffdaa520);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Gray"), 0xff808080);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Greenyellow"), 0xffadff2f);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Grey"), 0xff808080);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Honeydew"), 0xfff0fff0);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Hotpink"), 0xffff69b4);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Indianred"), 0xffcd5c5c);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Indigo"), 0xff4b0082);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Khaki"), 0xfff0e68c);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Lavender"), 0xffe6e6fa);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Lavenderblush"), 0xfffff0f5);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Lawngreen"), 0xff7cfc00);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Lemonchiffon"), 0xfffffacd);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Lightblue"), 0xffadd8e6);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Lightcoral"), 0xfff08080);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Lightcyan"), 0xffe0ffff);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Lightgoldenrodyellow"), 0xfffafad2);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Lightgray"), 0xffd3d3d3);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Lightgreen"), 0xff90ee90);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Lightgrey"), 0xffd3d3d3);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Lightpink"), 0xffffb6c1);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Lightsalmon"), 0xffffa07a);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Lightseagreen"), 0xff20b2aa);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Lightskyblue"), 0xff87cefa);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Yellowgreen"), 0xff9acd32);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Yellow"), 0xffffff00);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Wheat"), 0xfff5deb3);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Violet"), 0xffee82ee);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Turquoise"), 0xff40e0d0);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Tomato"), 0xffff6347);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Thistle"), 0xffd8bfd8);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Teal"), 0xff008080);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Tan"), 0xffd2b48c);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Steelblue"), 0xff4682b4);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Springgreen"), 0xff00ff7f);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Slategrey"), 0xff708090);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Slategray"), 0xff708090);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Slateblue"), 0xff6a5acd);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Slateblue"), 0xff6a5acd);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Skyblue"), 0xff87ceeb);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Silver"), 0xffc0c0c0);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Sienna"), 0xffa0522d);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Seagreen"), 0xff2e8b57);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Sandybrown"), 0xfff4a460);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Salmon"), 0xfffa8072);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Saddlebrown"), 0xff8b4513);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Royalblue"), 0xff4169e1);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Rosybrown"), 0xffbc8f8f);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Red"), 0xffff0000);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Purple"), 0xff800080);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Powderblue"), 0xffb0e0e6);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Plum"), 0xffdda0dd);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Pink"), 0xffffc0cb);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Peru"), 0xffcd853f);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Peachpuff"), 0xffffdab9);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Papayawhip"), 0xffffefd5);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Palevioletred"), 0xffdb7093);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Paleturquoise"), 0xffafeeee);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Palegreen"), 0xff98fb98);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Palegoldenrod"), 0xffeee8aa);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Orchid"), 0xffda70d6);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Orangered"), 0xffff4500);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Orange"), 0xffffa500);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Olivedrab"), 0xff6b8e23);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Olive"), 0xff808000);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Oldlace"), 0xfffdf5e6);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Navy"), 0xff000080);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Navajowhite"), 0xffffdead);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Moccasin"), 0xffffe4b5);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Mistyrose"), 0xffffe4e1);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Midnightblue"), 0xff191970);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Mediumvioletred"), 0xffc71585);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Mediumturquoise"), 0xff48d1cc);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Mediumspringgreen"), 0xff00fa9a);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Mediumslateblue"), 0xff7b68ee);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Mediumseagreen"), 0xff3cb371);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Mediumpurple"), 0xff9370db);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Mediumorchid"), 0xffba55d3);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Mediumblue"), 0xff0000cd);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Mediumaquamarine"), 0xff66cdaa);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Maroon"), 0xff800000);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Magenta"), 0xffff00ff);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Linen"), 0xfffaf0e6);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Lime"), 0xff00ff00);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Cream"), 0xfff0fbff);
	conf_ini->WriteFloat(_T("AvailColor"), _T("LegacySkyBlue"), 0xfff0caa6);
	conf_ini->WriteFloat(_T("AvailColor"), _T("MoneyGreen"), 0xffc0dcc0);
	conf_ini->WriteFloat(_T("AvailColor"), _T("DkGray"), 0xff808080);
	conf_ini->WriteFloat(_T("AvailColor"), _T("MedGray"), 0xffa0a0a0);
	conf_ini->WriteFloat(_T("AvailColor"), _T("LtGray"), 0xffc0c0c0);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Lightyellow"), 0xffffffe0);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Lightsteelblue"), 0xffb0c4de);
	conf_ini->WriteFloat(_T("AvailColor"), _T("Dodgerblue"), 0xff1e90ff);
	delete conf_ini;
	conf_ini = 0;
}

// ---------------------------------------------------------------------------
void __fastcall TForm1::build_matrix() {
	int i, j, k, l;
	pack_loc_base pk_lb;
	pack_artery pk_art;
	std::vector<pack_loc_base>::iterator a_storey;
	std::vector<pack_artery>::iterator a_artery;
	i = cfp.storeys.size();
	// storey_matrices.set_length(i);
	m.set_length(i);
	j = 0;
	for (a_storey = cfp.storeys.begin(); a_storey != cfp.storeys.end(); ++a_storey) {
		pk_lb = *a_storey;
		m[j].set_length(pk_lb.lb[y]);
		for (k = 0; k < pk_lb.lb[y]; ++k) {
			m[j][k].set_length(pk_lb.lb[x]);
			for (l = 0; l < pk_lb.lb[x]; ++l) {
				m[j][k][l] = 1;
			}
		}
		++j;
	}
	for (a_artery = cfp.arteries.begin(); a_artery != cfp.arteries.end(); ++a_artery) {
		pk_art = *a_artery;
		for (i = pk_art.a[bx]; i <= pk_art.a[ex]; ++i)
			for (j = pk_art.a[by]; j <= pk_art.a[ey]; ++j)
				m[pk_art.a[storey_artery]][j][i] = 0;
	}
}

// ---------------------------------------------------------------------------
void __fastcall TForm1::add_storey_buttons() {
	int n = m.Length + 1;
	int i;
	TCornerButton *b = 0;
	buttons_storey.set_length(n);
	hittest_b_storey.set_length(4 * n);

	for (i = 0; i < hittest_b_storey.Length; ++i) {
		hittest_b_storey[i] = 0;
	}

	for (i = n - 1; i > -1; --i) {
		b = (TCornerButton*)CornerButton6->Clone(this);
		b->Text = String(_T("ø‚≤„(S")) + IntToStr((int)i) + String(_T(')'));
		b->Tag = i * 4;
		b->OnClick = storey_click;
		ToolBar1->AddObject(b);
		buttons_storey[i] = b;
	}

	CornerButton9->Tag = 1;
	CornerButton6->Visible = false;
}
// ---------------------------------------------------------------------------

void __fastcall TForm1::storey_click(TObject * Sender) {
	TCornerButton *b = (TCornerButton*)Sender;
	size_t j = b->Tag / 4;
	for (size_t i = 0; i < m.Length + 1; ++i)
		Frame1->layouts[i]->Visible = (i != j) ? 0 : 1;
	Frame1->crt_layout = Frame1->layouts[j];
	CornerButton9->Text = Format(_T("S%u—°«¯"), ARRAYOFCONST(((j))));
	b_schdl->Text = Format(_T("S%uµ˜∂»"), ARRAYOFCONST(((j))));
	b_schdl_confirm->Text = Format(_T("S%uµ˜∂»»∑»œ"), ARRAYOFCONST(((j))));
	btn_fsm(j);
}

// ---------------------------------------------------------------------------
void __fastcall TForm1::FloatAnimation2Finish(TObject * Sender) {
	TFloatAnimation * a = (TFloatAnimation*)Sender;
	a->Enabled = false;
}
// ---------------------------------------------------------------------------

void __fastcall TForm1::add_color_buttons() {
	// TColorButton* b = 0;
	// TText * t = 0;
	// TFloatAnimation *fa = 0;
	// TImage * img = 0;
	// agv_buttons.set_length(agv_colors.Length);
	// fas.set_length(agv_colors.Length);
	// for (size_t i = 0; i < agv_buttons.Length; ++i) {
	// b = (TColorButton*)ColorButton1->Clone(0);
	// b->Color = (unsigned)agv_colors[i];
	// for (size_t j = 0; j < b->Children->Count; ++j) {
	// if (__classid(TText) == b->Children->operator[](j)->ClassType()) {
	// t = (TText *)b->Children->operator[](j);
	// t->Text = String(_T("–°≥µ")) + IntToStr((int)i + 1);
	// }
	// if (__classid(TImage) == b->Children->operator[](j)->ClassType()) {
	// img = (TImage *)b->Children->operator[](j);
	// for (size_t k = 0; k < img->Children->Count; ++k) {
	// if (__classid(TFloatAnimation) == img->Children->operator[](k)->ClassType()) {
	// fa = (TFloatAnimation *)(img->Children->operator[](k));
	// fa->StartFromCurrent = true;
	// fa->Duration = 0.15;
	// fa->Tag = 0;
	// fa->Enabled = true;
	// fa->OnFinish = FloatAnimation2Finish;
	// fas[i] = fa;
	// }
	// }
	// }
	// }
	//
	// b->OnClick = my_button_click;
	// b->Tag = i;
	// GridLayout1->AddObject(b);
	// agv_buttons[i] = b;
	// }
	// ColorButton1->Visible = false;
}

// ---------------------------------------------------------------------------
void __fastcall TForm1::FormDestroy(TObject * Sender) {
	CloseHandle(event_nextagvtask);
	shutdown(mysock, SD_BOTH);
	closesocket(mysock);
	drop_winsock();
	Frame1->DisposeOf();
	b_schdl->DisposeOf();
	b_schdl_confirm->DisposeOf();
	delete[](cfp.sl_ary);
	for (size_t i = 0; i < FLAGSPECPACKSEGSother; ++i) {
		delete[]records_sp.arys[i];
	}
}

// ---------------------------------------------------------------------------

void __fastcall TForm1::Image1Click(TObject * Sender) {
	Frame1->layout_zoomout(Frame1->crt_layout);
}
// ---------------------------------------------------------------------------

void __fastcall TForm1::Image2Click(TObject * Sender) {
	Frame1->layout_zoomin(Frame1->crt_layout);
}

// ---------------------------------------------------------------------------
void __fastcall TForm1::FormKeyUp(TObject * Sender, WORD & Key, System::WideChar & KeyChar,
	TShiftState Shift) {
	if (KeyChar == _T('='))
		Image1Click(Sender);
	else if (KeyChar == _T('-'))
		Image2Click(Sender);
}

// ---------------------------------------------------------------------------
void __fastcall TForm1::slct_addition() {
	b_schdl = (TCornerButton*)CornerButton9->Clone(0);
	b_schdl->Text = _T("S0µ˜∂»");
	b_schdl->Visible = 0;
	b_schdl->OnClick = schdl_click;
	b_schdl->Parent = ToolBar1;

	b_schdl_confirm = (TCornerButton*)CornerButton9->Clone(0);
	b_schdl_confirm->Text = _T("S0µ˜∂»»∑»œ");
	b_schdl_confirm->Visible = 0;
	b_schdl_confirm->OnClick = confirm_schdl_click;
	b_schdl_confirm->Parent = ToolBar1;

	b_schdl->Tag = 2;
	b_schdl_confirm->Tag = 3;
	ToolBar1->AddObject(b_schdl_confirm);
	ToolBar1->AddObject(b_schdl);
}

// ---------------------------------------------------------------------------
void __fastcall TForm1::confirm_schdl_click(TObject * Sender) {
	//
}
// ---------------------------------------------------------------------------

void __fastcall TForm1::CornerButton9Click(TObject * Sender) {
	TMySelection *s = 0;
	TCornerButton * c = 0;
	c = (TCornerButton*)Sender;
	hittest_b_storey[c->Tag] = !hittest_b_storey[c->Tag];
	if (hittest_b_storey[c->Tag]) {
		for (size_t i = 0; i < Frame1->crt_layout->Children->Count; ++i) {
			if (__classid(TMySelection) == Frame1->crt_layout->Children->operator[](i)->ClassType())
			{
				s = (TMySelection*)Frame1->crt_layout->Children->operator[](i);
				if (1 == s->Tag)
					s->Visible = 1;
			}
		}

		b_schdl->Visible = 1;
		Frame1->add_slct_title();
	}
	else {
		for (size_t i = 0; i < Frame1->crt_layout->Children->Count; ++i) {
			if (__classid(TMySelection) == Frame1->crt_layout->Children->operator[](i)->ClassType())
			{
				s = (TMySelection*)Frame1->crt_layout->Children->operator[](i);
				s->Visible = 0;
			}
		}
		b_schdl->Visible = 0;
		b_schdl_confirm->Visible = 0;
	}
}

// ---------------------------------------------------------------------------
void __fastcall TForm1::btn_fsm(size_t tag) {
	TMySelection *s = 0;
	tag *= 4;
	CornerButton9->Tag = tag + 1;
	b_schdl->Tag = tag + 2;
	b_schdl_confirm->Tag = tag + 3;

	if (hittest_b_storey[CornerButton9->Tag]) {
		b_schdl->Visible = 1;
		for (size_t i = 0; i < Frame1->crt_layout->Children->Count; ++i) {
			if (__classid(TMySelection) == Frame1->crt_layout->Children->operator[](i)->ClassType())
			{
				s = (TMySelection*)Frame1->crt_layout->Children->operator[](i);
				if (1 == s->Tag)
					s->Visible = 1;
			}
		}
		if (hittest_b_storey[b_schdl->Tag]) {
			b_schdl_confirm->Visible = 1;
			for (size_t i = 0; i < Frame1->crt_layout->Children->Count; ++i) {
				if (__classid(TMySelection) == Frame1->crt_layout->Children->operator[](i)
					->ClassType()) {
					s = (TMySelection*)Frame1->crt_layout->Children->operator[](i);
					if (-1 == s->Tag)
						s->Visible = 1;
				}
			}
		}
		else {
			b_schdl_confirm->Visible = 0;
			for (size_t i = 0; i < Frame1->crt_layout->Children->Count; ++i) {
				if (__classid(TMySelection) == Frame1->crt_layout->Children->operator[](i)
					->ClassType()) {
					s = (TMySelection*)Frame1->crt_layout->Children->operator[](i);
					if (-1 == s->Tag)
						s->Visible = 0;
				}
			}
		}
	}
	else {
		b_schdl->Visible = 0;
		b_schdl_confirm->Visible = 0;
		for (size_t i = 0; i < Frame1->crt_layout->Children->Count; ++i) {
			if (__classid(TMySelection) == Frame1->crt_layout->Children->operator[](i)->ClassType())
			{
				s = (TMySelection*)Frame1->crt_layout->Children->operator[](i);
				s->Visible = 0;
			}
		}
	}
}

// ---------------------------------------------------------------------------
void __fastcall TForm1::btn_context_init() {
	add_storey_buttons();
	slct_addition();
}

// ---------------------------------------------------------------------------
void __fastcall TForm1::schdl_click(TObject * Sender) {
	TMySelection *s = 0;

	hittest_b_storey[b_schdl->Tag] = !hittest_b_storey[b_schdl->Tag];
	if (hittest_b_storey[b_schdl->Tag]) {
		for (size_t i = 0; i < Frame1->crt_layout->Children->Count; ++i) {
			if (__classid(TMySelection) == Frame1->crt_layout->Children->operator[](i)->ClassType())
			{
				s = (TMySelection*)Frame1->crt_layout->Children->operator[](i);
				if (-1 == s->Tag) {
					s->Visible = 1;
				}
			}
		}
		b_schdl_confirm->Visible = 1;
		Frame1->add_schdl_title();
	}
	else {
		for (size_t i = 0; i < Frame1->crt_layout->Children->Count; ++i) {
			if (__classid(TMySelection) == Frame1->crt_layout->Children->operator[](i)->ClassType())
			{
				s = (TMySelection*)Frame1->crt_layout->Children->operator[](i);
				if (-1 == s->Tag) {
					s->Visible = 0;
				}
			}
		}
		b_schdl_confirm->Visible = 0;
	}
}

// ---------------------------------------------------------------------------
void __fastcall TForm1::client_conn() {
	int v;
	int numbytes;
	int rv;
	unsigned t_id;
	dx *a = 0;
	uint32_t the_crc32;
	u_long addr_b;
	addrinfo hints, *servinfo, *p;
	char s[INET6_ADDRSTRLEN];
	unsigned char buf_once[512] = {0};
	memset(buf_once, 0, 512);
	SecureZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	// if ((rv = getaddrinfo("localhost", PORT, &hints, &servinfo)) != 0) {
	if ((rv = getaddrinfo("192.168.200.21", PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return;
	}
	for (p = servinfo; p != NULL; p = p->ai_next) {
		if ((mysock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}
		// linger lg;
		// lg.l_onoff = 1;
		// lg.l_linger = 0;
		// setsockopt(mysock, SOL_SOCKET, SO_LINGER, (const char *)&lg, sizeof(lg));
		if (connect(mysock, p->ai_addr, p->ai_addrlen) == -1) {
			closesocket(mysock);
			perror("client: connect");
			continue;
		}
		break;
	}
	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return;
	}
	inet_ntop(p->ai_family, get_in_addr((struct sockaddr*)p->ai_addr), s, sizeof s);
	get_in_addr4(&addr_b, p->ai_addr);
	my_addr_b = addr_b;
	freeaddrinfo(servinfo);

	v = 0;
	CloseHandle((void *)_beginthreadex(0, 0, display_offline_status, &v, 0, &t_id));

	a = (dx*)buf_once;
	a->head[0] = a->head[1] = a->head[2] = UINT_MAX;
	a->len_pack = sizeof(*a)+sizeof(uint32_t);
	a->st = time(0);
	a->cmd = 757171;
	the_crc32 = my_crc32((char *)buf_once, a->len_pack -sizeof(uint32_t));
	*(uint32_t*)(buf_once +sizeof(*a)) = the_crc32;
	send(mysock, (const char *)buf_once, a->len_pack, 0);

	// test
	// Sleep(500);
	// shutdown(mysock,SD_BOTH);
	// closesocket(mysock);

	WaitForSingleObject(draw_basis_end, INFINITE);
	memset(buf_once, 0, 512);
	a = (dx*)buf_once;
	a->head[0] = a->head[1] = a->head[2] = UINT_MAX;
	a->st = time(0);
	a->cmd = 757174;
	AnsiString sqlcmd = "SELECT * FROM location_storage";
	a->len_pack = sizeof(*a)+sizeof(uint32_t) + sqlcmd.Length();
	MoveMemory(buf_once +sizeof(*a), sqlcmd.c_str(), sqlcmd.Length());
	the_crc32 = my_crc32((char *)buf_once, a->len_pack -sizeof(uint32_t));
	*(uint32_t*)(buf_once +sizeof(*a) + sqlcmd.Length()) = the_crc32;
	send(mysock, (const char *)buf_once, a->len_pack, 0);
}

// ---------------------------------------------------------------------------

bool __fastcall TForm1::get_agvtask_type(size_t * type_at) {
	return *type_at = ComboBox1->ItemIndex ? ComboBox1->ItemIndex + 4999 : 0;
}
// ---------------------------------------------------------------------------

void __fastcall TForm1::Edit1Change(TObject * Sender) {
	TEdit *e = (TEdit*)Sender;
	TRectangle *r = 0;
	TCircle *c = 0;
	specloc *spl = 0;
	storloc *stl = 0;
	if (e->TagObject) {
		size_t ctrlt = Frame1->control_type(e->TagObject);
		if (ctrlt == ctrl_specloc) {
			r = (TRectangle*)e->TagObject;
			spl = (specloc*)r->Tag;
			e->TextSettings->FontColor = spl->signal ? spl->color[1] : spl->color[0];
		}
		else if (ctrlt == ctrl_storloc) {
			c = (TCircle*)e->TagObject;
			stl = (storloc*)c->Tag;
			e->TextSettings->FontColor = stl->full ? stl->color[1] : stl->color[0];
		}
	}
	else {
		e->TextSettings->FontColor = TAlphaColors::Coral;
	}
	allow_issue();
}
// ---------------------------------------------------------------------------

void __fastcall TForm1::SpeedButton1Click(TObject * Sender) {
	/*
	 unsigned char *p = 0;
	 agvtask_view *av = 0;
	 alloc_smlbuf(&p);
	 av = (agvtask_view*)p;
	 init_agvtask_view(av);
	 Frame1->agvtask_lineto(av);
	 // issue_task(av);
	 issue_task_http(av);
	 Frame1->restore_task_end_point(av);
	 */

	if (WAIT_OBJECT_0 != WaitForSingleObject(event_nextagvtask, 0));
	else {
		TDateTime dt;

		if ((1 == ComboBox1->ItemIndex) || (2 == ComboBox1->ItemIndex))
			if (mrYes != Form6->ShowModal())
				return;

		init_agvtask_view(&av_pending);
		Frame1->agvtask_lineto(&av_pending);

		agvtask_listitem at_item;
		at_item.row_number = -1;
		SecureZeroMemory(&(at_item.v), sizeof(at_item.v));
		for (size_t i = 0; i < GRIDHEADother; ++i) {
			at_item.txts_listitems[i] = _T("");
		}

		issue_task_http(&dt, &av_pending);

		at_item.v = av_pending;
		at_item.txts_listitems[h_num_taskissue] = av_pending.pc.t_id;
		at_item.txts_listitems[h_b] = Edit1->Text;
		at_item.txts_listitems[h_e] = Edit2->Text;
		at_item.txts_listitems[h_type_task] = ComboBox1->Items->operator[](ComboBox1->ItemIndex);
		// at_item.txts_listitems[h_type_prod] = ComboBox2->Items->operator[](ComboBox2->ItemIndex);
		if (1 == Form6->product_small_or_big())
			at_item.txts_listitems[h_type_prod] = _T("π˙ƒ⁄");
		else if (2 == Form6->product_small_or_big())
			at_item.txts_listitems[h_type_prod] = _T("π˙Õ‚");
		at_item.txts_listitems[h_id_warehouse] = Format(_T("#%d"), ARRAYOFCONST(((id_warehouse))));
		at_item.txts_listitems[h_st] = DateTimeToStr(dt);
		at_item.row_number = Form2->myrowcount;
		EnterCriticalSection(&(Frame1->cs_at_items));
		Frame1->at_items.push_back(at_item);
		LeaveCriticalSection(&(Frame1->cs_at_items));

		Form2->StringGrid1->Cells[h_b][Form2->myrowcount] = Edit1->Text;
		Form2->StringGrid1->Cells[h_e][Form2->myrowcount] = Edit2->Text;
		Form2->StringGrid1->Cells[h_type_task][Form2->myrowcount] =
			ComboBox1->Items->operator[](ComboBox1->ItemIndex);
		// Form2->StringGrid1->Cells[h_type_prod][Form2->myrowcount] =
		// ComboBox2->Items->operator[](ComboBox2->ItemIndex);
		Form2->StringGrid1->Cells[h_type_prod][Form2->myrowcount] =
			at_item.txts_listitems[h_type_prod];
		Form2->StringGrid1->Cells[h_st][Form2->myrowcount] = DateTimeToStr(dt);
		Form2->StringGrid1->Cells[h_id_warehouse][Form2->myrowcount] =
			Format(_T("#%d"), ARRAYOFCONST(((id_warehouse))));
		Form2->StringGrid1->Cells[h_num_taskissue][Form2->myrowcount] =
			UIntToStr(av_pending.pc.t_id);
		++Form2->myrowcount;
	}
	Frame1->restore_task_end_point(&av_pending);
}
// ---------------------------------------------------------------------------

void __fastcall TForm1::issue_task_http(TDateTime *st_ti, agvtask_view * av) {
	int st;
	unsigned type_product;
	String id_ti;
	task_issue new_ti;
	String txt_sn_loc("");

	TStringList *prms_post = new TStringList();
#ifdef _DEBUG
	String myurl = Format(_T("http://%s:19998"), ARRAYOFCONST(((urls[0]))));
#else
	String myurl = Format(_T("http://%s:19998"), ARRAYOFCONST(((urls[1]))));
#endif
	SecureZeroMemory(&new_ti, sizeof(new_ti));
	av->r_ti = rti_pending;
	new_ti.r_ti = rti_pending;
	prms_post->AddPair(_T("cmd"), _T("657194"));
	prms_post->AddPair(_T("id_client"), UIntToStr((unsigned)DataModule1->id_client));
	new_ti.id_client = DataModule1->id_client;
	st = Form6->product_small_or_big();

	fix_task_type(av);

	// prms_post->AddPair(_T("type_product"), IntToStr((st)));
	prms_post->AddPair(_T("type_product"), IntToStr((int)(av->pc.type_product)));
	prms_post->AddPair(_T("id_warehouse"), IntToStr(id_warehouse));
	new_ti.id_warehouse = id_warehouse;
	prms_post->AddPair(_T("tsk"), IntToStr((int)av->pc.tsk));
	st = time(0);
	*st_ti = UnixToDateTime(st);
	new_ti.st = st;
	new_ti.one_pc = av->pc;

	prms_post->AddPair(_T("st"), IntToStr(st));
	prms_post->AddPair(_T("color"), UIntToStr((unsigned)Form6->rspwc->color));
	prms_post->AddPair(_T("packo"), Form6->rspwc->rep_sp.packo);
	prms_post->AddPair(_T("spec"), Form6->rspwc->rep_sp.spec);
	prms_post->AddPair(_T("packi"), Form6->rspwc->rep_sp.packi);
	prms_post->AddPair(_T("company"), Form6->rspwc->rep_sp.company);

	for (size_t i = 0; i < 2; ++i) {
		for (size_t j = 0; j < 4; ++j) {
			txt_sn_loc += UIntToStr(av->pc.sn_loc[i][j]);
			txt_sn_loc += _T(',');
		}
		txt_sn_loc.Delete(txt_sn_loc.Length(), 1);
		txt_sn_loc += _T(';');
	}
	txt_sn_loc.Delete(txt_sn_loc.Length(), 1);
	prms_post->AddPair(_T("sn_loc"), txt_sn_loc);
	try {
		id_ti = DataModule1->request_task_issue->Post(myurl, prms_post)->ContentAsString();
		TryStrToUInt(id_ti, av->pc.t_id);
		new_ti.one_pc.t_id = av->pc.t_id;
		tis.push_back(new_ti);
		DataModule1->timer_query_rti->Enabled = 1;
	}
	catch (...) {
		Form1->Label1->Text = _T("¡¨Ω”¥ÌŒÛ");
	}
	delete prms_post;
}

// ---------------------------------------------------------------------------

void __fastcall TForm1::issue_task(agvtask_view * av) {
	uint32_t crc_task = 0;
	unsigned char *buf = 0;
	dx *t = 0;
	alloc_smlbuf(&buf);
	t = (dx*)buf;
	t->head[0] = t->head[1] = t->head[2] = UINT_MAX;
	t->cmd = 757170;
	t->st = time(0);
	t->No = number_cmd();
	t->ip = 0;
	t->len_pack = sizeof(dx)+sizeof(pair_loc)+sizeof(uint32_t);
#ifdef _DEBUG
	// av->pc.type_product = Edit3->Text.ToInt();
	// av->pc.type_product = ComboBox2->ItemIndex;
	av->pc.type_product = Form6->product_small_or_big();
#endif
	MoveMemory(buf +sizeof(dx), &(av->pc), sizeof(av->pc));
	crc_task = my_crc32((char *)buf, sizeof(dx)+sizeof(pair_loc));
	*(uint32_t*)(buf +sizeof(dx)+sizeof(pair_loc)) = crc_task;
	send(mysock, (const char *)buf, t->len_pack, 0);

	// t->len_pack = sizeof(dx) + sizeof(uint32_t);
	// if(L1 == av->pc.tsk){
	// t->cmd = 757172;
	// }else if(L2 == av->pc.tsk){
	// t->cmd = 757173;
	// }
	// crc_task = my_crc32((char *)buf, sizeof(dx));
	// *(uint32_t*)(buf +sizeof(dx)) = crc_task;
	// send(mysock, (const char *)buf, t->len_pack, 0);

	recycle_smlbuf(buf);
}
// ---------------------------------------------------------------------------

size_t __fastcall TForm1::number_cmd() {
	++No_cmd_crt;
	if (No_cmd_crt < No_cmd_limit);
	else
		No_cmd_crt = No_cmd_base;
	return No_cmd_crt;
}

// ---------------------------------------------------------------------------
bool __fastcall TForm1::allow_issue() {
	bool b = false;
	if (ComboBox1->ItemIndex == 1 || ComboBox1->ItemIndex == 2) {
		b = (ComboBox1->ItemIndex && Edit1->TagObject && Edit2->TagObject);
	}
	else {
		b = ComboBox1->ItemIndex && Edit1->TagObject && Edit2->TagObject;
	}
	SpeedButton1->Enabled = b;
	return b;
}

// ---------------------------------------------------------------------------

void __fastcall TForm1::ComboBox1Change(TObject * Sender) {
	allow_issue();
}
// ---------------------------------------------------------------------------

void __fastcall TForm1::init_agvtask_view(agvtask_view * av) {
	SecureZeroMemory(av, sizeof(*av));
	av->indx_s1 = -1;
	av->indx_s2 = -1;
	av->indx_agv1 = -1;
	av->indx_agv2 = -1;
}

// ---------------------------------------------------------------------------
__fastcall ThreadUpdateOffline::ThreadUpdateOffline(int ofl) : TThread(false), offline(ofl) {
}

// ---------------------------------------------------------------------------
__fastcall ThreadUpdateOffline::ThreadUpdateOffline() {

}

// ---------------------------------------------------------------------------
__fastcall ThreadUpdateOffline::~ThreadUpdateOffline() {
}

// ---------------------------------------------------------------------------
void __fastcall ThreadUpdateOffline::update_offline_status() {
	for (size_t i = 0; i < Form1->StatusBar1->ChildrenCount; ++i) {
		if (Form1->StatusBar1->Children->operator[](i)->TagString == _T("connect status")) {
			if (offline)
				((TLabel*)(Form1->StatusBar1->Children->operator[](i)))->Text = _T("¿Îœﬂ");
			else
				((TLabel*)(Form1->StatusBar1->Children->operator[](i)))->Text = _T("‘⁄œﬂ");
		}
	}
}

// ---------------------------------------------------------------------------
void __fastcall ThreadUpdateOffline::Execute() {
	Synchronize(update_offline_status);
}

// ---------------------------------------------------------------------------
void __fastcall TForm1::CornerButton2Click(TObject *Sender) {
	Form2->ShowModal();
}

// ---------------------------------------------------------------------------
void __fastcall TForm1::ColorComboBox1Change(TObject *Sender) {
	allow_issue();
}
// ---------------------------------------------------------------------------

// void __fastcall TForm1::ComboBox2Change(TObject *Sender) {
// allow_issue();
// }
// ---------------------------------------------------------------------------

void __fastcall TForm1::Edit3ChangeTracking(TObject *Sender) {
	allow_issue();
}

// ---------------------------------------------------------------------------
void __fastcall TForm1::spec_package_template() {
	TIniFile *spec_package_ini = new TIniFile(spec_package);
	// array length = 72+1
	spec_package_ini->WriteInteger(_T("PackageO"), _T("num"), 72);
	// <18-cn, >=18-foreign
	spec_package_ini->WriteInteger(_T("PackageO"), _T("divider_cn_foreign"), 18);
	spec_package_ini->WriteString(_T("PackageO"), _T("1"), _T("66∫≈-2.5KG-BZ"));
	spec_package_ini->WriteString(_T("PackageO"), _T("2"), _T("66∫≈-2.5KG-JJ"));
	spec_package_ini->WriteString(_T("PackageO"), _T("3"), _T("66∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("4"), _T("66∫≈-25KG-BZ"));
	spec_package_ini->WriteString(_T("PackageO"), _T("5"), _T("66∫≈-25KG-JJ"));
	spec_package_ini->WriteString(_T("PackageO"), _T("6"), _T("66∫≈-5kg"));
	spec_package_ini->WriteString(_T("PackageO"), _T("7"), _T("66∫≈-5kg-BZ"));
	spec_package_ini->WriteString(_T("PackageO"), _T("8"), _T("66∫≈-5kg-JJ"));
	spec_package_ini->WriteString(_T("PackageO"), _T("9"), _T("ZJ-2.5KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("10"), _T("ZJ-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("11"), _T("ZJ-5KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("12"), _T("ø’∞◊≈£∆§÷Ωœ‰"));
	spec_package_ini->WriteString(_T("PackageO"), _T("13"), _T("ø’∞◊≈£∆§÷Ωœ‰-2.5KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("14"), _T("ø’∞◊≈£∆§÷Ωœ‰-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("15"), _T("ø’∞◊÷Ωœ‰"));
	spec_package_ini->WriteString(_T("PackageO"), _T("16"), _T("ø’∞◊÷Ωœ‰-2.5KG-BZ"));
	spec_package_ini->WriteString(_T("PackageO"), _T("17"), _T("—©ª®Õ∞-160KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("18"), _T("10-1∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("19"), _T("102∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("20"), _T("10∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("21"), _T("19-2∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("22"), _T("27∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("23"), _T("37-R∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("24"), _T("37∫≈-2.5KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("25"), _T("37∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("26"), _T("40∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("27"), _T("4-1∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("28"), _T("4-2∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("29"), _T("43-1∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("30"), _T("48-1∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("31"), _T("4∫≈Ã˙Õ∞-150KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("32"), _T("53∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("33"), _T("54∫≈-25kg"));
	spec_package_ini->WriteString(_T("PackageO"), _T("34"), _T("57∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("35"), _T("59-1∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("36"), _T("60-1∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("37"), _T("60-2∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("38"), _T("61∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("39"), _T("62∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("40"), _T("63∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("41"), _T("67-3∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("42"), _T("68-1∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("43"), _T("68-2∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("44"), _T("70∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("45"), _T("71∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("46"), _T("73∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("47"), _T("77∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("48"), _T("78∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("49"), _T("81∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("50"), _T("86∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("51"), _T("87-1∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("52"), _T("88-2∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("53"), _T("88∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("54"), _T("90∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("55"), _T("92∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("56"), _T("93∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("57"), _T("94∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("58"), _T("95∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("59"), _T("97∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("60"), _T("98∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("61"), _T("99-1∫≈-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("62"), _T("ºØ◊∞¥¸-600KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("63"), _T("øæ∆·Õ∞-150KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("64"), _T("ø’∞◊ﬂÈÕ∑-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("65"), _T("ø’∞◊÷Ωœ‰"));
	spec_package_ini->WriteString(_T("PackageO"), _T("66"), _T("ø’∞◊÷Ωœ‰-25KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("67"), _T("Àıø⁄Õ∞-150KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("68"), _T("Ã˙Õ∞-150KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("69"), _T("—©ª®Õ∞-150KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("70"), _T("÷ΩÕ∞-140KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("71"), _T("÷ΩÕ∞-160KG"));
	spec_package_ini->WriteString(_T("PackageO"), _T("72"), _T("÷‹◊™œ‰-25KG"));
	// array length = 21 + 1 =22
	spec_package_ini->WriteInteger(_T("Spec"), _T("num"), 21);
	// <8-cn, >=8-foreign
	spec_package_ini->WriteInteger(_T("Spec"), _T("divider_cn_foreign"), 8);
	spec_package_ini->WriteString(_T("Spec"), _T("1"), _T("0.5-0.88"));
	spec_package_ini->WriteString(_T("Spec"), _T("2"), _T("0.5-0.9"));
	spec_package_ini->WriteString(_T("Spec"), _T("3"), _T("0.7-0.9"));
	spec_package_ini->WriteString(_T("Spec"), _T("4"), _T("0.8-1.2"));
	spec_package_ini->WriteString(_T("Spec"), _T("5"), _T("1.0-1.5"));
	spec_package_ini->WriteString(_T("Spec"), _T("6"), _T("1.5-2.0"));
	spec_package_ini->WriteString(_T("Spec"), _T("7"), _T("2.0-2.5"));
	spec_package_ini->WriteString(_T("Spec"), _T("8"), _T("0.5-0.8"));
	spec_package_ini->WriteString(_T("Spec"), _T("9"), _T("0.5-0.88"));
	spec_package_ini->WriteString(_T("Spec"), _T("10"), _T("0.5-0.9"));
	spec_package_ini->WriteString(_T("Spec"), _T("11"), _T("0.5-1.0"));
	spec_package_ini->WriteString(_T("Spec"), _T("12"), _T("0.6-0.85"));
	spec_package_ini->WriteString(_T("Spec"), _T("13"), _T("0.8-1.2"));
	spec_package_ini->WriteString(_T("Spec"), _T("14"), _T("1.0-1.5"));
	spec_package_ini->WriteString(_T("Spec"), _T("15"), _T("1.2-1.8"));
	spec_package_ini->WriteString(_T("Spec"), _T("16"), _T("1.4-1.8"));
	spec_package_ini->WriteString(_T("Spec"), _T("17"), _T("1.5-2.0"));
	spec_package_ini->WriteString(_T("Spec"), _T("18"), _T("1.6-2.0"));
	spec_package_ini->WriteString(_T("Spec"), _T("19"), _T("1.7-2.5"));
	spec_package_ini->WriteString(_T("Spec"), _T("20"), _T("2.0-2.5"));
	spec_package_ini->WriteString(_T("Spec"), _T("21"), _T("2.2-2.5"));
	// array length = 24 + 1 =25
	spec_package_ini->WriteInteger(_T("PackageI"), _T("num"), 24);
	// <13-cn, >=8-foreign
	spec_package_ini->WriteInteger(_T("PackageI"), _T("divider_cn_foreign"), 13);
	spec_package_ini->WriteString(_T("PackageI"), _T("1"), _T("2.5"));
	spec_package_ini->WriteString(_T("PackageI"), _T("2"), _T("25"));
	spec_package_ini->WriteString(_T("PackageI"), _T("3"), _T("140"));
	spec_package_ini->WriteString(_T("PackageI"), _T("4"), _T("150"));
	spec_package_ini->WriteString(_T("PackageI"), _T("5"), _T("160"));
	spec_package_ini->WriteString(_T("PackageI"), _T("6"), _T("600"));
	spec_package_ini->WriteString(_T("PackageI"), _T("7"), _T("2.5kg ¬¡ƒ§¥¸"));
	spec_package_ini->WriteString(_T("PackageI"), _T("8"), _T("25kg ¬¡ƒ§¥¸"));
	spec_package_ini->WriteString(_T("PackageI"), _T("9"), _T("5kg ¬¡ƒ§¥¸"));
	spec_package_ini->WriteString(_T("PackageI"), _T("10"), _T("≤ªº”ƒ⁄≥ƒ"));
	spec_package_ini->WriteString(_T("PackageI"), _T("11"), _T("ƒ⁄≥ƒ¬¡ƒ§¥¸"));
	spec_package_ini->WriteString(_T("PackageI"), _T("12"), _T(""));
	spec_package_ini->WriteString(_T("PackageI"), _T("13"), _T("2.5"));
	spec_package_ini->WriteString(_T("PackageI"), _T("14"), _T("25"));
	spec_package_ini->WriteString(_T("PackageI"), _T("15"), _T("140"));
	spec_package_ini->WriteString(_T("PackageI"), _T("16"), _T("150"));
	spec_package_ini->WriteString(_T("PackageI"), _T("17"), _T("160"));
	spec_package_ini->WriteString(_T("PackageI"), _T("18"), _T("600"));
	spec_package_ini->WriteString(_T("PackageI"), _T("19"), _T("2.5kg ¬¡ƒ§¥¸"));
	spec_package_ini->WriteString(_T("PackageI"), _T("20"), _T("25kg ¬¡ƒ§¥¸"));
	spec_package_ini->WriteString(_T("PackageI"), _T("21"), _T("5kg ¬¡ƒ§¥¸"));
	spec_package_ini->WriteString(_T("PackageI"), _T("22"), _T("≤ªº”ƒ⁄≥ƒ"));
	spec_package_ini->WriteString(_T("PackageI"), _T("23"), _T("ƒ⁄≥ƒ¬¡ƒ§¥¸"));
	spec_package_ini->WriteString(_T("PackageI"), _T("24"), _T(""));
	// array length = 2 + 1 =3
	spec_package_ini->WriteInteger(_T("Company"), _T("num"), 2);
	// <3-cn, >=3-foreign
	spec_package_ini->WriteInteger(_T("Company"), _T("divider_cn_foreign"), 3);
	spec_package_ini->WriteString(_T("Company"), _T("1"), _T("¬Ãƒ‹"));
	spec_package_ini->WriteString(_T("Company"), _T("2"), _T("÷¡ºÚ"));

	delete spec_package_ini;
	spec_package_ini = 0;
}

// ---------------------------------------------------------------------------
void __fastcall TForm1::parse_spec_package() {
	TIniFile *spec_package_ini = 0;
	try {
		spec_package_ini = new TIniFile(spec_package);
		for (size_t i = 0; i < FLAGSPECPACKSEGSother; ++i) {
			records_sp.num[i] = spec_package_ini->ReadInteger(spec_package_segments[i],
				_T("num"), 0);
			++records_sp.num[i];
			records_sp.divider[i] = spec_package_ini->ReadInteger(spec_package_segments[i],
				_T("divider_cn_foreign"), 0);
			records_sp.arys[i] = new String[records_sp.num[i]];
			for (size_t j = 1; j < records_sp.num[i]; ++j) {
				String key(j);
				records_sp.arys[i][j] = spec_package_ini->ReadString(spec_package_segments[i], key,
					_T(""));
			}
		}
	}
	catch (...) {

	}
	delete spec_package_ini;
	spec_package_ini = 0;
#ifdef _DEBUG
	for (size_t i = 0; i < FLAGSPECPACKSEGSother; ++i) {
		for (size_t j = 1; j < records_sp.num[i]; ++j) {
			OutputDebugString(records_sp.arys[i][j].t_str());
		}
	}
#endif
}
// ---------------------------------------------------------------------------
