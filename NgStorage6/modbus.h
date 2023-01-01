#ifndef MODBUSPP_MODBUS_H
#define MODBUSPP_MODBUS_H

#include <fmx.h>
#include <cstring>
#include <iostream>
#include <winsock2.h>
#include <windows.h>
#include "unistd.h"

#define MAX_MSG_LENGTH 0x104

#define     READ_COILS        0x01
#define     READ_INPUT_BITS   0x02
#define     READ_REGS         0x03
#define     READ_INPUT_REGS   0x04
#define     WRITE_COIL        0x05
#define     WRITE_REG         0x06
#define     WRITE_COILS       0x0F
#define     WRITE_REGS        0x10

#define    EX_ILLEGAL_FUNCTION  0x01
#define    EX_ILLEGAL_ADDRESS   0x02
#define    EX_ILLEGAL_VALUE     0x03
#define    EX_SERVER_FAILURE    0x04
#define    EX_ACKNOWLEDGE       0x05
#define    EX_SERVER_BUSY       0x06
#define    EX_NEGATIVE_ACK      0x07
#define    EX_MEM_PARITY_PROB   0x08
#define    EX_GATEWAY_PROBLEMP  0x0A
#define    EX_GATEWYA_PROBLEMF  0x0B
#define    EX_BAD_DATA          0XFF

#define    BAD_CON              -1

class modbus {
private:
	bool _connected;
	unsigned short PORT1;
	int _socket;
	unsigned int _msg_id;
	int _slaveid;

	std::string HOST;

	struct sockaddr_in _server;

	inline void modbus_build_request(unsigned char *to_send, unsigned address, int func) const ;

	int modbus_read(int address, unsigned amount, int func);
	int modbus_write(int address, unsigned amount, int func, const unsigned short *value);

	int modbus_send(unsigned char *to_send, int length);
	int modbus_receive(unsigned char *buffer) const ;

	void modbuserror_handle(const unsigned char *msg, int func);

	inline void set_bad_con();
	inline void set_bad_input();

public:
	bool err;
	int err_no;

	std::string error_msg;

	modbus(std::string host, unsigned short port);
	~modbus();

	bool modbus_connect();
	void modbus_close() const ;

	void modbus_set_slave_id(int id);

	int modbus_read_coils(int address, int amount, bool* buffer);
	int modbus_read_input_bits(int address, int amount, bool* buffer);
	int modbus_read_holding_registers(int address, int amount, unsigned short *buffer);
	int modbus_read_input_registers(int address, int amount, unsigned short *buffer);

	int modbus_write_coil(int address, const bool& to_write);
	int modbus_write_register(int address, const unsigned short& value);
	int modbus_write_coils(int address, int amount, const bool *value);
	int modbus_write_registers(int address, int amount, const unsigned short *value);
};

modbus::modbus(std::string host, unsigned short port = 502) {
	HOST = host;
	PORT1 = port;
	_slaveid = 1;
	_msg_id = 1;
	_connected = false;
	err = false;
	err_no = 0;
	error_msg = "";
}

modbus::~modbus() {
};

void modbus::modbus_set_slave_id(int id) {
	_slaveid = id;
}

bool modbus::modbus_connect() {
	if (HOST.empty() || PORT1 == 0) {
		std::cout << "Missing Host and Port" << std::endl;
		return false;
	}
	else {
		std::cout << "Found Proper Host " << HOST.c_str() << " with Port " << PORT1 << std::endl;
	}

	_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket == -1) {
		std::cout << "Error Opening Socket" << std::endl;
		return false;
	}
	else {
		std::cout << "PLC Socket Opened Successfully" << std::endl;
	}

	struct timeval timeout;
	timeout.tv_sec = 20;
	timeout.tv_usec = 0;
	setsockopt(_socket, SOL_SOCKET, SO_SNDTIMEO, (const char *)&timeout, sizeof(timeout));
	setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, (const char *)&timeout, sizeof(timeout));

	_server.sin_family = AF_INET;
	_server.sin_addr.s_addr = inet_addr(HOST.c_str());
	_server.sin_port = htons(PORT1);

	if (connect(_socket, (struct sockaddr*)&_server, sizeof(_server)) < 0) {
		std::cout << "Connection Error" << std::endl;
		return false;
	}

	std::cout << "Modbus TCP Connected" << std::endl;
	_connected = true;
	return true;
}

