#ifndef AGV_DISPATCHER_H
#define AGV_DISPATCHER_H

#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
#include <process.h>
#include <limits>
#include <boost/crc.hpp>
#include <vector>

#define LEN_SAS 16
#define LEN_AGVS 8
#define PORT "29998"
// #define PORT "29997"
#define NS_IN6ADDRSZ 16
#define NS_INT16SZ 2
#define LEN_SOCK_ADDR_BUF 4096
#define LEN_AGV_WPS 1024
#define LEN_DX sizeof(dx)
#define LEN_DX_RET sizeof(dx_ret)

enum plcs {
	PLC_lift1, PLC_Produce, PLCother
};

enum speclocflag {
	B = 100000, D, F, G, L, O, P, Q, T, W, SPECLOCFLAGother
};

enum agvtask {
	I1 = 5000, I2, U1, U2, X1, X2, X12, X21, CHRG, L1, L2, ZERO, AGVTASKother
};

enum agvtask_state {
	agvTASKB, agvTASKE, agvRUNNING, agvLOAD, agvUNLOAD, agvZEROLOAD, agvCHRG, agvCHRGFULL,
	AGVTASKSTATEother
};

enum sock_type {
	sInner = 1000, sDisp, sUser, SOCKTYPEother
};

enum gridhead {
	h_num_taskissue, h_b, h_e, h_type_task, h_type_prod, h_id_warehouse, h_st, h_rti,
	h_state_running, GRIDHEADother
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

struct rt_wp {
	size_t t_id; // 路点所属任务
	size_t state; // agvtask_state枚举
	size_t indx_wps; // 在路点数组中的索引
};

struct pair_loc {
	size_t t_id; // 任务编号
	size_t tsk; // 任务类型(agvtask)
	size_t type_product; // 0忽略, 1小, 2大
	size_t sn_loc[2][4]; // (层,区,行,列)元组或[][0]特殊点枚举值，[][1]特殊点编码
};

struct dx { // 32 bytes
	size_t head[3]; // UINT_MAX
	size_t len_pack; // 整包长度
	size_t No; // cmd编号
	size_t cmd; // 命令标识
	size_t ip; // ip地址
	time_t st; // unix时间戳
};

struct dx_ret {
	size_t ip; // ip地址
	size_t No; // 编号
	uint32_t the_crc32; // crc32
};

struct rep_storloc {
	size_t color, st, spec, sn_stack, count_stack;
	size_t loc[5]; // warehouse, storey, area, row, column
	unsigned char txt[12]; // the text of loc
};

typedef char txt_addr_v4[16];

struct pack_txt_addr_v4 {
	txt_addr_v4 addr_txt;
};

struct sock_attr {
	int fd;
	int s_type;
	size_t offset_buf;
#define RANGE 10000
	size_t No_base;
	u_long addr_b;
	pack_txt_addr_v4 addr_txt;
	unsigned char buf[LEN_SOCK_ADDR_BUF];
};

struct agv {
	u_long addr_b;
	size_t No_cmd;
	size_t storey;
	size_t battery;
	pair_loc crt_task;
	waypoint crt_wp;
	waypoint wps[LEN_AGV_WPS];
};

struct agv_n {
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

struct state_agv_online {
	size_t battery;
	size_t id_task;
	u_long addr_b;
};

struct task_issue {
	size_t r_ti;
	u_int id_client, id_warehouse;
	u_int st;
	pair_loc one_pc;
};

#endif
