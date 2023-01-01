// ---------------------------------------------------------------------------

#pragma hdrstop

#include "common.h"
#include "compones.h"
// ---------------------------------------------------------------------------
#pragma package(smart_init)

const size_t len_dx = sizeof(dx);
const size_t len_size_t = sizeof(size_t);
const size_t len_crc32 = sizeof(uint32_t);

const size_t colors_available[135] = {
	0xfffaebd7, 0xff00ffff, 0xff7fffd4, 0xfff5f5dc, 0xffffe4c4, 0xffffebcd, 0xff0000ff, 0xffa52a2a,
	0xffdeb887, 0xff7fff00, 0xffd2691e, 0xffff7f50, 0xfffff8dc, 0xffdc143c, 0xff00ffff, 0xff00008b,
	0xff008b8b, 0xffb8860b, 0xffa9a9a9, 0xff006400, 0xffa9a9a9, 0xffbdb76b, 0xff8b008b, 0xff556b2f,
	0xffff8c00, 0xff9932cc, 0xff8b0000, 0xffe9967a, 0xff8fbc8f, 0xff483d8b, 0xff2f4f4f, 0xff2f4f4f,
	0xff00ced1, 0xff9400d3, 0xffff1493, 0xff00bfff, 0xff696969, 0xff696969, 0xffb22222, 0xff228b22,
	0xffff00ff, 0xffdcdcdc, 0xffffd700, 0xffdaa520, 0xff808080, 0xffadff2f, 0xff808080, 0xfff0fff0,
	0xffff69b4, 0xffcd5c5c, 0xff4b0082, 0xfff0e68c, 0xffe6e6fa, 0xfffff0f5, 0xff7cfc00, 0xfffffacd,
	0xffadd8e6, 0xfff08080, 0xffe0ffff, 0xfffafad2, 0xffd3d3d3, 0xff90ee90, 0xffd3d3d3, 0xffffb6c1,
	0xffffa07a, 0xff20b2aa, 0xff87cefa, 0xff9acd32, 0xffffff00, 0xfff5deb3, 0xffee82ee, 0xff40e0d0,
	0xffff6347, 0xffd8bfd8, 0xff008080, 0xffd2b48c, 0xff4682b4, 0xff00ff7f, 0xff708090, 0xff708090,
	0xff6a5acd, 0xff6a5acd, 0xff87ceeb, 0xffc0c0c0, 0xffa0522d, 0xff2e8b57, 0xfff4a460, 0xfffa8072,
	0xff8b4513, 0xff4169e1, 0xffbc8f8f, 0xffff0000, 0xff800080, 0xffb0e0e6, 0xffdda0dd, 0xffffc0cb,
	0xffcd853f, 0xffffdab9, 0xffffefd5, 0xffdb7093, 0xffafeeee, 0xff98fb98, 0xffeee8aa, 0xffda70d6,
	0xffff4500, 0xffffa500, 0xff6b8e23, 0xff808000, 0xfffdf5e6, 0xff000080, 0xffffdead, 0xffffe4b5,
	0xffffe4e1, 0xff191970, 0xffc71585, 0xff48d1cc, 0xff00fa9a, 0xff7b68ee, 0xff3cb371, 0xff9370db,
	0xffba55d3, 0xff0000cd, 0xff66cdaa, 0xff800000, 0xffff00ff, 0xff00ff00, 0xfff0fbff, 0xfff0caa6,
	0xffc0dcc0, 0xff808080, 0xffa0a0a0, 0xffc0c0c0, 0xffffffe0, 0xffb0c4de, 0xff1e90ff};

const int home_foreign_delta = -100;

const char *ips_plcs[PLCOTHER] = {"192.168.0.2", "192.168.26.211"};
// modbus *mb_tcp[PLCOTHER] = {0};
modbus_t *ctx_tcp_modbus[PLCOTHER] = {0};

// modbus mb_lift1_slave2("192.168.26.3");

const size_t group_reg_s[4] = {100, 101, 104, 106};
const size_t group_reg_n[4] = {102, 103, 105, 107};

bool column_matrix_uploaded = false;

HANDLE event_lift1 = NULL;
HANDLE event_lift2 = NULL;
size_t goal_lift1 = 0;
size_t goal_lift2 = 0;

SOCKET listener, start_conn_self, dispatcher, httpcn;

