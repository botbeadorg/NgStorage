// ---------------------------------------------------------------------------

#ifndef commonH
#define commonH
// ---------------------------------------------------------------------------

#include <vcl.h>
#include <stdio.h>
#include <time.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <windows.h>
#include <process.h>
#include <limits>
#include <vector>
#include <list>
#include <boost/crc.hpp>
#include "modbus.h"

#define PORT_LISTENING "29998"
#define NS_IN6ADDRSZ 16
#define NS_INT16SZ 2
#define LEN_BUF_RCV 17408
#define LEN_SMLBUF 4096

enum type_conn {
	inner = 1, disp, http, TYPECONNOTHER
};

enum plcs {
	plclift1, plcpcm, PLCOTHER
};

// s——south, n——north, f——foreign
enum info_product {
	spec_s, count_stack_fs = 3, batchno_fs0 = 10, batchno_fs1 = 11, batchno_fs2 = 4, batchno_fs3 =
		5, batchno_s0 = 20, batchno_s1 = 21, batchno_s2 = 6, batchno_s3 = 7, spec_n = 30,
	count_stack_fn = 33, batchno_fn0 = 40, batchno_fn1 = 41, batchno_fn2 = 34, batchno_fn3 = 35,
	batchno_n0 = 50, batchno_n1 = 51, batchno_n2 = 36, batchno_n3 = 37, home = 100, foreign = 101,
	INFOPRODUCTother
};

enum speclocflag {
	B = 100000, D, F, G, L, O, P, Q, T, W, SPECLOCFLAGother
};

enum agvtasktype {
	I1 = 5000, I2, U1, U2, X1, X2, X12, X21, CHRG, L1, L2, ZERO, AGVTASKTYPEother
};

enum forkact {
	faNone, faUp, faDown, FAother
};

enum agvtask_state {
	agvTASKB, agvTASKE, agvRUNNING, agvCHRG, agvCHRGFULL, AGVTASKSTATEother
};

enum result_task_issue {
	rti_failure, rti_success, rti_pending, RTIOTHER
};

enum group_reg {
	package_o, spec, package_i, GROUP_REGother
};

struct pack_txt_addr_v4 {
	typedef char txt_addr_v4[16];
	txt_addr_v4 ip_v4;
};

struct sock_kit {
	int type_cn;
	SOCKET fd;
	size_t buf_offset;
	unsigned char buf_rcv[LEN_BUF_RCV];
};

struct dx { // 32 bytes
	size_t head[3]; // UINT_MAX
	size_t len_pack; // 整包长度
	size_t No; // cmd编号
	size_t cmd; // 命令标识
	size_t ip; // ip地址
	time_t st; // unix时间戳
};

struct pair_loc {
	size_t t_id; // 任务编号
	size_t tsk; // 任务类型(agvtask)
	size_t type_product; // 0忽略, 1小, 2大
	size_t sn_loc[2][4]; // (层,区,行,列)元组或[][0]特殊点枚举值，[][1]特殊点编码
};

struct task_issue {
	size_t r_ti;
	u_int id_client, id_warehouse;
	u_int st;
	pair_loc one_pc;
	bool steps[3];
	// extra
	unsigned color;
	String packo, spec, packi, company;
};

struct task_issue_over {
	bool handled;
	size_t color;
	task_issue ti;
};

struct waypoint {
	size_t t_id; // 路点所属任务
	size_t state; // agvtask_state枚举
	size_t act_load; // 0忽略,1取货,2卸货
	int coord_loc[2]; // x/y坐标
	size_t sn_loc[4]; // (层,区,行,列)元组或[0]特殊点枚举值，[1]特殊点编码
};

struct wp_with_ip {
	size_t ip;
	waypoint wp;
};

struct agv {
	bool charging;
	int id;
	size_t id_warehouse;
	size_t storey;
	size_t No_cmd;
	size_t battery;
	time_t latest;
	u_long addr_b;
	char name[8];
};

struct agvx {
	int id;
	size_t id_warehouse;
	size_t storey;
	u_long addr_b;
	char name[8];
};

struct ext_agvx {
	bool lowbattery;
	size_t id_ti;
	size_t battery;
	time_t latest;
};

struct client_read {
	u_int id_client;
	int prev, new_prev;
};

struct records_reader {
	size_t count_readers;
	client_read pst[64];
};

struct quant_elec_agv {
	u_long addr_b_agv;
	u_int quant;
};

struct result_ti {
	size_t t_id;
	size_t result;
};

struct rep_storloc {
	size_t color, st, spec, sn_stack, count_stack;
	size_t loc[5]; // warehouse, storey, area, row, column
	unsigned char txt[12]; // the text of loc
};

