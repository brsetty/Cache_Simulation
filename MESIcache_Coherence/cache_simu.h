#pragma once
#include "cache.h"
#include<queue>
class simu_cache {
public:
	int nPros, cachesize, blocksize, cycle;
	vector<processor> pros_list; // vector of objects of class processor
	memory Main_mem;
	Inst_code inst_set; // instance of the class Inst_code which deals with the instruction vector
	string mes;
	char pp[4];
	queue<string> simu_que;
	simu_cache() {
	  nPros = Assumed_Const().PROCS;
	  cachesize = Assumed_Const().CacheSize;
	  blocksize = Assumed_Const().CacheBlocksz;
	  cycle = 0;
	  for (int i = 0; i < nPros; i++)
		  pros_list.push_back(processor(cachesize, blocksize, i));
	  mes = "";
	  pp[0] = pp[1] = pp[2] = pp[3] = 'I';
	 // bus = Bus();
	  //Main_mem = memory();
	}
	void init_simu();
	void start_simu();
	bool exists_in_cache(int pid, int addr);
	void BusRdx(cache_access_res res, Inst inst_addr);
	void BusRd(cache_access_res res, Inst inst_addr);
	void print_message(char pp[], string busRq, string dataSup, int clks);
	void runSimu();
};