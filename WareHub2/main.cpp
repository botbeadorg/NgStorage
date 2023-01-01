#include <vcl.h>
#include <windows.h>

#pragma hdrstop
#pragma argsused

#include <tchar.h>
#include <stdio.h>
#include <boost/pool/singleton_pool.hpp>
#include <System.IOUtils.hpp>
#include "common.h"
#include "compones.h"

typedef boost::singleton_pool < struct smlbuf_tag {
}, LEN_SMLBUF > pool_smlbuf;

void __fastcall get_smlbuf(unsigned char* *p) {
	*p = (unsigned char *)pool_smlbuf::malloc();
	SecureZeroMemory(*p, LEN_SMLBUF);
}

void __fastcall putback_smlbuf(unsigned char *p) {
	pool_smlbuf::free(p);
}

void __fastcall recycle_smlbuf() {
	pool_smlbuf::purge_memory();
}

int _tmain(int argc, _TCHAR* argv[]) {
	bool yes = 1;
	unsigned short wVersionRequested;
	int listener, newfd, fdmax, i, j, rv;
	unsigned id_thrd;
	fd_set master;
	fd_set read_fds;
	struct sockaddr_storage remoteaddr;
	socklen_t addrlen;
	u_long addr_b;
	struct addrinfo hints, *ai, *p;
	WSADATA wsaData;

	wVersionRequested = MAKEWORD(2, 2);
	rv = WSAStartup(wVersionRequested, &wsaData);
	if (rv != 0) {
		printf("WSAStartup failed with error: %d.\n", rv);
		return 1;
	}
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
		printf("Could not find a usable version of Winsock.dll.\n");
		WSACleanup();
		return 1;
	}
#ifndef _ATHOME
	for (size_t k = plclift1; k < PLCOTHER; ++k) {
		int r;
		TCP_MODBUS_CONNECT(r, ctx_tcp_modbus[k], ips_plcs[k], 502);
		if (r);
		else {
			OutputDebugStringA("无法满足全部PLC环境");
		}
	}
