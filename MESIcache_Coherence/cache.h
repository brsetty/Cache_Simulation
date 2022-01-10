#pragma once
#include<iostream>
#include<vector>
#include<string>
//#include <list>
using namespace std;
enum class cache_access_res {
	ReadHit, ReadMiss, WriteHit, WriteMiss, Nostate
};
 struct Assumed_Const {
	 const int PROCS = 4;
	 const int address_len = 6;
	 const int MainMemoryAcessTime = 3;
	 const int MainMemorysize = 32;
	 const int CacheBlocksz = 1;
	 const int CacheSize = 8;
	 const int MaxInst = 10;
};

struct Inst {
	char op;
	int addr;
	int pid;
	inline string out_string() { return (string(1,op) + " "+ to_string(addr) + " " + to_string(pid)) + " "; }
};
/*class has a vector just to represent the vector of instructions*/
class Inst_code {
public:
	
	vector <Inst> inst_list;
	Inst_code() {}
};

class memory {
public:
	int mem[32];
	int mem_idx;
	memory() {
		mem_idx = 0;
	for(int i=0; i<32; i++) mem[i]=0;
		}
	int Readfrom_mem(int addr) { return mem[addr]; }
	void writeto_mem(int addr, int value) { mem[addr] = value; }
};

class cache_line {
public:
	int blockid;
//	MESI_state status_bit;
	char status_bit;
	//int valid;
	int dirtybit;
	int data;
	cache_line() { blockid = 0; status_bit = 'I'; dirtybit = false; data = 0; }
	cache_line(int b) { blockid = b; status_bit = 'I'; dirtybit = false; data = 0; }
	//invalidate()
};
class cache {
public:
	int Csize, blocksize;
	cache_line cache_mem[8]; // this is an array of class cache_line 
	cache() { Csize = 8; blocksize = 1;
	for (int i = 0; i < 8; i++) {
		cache_mem[i] = cache_line(i);
	}
	}
	cache(int s, int b) {
		Csize = s; blocksize = b;
		for (int i = 0; i < blocksize; i++) {
			cache_mem[i] = cache_line(i);
		}
	}
};

class processor {
public:
	cache L1; // this is an object of class cache 
	int csize, bsize, proc_id;
	int readhit, readmiss, writehit, writemiss;
	processor(int s, int b, int id) {
		csize = s; bsize = b; proc_id = id;
		readhit = readmiss = writehit = writemiss = 0;
		
	}
};

class Bus {
public:
	Bus(){}
	//void BusRd(cache_access_res res, Inst inst_addr);
	//void BusRdX(cache_access_res res, Inst inst_addr);

};