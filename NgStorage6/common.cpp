// ---------------------------------------------------------------------------

#pragma hdrstop

#include "common.h"
#include "MainForm.h"
// ---------------------------------------------------------------------------
#pragma package(smart_init)

const String exe_names[exeOther] = {
	_T("tcpSrv.exe"), _T("httpsSrv.exe"), _T("function.exe"), _T("dbComm.exe"), _T("logOp.exe"),
		_T("NgStorageUI.exe")};

const _TCHAR* params[exeOther] = {
	_T("version_that_has"), _T("mainly_organized_according"), _T("for_strtoul_and_relatives"),
		_T("than_the_original_block"), _T("functions_from_the_first"), _T("demo_shows_when")};

const String proj_names[exeOther] = {
	_T("tcpSrv"), _T("httpsSrv"), _T("function"), _T("dbComm"), _T("logOp"), _T("NgStorageUI")};

const String ports_listening[exeOther] = {
	_T("25258"), _T("25259"), _T("25260"), _T("25261"), _T("25262"), _T("25257")};

const char* cmd_names[cmdOther] = {"quit_all"};

const String log_table_names[other_log_table] = {_T("ss"), _T("dx"), _T("ua")};

const String conffix[confOther] = {
	_T("storey_x"), _T("storey_y"), _T("storey_artery_x"), _T("storey_artery_y")};

const String area_params[area_paramOther] = {
	_T("area_range"), _T("area_light"), _T("area_dark"), _T("area_storey"), _T("area_base_point"),
	_T("area_txt")};

const String specloc_params[speclocOther] = {
	_T("sl_type"), _T("sl_sn"), _T("sl_txt"), _T("sl_range"), _T("sl_storey"), _T("sl_color0"),
		_T("sl_color1"), _T("hittest"), _T("sl_type_alt"), _T("sl_sn_alt"), _T("sl_txt_alt")};
// const String agvtasks[AGVTASKother] = {
// _T("I1"), _T("I2"), _T("U1"), _T("U2"), _T("X1"), _T("X2"), _T("X12"), _T("X21"), _T("CHRG"),
// _T("L1"), _T("L2"), _T("ZERO")};

const String txt_agvtask[AGVTASKother - I1] = {
	_T("1层入库"), _T("2层入库"), _T("1层出库"), _T("2层出库"), _T("1层调货"), _T("2层调货"), _T("1层调货至2层"),
		_T("2层调货至1层"), _T("充电"), _T("至1层升降机"), _T("至2层升降机"), _T("空任务")};

// ips for test/production
const String urls[2] = {_T("127.0.0.1"), _T("192.168.200.21")};
// const String urls[2] = {_T("192.168.200.21"), _T("192.168.200.21")};

const String coords_suffix[7] = {
	_T("x"), _T("y"), _T("axis"), _T("base"), _T("len"), _T("storey"), _T("area")};
size_t num_corrds;

// const wchar_t vehicle_cn_txt[3] = {0x5c0f, 0x8f66, 0x0};
const wchar_t vehicle_cn_txt[4] = {_T('A'), _T('G'), _T('V'), 0x0};

// const size_t group_reg_s[FLAGSPECPACKSEGSother] = {packO_s, spec_s, packI_s, comp_s};
// const size_t group_reg_n[FLAGSPECPACKSEGSother] = {packO_n, spec_n, packI_n, comp_n};

const String spec_package_segments[FLAGSPECPACKSEGSother] = {
	_T("PackageO"), _T("Spec"), _T("PackageI"), _T("Company")};

const size_t group_reg_s[FLAGSPECPACKSEGSother] = {100, 101, 104, 106};
const size_t group_reg_n[FLAGSPECPACKSEGSother] = {102, 103, 105, 107};

const size_t columns_row0available[5] = {19, 18, 10, 9, 8};

const char *ips_plcs[PLCOTHER] = {"192.168.26.211", "192.168.1.22"};
modbus *mb_tcp[PLCOTHER] = {0};

bool worker_goon;
uintptr_t thrd_srv, thrd_worker;
size_t thrd_srv_id;
size_t thrd_worker_id;
size_t thrd_inter_link_id;
size_t thrd_self_link_id;
fd_set master;
SOCKET listener = INVALID_SOCKET;
SOCKET s_self = INVALID_SOCKET;

