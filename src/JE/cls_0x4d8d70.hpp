// Copyright 2024 OpenJE

#ifndef JE_CLS_0X4D8D70_HPP_
#define JE_CLS_0X4D8D70_HPP_

#include "types.hpp"

namespace JE {
	class cls_0x4d8d70 {
		//: Members
	public:
		dword mbr_0x0;
		byte mbr_0x1;
		dword mbr_0x10;
		dword mbr_0x14;
		dword mbr_0x18;
		dword mbr_0x1c;
		byte mbr_0x2;
		dword mbr_0x20;
		dword mbr_0x30;
		dword mbr_0x34;
		dword mbr_0x4;
		dword mbr_0x8;
		dword mbr_0xc;
		char _pad[4];  // IDA reports total size = 56 bytes (0x38)
	//: Methods (only implemented methods declared — remaining ~150 from IDA are stubs)
	public:
		int* meth_0x545710(int *param_1);
		int* meth_0x56f520(int *param_1, int *param_2);
		int* meth_0x59f200(int param_1, char param_2, int param_3, int param_4);
	int* meth_0x59f3e0(int *param_1, int *param_2);
		int meth_0x5ace40(int *param_1);
		int meth_0x4cbdf0(int *param_1);
	};
} // namespace JE

#endif // JE_CLS_0X4D8D70_HPP_
