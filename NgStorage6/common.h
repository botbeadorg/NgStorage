// ---------------------------------------------------------------------------

#ifndef commonH
#define commonH
// ---------------------------------------------------------------------------

#include <System.Classes.hpp>
#include <windows.h>
#include <synchapi.h>
#include <Winsock2.h>
#include <Ws2tcpip.h>
#include <Mswsock.h>
#include <queue>
#include <System.SysUtils.hpp>
#include <set>
#include <vector>
#include <FMX.Controls.hpp>
#include <FMX.Objects.hpp>
#include <algorithm>
#include "dispatcher.h"
#include "modbus.h"

enum ind_exe_names {
	tcpSrv, httpsSrv, function, dbComm, logOp, UI, exeOther
};

enum cmds {
	quit_all, cmdOther
};

enum buf_sizes {
	SMALL = 4096, MIDDLE = 8192, BIG = 16384
};

enum prefix_log_table_name {
	srv_state, data_exchange, usr_action, other_log_table
};

enum indx_loc_base {
	x, y, storey, locOther
};

enum indx_conf {
	storey_x, storey_y, storey_artery_x, storey_artery_y, confOther
};

enum indx_artery {
	bx, by, ex, ey, storey_artery, axis, arteryOther
};

enum indx_axis {
	axisX, axisY, axisNone
};

enum indx_cmds {
	loc_stor, highway, job, sched, loc_vehicle, tactic, interf_cmdOther
};

enum indx_area {
	area_range, area_light, area_dark, area_storey, area_base_point, area_txt, area_paramOther
};

enum indx_specloc {
	sl_type, sl_sn, sl_txt, sl_range, sl_storey, sl_color0, sl_color1, hittest, sl_type_alt,
	sl_sn_alt, sl_txt_alt, speclocOther
};

enum indx_loc_control_type {
	ctrl_specloc, ctrl_storloc, loc_controlOther
};

enum result_task_issue {
	rti_failure, rti_success, rti_pending, RTIOTHER
};

enum flag_registers {
	packO_s = 100, spec_s, packO_n, spec_n, packI_s, packI_n, comp_s, comp_n, FLAGREGISTERSother
};

enum flag_spec_pack_segs {
	packageo, spec, packagei, company, FLAGSPECPACKSEGSother
};

enum myplcs {
	plcproduct1, plcplatelinkchain1, PLCOTHER
};

struct area {
	size_t No_area;
	size_t color_empty, color_occupied;
	size_t base_point[2];
	size_t range[axis];
	String txt;
};

struct specloc {
	bool hittest;
	bool signal;
	size_t pnt_control;
	size_t sn_flag[sl_txt];
	size_t sn_flag_alt[sl_txt];
	size_t color[sl_txt];
	size_t range[axis];
	String txt;
	String txt_alt;
};

struct storloc {
	bool full;
	size_t pnt_control;
	size_t color[sl_txt];
	int map_xyz[3]; // x,y,z
	size_t sn_flag[4]; // storey, area, r, c
	String txt;
	rep_storloc ware_info;
};

struct storloc_net {
	int map_xyz[3]; // x,y,z
	size_t sn_flag[4]; // storey, area, r, c
	char txt[16];
};

struct slcted_ctrl_old {
	size_t type;
	TCircle *c;
	TRectangle *r;
};

struct agvtask_view {
	size_t r_ti;
	int indx_s1, indx_s2;
	int indx_agv1, indx_agv2;
	TLine *l;
	pair_loc pc;
	slcted_ctrl_old s;
	slcted_ctrl_old e;
	size_t status1[16];
	size_t status2[16];
};

struct agvtask_listitem {
	int row_number;
	size_t state_running; // 0-unknow, 1-running, 2-finished
	agvtask_view v;
	String txts_listitems[GRIDHEADother];
};

struct running_status_taskissue {
	size_t state_running; // 0-unknow, 1-running, 2-finished
	size_t t_id;
	size_t color;
};