u_int gId_client;
CRITICAL_SECTION cs_id_client;

size_t gId_task_issue;
CRITICAL_SECTION cs_id_task;

size_t quota_agvs;

String path_db;

records_reader rcr_wp;
CRITICAL_SECTION cs_rcr_wp;

std::vector<agv>agvs;
CRITICAL_SECTION cs_agvs;

std::vector<sock_kit*>bus_conn;

std::vector<wp_with_ip>wps;
CRITICAL_SECTION cs_wps;

std::vector<task_issue>tis;
CRITICAL_SECTION cs_tis;

bool incoming;
unsigned count_handled = 0;
std::vector<task_issue_over>tis_over;
CRITICAL_SECTION cs_tis_over;
// CONDITION_VARIABLE cv_tis_over = CONDITION_VARIABLE_INIT;
CONDITION_VARIABLE cv_tis_over;

std::list<size_t>colors_unused;
std::list<size_t>colors_used;

CRITICAL_SECTION cs_send2tcpsrv;

// 0-storey1, 1-storey2
charge_pill cps[2][8];
size_t count_cps[2];

void* __fastcall get_in_addr(sockaddr *sa) {
	if (AF_INET == sa->sa_family) {
		return &(((sockaddr_in*)sa)->sin_addr);
	}
	return &(((sockaddr_in6*)sa)->sin6_addr);
}

void __fastcall get_in_addr4(u_long *a, sockaddr *sa) {
	if (AF_INET == sa->sa_family) {
		*a = ((sockaddr_in*)sa)->sin_addr.S_un.S_addr;
	}
	else
		*a = 0;
}

uint32_t __fastcall my_crc32(char *base, size_t len) {
	boost::crc_32_type result;
	result.process_bytes(base, len);
	return result.checksum();
}

size_t __fastcall lift1_ready(size_t addr_b) {
	unsigned short status[1] = {0};
	int code_err_plc;
	size_t agv_storey = 0;
	// use default value for cmd_result
	size_t cmd_result = 657191;
	std::vector<agv>::iterator b;
	AnsiString msg("");
	READS_LIFT1(code_err_plc, status, ctx_tcp_modbus[plclift1]);
	if (code_err_plc);
	else {
		OutputDebugStringA("³¢ÊÔ¶ÁÈ¡Éý½µ»úLIFT1Ê§°Ü");
		return cmd_result;
	}
	EnterCriticalSection(&cs_agvs);
	for (b = agvs.begin(); agvs.end() != b; ++b) {
		if ((*b).addr_b == addr_b) {
			agv_storey = (*b).storey;
			break;
		}
	}
	// be care for doing return; within EnterCriticalSection/LeaveCriticalSection
	// if (agvs.end() == b) {
	// LeaveCriticalSection(&cs_agvs);
	// return cmd_result = 657191;
	// }
	LeaveCriticalSection(&cs_agvs);

	if (status[0] == agv_storey) {
		cmd_result = 657190;
	}
	else {
		if (1 == status[0]) {
			RAISES_LIFT1(code_err_plc, ctx_tcp_modbus[plclift1]);
			if (code_err_plc);
			else
				OutputDebugStringA("³¢ÊÔÉýÆðÉý½µ»úLIFT1Ê§°Ü");
		}
		else if (2 == status[0]) {
			DROPS_LIFT1(code_err_plc, ctx_tcp_modbus[plclift1]);
			if (code_err_plc);
			else
				OutputDebugStringA("³¢ÊÔ½µÏÂÉý½µ»úLIFT1Ê§°Ü");
		}
	}
	return cmd_result;
}