struct rep_spec_pack {
	char packo[32];
	char spec[32];
	char packi[32];
	char company[32];
};

struct site_client {
	u_int id_client;
	unsigned http_common, http_wp, http_charg;
	unsigned char url[32];
};

struct charge_pill {
	size_t idle; // 0,1
	size_t flag_G;
	size_t sn;
};

struct running_status_taskissue {
	size_t state_running; // 0-unknow, 1-running, 2-finished
	size_t t_id;
	size_t color;
};

extern const int home_foreign_delta;

extern const size_t len_dx, len_size_t, len_crc32;

extern HANDLE event_lift1, event_lift2;
extern size_t goal_lift1, goal_lift2;

extern SOCKET listener, start_conn_self, dispatcher, httpcn;

extern const char *ips_plcs[PLCOTHER];
extern modbus_t *ctx_tcp_modbus[PLCOTHER];

extern u_int gId_client;
extern CRITICAL_SECTION cs_id_client;

extern size_t gId_task_issue;
extern CRITICAL_SECTION cs_id_task;

extern size_t quota_agvs;

extern String path_db;

extern std::vector<sock_kit*>bus_conn;

extern std::vector<agv>agvs;
extern CRITICAL_SECTION cs_agvs;

extern std::vector<wp_with_ip>wps;
extern CRITICAL_SECTION cs_wps;

extern records_reader rcr_wp;
extern CRITICAL_SECTION cs_rcr_wp;

extern std::vector<task_issue>tis;
extern CRITICAL_SECTION cs_tis;

extern bool incoming;
extern unsigned count_handled;
extern std::vector<task_issue_over>tis_over;
extern CRITICAL_SECTION cs_tis_over;
extern CONDITION_VARIABLE cv_tis_over;

extern CRITICAL_SECTION cs_send2tcpsrv;

extern const size_t colors_available[135];
extern std::list<size_t>colors_unused;
extern std::list<size_t>colors_used;

extern charge_pill cps[2][8];
extern size_t count_cps[2];

extern const size_t cn_f_index_divider;
extern const String type_txt_prods_db[73];
extern const size_t group_reg_s[4];
extern const size_t group_reg_n[4];
extern const String query_prods;

extern void __fastcall get_smlbuf(unsigned char* *);
extern void __fastcall putback_smlbuf(unsigned char *);
extern void __fastcall recycle_smlbuf(unsigned char *);
extern void* __fastcall get_in_addr(sockaddr *);
extern void __fastcall get_in_addr4(u_long *, sockaddr *);
extern uint32_t __fastcall my_crc32(char *, size_t);
extern size_t __fastcall lift1_ready(size_t);
extern void __fastcall lift1_go(size_t);
extern unsigned __stdcall timer_lift1(void *);
extern unsigned __stdcall self_conn(void *);
extern unsigned __stdcall http_conn(void *);
extern void __fastcall data_handler(sock_kit *, dx*);
extern unsigned __stdcall clean_wps(void *);
extern int __fastcall charge(size_t);
extern unsigned __stdcall task_completed(void *);
extern unsigned __stdcall cleaning_ti_handled(void *);

#define BUILD_DX_BLOCK(p, p_dx, _cmd, _ip_int, _st, _No, p_data_ex, len_data_ex)\
do {\
	uint32_t v_crc32;\
	get_smlbuf(&(p));\
	(p_dx) = (dx*)(p);\
	(p_dx)->head[0] = (p_dx)->head[1] = (p_dx)->head[2] = UINT_MAX;\
	(p_dx)->cmd = (_cmd);\
	if ((_ip_int))\
		(p_dx)->ip = (_ip_int);\
	if ((_st))\
		(p_dx)->st = (_st);\
	if ((_No))\
		(p_dx)->No = (_No);\
	(p_dx)->len_pack = len_dx + len_crc32 + (len_data_ex);\
	if(p_data_ex)MoveMemory((p) + len_dx, (char *)(p_data_ex), (len_data_ex));\
	v_crc32 = my_crc32((char *)(p), len_dx + (len_data_ex));\
	*(uint32_t*)((p) + len_dx + (len_data_ex)) = v_crc32;\
}\
while (0)

#define TEST_CRC32(result_test, p_data, len_data)\
do {\
	uint32_t crc32_test = 0;\
	uint32_t crc32_got = 0;\
	size_t len_data_without_crc32 = (len_data) - len_crc32;\
	crc32_test = my_crc32((char *)(p_data), len_data_without_crc32);\
	crc32_got = *(uint32_t*)((char *)(p_data) + len_data_without_crc32);\
	(result_test) = (crc32_test == crc32_got);\
}\
while (0)

