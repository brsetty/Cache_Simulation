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
class Directory {
public:
	int dir[32];
	Directory() {
		for (int i = 0; i < 32; i++) {
			dir[i] = 0;
				}
	}
	void init_Directory(){
		for (int i = 0; i < 32; i++) {
			dir[i] = 0;
			}
		
	}
	void set_Dirtybit(int curr_add) {
		//cout << "function" << curr_add<<endl;
		dir[curr_add] = dir[curr_add] | (1<<8);       //0b0000000100000000;   //8th bit is dirty bit and it is set 
	}
	void clear_Dirtybit(int curr_add) {
		//cout << "function" << curr_add<<endl;
		dir[curr_add] = (dir[curr_add] & (~(1 << 8)));  //8th bit is dirty bit and it is cleared 
	}
	void set_Directory(int curr_add, int proc)
	{
		dir[curr_add] = (dir[curr_add] | (1<<proc));
	}
	void clear_Directory(int curr_add, int proc)
	{
		dir[curr_add] = (dir[curr_add] & (~(1 << proc)) );
	}
	bool is_set_pros(int p, int curr_add) {   //is the processor p set at the address curr_add
		int mask = (1<<p);
		if ((dir[curr_add] & mask) == mask) return true; // dir[curr_add] & 010 == 010 then processor 1 is set 
		else return false;
	}
	int check_Directoryentry(int curr_add)
	{
		/*for (int i = 1; i <= 4; i++) {
			if ((dir[curr_add][i]) == 1) {
				//cout << i << endl;
				return i;
			}
		}*/
		int mask = 1;
		for (int i = 0; i < Assumed_Const().PROCS; i++) {     //set 1 for each processor
			mask = mask | (1 << i);           
		}          // mask is 0000001111 all 0 to 3
		return(dir[curr_add] | mask);
	}
};

class processor {
public:
	cache L1; // this is an object of class cache 
	int csize, bsize, proc_id;
	int readhit, readmiss, writehit, writemiss,modified;
	processor(int s, int b, int id) {
		csize = s; bsize = b; proc_id = id;
		readhit = readmiss = writehit = writemiss = modified =0;
		
	}
};