void modbus::modbus_close() const {
	closesocket(_socket);
	std::cout << "Socket Closed" << std::endl;
}

void modbus::modbus_build_request(unsigned char *to_send, unsigned address, int func) const {
	to_send[0] = (unsigned char) _msg_id >> 8u;
	to_send[1] = (unsigned char)(_msg_id & 0x00FFu);
	to_send[2] = 0;
	to_send[3] = 0;
	to_send[4] = 0;
	to_send[6] = (unsigned char) _slaveid;
	to_send[7] = (unsigned char) func;
	to_send[8] = (unsigned char)(address >> 8u);
	to_send[9] = (unsigned char)(address & 0x00FFu);
}

int modbus::modbus_write(int address, unsigned amount, int func, const unsigned short *value) {
	int status = 0;
	if (func == WRITE_COIL || func == WRITE_REG) {
		unsigned char to_send[12];
		modbus_build_request(to_send, address, func);
		to_send[5] = 6;
		to_send[10] = (unsigned char)(value[0] >> 8u);
		to_send[11] = (unsigned char)(value[0] & 0x00FFu);
		status = modbus_send(to_send, 12);
	}
	else if (func == WRITE_REGS) {
		unsigned char *to_send = new unsigned char[13 + 2 * amount];
		modbus_build_request(to_send, address, func);
		to_send[5] = (unsigned char)(7 + 2 * amount);
		to_send[10] = (unsigned char)(amount >> 8u);
		to_send[11] = (unsigned char)(amount & 0x00FFu);
		to_send[12] = (unsigned char)(2 * amount);
		for (int i = 0; i < amount; i++) {
			to_send[13 + 2 * i] = (unsigned char)(value[i] >> 8u);
			to_send[14 + 2 * i] = (unsigned char)(value[i] & 0x00FFu);
		}
		status = modbus_send(to_send, 13 + 2 * amount);
		delete[]to_send;
	}
	else if (func == WRITE_COILS) {
		unsigned char *to_send = new unsigned char[14 + (amount - 1) / 8];
		modbus_build_request(to_send, address, func);
		to_send[5] = (unsigned char)(7 + (amount + 7) / 8);
		to_send[10] = (unsigned char)(amount >> 8u);
		to_send[11] = (unsigned char)(amount & 0x00FFu);
		to_send[12] = (unsigned char)((amount + 7) / 8);
		for (int i = 0; i < (amount + 7) / 8; i++)
			to_send[13 + i] = 0;
		for (int i = 0; i < amount; i++) {
			to_send[13 + i / 8] += (unsigned char)(value[i] << (i % 8u));
		}
		status = modbus_send(to_send, 14 + (amount - 1) / 8);
		delete[]to_send;
	}
	return status;

}

int modbus::modbus_read(int address, unsigned amount, int func) {
	unsigned char to_send[12];
	modbus_build_request(to_send, address, func);
	to_send[5] = 6;
	to_send[10] = (unsigned char)(amount >> 8u);
	to_send[11] = (unsigned char)(amount & 0x00FFu);
	return modbus_send(to_send, 12);
}