void __fastcall lift1_go(size_t addr_b) {
	int code_err_plc;
	size_t agv_storey = 0;
	std::vector<agv>::iterator b;
	EnterCriticalSection(&cs_agvs);
	for (b = agvs.begin(); agvs.end() != b; ++b) {
		if ((*b).addr_b == addr_b) {
			agv_storey = (*b).storey;
			break;
		}
	}
	// be care for doing return; within EnterCriticalSection/LeaveCriticalSection
	// if (agvs.end() == b) {
	// LeaveCriticalSection(&cs_agvs);
	// return;
	// }
	LeaveCriticalSection(&cs_agvs);
	if (1 == agv_storey) {
		goal_lift1 = 2;
		RAISES_LIFT1(code_err_plc, ctx_tcp_modbus[plclift1]);
		if (code_err_plc) {
			SetEvent(event_lift1);
			AnsiString msg_hint = AnsiString::Format("Éý½µ»ú½«ÉýÖÁµÚ %u ²ã", ARRAYOFCONST((goal_lift1)));
			OutputDebugStringA(msg_hint.c_str());
		}
		else
			OutputDebugStringA("³¢ÊÔÉýÆðÉý½µ»úLIFT1Ê§°Ü");
	}
	else if (2 == agv_storey) {
		goal_lift1 = 1;
		DROPS_LIFT1(code_err_plc, ctx_tcp_modbus[plclift1]);
		if (code_err_plc) {
			SetEvent(event_lift1);
			AnsiString msg_hint = AnsiString::Format("Éý½µ»ú½«½µÖÁµÚ %u ²ã", ARRAYOFCONST((goal_lift1)));
			OutputDebugStringA(msg_hint.c_str());
		}
		else
			OutputDebugStringA("³¢ÊÔ½µÏÂÉý½µ»úLIFT1Ê§°Ü");
	}
}

unsigned __stdcall timer_lift1(void *p) {
	unsigned short status[1] = {0};
	int code_err_plc;
NEXT_WAIT:
	WaitForSingleObject(event_lift1, INFINITE);
	for (; ;) {
		Sleep(3000);
		READS_LIFT1(code_err_plc, status, ctx_tcp_modbus[plclift1]);
		if (code_err_plc);
		else {
			OutputDebugStringA("³¢ÊÔ¶ÁÈ¡Éý½µ»úLIFT1Ê§°Ü");
		}
		if (goal_lift1 == status[0]) {
			unsigned char *new_buf = 0;
			dx *new_dx = 0;
			BUILD_DX_BLOCK(new_buf, new_dx, 523100, 0, time(0), 0, 0, 0);
			send(start_conn_self, new_buf, new_dx->len_pack, 0);
			putback_smlbuf(new_buf);
			goto NEXT_WAIT;
		}
	}
	return 0;
}

unsigned __stdcall self_conn(void *p) {
	SELF_CONN(start_conn_self);
	unsigned char *new_buf = 0;
	dx *new_dx = 0;
	BUILD_DX_BLOCK(new_buf, new_dx, 523101, 0, time(0), 0, 0, 0);
	send(start_conn_self, new_buf, new_dx->len_pack, 0);
	putback_smlbuf(new_buf);
	return 0;
}

unsigned __stdcall http_conn(void *p) {
	SELF_CONN(httpcn);
	unsigned char *new_buf = 0;
	dx *new_dx = 0;
	BUILD_DX_BLOCK(new_buf, new_dx, 522100, 0, time(0), 0, 0, 0);
	send(httpcn, new_buf, new_dx->len_pack, 0);
	putback_smlbuf(new_buf);
	return 0;
}