struct coords {
    int area;
	size_t x, y;
#define MYX 0
#define MYY 1
	size_t axis;
	size_t base;
	size_t len;
	size_t storey;
};

class ThreadUpdateAgview : public TThread {
protected:
	void __fastcall Execute();

public:
	__fastcall ThreadUpdateAgview(agvtask_view *agview);
	__fastcall ThreadUpdateAgview();
	__fastcall ~ThreadUpdateAgview();

private:
	void __fastcall update_agvtask_view();

	agvtask_view *agview;
};

class ThreadUpdateOffline : public TThread {
protected:
	void __fastcall Execute();

public:
	__fastcall ThreadUpdateOffline(int ofl);
	__fastcall ThreadUpdateOffline();
	__fastcall ~ThreadUpdateOffline();

private:
	void __fastcall update_offline_status();

	int offline;
};

typedef char small_buf[SMALL];
typedef char middle_buf[MIDDLE];
typedef char big_buf[BIG];
typedef size_t loc_base[locOther];
typedef size_t artery[arteryOther];
typedef DynamicArray<DynamicArray<DynamicArray<uintptr_t> > >matrix;
typedef DynamicArray<DynamicArray<uintptr_t> >matrix1;

struct pack_loc_base {
	loc_base lb;
};

struct pack_artery {
	artery a;
};

struct data_io {
	size_t nbytes;
	middle_buf buf;
};

struct data_r {
	size_t from;
	size_t nbytes;
	SOCKET s;
	void * handle;
	middle_buf buf;
};

struct s_data {
	SOCKET s;

	std::queue<data_io*>qr;
	std::queue<data_io*>qs;

	CRITICAL_SECTION cs_qr;
	CRITICAL_SECTION cs_qs;

	CONDITION_VARIABLE cv_qr;
	CONDITION_VARIABLE cv_qs;
};

struct conn_pair {
	int who;
	int me;
};

struct loc_ware {
	size_t reserved;
	loc_base loc;
	String order_number, transport_number, material_number;
};

struct conf_profile {
	std::vector<pack_loc_base>storeys;
	std::vector<pack_artery>arteries;
	std::vector<size_t>used_colors;
	std::vector<size_t>all_colors;
	std::vector<area>areas;

	specloc *sl_ary;
	size_t len_sl_ary;

	specloc *sl_t_ary;
	size_t len_sl_t_ary;
};

struct result_ti {
	size_t t_id;
	size_t result;
};

struct records_spec_pack {
	size_t num[FLAGSPECPACKSEGSother];
	size_t divider[FLAGSPECPACKSEGSother];
	String *arys[FLAGSPECPACKSEGSother];
};

struct rep_spec_pack {
	char packo[32];
	char spec[32];
	char packi[32];
	char company[32];
};

struct rep_spec_pack_with_color {
	rep_spec_pack rep_sp;
	unsigned color;
};

struct storlco_wareinfo {
	bool in_task;
	size_t r;
	unsigned spec_pack_info[FLAGSPECPACKSEGSother];
	storloc_net sn;
	rep_spec_pack rsp;
};

struct column_storloc {
	bool in, out;
	size_t storey, area, c;

	std::vector<storlco_wareinfo>rows;
};

struct area_storloc {
	int area;

	std::vector<column_storloc>columns;
};

// struct dx {
// int header;
// size_t len;
// time_t t;
// char cmd;
// small_buf buf;
// };

class TMySelection : public Fmx::Objects::TSelection {
protected:
	void __fastcall DrawHandle(Fmx::Graphics::TCanvas* const Canvas,
		const Fmx::Objects::TSelection::TGrabHandle Handle, const System::Types::TRectF &Rect);

public:
	__fastcall TMySelection(System::Classes::TComponent* AOwner);
	__fastcall ~TMySelection();
};

#define SET_MAXFD(x) \
do{\
	FD_SET((x), &master); \
	fdmax = ((x) > fdmax) ? (x) : fdmax; \
}while(0)