#define SELF_CONN(sock)\
do {\
	int r;\
	addrinfo hints, *servinfo, *p;\
	memset(&hints, 0, sizeof(hints));\
	hints.ai_family = AF_INET;\
	hints.ai_socktype = SOCK_STREAM;\
	(sock) = INVALID_SOCKET;\
	try {\
		if (r = getaddrinfo("localhost", PORT_LISTENING, &hints, &servinfo)) {\
			printf("self-conn getaddrinfo() failed: %s\n", r);\
			break;\
		}\
		for (p = servinfo; p; p = p->ai_next) {\
			if (INVALID_SOCKET == ((sock) = socket(p->ai_family, p->ai_socktype, p->ai_protocol))) {\
				printf("self-conn socket() failed\n");\
				continue;\
			}\
			if (SOCKET_ERROR == connect((sock), p->ai_addr, p->ai_addrlen)) {\
				closesocket((sock));\
				(sock) = INVALID_SOCKET;\
				printf("self-conn connect() failed\n");\
				continue;\
			}\
			break;\
		}\
		if (!p) {\
			printf("self-conn can't establish a new connection\n");\
			freeaddrinfo(servinfo);\
			break;\
		}\
		freeaddrinfo(servinfo);\
	}\
	catch (...) {\
		printf("self-conn failed\n");\
	}\
}\
while (0)

#define SET_SOCK_KIT(p_sock_kit, sock)\
do {\
	char *addr_txt = 0;\
	unsigned id_thrd;\
	char remoteIP[INET6_ADDRSTRLEN];\
	try {\
		(p_sock_kit) = new sock_kit;\
		memset((p_sock_kit), 0, sizeof(sock_kit));\
		(p_sock_kit)->type_cn = 0;\
		(p_sock_kit)->fd = (sock);\
		(p_sock_kit)->buf_offset = 0;\
	}\
	catch (...) {\
		printf("the socket %d can't work n", (sock));\
	}\
}\
while (0)

#define CRC_ERROR(a_sk)\
do {\
	unsigned char *new_buf = 0;\
	dx *new_dx = 0;\
	BUILD_DX_BLOCK(new_buf, new_dx, 657300, 0, 0, 0, 0, 0);\
	EnterCriticalSection(&cs_send2tcpsrv);\
	send(dispatcher, new_buf, new_dx->len_pack, 0);\
    LeaveCriticalSection(&cs_send2tcpsrv);\
	putback_smlbuf(new_buf);\
}\
while (0)

#define UI_CONVERT_ERR_RESP(str, i_str, ec, err_resp)\
do {\
	err = TryStrToUInt((str), (i_str));\
	if (err);\
	else {\
		err_code = (ec);\
		err_str = (err_resp);\
		goto ERR_END;\
	}\
}\
while (0)

#define I_CONVERT_ERR_RESP(str, i_str, ec, err_resp)\
do {\
	err = TryStrToInt((str), (i_str));\
	if (err);\
	else {\
		err_code = (ec);\
		err_str = (err_resp);\
		goto ERR_END;\
	}\
}\
while (0)

#define I64_CONVERT_ERR_RESP(str, i_str, ec, err_resp)\
do {\
	err = TryStrToInt64((str), (i_str));\
	if (err);\
	else {\
		err_code = (ec);\
		err_str = (err_resp);\
		goto ERR_END;\
	}\
}\
while (0)

#define txtLocToInt(ary2d, txtPairLoc)\
do {\
	int p = (txtPairLoc).Pos(";");\
	String s_t = (txtPairLoc).SubString(1, p - 1);\
	for (size_t j = 0; j < 3; ++j) {\
		int k = s_t.Pos(',');\
		(ary2d)[0][j] = (s_t.SubString(1, k - 1)).ToInt();\
		s_t.Delete(1, k);\
	}\
	(ary2d)[0][3] = s_t.ToInt();\
	s_t = (txtPairLoc).SubString(p + 1, (txtPairLoc).Length() - p);\
	for (size_t j = 0; j < 3; ++j) {\
		int k = s_t.Pos(',');\
		(ary2d)[1][j] = (s_t.SubString(1, k - 1)).ToInt();\
		s_t.Delete(1, k);\
	}\
	(ary2d)[1][3] = s_t.ToInt();\
}\
while (0)