int modbus::modbus_read_holding_registers(int address, int amount, unsigned short *buffer) {
	if (_connected) {
		if (amount > 65535 || address > 65535) {
			set_bad_input();
			return EX_BAD_DATA;
		}
		modbus_read(address, amount, READ_REGS);
		unsigned char to_rec[MAX_MSG_LENGTH];
		int k = modbus_receive(to_rec);
		if (k == -1) {
			set_bad_con();
			return BAD_CON;
		}
		modbuserror_handle(to_rec, READ_REGS);
		if (err)
			return err_no;
		for (unsigned i = 0; i < amount; i++) {
			buffer[i] = ((unsigned short)to_rec[9u + 2u * i]) << 8u;
			buffer[i] += (unsigned short) to_rec[10u + 2u * i];
		}
		return 0;
	}
	else {
		set_bad_con();
		return BAD_CON;
	}
}

int modbus::modbus_read_input_registers(int address, int amount, unsigned short *buffer) {
	if (_connected) {
		if (amount > 65535 || address > 65535) {
			set_bad_input();
			return EX_BAD_DATA;
		}
		modbus_read(address, amount, READ_INPUT_REGS);
		unsigned char to_rec[MAX_MSG_LENGTH];
		int k = modbus_receive(to_rec);
		if (k == -1) {
			set_bad_con();
			return BAD_CON;
		}
		modbuserror_handle(to_rec, READ_INPUT_REGS);
		if (err)
			return err_no;
		for (unsigned i = 0; i < amount; i++) {
			buffer[i] = ((unsigned short)to_rec[9u + 2u * i]) << 8u;
			buffer[i] += (unsigned short) to_rec[10u + 2u * i];
		}
		return 0;
	}
	else {
		set_bad_con();
		return BAD_CON;
	}
}

int modbus::modbus_read_coils(int address, int amount, bool *buffer) {
	if (_connected) {
		if (amount > 2040 || address > 65535) {
			set_bad_input();
			return EX_BAD_DATA;
		}
		modbus_read(address, amount, READ_COILS);
		unsigned char to_rec[MAX_MSG_LENGTH];
		int k = modbus_receive(to_rec);
		if (k == -1) {
			set_bad_con();
			return BAD_CON;
		}
		modbuserror_handle(to_rec, READ_COILS);
		if (err)
			return err_no;
		for (unsigned i = 0; i < amount; i++) {
			buffer[i] = (bool)((to_rec[9u + i / 8u] >> (i % 8u)) & 1u);
		}
		return 0;
	}
	else {
		set_bad_con();
		return BAD_CON;
	}
}

int modbus::modbus_read_input_bits(int address, int amount, bool* buffer) {
	if (_connected) {
		if (amount > 2040 || address > 65535) {
			set_bad_input();
			return EX_BAD_DATA;
		}
		modbus_read(address, amount, READ_INPUT_BITS);
		unsigned char to_rec[MAX_MSG_LENGTH];
		int k = modbus_receive(to_rec);
		if (k == -1) {
			set_bad_con();
			return BAD_CON;
		}
		if (err)
			return err_no;
		for (unsigned i = 0; i < amount; i++) {
			buffer[i] = (bool)((to_rec[9u + i / 8u] >> (i % 8u)) & 1u);
		}
		modbuserror_handle(to_rec, READ_INPUT_BITS);
		return 0;
	}
	else {
		return BAD_CON;
	}
}

int modbus::modbus_write_coil(int address, const bool& to_write) {
	if (_connected) {
		if (address > 65535) {
			set_bad_input();
			return EX_BAD_DATA;
		}
		int value = to_write * 0xFF00;
		modbus_write(address, 1, WRITE_COIL, (unsigned short *)&value);
		unsigned char to_rec[MAX_MSG_LENGTH];
		int k = modbus_receive(to_rec);
		if (k == -1) {
			set_bad_con();
			return BAD_CON;
		}
		modbuserror_handle(to_rec, WRITE_COIL);
		if (err)
			return err_no;
		return 0;
	}
	else {
		set_bad_con();
		return BAD_CON;
	}
}