void __fastcall data_handler(sock_kit *sk, dx* onepack) {
	switch (onepack->cmd) {
	case 523101: // inner
		sk->type_cn = inner;
		break;
	case 522100: // http connection
		sk->type_cn = http;
		break;
	case 657184: {
			// std::vector<agv>::iterator it;
			// EnterCriticalSection(&cs_agvs);
			// for (it = agvs.begin(); agvs.end() != it; ++it) {
			// if ((*it).addr_b == onepack->ip) {
			// break;
			// }
			// }
			// if (agvs.end() != it) {
			// agvs.erase(it);
			// }
			// LeaveCriticalSection(&cs_agvs);
			break;
		}
	case 657185:
	case 657186: { // disp & register new agv
			sk->type_cn = disp;
			dispatcher = sk->fd;
			DataModule1->reg_agv(onepack);
			break;
		}
	case 657189: { // query the lift is available, disp--->srv
			size_t r = lift1_ready(onepack->ip);
#ifdef _DEBUG
			printf("the lift1's state is %u for %u\n", r, onepack->ip);
#endif
			unsigned char *new_buf = 0;
			dx *new_dx = 0;
			BUILD_DX_BLOCK(new_buf, new_dx, r, onepack->ip, onepack->st, onepack->No, 0, 0);
			EnterCriticalSection(&cs_send2tcpsrv);
			send(dispatcher, new_buf, new_dx->len_pack, 0);
			LeaveCriticalSection(&cs_send2tcpsrv);
			putback_smlbuf(new_buf);
			break;
		}
	case 657192:
		// the lift raises or falls after the agv puts sth. on/takes sth. from the lift, disp--->srv
		lift1_go(onepack->ip);
		break;
	case 523100: { // the lift reached the target floor, srv--->disp
			unsigned char *new_buf = 0;
			dx *new_dx = 0;
#ifdef _DEBUG
			printf("the lift1 has arrived to the %u storey\n", goal_lift1);
#endif
			BUILD_DX_BLOCK(new_buf, new_dx, 657193, onepack->ip, onepack->st, onepack->No, 0, 0);
			EnterCriticalSection(&cs_send2tcpsrv);
			if (SOCKET_ERROR != send(dispatcher, new_buf, new_dx->len_pack, 0)) {
#ifdef _DEBUG
				printf("the srv send 657193 to disp\n");
#endif
			}
			LeaveCriticalSection(&cs_send2tcpsrv);
			putback_smlbuf(new_buf);
			break;
		}
	case 657202: { // waypoint, disp--->srv, 2 seconds
			wp_with_ip a_wp;
			a_wp.wp = *(waypoint*)((unsigned char *)onepack + len_dx);
			a_wp.ip = onepack->ip;

			EnterCriticalSection(&cs_agvs);
			for (std::vector<agv>::iterator it = agvs.begin(); agvs.end() != it; ++it) {
				if ((*it).addr_b == a_wp.ip) {
					(*it).No_cmd = a_wp.wp.t_id;
					break;
				}
			}
			LeaveCriticalSection(&cs_agvs);

			if (agvTASKE == a_wp.wp.state) {
				bool bein = false;
				std::vector<task_issue>::iterator it;
				task_issue_over tio;
				EnterCriticalSection(&cs_tis);
				for (it = tis.begin(); tis.end() != it; ++it) {
					if (a_wp.wp.t_id != (*it).one_pc.t_id);
					else {
						(*it).steps[2] = 1;
						if ((*it).steps[0] && (*it).steps[1] && (*it).steps[2]) {
							(*it).steps[0] = (*it).steps[1] = (*it).steps[2] = 0;
							SecureZeroMemory(&tio, sizeof(tio));
							tio.ti = *it;
							bein = true;
							break;
						}
					}
				}
				if (bein) {
					// Ñ­»·ÖÐÎÞ·¨ÒÆ³ý
					tis.erase(it);
				}
				LeaveCriticalSection(&cs_tis);

				if (bein) {
					EnterCriticalSection(&cs_agvs);
					for (std::vector<agv>::iterator it = agvs.begin(); it != agvs.end(); ++it) {
						if ((*it).addr_b == a_wp.ip) {
							(*it).No_cmd = 0;
							break;
						}
					}
					LeaveCriticalSection(&cs_agvs);
					EnterCriticalSection(&cs_tis_over);
					tis_over.insert(tis_over.begin(), tio);
					incoming = true;
					LeaveCriticalSection(&cs_tis_over);
					WakeConditionVariable(&cv_tis_over);
					// charge(a_wp.ip);
				}
			}
			// else if (agvCHRGFULL == a_wp.wp.state) {
			// bool bein = false;
			// std::vector<task_issue>::iterator it;
			// task_issue_over tio;
			// EnterCriticalSection(&cs_tis);
			// for (it = tis.begin(); tis.end() != it; ++it) {
			// if (a_wp.wp.t_id != (*it).one_pc.t_id);
			// else {
			// (*it).steps[1] = 1;
			// if ((*it).steps[0] && (*it).steps[1]) {
			// (*it).steps[0] = (*it).steps[1] = 0;
			// SecureZeroMemory(&tio, sizeof(tio));
			// bein = true;
			// tio.ti = *it;
			// break;
			// }
			// }
			// }
			// if (bein) {
			// // Ñ­»·ÖÐÎÞ·¨ÒÆ³ý
			// tis.erase(it);
			// }
			// LeaveCriticalSection(&cs_tis);
			// if (bein) {
			// EnterCriticalSection(&cs_agvs);
			// for (std::vector<agv>::iterator it = agvs.begin(); it != agvs.end(); ++it) {
			// if ((*it).addr_b == a_wp.ip) {
			// (*it).No_cmd = 0;
			// (*it).charging = 0;
			// break;
			// }
			// }
			// LeaveCriticalSection(&cs_agvs);
			//
			// EnterCriticalSection(&cs_tis_over);
			// tis_over.insert(tis_over.begin(), tio);
			// incoming = true;
			// LeaveCriticalSection(&cs_tis_over);
			// WakeConditionVariable(&cv_tis_over);
			// }
			// }
			else if (agvTASKB == a_wp.wp.state) {
				EnterCriticalSection(&cs_tis);
				for (std::vector<task_issue>::iterator it = tis.begin(); tis.end() != it; ++it) {
					if (a_wp.wp.t_id != (*it).one_pc.t_id);
					else {
						(*it).steps[0] = 1;
						break;
					}
				}
				LeaveCriticalSection(&cs_tis);
			}
			else if (agvRUNNING == a_wp.wp.state) {
				for (std::vector<task_issue>::iterator it = tis.begin(); tis.end() != it; ++it) {
					if (a_wp.wp.t_id != (*it).one_pc.t_id);
					else {
						(*it).steps[1] = 1;
						break;
					}
				}
				LeaveCriticalSection(&cs_tis);
			}
			// else if (agvCHRG == a_wp.wp.state) {
			// for (std::vector<task_issue>::iterator it = tis.begin(); tis.end() != it; ++it) {
			// if (a_wp.wp.t_id != (*it).one_pc.t_id);
			// else {
			// (*it).steps[0] = 1;
			// break;
			// }
			// }
			// LeaveCriticalSection(&cs_tis);
			// }

			break;
		}
	case 657203: { // battery power, disp--->srv, 60 seconds
			EnterCriticalSection(&cs_agvs);
			for (std::vector<agv>::iterator it_agv = agvs.begin(); agvs.end() != it_agv; ++it_agv)
				if ((*it_agv).addr_b != onepack->ip);
				else {
					(*it_agv).battery = *(u_int*)((unsigned char *)onepack + len_dx);
					(*it_agv).latest = onepack->st;
					break;
				}
			LeaveCriticalSection(&cs_agvs);

			break;
		}
	case 657301: { // the result of task issues, disp--->srv
			result_ti rti = *(result_ti*)((unsigned char *)onepack + len_dx);
			EnterCriticalSection(&cs_tis);
			for (std::vector<task_issue>::iterator it_ti = tis.begin(); tis.end() != it_ti; ++it_ti)
				if ((*it_ti).one_pc.t_id == rti.t_id)
					(*it_ti).r_ti = rti.result;
			LeaveCriticalSection(&cs_tis);
			break;
		}
	case 657400: {

			break;
		}
	default: ;
	}
}