#ifdef _DEBUG
#define OUTPUT_DEBUGSTR(x) \
do{ \
	String dbg_str = _T("\nNGSTORAGE::::::::"); \
	dbg_str.operator += ((x)); \
	dbg_str.operator += (_T("\n")); \
	OutputDebugString(dbg_str.c_str()); \
}while(0)
#else
#define OUTPUT_DEBUGSTR(x)
#endif

#define OUTPUT_SRVMSG(x) \
do{ \
	String dbg_str; \
	DateTimeToString(dbg_str, String(_T("[yyyy/mm/dd-hh:nn:ss.zzz]\t")), Now());\
	Format(_T("%s"), ARRAYOFCONST((dbg_str)));\
	dbg_str.operator += ((x)); \
	TMessageManager::DefaultManager->SendMessage(0, \
	new TMessage__1<UnicodeString>(dbg_str), true); \
}while(0)

#define HTTPPORT 80
#define HTTPSPORT 443
#define SSLPWD _T("10174<-RTSGN.hyW|daebtob")

extern const String exe_names[exeOther];
extern const String proj_names[exeOther];
extern const String ports_listening[exeOther];
extern const char* cmd_names[cmdOther];
extern const _TCHAR* params[exeOther];
extern const String log_table_names[other_log_table];
extern const String conffix[confOther];
extern const wchar_t vehicle_cn_txt[4];
extern const String area_params[area_paramOther];
extern const String specloc_params[speclocOther];
extern const String txt_agvtask[AGVTASKother - I1];

extern TIntegerSet set_conn;

extern void *event_off;

extern uintptr_t thrd_select;

extern s_data data[exeOther];

extern const size_t link_header;
extern const size_t quit_header;
extern const size_t content_header;

extern bool worker_goon;
extern uintptr_t thrd_srv, thrd_worker;

extern size_t thrd_srv_id;
extern size_t thrd_worker_id;
extern size_t thrd_inter_link_id;
extern size_t thrd_self_link_id;
extern fd_set master;
extern SOCKET listener;
extern SOCKET s_self;

extern std::queue<data_r*>q_dr;
extern std::queue<dx*>q_dx;
extern SOCKET ary_sock[exeOther];

extern CRITICAL_SECTION cs_rcv_data;
extern CONDITION_VARIABLE cv_rcv_data;

extern const size_t group_reg_s[FLAGSPECPACKSEGSother];
extern const size_t group_reg_n[FLAGSPECPACKSEGSother];
extern const String spec_package_segments[FLAGSPECPACKSEGSother];

extern const String coords_suffix[7];
extern size_t num_corrds;

extern const String urls[2];
extern HANDLE event_nextagvtask;

extern const char *ips_plcs[PLCOTHER];
extern modbus *mb_tcp[PLCOTHER];

extern bool use_winsock(const int, const int);
extern void drop_winsock();

extern unsigned __stdcall display_offline_status(void *);
extern void __fastcall connect_modbus();
extern void __fastcall disconnect_modbus();
extern rep_spec_pack __fastcall modbus_read_product_info_s();
extern rep_spec_pack __fastcall modbus_read_product_info_n();
extern void __fastcall modbus_read_product_info_s2(unsigned short *);
extern void __fastcall modbus_read_product_info_n2(unsigned short *);
extern bool __fastcall inavailable_storloc(storloc *);
extern bool columnDESC(column_storloc, column_storloc);
extern bool columnASC(column_storloc, column_storloc);
extern bool rowASC(storlco_wareinfo, storlco_wareinfo);
extern bool areaASC(area_storloc, area_storloc);

extern String get_packo(bool *, size_t);
extern String get_spec(size_t);
extern String get_packi(size_t);
extern String get_company(size_t);

extern void __fastcall fix_task_type(agvtask_view *);

size_t be_host_sock(SOCKET);
void find_maxfd(SOCKET *, fd_set *);
unsigned __stdcall inter_link(void *);
unsigned __stdcall self_link(void *);
unsigned __stdcall connect_who(void *);

size_t write_content(char *, char *, size_t);
void write_link(char *, int *);

void init_data_r(data_r *);
void __fastcall cpu_nums(size_t*);

#endif