#define UNICONN_MYSQL(uniconn, uniq)\
do {\
	(uniconn) = new TUniConnection(0);\
	(uniconn)->ConnectString =\
		L"Provider Name=MySQL;User ID=warehouse;Password=\"8jsjSS8B3jdYnOqu\";Data Source=192.168.200.10;Database=natergy;Port=12310;Login Prompt=False";\
	(uniconn)->SpecificOptions->Values[L"Charset"] = L"utf8mb4";\
	(uniconn)->SpecificOptions->Values[L"UseUnicode"] = L"True";\
	(uniq) = new TUniQuery(0);\
	(uniq)->Connection = (uniconn);\
    (uniq)->FetchRows = 1000;\
	try {\
		(uniconn)->Connect();\
	}\
	catch (...) {\
		printf("failure when try to connect to the mysql database");\
	}\
}\
while (0)

#define UNUNICONN_MYSQL(uniconn, uniq)\
do {\
	delete (uniq);\
	(uniconn)->Disconnect();\
	delete (uniconn);\
}\
while (0)

#define RECONNECT_LIFT1SLAVE1()\
do {\
	printf("reading lift1 plc slave1 failed, reconnect...\n");\
	mb_tcp[plclift1]->modbus_close();\
	delete mb_tcp[plclift1];\
	mb_tcp[plclift1] = 0;\
	mb_tcp[plclift1] = new modbus(ips_plcs[plclift1]);\
	mb_tcp[plclift1]->modbus_set_slave_id(1);\
	mb_tcp[plclift1]->modbus_connect();\
}\
while (0)

#define RECONNECT_LIFT1SLAVE2()\
do {\
	printf("writing lift1 plc slave2 failed, reconnect...\n");\
	mb_lift1_slave2.modbus_close();\
	mb_lift1_slave2 = modbus(ips_plcs[plclift1]);\
	mb_lift1_slave2.modbus_set_slave_id(2);\
	mb_lift1_slave2.modbus_connect();\
}\
while (0)

#define TCP_MODBUS_CONNECT(rc, ctx, ip_str, port)\
do {\
    (rc) = 0;\
	(ctx) = modbus_new_tcp((ip_str), (port));\
	if ((ctx));\
	else {\
		AnsiString str_err = "创建MODBUS TCP环境失败";\
		OutputDebugStringA(str_err.c_str());\
        break;\
	}\
	if (-1 != modbus_connect((ctx)))(rc) = 1;\
	else {\
		AnsiString msg_err = AnsiString::Format("TCP 连接失败: %s",\
			ARRAYOFCONST((modbus_strerror(errno))));\
		modbus_free((ctx));\
		OutputDebugStringA(msg_err.c_str());\
	}\
}\
while (0)

#define CONNECT_LIFT1(rc, ctx)\
do {\
	size_t i;\
	for (i = 0; i < 3; ++i) {\
		TCP_MODBUS_CONNECT((rc), (ctx), "192.168.0.2", 502);\
		if ((rc))\
			break;\
	}\
	if (3 != i);\
	else {\
		OutputDebugStringA("连接升降机PLC失败，请检查网络");\
	}\
}\
while (0)

#define TCP_MODBUS_DISCONNECT(ctx)\
do {\
	if ((ctx)) {\
		modbus_close((ctx));\
		modbus_free((ctx));\
		(ctx) = 0;\
	}\
}\
while (0)

#define READ_LIFT1_STATE(rc, p, ctx)\
do {\
	(rc) = -1;\
	(rc) = modbus_read_registers((ctx), 20, 1, (p));\
	if (-1 != (rc))(rc) = 1;\
	else {\
		AnsiString msg_err = AnsiString::Format("读寄存器 20 失败: %s",\
			ARRAYOFCONST((modbus_strerror(errno))));\
		OutputDebugStringA(msg_err.c_str());\
        (rc) = 0;\
	}\
}\
while (0)

#define READS_LIFT1(rc, p, ctx)\
do {\
	bool reconn = 0;\
	(rc) = 0;\
	size_t i;\
TRY_READS_LIFT1:\
	for (i = 0; i < 10; ++i) {\
		READ_LIFT1_STATE((rc), (p), (ctx));\
		if ((rc))\
			break;\
	}\
	if (10 != i);\
	else {\
		if (reconn)\
			break;\
		else {\
			TCP_MODBUS_DISCONNECT((ctx));\
			TCP_MODBUS_CONNECT((rc), (ctx), "192.168.0.2", 502);\
			reconn = 1;\
			goto TRY_READS_LIFT1;\
		}\
	}\
}\
while (0)

#define RAISE_LIFT1(rc, ctx)\
do {\
	(rc) = -1;\
    (rc) = modbus_write_register((ctx), 22, 0);\
	(rc) = modbus_write_register((ctx), 22, 100);\
	if (1 != (rc)) {\
		AnsiString msg_err = AnsiString::Format("写 100 至 寄存器 22 失败: %s",\
			ARRAYOFCONST((modbus_strerror(errno))));\
		OutputDebugStringA(msg_err.c_str());\
		(rc) = 0;\
	}\
}\
while (0)