unsigned __stdcall clean_wps(void *p) {
	for (; ;) {
		int p_min = INT_MAX;
		records_reader t_wp;

		Sleep(60000);

		EnterCriticalSection(&cs_rcr_wp);
		MoveMemory(&t_wp, &rcr_wp, sizeof(rcr_wp));
		LeaveCriticalSection(&cs_rcr_wp);

		for (size_t i = 0; i < t_wp.count_readers; ++i)
			if (t_wp.pst[i].new_prev < p_min)
				p_min = t_wp.pst[i].new_prev;

		EnterCriticalSection(&cs_wps);
		if (p_min != INT_MAX)
			wps.erase(wps.begin(), wps.begin() + p_min);
		LeaveCriticalSection(&cs_wps);

		EnterCriticalSection(&cs_rcr_wp);
		for (size_t i = 0; i < rcr_wp.count_readers;) {
			rcr_wp.pst[i].prev -= p_min;
			rcr_wp.pst[i].new_prev -= p_min;
			if (rcr_wp.pst[i].prev != rcr_wp.pst[i].new_prev) {
				rcr_wp.pst[i].prev = rcr_wp.pst[i].new_prev;
				++i;
			}
			else {
				MoveMemory(&(rcr_wp.pst[i]), &(rcr_wp.pst[rcr_wp.count_readers - 1]),
					sizeof(rcr_wp.pst[rcr_wp.count_readers - 1]));
				--rcr_wp.count_readers;
			}
		}
		LeaveCriticalSection(&cs_rcr_wp);
	}
	return 0;
}