#endif
	event_lift1 = CreateEvent(NULL, false, false, NULL);
	path_db = Ioutils::TPath::GetDirectoryName(ParamStr(0)) + PathDelim + _T("sdrocer.vrs");

	InitializeCriticalSectionAndSpinCount(&cs_id_client, 0x400);
	InitializeCriticalSectionAndSpinCount(&cs_id_task, 0x400);
	InitializeCriticalSectionAndSpinCount(&cs_agvs, 0x400);
	InitializeCriticalSectionAndSpinCount(&cs_wps, 0x400);
	InitializeCriticalSectionAndSpinCount(&cs_rcr_wp, 0x800);
	InitializeCriticalSectionAndSpinCount(&cs_tis, 0x400);
	InitializeCriticalSectionAndSpinCount(&cs_send2tcpsrv, 0x400);
	InitializeCriticalSectionAndSpinCount(&cs_tis_over, 0x400);
	InitializeConditionVariable(&cv_tis_over);
	DataModule1 = new TDataModule1(0);
	gId_client = 0;
	// gId_task_issue = 1;
	DataModule1->next_id_ti();
	quota_agvs = 9; // it should be from the config file

	SecureZeroMemory(&cps[0][0], sizeof(cps));
	cps[0][0].flag_G = G;
	cps[0][0].idle = 1;
	cps[1][0].idle = 1;
	cps[1][0].flag_G = G;
	cps[1][0].sn = 2;
	count_cps[0] = 1;
	count_cps[1] = 1;

	for (size_t w = 0; w < 135; ++w) {
		colors_unused.push_back(colors_available[w]);
	}

	DataModule1->load_agvs();

	FD_ZERO(&master);
	FD_ZERO(&read_fds);
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	if ((rv = getaddrinfo(0, PORT_LISTENING, &hints, &ai)) != 0) {
		fprintf(stderr, "Server: %s\n", gai_strerror(rv));
		return 1;
	}
	for (p = ai; p != NULL; p = p->ai_next) {
		listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (listener < 0)
			continue;
		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, (const char *)&yes, sizeof(bool));
		if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
			closesocket(listener);
			continue;
		}
		break;
	}
	if (p == NULL) {
		fprintf(stderr, "Server: failed to bind\n");
		freeaddrinfo(ai);
		return 2;
	}
	freeaddrinfo(ai);
	if (listen(listener, 10) == -1) {
		perror("Listen");
		return 3;
	}
	FD_SET((UINT_PTR)listener, &master);
	CloseHandle((void *)_beginthreadex(0, 0, self_conn, 0, 0, &id_thrd));
	CloseHandle((void *)_beginthreadex(0, 0, http_conn, 0, 0, &id_thrd));
	CloseHandle((void *)_beginthreadex(0, 0, timer_lift1, 0, 0, &id_thrd));
	// CloseHandle((void *)_beginthreadex(0, 0, clean_wps, 0, 0, &id_thrd));
	CloseHandle((void *)_beginthreadex(0, 0, task_completed, 0, 0, &id_thrd));
	CloseHandle((void *)_beginthreadex(0, 0, cleaning_ti_handled, 0, 0, &id_thrd));
	for (; ;) {
		read_fds = master;
		if (select(0, &read_fds, 0, 0, 0) == -1) {
			perror("Select");
			return 4;
		}
		for (i = 0; i < read_fds.fd_count; ++i) {
			int sock_crt = read_fds.fd_array[i];
			if (FD_ISSET(sock_crt, &read_fds)) {
				if (sock_crt == listener) {
					addrlen = sizeof remoteaddr;
					newfd = accept(listener, (struct sockaddr*)&remoteaddr, &addrlen);
					if (newfd == -1)
						perror("Accept");
					else {
						sock_kit *a_sk = 0;
						SET_SOCK_KIT(a_sk, newfd);
						FD_SET((UINT_PTR)newfd, &master);
						bus_conn.push_back(a_sk);
					}
				}
				else {
					int nbytes = 0;
					size_t offset = 0;
					size_t capacity = 0;
					unsigned char *buf = 0;
					sock_kit *sk = 0;
					std::vector<sock_kit*>::iterator it_tcpcn;
					for (it_tcpcn = bus_conn.begin(); bus_conn.end() != it_tcpcn; ++it_tcpcn) {
						sk = *it_tcpcn;
						if (sk->fd == sock_crt)
							break;
					}
					if (it_tcpcn != bus_conn.end()) {
						offset = sk->buf_offset;
						buf = sk->buf_rcv + sk->buf_offset;
						capacity = LEN_BUF_RCV - sk->buf_offset;
						if ((nbytes = recv(sk->fd, (char *)buf, capacity, 0)) > 0) {
							size_t len_taken = offset + nbytes;
							unsigned char * buf_base = 0;
						BUF_PARSE:
							if (len_taken < len_dx) {
								sk->buf_offset = len_taken;
								continue;
							}
							buf_base = &(sk->buf_rcv[0]);
							if (((UINT_MAX == *(size_t*)(buf_base)) && (UINT_MAX == *(size_t*)
								(buf_base + len_size_t)) && (UINT_MAX == *(size_t*)(buf_base +
								2 * len_size_t)))) {
								bool r = false;
								size_t len = *(size_t*)(buf_base + len_size_t * 3);
								if (len > len_taken) {
									sk->buf_offset = len_taken;
									continue;
								}

								TEST_CRC32(r, buf_base, len);
								if (r) {
									data_handler(sk, (dx *)buf_base);
								}
								else
									CRC_ERROR(sk);

								len_taken -= len;
								MoveMemory(buf_base, buf_base + len, len_taken);
								goto BUF_PARSE;
							}
							else {
								size_t mod = 3 * len_size_t;
								size_t num_mod = len_taken / mod;
								size_t rem = len_taken % mod;
								size_t k;
								unsigned char *p_b = buf_base;
								for (k = 0; k < num_mod; (++k), (p_b += mod))
									if ((UINT_MAX == *(size_t*)p_b) &&
										(UINT_MAX == *(size_t*)(p_b + len_size_t)) &&
										(UINT_MAX == *(size_t*)(p_b + 2 * len_size_t)))
										break;
								if (k != num_mod) {
									MoveMemory(buf_base, p_b, len_taken -=
										((size_t)p_b - (size_t)buf_base));
									goto BUF_PARSE;
								}
								else {
									if (rem) {
										MoveMemory(buf_base, p_b, rem);
										sk->buf_offset = rem;
									}
									else
										sk->buf_offset = 0;
								}
							}
						}
						else {
							UINT_PTR tmp_s = 0;
							shutdown(sock_crt, SD_RECEIVE);
							closesocket(sock_crt);
							FD_CLR((UINT_PTR)sock_crt, &master);
							std::vector<sock_kit*>::iterator it_tcpcn;
							for (it_tcpcn = bus_conn.begin(); bus_conn.end() != it_tcpcn;
							++it_tcpcn) {
								sk = *it_tcpcn;
								if (sock_crt == sk->fd) {
									delete sk;
									break;
								}
							}
							bus_conn.erase(it_tcpcn);
							delete *it_tcpcn;
						}
					}
					else {
						unsigned char *just_read = new unsigned char[1024];
						recv(sock_crt, (char *)just_read, 1024, 0);
						delete[]just_read;
					}
				}
			}
		}
	}
#ifndef _ATHOME
	for (size_t k = plclift1; k < PLCOTHER; ++k) {
		TCP_MODBUS_DISCONNECT(ctx_tcp_modbus[k]);
	}
#endif
	DeleteCriticalSection(&cs_tis_over);
	DeleteCriticalSection(&cs_id_client);
	DeleteCriticalSection(&cs_id_task);
	DeleteCriticalSection(&cs_agvs);
	DeleteCriticalSection(&cs_wps);
	DeleteCriticalSection(&cs_rcr_wp);
	DeleteCriticalSection(&cs_tis);
	DeleteCriticalSection(&cs_send2tcpsrv);
	delete DataModule1;
	CloseHandle(event_lift1);
	recycle_smlbuf();
	WSACleanup();
	return 0;
}