#define RAISES_LIFT1(rc, ctx)\
do {\
	bool reconn = 0;\
	(rc) = 0;\
	size_t i;\
TRY_RAISES_LIFT1:\
	for (i = 0; i < 10; ++i) {\
		RAISE_LIFT1((rc), (ctx));\
		if ((rc))\
			break;\
	}\
	if (10 != i);\
	else {\
		if (reconn)\
			break;\
		else {\
			TCP_MODBUS_DISCONNECT((ctx));\
			TCP_MODBUS_CONNECT((rc), (ctx), "192.168.0.2", 502);\
			reconn = 1;\
			goto TRY_RAISES_LIFT1;\
		}\
	}\
}\
while (0)

#define DROP_LIFT1(rc, ctx)\
do {\
	(rc) = -1;\
    (rc) = modbus_write_register((ctx), 22, 0);\
	(rc) = modbus_write_register((ctx), 22, 101);\
	if (1 != (rc)) {\
		AnsiString msg_err = AnsiString::Format("写 101 至 寄存器 22 失败: %s",\
			ARRAYOFCONST((modbus_strerror(errno))));\
		OutputDebugStringA(msg_err.c_str());\
		(rc) = 0;\
	}\
}\
while (0)

#define DROPS_LIFT1(rc, ctx)\
do {\
	bool reconn = 0;\
	(rc) = 0;\
	size_t i;\
TRY_DROPS_LIFT1:\
	for (i = 0; i < 10; ++i) {\
		DROP_LIFT1((rc), (ctx));\
		if ((rc))\
			break;\
	}\
	if (10 != i);\
	else {\
		if (reconn)\
			break;\
		else {\
			TCP_MODBUS_DISCONNECT((ctx));\
			TCP_MODBUS_CONNECT((rc), (ctx), "192.168.0.2", 502);\
			reconn = 1;\
			goto TRY_DROPS_LIFT1;\
		}\
	}\
}\
while (0)

#define READ_COIL_SOUTH(r, v) \
do { \
	int j = 3; \
	int tr; \
	size_t i; \
	(r) = 0; \
TRY_READ_COIL_SOUTH: \
	for (i = 0; i < 10; ++i) { \
		tr = -1; \
		tr = modbus_read_bits((ctx), 2, 1, &(v)); \
		if (-1 != tr){ \
			break; \
		} \
	} \
	if(10 != i){ \
		(r) = 1; \
	} \
	else{ \
		if(j){ \
			TCP_MODBUS_DISCONNECT((ctx)); \
			TCP_MODBUS_CONNECT((tr), (ctx), "192.168.26.211", 502); \
			--j; \
			goto TRY_READ_COIL_SOUTH; \
		}else{ \
			AnsiString msg_err = AnsiString::Format("读取 线圈 2 失败: %s", \
				ARRAYOFCONST((modbus_strerror(errno)))); \
			OutputDebugStringA(msg_err.c_str()); \
		} \
	} \
} \
while (0)

#define READ_COIL_NORTH(r, v) \
do { \
	int j = 3; \
	int tr; \
	size_t i; \
	(r) = 0; \
TRY_READ_COIL_NORTH: \
	for (i = 0; i < 10; ++i) { \
		tr = -1; \
		tr = modbus_read_bits((ctx), 3, 1, &(v)); \
		if (-1 != tr){ \
			break; \
		} \
	} \
	if(10 != i){ \
		(r) = 1; \
	} \
	else{ \
		if(j){ \
			TCP_MODBUS_DISCONNECT((ctx)); \
			TCP_MODBUS_CONNECT((tr), (ctx), "192.168.26.211", 502); \
			--j; \
			goto TRY_READ_COIL_NORTH; \
		}else{ \
			AnsiString msg_err = AnsiString::Format("读取 线圈 3 失败: %s", \
				ARRAYOFCONST((modbus_strerror(errno)))); \
			OutputDebugStringA(msg_err.c_str()); \
		} \
	} \
} \
while (0)

#define CONNECT_PCM(rc, ctx)\
do {\
	size_t i;\
	for (i = 0; i < 3; ++i) {\
		TCP_MODBUS_CONNECT((rc), (ctx), "192.168.26.211", 502);\
		if ((rc))\
			break;\
	}\
	if (3 != i);\
	else {\
		OutputDebugStringA("连接板链机PLC失败，请检查网络");\
	}\
}\
while (0)

#endif