size_t __fastcall idle_G(size_t storey) {
	size_t i = UINT_MAX;
	for (i = 0; i < count_cps[1]; ++i) {
		if (cps[storey][i].idle)
			break;
	}
	return i;
}

// call after receiving the task end waypoint
// -1---can not perform charging because of some reasons
// 0---not need charging
// 1---perform charging
int __fastcall charge(size_t ip_agv) {
	bool doit1 = 0;
	int r = 0;
	size_t storey;
	std::vector<agv>::iterator it_agv;
	pair_loc pl;
	EnterCriticalSection(&cs_agvs);
	for (it_agv = agvs.begin(); agvs.end() != it_agv; ++it_agv) {
		if ((*it_agv).addr_b != ip_agv);
		else {
			storey = (*it_agv).storey;
			break;
		}
	}
	if (it_agv == agvs.end()) {
		// ´Ë´¦¶ªÊ§ÁËLeaveCriticalSectionµ¼ÖÂËÀËø
		LeaveCriticalSection(&cs_agvs);
		return -1;
	}
	else {
		agv a = *it_agv;
		if (((*it_agv).battery < 15) && (!(*it_agv).charging)) {
			(*it_agv).charging = 1;
			doit1 = 1;
		}
	}

	LeaveCriticalSection(&cs_agvs);

	if (doit1) {
		SecureZeroMemory(&pl, sizeof(pl));

		EnterCriticalSection(&cs_id_task);
		pl.t_id = gId_task_issue;
		++gId_task_issue;
		gId_task_issue = gId_task_issue ? gId_task_issue : 1;
		LeaveCriticalSection(&cs_id_task);

		pl.tsk = CHRG;
		pl.sn_loc[1][0] = G;
		pl.sn_loc[1][1] = idle_G(storey - 1);
		if (UINT_MAX == pl.sn_loc[1][1]) {
			r = -1;
		}
		else {

			unsigned char *new_buf = 0;
			dx *new_dx = 0;
			pair_loc *pl2 = &pl;
			long ts = time(0);
			BUILD_DX_BLOCK(new_buf, new_dx, 657194, ip_agv, ts, 0, pl2, sizeof(pl));
			EnterCriticalSection(&cs_send2tcpsrv);
			send(dispatcher, new_buf, new_dx->len_pack, 0);
			LeaveCriticalSection(&cs_send2tcpsrv);
			putback_smlbuf(new_buf);
			r = 1;

			task_issue ti;
			SecureZeroMemory(&ti, sizeof(ti));
			ti.r_ti = rti_pending;
			ti.id_warehouse = 1;
			ti.st = ts;
			ti.one_pc = pl;

			EnterCriticalSection(&cs_tis);
			tis.push_back(ti);
			LeaveCriticalSection(&cs_tis);
		}
	}
	return r;
}