int modbus::modbus_write_register(int address, const unsigned short& value) {
	if (_connected) {
		if (address > 65535) {
			set_bad_input();
			return EX_BAD_DATA;
		}
		modbus_write(address, 1, WRITE_REG, &value);
		unsigned char to_rec[MAX_MSG_LENGTH];
		int k = modbus_receive(to_rec);
		if (k == -1) {
			set_bad_con();
			return BAD_CON;
		}
		modbuserror_handle(to_rec, WRITE_COIL);
		if (err)
			return err_no;
		return 0;
	}
	else {
		set_bad_con();
		return BAD_CON;
	}
}

int modbus::modbus_write_coils(int address, int amount, const bool *value) {
	if (_connected) {
		if (address > 65535 || amount > 65535) {
			set_bad_input();
			return EX_BAD_DATA;
		}
		unsigned short *temp = new unsigned short[amount];
		for (int i = 0; i < amount; i++) {
			temp[i] = (unsigned short)value[i];
		}
		modbus_write(address, amount, WRITE_COILS, temp);
		unsigned char to_rec[MAX_MSG_LENGTH];
		int k = modbus_receive(to_rec);
		if (k == -1) {
			set_bad_con();
			return BAD_CON;
		}
		modbuserror_handle(to_rec, WRITE_COILS);
		delete[]temp;
		if (err)
			return err_no;
		return 0;
	}
	else {
		set_bad_con();
		return BAD_CON;
	}
}

int modbus::modbus_write_registers(int address, int amount, const unsigned short *value) {
	if (_connected) {
		if (address > 65535 || amount > 65535) {
			set_bad_input();
			return EX_BAD_DATA;
		}
		modbus_write(address, amount, WRITE_REGS, value);
		unsigned char to_rec[MAX_MSG_LENGTH];
		int k = modbus_receive(to_rec);
		if (k == -1) {
			set_bad_con();
			return BAD_CON;
		}
		modbuserror_handle(to_rec, WRITE_REGS);
		if (err)
			return err_no;
		return 0;
	}
	else {
		set_bad_con();
		return BAD_CON;
	}
}

int modbus::modbus_send(unsigned char *to_send, int length) {
	_msg_id++;
	return send(_socket, (const char *)to_send, (size_t)length, 0);
}

int modbus::modbus_receive(unsigned char *buffer) const {
	return recv(_socket, (char *) buffer, 1024, 0);
}

void modbus::set_bad_con() {
	err = true;
	error_msg = "BAD CONNECTION";
}

void modbus::set_bad_input() {
	err = true;
	error_msg = "BAD FUNCTION INPUT";
}

void modbus::modbuserror_handle(const unsigned char *msg, int func) {
	if (msg[7] == func + 0x80) {
		err = true;
		switch (msg[8]) {
		case EX_ILLEGAL_FUNCTION:
			error_msg = "1 Illegal Function";
			break;
		case EX_ILLEGAL_ADDRESS:
			error_msg = "2 Illegal Address";
			break;
		case EX_ILLEGAL_VALUE:
			error_msg = "3 Illegal Value";
			break;
		case EX_SERVER_FAILURE:
			error_msg = "4 Server Failure";
			break;
		case EX_ACKNOWLEDGE:
			error_msg = "5 Acknowledge";
			break;
		case EX_SERVER_BUSY:
			error_msg = "6 Server Busy";
			break;
		case EX_NEGATIVE_ACK:
			error_msg = "7 Negative Acknowledge";
			break;
		case EX_MEM_PARITY_PROB:
			error_msg = "8 Memory Parity Problem";
			break;
		case EX_GATEWAY_PROBLEMP:
			error_msg = "10 Gateway Path Unavailable";
			break;
		case EX_GATEWYA_PROBLEMF:
			error_msg = "11 Gateway Target Device Failed to Respond";
			break;
		default:
			error_msg = "UNK";
			break;
		}
	}
	err = false;
	error_msg = "NO ERR";
}

#endif