std::queue<data_r*>q_dr;
SOCKET ary_sock[exeOther] = {INVALID_SOCKET};

CRITICAL_SECTION cs_rcv_data;
CONDITION_VARIABLE cv_rcv_data;

s_data data[exeOther];

HANDLE event_nextagvtask;

// link_header|content_header(4bytes) all_length(4bytes) command[command_value](all_length-8bytes)
const size_t link_header = 0x92E18;
const size_t quit_header = 0xBF914;
const size_t content_header = 0x3FE14;
const size_t heartbeat_header = 0xFFAD5;

bool use_winsock(const int low_byte, const int high_byte) {
	unsigned short version = MAKEWORD(low_byte, high_byte);
	WSADATA wsadata;
	if (WSAStartup(version, &wsadata))
		return false;
	if (LOBYTE(wsadata.wVersion) != low_byte || HIBYTE(wsadata.wVersion) != high_byte) {
		WSACleanup();
		return false;
	}
	return true;
}

void drop_winsock() {
	WSACleanup();
}

size_t be_host_sock(SOCKET i) {
	size_t k = SIZE_MAX;
	for (size_t j = 0; j < exeOther; ++j)
		if (data[j].s == i) {
			k = j;
			break;
		}
	return k;
}

unsigned __stdcall inter_link(void *param) {
	int self;
	int rv;
	struct addrinfo hints, *servinfo, *p;
	SecureZeroMemory(&hints, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	self = (*(int *)param);

	if (getaddrinfo("127.0.0.1", AnsiString(ports_listening[self]).c_str(), &hints, &servinfo))
		goto END;

	for (p = servinfo; p; p = p->ai_next) {
		if ((data[self].s = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == INVALID_SOCKET)
			continue;

		if (SOCKET_ERROR == connect(data[self].s, p->ai_addr, p->ai_addrlen)) {
			closesocket(data[self].s);
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo);
END:
	return 0;
}

unsigned __stdcall self_link(void *param) {
	int compon_id;
	int rv;
	struct addrinfo hints, *servinfo, *p;
	char self_link_cmd[12] = {0};

	SecureZeroMemory(&hints, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	compon_id = (*(int *)param);

	if (getaddrinfo("127.0.0.1", AnsiString(ports_listening[compon_id]).c_str(), &hints, &servinfo))
		goto END;

	for (p = servinfo; p; p = p->ai_next) {
		if ((s_self = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == INVALID_SOCKET)
			continue;

		if (SOCKET_ERROR == connect(s_self, p->ai_addr, p->ai_addrlen)) {
			closesocket(s_self);
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo);
	if (!p)
		goto END;

	*(int *)&(self_link_cmd[0]) = link_header;
	*(int *)&(self_link_cmd[8]) = compon_id;

	send(s_self, &(self_link_cmd[0]), 12, 0);

END:
	return 0;
}

unsigned __stdcall connect_who(void *param) {
	int rv;
	conn_pair *cp;
	SOCKET s;
	struct addrinfo hints, *servinfo, *p;
	char link_cmd[12] = {0};

	SecureZeroMemory(&hints, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	cp = (conn_pair*)param;

	if (getaddrinfo("127.0.0.1", AnsiString(ports_listening[cp->who]).c_str(), &hints, &servinfo))
		goto END;

	for (p = servinfo; p; p = p->ai_next) {
		if (INVALID_SOCKET == (s = socket(p->ai_family, p->ai_socktype, p->ai_protocol)))
			continue;

		if (SOCKET_ERROR == connect(s, p->ai_addr, p->ai_addrlen)) {
			closesocket(s);
			continue;
		}

		break;
	}

	freeaddrinfo(servinfo);
	if (!p)
		goto END;

	ary_sock[cp->who] = s;

	*(int *)&(link_cmd[0]) = link_header;
	*(int *)&(link_cmd[8]) = cp->me;

	send(s, &(link_cmd[0]), 12, 0);
END:
	return 0;
}

void find_maxfd(SOCKET * maxfd, fd_set * s) {
	SOCKET t_maxfd = s->fd_array[0];
	for (size_t i = 0; i < s->fd_count - 1; ++i)
		t_maxfd = (s->fd_array[i + 1] > t_maxfd) ? s->fd_array[i + 1] : t_maxfd;
	*maxfd = t_maxfd;
}

size_t write_content(char *a, char *b, size_t len) {
	size_t len_total;
	*(int *)a = content_header;
	len_total = *(int *)(a + 4) = 8 + len;
	MoveMemory(a + 8, b, len);
	return len_total;
}

void write_link(char *a, int *whoami) {
	*(int *)a = link_header;
	MoveMemory(a + 8, (char *)whoami, sizeof*whoami);
}

void init_data_r(data_r * rd) {
	rd->from = SIZE_MAX;
	rd->s = INVALID_SOCKET;
	rd->nbytes = SIZE_MAX;
	SecureZeroMemory(&(rd->buf[0]), MIDDLE * (sizeof(char)));
}

void __fastcall cpu_nums(size_t * n) {
	SYSTEM_INFO sysinfo;
	GetSystemInfo(&sysinfo);
	*n = sysinfo.dwNumberOfProcessors;
}

unsigned __stdcall display_offline_status(void *p) {
	int v = *(int *)p;
	ThreadUpdateOffline *thrd_uo = new ThreadUpdateOffline(v);
	// thrd_uo->Terminate();
	thrd_uo->WaitFor();
	// thrd_uo->CheckTerminated();
	thrd_uo->DisposeOf();
	return 0;
}

void __fastcall connect_modbus() {
#ifndef _ATHOME
	for (size_t k = plcproduct1; k < PLCOTHER; ++k) {
		mb_tcp[k] = new modbus(ips_plcs[k], 502);
		mb_tcp[k]->modbus_set_slave_id(1);
		mb_tcp[k]->modbus_connect();
	}
#endif
}

void __fastcall disconnect_modbus() {
#ifndef _ATHOME
	for (size_t k = plcproduct1; k < PLCOTHER; ++k) {
		mb_tcp[k]->modbus_close();
		delete mb_tcp[k];
	}
#endif
}

rep_spec_pack __fastcall modbus_read_product_info_s() {
	unsigned short status[FLAGSPECPACKSEGSother] = {0, 0, 0, 0};
	rep_spec_pack rsp;
	SecureZeroMemory(&rsp, sizeof(rsp));
	for (size_t i = 0; i < FLAGSPECPACKSEGSother; ++i) {
		mb_tcp[plcproduct1]->modbus_read_holding_registers(group_reg_s[i], 1, &(status[i]));
	}

	AnsiString a_packo = AnsiString(Form1->records_sp.arys[packageo][status[packageo]]);
	AnsiString a_spec = AnsiString(Form1->records_sp.arys[spec][status[spec]]);
	AnsiString a_packi = AnsiString(Form1->records_sp.arys[packagei][status[packagei]]);
	AnsiString a_company = AnsiString(Form1->records_sp.arys[company][status[company]]);

	MoveMemory(rsp.packo, a_packo.c_str(), a_packo.Length());
	MoveMemory(rsp.spec, a_spec.c_str(), a_spec.Length());
	MoveMemory(rsp.packi, a_packi.c_str(), a_packi.Length());
	MoveMemory(rsp.company, a_company.c_str(), a_company.Length());

	return rsp;
}

void __fastcall modbus_read_product_info_s2(unsigned short *product_info) {
	for (size_t i = 0; i < FLAGSPECPACKSEGSother; ++i) {
		mb_tcp[plcproduct1]->modbus_read_holding_registers(group_reg_s[i], 1, &(product_info[i]));
	}
}

void __fastcall modbus_read_product_info_n2(unsigned short *product_info) {
	for (size_t i = 0; i < FLAGSPECPACKSEGSother; ++i) {
		mb_tcp[plcproduct1]->modbus_read_holding_registers(group_reg_n[i], 1, &(product_info[i]));
	}
}

String get_packo(bool *cn, size_t index) {
	*cn = (index < Form1->records_sp.num[packageo]) ? 1 : 0;
	return (Form1->records_sp.arys[packageo][index]);
}

String get_spec(size_t index) {
	return (Form1->records_sp.arys[spec][index]);
}

String get_packi(size_t index) {
	return (Form1->records_sp.arys[packagei][index]);
}

String get_company(size_t index) {
	return (Form1->records_sp.arys[company][index]);
}

rep_spec_pack __fastcall modbus_read_product_info_n() {
	unsigned short status[FLAGSPECPACKSEGSother] = {0, 0, 0, 0};
	rep_spec_pack rsp;
	SecureZeroMemory(&rsp, sizeof(rsp));
	for (size_t i = 0; i < FLAGSPECPACKSEGSother; ++i) {
		mb_tcp[plcproduct1]->modbus_read_holding_registers(group_reg_n[i], 1, &(status[i]));
	}

	AnsiString a_packo = AnsiString(Form1->records_sp.arys[packageo][status[packageo]]);
	AnsiString a_spec = AnsiString(Form1->records_sp.arys[spec][status[spec]]);
	AnsiString a_packi = AnsiString(Form1->records_sp.arys[packagei][status[packagei]]);
	AnsiString a_company = AnsiString(Form1->records_sp.arys[company][status[company]]);

	MoveMemory(rsp.packo, a_packo.c_str(), a_packo.Length());
	MoveMemory(rsp.spec, a_spec.c_str(), a_spec.Length());
	MoveMemory(rsp.packi, a_packi.c_str(), a_packi.Length());
	MoveMemory(rsp.company, a_company.c_str(), a_company.Length());

	return rsp;
}

bool __fastcall inavailable_storloc(storloc *s) {
	bool available = 1;
	if (s->sn_flag[1]) {
		if (2 == s->sn_flag[1]) { // area 2
			if (1 == s->sn_flag[3] || 3 == s->sn_flag[3]) {
				if (!(s->txt.operator == (_T("A02R04C01")) || s->txt.operator == (_T("A02R04C03"))))
					available = 0;
			}
		}
		else if (3 == s->sn_flag[1]) { // area 3
			if (s->txt.operator == (_T("A03R07C00")))
				available = 0;
		}
		else if (1 == s->sn_flag[1]) { // area 1
			if (!s->sn_flag[3])
				available = 0;
		}
	}
	else { // area 0
		size_t c = s->sn_flag[3];
		size_t r = s->sn_flag[2];
		size_t i = 0;
		for (; i < 5; ++i) {
			if (c == columns_row0available[i])
				break;
		}
		if (5 != i) {
			if (19 == c || 18 == c) {
				available = 0;
			}
		}
		else {
			// if ((c > 4 && r < 3) || s->txt.operator == (_T("A00R03C12"))) {
			// available = 0;
			// }
			if ((c == 12) && (r < 4)) {
				available = 0;
			}
		}
		if ((8 == r) && (10 == c || 9 == c || 8 == c)) {
			available = 0;
		}
	}
	return available;
}

bool columnDESC(column_storloc c1, column_storloc c2) {
	return (c1.c > c2.c);
}

bool columnASC(column_storloc c1, column_storloc c2) {
	return (c1.c < c2.c);
}

bool rowASC(storlco_wareinfo r1, storlco_wareinfo r2) {
	return (r1.r < r2.r);
}

bool areaASC(area_storloc a1, area_storloc a2) {
	return (a1.area < a2.area);
}

void __fastcall fix_task_type(agvtask_view *one_av) {
	// if ((I1 == one_av->pc.tsk) && (2 == one_av->pc.sn_loc[1][0])) {
	// one_av->pc.tsk = I2;
	// one_av->pc.type_product = 2;
	// r = 2;
	// }else if((I2 == one_av->pc.tsk) && (1 == one_av->pc.sn_loc[1][0]) && (1!=one_av->pc.sn_loc[1][1])){
	// one_av->pc.tsk = I1;
	// one_av->pc.type_product = 1;
	// r = 1;
	// }
	// if (1 == one_av->pc.sn_loc[1][1] || 3 == one_av->pc.sn_loc[1][1]) {
	// one_av->pc.type_product = 2;
	// }
	if ((I1 == one_av->pc.tsk) || (I2 == one_av->pc.tsk)) {
		switch (one_av->pc.sn_loc[1][1]) {
		case 1:
		case 3:
			one_av->pc.type_product = 2;
			break;
		case 0:
			one_av->pc.type_product = 1;
			break;
		default: ;
		}
	}
}