unsigned __stdcall task_completed(void *p) {
	std::vector<task_issue_over>copy_unhandled;
	for (; ;) {
		EnterCriticalSection(&cs_tis_over);
		while (!incoming) {
			SleepConditionVariableCS(&cv_tis_over, &cs_tis_over, INFINITE);
		}
		for (std::vector<task_issue_over>::iterator it = tis_over.begin();
		tis_over.end() != it; ++it) {
			if ((*it).handled)
				break;
			else {
				copy_unhandled.push_back(*it);
			}
		}
		incoming = 0;
		LeaveCriticalSection(&cs_tis_over);

		for (std::vector<task_issue_over>::iterator it = copy_unhandled.begin();
		copy_unhandled.end() != it; ++it) {
			task_issue_over t_tio = *it;
			if (I1 == t_tio.ti.one_pc.tsk || I2 == t_tio.ti.one_pc.tsk) {
				// size_t color = colors_unused.front();
				size_t color = TAlphaColors::Sandybrown;
				// size_t color = t_tio.ti.color;
				String name_strloc = Sysutils::Format(_T("A%.*dR%.*dC%.*d"),
					ARRAYOFCONST((2, t_tio.ti.one_pc.sn_loc[1][1], 2, t_tio.ti.one_pc.sn_loc[1][2],
					2, t_tio.ti.one_pc.sn_loc[1][3])));

				DataModule1->result_storloc->SQL->Clear();
				DataModule1->result_storloc->SQL->Text =
					Sysutils::Format(_T("SELECT st FROM ware_storloc WHERE name_storloc = '%s';"),
					ARRAYOFCONST((name_strloc)));
				DataModule1->result_storloc->Execute();

				if (DataModule1->result_storloc->RecordCount) {
					DataModule1->noresult_storloc->SQL->Clear();
					DataModule1->noresult_storloc->SQL->Text =
						Sysutils::Format
						("UPDATE ware_storloc SET packo = '%s', spec = '%s', packi = '%s', company = '%s', st = %u WHERE name_storloc = '%s';",
						ARRAYOFCONST((t_tio.ti.packo, t_tio.ti.spec, t_tio.ti.packi,
						t_tio.ti.company, time(0), name_strloc)));
				}
				else {
					DataModule1->noresult_storloc->SQL->Clear();
					DataModule1->noresult_storloc->SQL->Text =
						Sysutils::Format
						("INSERT INTO ware_storloc (storey, area, row, column, st, name_storloc, packo, spec, packi, company) VALUES(%u, %u, %u, %u, %u,'%s','%s','%s','%s','%s');",
						ARRAYOFCONST((t_tio.ti.one_pc.sn_loc[1][0], t_tio.ti.one_pc.sn_loc[1][1],
						t_tio.ti.one_pc.sn_loc[1][2], t_tio.ti.one_pc.sn_loc[1][3], t_tio.ti.st,
						name_strloc, t_tio.ti.packo, t_tio.ti.spec, t_tio.ti.packi,
						t_tio.ti.company)));
				}

				for (; DataModule1->sqlcn_storloc->InTransaction;);
				DataModule1->sqlcn_storloc->StartTransaction();
				try {
					DataModule1->noresult_storloc->Execute();
					DataModule1->sqlcn_storloc->Commit();
				}
				catch (...) {
					DataModule1->sqlcn_storloc->Rollback();
				}

				EnterCriticalSection(&cs_tis_over);
				for (std::vector<task_issue_over>::iterator it = tis_over.begin();
				tis_over.end() != it; ++it) {
					task_issue_over a_tio = *it;
					if (a_tio.ti.one_pc.t_id == t_tio.ti.one_pc.t_id) {
						(*it).color = color;
						(*it).handled = true;
						++count_handled;
						break;
					}
				}
				LeaveCriticalSection(&cs_tis_over);
			}
			// else if (CHRG == t_tio.ti.one_pc.tsk) {
			// EnterCriticalSection(&cs_tis_over);
			// for (std::vector<task_issue_over>::iterator it = tis_over.begin();
			// tis_over.end() != it; ++it) {
			// task_issue_over a_tio = *it;
			// if (a_tio.ti.one_pc.t_id == t_tio.ti.one_pc.t_id) {
			// (*it).handled = true;
			// ++count_handled;
			// break;
			// }
			// }
			// LeaveCriticalSection(&cs_tis_over);
			// }
			else if (X1 == t_tio.ti.one_pc.tsk || X2 == t_tio.ti.one_pc.tsk ||
				X12 == t_tio.ti.one_pc.tsk || X21 == t_tio.ti.one_pc.tsk) {
				size_t color = TAlphaColors::Sandybrown;
				task_issue_over a_tio;
				String name_storloc1, name_storloc2;
				String packo_org, spec_org, packi_org, company_org;

				name_storloc1 = Sysutils::Format(_T("A%.*dR%.*dC%.*d"),
					ARRAYOFCONST((2, t_tio.ti.one_pc.sn_loc[0][1], 2, t_tio.ti.one_pc.sn_loc[0][2],
					2, t_tio.ti.one_pc.sn_loc[0][3])));
				name_storloc2 = Sysutils::Format(_T("A%.*dR%.*dC%.*d"),
					ARRAYOFCONST((2, t_tio.ti.one_pc.sn_loc[1][1], 2, t_tio.ti.one_pc.sn_loc[1][2],
					2, t_tio.ti.one_pc.sn_loc[1][3])));

				DataModule1->result_storloc->SQL->Clear();
				DataModule1->result_storloc->SQL->Text =
					Sysutils::Format
					(_T("SELECT packo, spec, packi, company FROM ware_storloc WHERE name_storloc = '%s';"),
					ARRAYOFCONST((name_storloc1)));
				DataModule1->result_storloc->Execute();
				if (DataModule1->result_storloc->RecordCount) {
					DataModule1->result_storloc->First();
					packo_org = DataModule1->result_storloc->FieldByName(_T("packo"))->AsString;
					spec_org = DataModule1->result_storloc->FieldByName(_T("spec"))->AsString;
					packi_org = DataModule1->result_storloc->FieldByName(_T("packi"))->AsString;
					company_org = DataModule1->result_storloc->FieldByName(_T("company"))->AsString;
				}
				else
					continue;

				DataModule1->result_storloc->SQL->Clear();
				DataModule1->result_storloc->SQL->Text =
					Sysutils::Format(_T("SELECT st FROM ware_storloc WHERE name_storloc = '%s';"),
					ARRAYOFCONST((name_storloc2)));
				DataModule1->result_storloc->Execute();
				if (DataModule1->result_storloc->RecordCount) {
					DataModule1->noresult_storloc->SQL->Clear();
					DataModule1->noresult_storloc->SQL->Add
						(Sysutils::Format
						("UPDATE ware_storloc SET packo = '%s', spec = '%s', packi = '%s', company = '%s', st = %u WHERE name_storloc = '%s';",
						ARRAYOFCONST((packo_org, spec_org, packi_org, company_org, time(0),
						name_storloc2))));

					DataModule1->noresult_storloc->SQL->Add
						(Sysutils::Format("DELETE FROM ware_storloc WHERE name_storloc = '%s';",
						ARRAYOFCONST((name_storloc1))));
				}
				else {
					DataModule1->noresult_storloc->SQL->Clear();
					DataModule1->noresult_storloc->SQL->Add
						(Sysutils::Format
						("INSERT INTO ware_storloc (storey, area, row, column, st, packo, spec, packi, company, name_storloc) VALUES(%u, %u, %u, %u, %u, '%s', '%s', '%s', '%s', '%s');",
						ARRAYOFCONST((t_tio.ti.one_pc.sn_loc[1][0], t_tio.ti.one_pc.sn_loc[1][1],
						t_tio.ti.one_pc.sn_loc[1][2], t_tio.ti.one_pc.sn_loc[1][3], time(0),
						packo_org, spec_org, packi_org, company_org, name_storloc2))));

					DataModule1->noresult_storloc->SQL->Add
						(Sysutils::Format("DELETE FROM ware_storloc WHERE name_storloc = '%s';",
						ARRAYOFCONST((name_storloc1))));
				}
				for (; DataModule1->sqlcn_storloc->InTransaction;);
				DataModule1->sqlcn_storloc->StartTransaction();
				try {
					DataModule1->noresult_storloc->Execute();
					DataModule1->sqlcn_storloc->Commit();
				}
				catch (...) {
					DataModule1->sqlcn_storloc->Rollback();
				}

				EnterCriticalSection(&cs_tis_over);
				for (std::vector<task_issue_over>::iterator it = tis_over.begin();
				tis_over.end() != it; ++it) {
					a_tio = *it;
					if (a_tio.ti.one_pc.t_id == t_tio.ti.one_pc.t_id) {
						(*it).color = color;
						(*it).handled = true;
						++count_handled;
						break;
					}
				}
				LeaveCriticalSection(&cs_tis_over);
			}
		}
	}
	return 0;
}

unsigned __stdcall cleaning_ti_handled(void *p) {
	for (; ;) {
		Sleep(60000);
		// EnterCriticalSection(&cs_tis_over);
		// std::vector<task_issue_over>::iterator it1 = tis_over.end();
		// if (count_handled > 40) {
		// it1 -= 20;
		// tis_over.erase(it1, tis_over.end());
		// }
		// LeaveCriticalSection(&cs_tis_over);
		// std::vector<task_issue>::iterator it_ti;
		// EnterCriticalSection(&cs_tis);
		// for (it_ti = tis.begin(); tis.end() != it_ti; ++it_ti){
		// task_issue ati = (*it_ti);
		// if(time(0) - ati.st > 600000);
		////			if ((*it_ti).one_pc.t_id == rti.t_id)
		////				(*it_ti).r_ti = rti.result;
		// }
		// LeaveCriticalSection(&cs_tis);
	}
	return 0;
}
