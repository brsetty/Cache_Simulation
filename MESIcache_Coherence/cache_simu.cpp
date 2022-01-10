#include "cache_simu.h"
void simu_cache::init_simu()
{
	for(int i=0; i<32; i++)
	Main_mem.mem[i] = i + 1;

	inst_set.inst_list.push_back({ 'r', 1, 0 }); // op, addr, processor    
	inst_set.inst_list.push_back({ 'r', 1, 1 });
	inst_set.inst_list.push_back({ 'r', 1, 2 });
	inst_set.inst_list.push_back({ 'w', 1, 1 });
	inst_set.inst_list.push_back({ 'r', 1, 3 });
	inst_set.inst_list.push_back({ 'w', 1, 3 });
	inst_set.inst_list.push_back({ 'w', 4, 3 });
	inst_set.inst_list.push_back({ 'r', 4, 1 });
	inst_set.inst_list.push_back({ 'r', 4, 1 });
	inst_set.inst_list.push_back({ 'w', 4, 1 });
	//Inst inst_addr;
}

void simu_cache::start_simu()
{
	cache_access_res res =cache_access_res::Nostate;
	init_simu();

	for(Inst& inst_addr : inst_set.inst_list)
	{
		mes += inst_addr.out_string()+" ";
		int curr_pid = inst_addr.pid;
		char op = inst_addr.op; int curr_addr = inst_addr.addr;
		
		switch(op)
		{
		case 'r':
			if (exists_in_cache(curr_pid, curr_addr))
			{   //Readhit
				cycle++;
			pros_list[curr_pid].readhit++;
			print_message( pp, "local cache read", to_string(curr_pid), cycle);
			}
			else {
			//read miss
				//cout << "read miss\n";
				pros_list[curr_pid].readmiss++;
			res = cache_access_res::ReadMiss;
			BusRd(res, inst_addr);
			}
				break;
		case 'w': 
			if (exists_in_cache(curr_pid, curr_addr))
			{
				pros_list[curr_pid].writehit++;
					res = cache_access_res::WriteHit;
			}
				else {
				pros_list[curr_pid].writemiss++;
					res =cache_access_res::WriteMiss;
				}
				BusRdx(res, inst_addr);
				break;
		default:    break;

		}  //end of switch
	}
}

bool simu_cache::exists_in_cache(int pid, int addr)
{
	//processor p = pros_list.at(pid);
	processor p = pros_list[pid];
	if (p.L1.cache_mem[addr % 8].status_bit != 'I') // trying to access processor class which has L1 which has cache_mem array which is again 
	{	return true;// an array of class cache_line and status bit is its member. 
	}
	else
	{
		return false;
	}
}

void simu_cache::BusRdx(cache_access_res res, Inst inst_addr)
{
	int found = 0;
	int curr_add = inst_addr.addr;
	int curr_pid = inst_addr.pid;
	cycle++;
	switch (res)
	{
	case cache_access_res::WriteHit:              //data comming form processor and not included in this example
		if (pros_list[curr_pid].L1.cache_mem[curr_add % 8].status_bit == 'E') {  //if E 
			pros_list[curr_pid].L1.cache_mem[curr_add % 8].status_bit ='M';
			pros_list[curr_pid].L1.cache_mem[curr_add % 8].dirtybit = 1;
			pp[curr_pid] = 'M';
		   }
		else if (pros_list[curr_pid].L1.cache_mem[curr_add % 8].status_bit == 'S')  //if S, change the value, status is M and invalidate all other
		{
			pros_list[curr_pid].L1.cache_mem[curr_add % 8].status_bit = 'M';
			pros_list[curr_pid].L1.cache_mem[curr_add % 8].dirtybit = 0;//dr=1
			pp[curr_pid] = 'M';
			//invalidate others
			for (processor p : pros_list) {
				if (p.proc_id == curr_pid) continue;
				if (exists_in_cache(p.proc_id, curr_add)) {
					pros_list[p.proc_id].L1.cache_mem[curr_add % 8].status_bit = 'I';
					pros_list[p.proc_id].L1.cache_mem[curr_add % 8].dirtybit = 0;
					pp[p.proc_id] = 'I';
				}
			}
			cycle++;  //for snooping invalidate others
		}
		//print_message(pp, "BusRdX","pros", cycle);
		print_message(pp, "BusRdX", "ros", cycle);
			found = 1;
			break;
		
	case cache_access_res::WriteMiss:     //check if any other pros have this, then copy it and become owner(dr=1), invalidate the doner
		for (processor p : pros_list) 
		{
			if (p.proc_id == curr_pid) continue;
			if (exists_in_cache(p.proc_id, curr_add))
			{
				char c = p.L1.cache_mem[curr_add % 8].status_bit;
				switch (c) 
				{
				case 'E' : 
					pros_list[curr_pid].L1.cache_mem[curr_add % 8].data = p.L1.cache_mem[curr_add % 8].data;
					pros_list[p.proc_id].L1.cache_mem[curr_add % 8].status_bit = 'S';
					pp[p.proc_id] = 'S';
					pros_list[curr_pid].L1.cache_mem[curr_add % 8].status_bit = 'S';
					pros_list[curr_pid].L1.cache_mem[curr_add % 8].dirtybit = 0;
					pp[curr_pid] = 'S';
					found = 1;
					print_message(pp, "BusRdX", to_string(p.proc_id), cycle);
					break;
				case 'M':
					pros_list[curr_pid].L1.cache_mem[curr_add % 8].data = p.L1.cache_mem[curr_add % 8].data;
					pros_list[p.proc_id].L1.cache_mem[curr_add % 8].status_bit = 'S';
					pros_list[p.proc_id].L1.cache_mem[curr_add % 8].dirtybit = 0;
					pp[p.proc_id] = 'S';

					pros_list[curr_pid].L1.cache_mem[curr_add % 8].status_bit = 'S';
					pros_list[curr_pid].L1.cache_mem[curr_add % 8].dirtybit = 0;
					pp[curr_pid] = 'S';
					found = 1;
					print_message(pp, "BusRdX", to_string(p.proc_id), cycle);
					break;
				case 'S' : 
						pros_list[curr_pid].L1.cache_mem[curr_add % 8].data = p.L1.cache_mem[curr_add % 8].data;
						pros_list[p.proc_id].L1.cache_mem[curr_add % 8].dirtybit = 0;
						pros_list[curr_pid].L1.cache_mem[curr_add % 8].status_bit = 'S';
						pros_list[curr_pid].L1.cache_mem[curr_add % 8].dirtybit = 0;//dr=1
						pp[curr_pid] = 'S';
						found = 1;
						print_message(pp, "BusRdX", to_string(p.proc_id), cycle);
					break;
				}  //end of switdh
				
			}  //end of if exists
			if (found == 1) break; //break the for loop
		}    // end of for
		cycle++; //for snooping cache to cache communication
		if (found == 0)   //read from mem
		{
			pros_list[curr_pid].L1.cache_mem[curr_add % 8].data = Main_mem.mem[curr_add];
			cycle += 3;
			pros_list[curr_pid].L1.cache_mem[curr_add % 8].status_bit = 'E';
			pros_list[curr_pid].L1.cache_mem[curr_add % 8].dirtybit = 0;
			pp[curr_pid] = 'E';
			print_message(pp, "BusRdX", "mem", cycle);
		}
		break;
	default:
		break;
	}
}

void simu_cache::BusRd(cache_access_res res, Inst inst_addr)
{
	int found = 0;//read miss
	int curr_add = inst_addr.addr;
	int curr_pid = inst_addr.pid;
	//cout << "read miss with " << curr_add << " " << curr_pid;
	cycle++;
	for (processor p : pros_list) 
	{
		//cout << "in for in busrd proc id :" << p.proc_id<< " status: " << p.L1.cache_mem[curr_add % 8].status_bit;
		if (p.proc_id == curr_pid) {
			continue;
		}
		char c = p.L1.cache_mem[curr_add % 8].status_bit;
		if (exists_in_cache(p.proc_id, curr_add))   //if c != 'I && exists
		{
			found = 1;
				//cout << "read miss but found in " << p.proc_id<<endl;
			switch (c)
			{
			case 'E':
				pros_list[p.proc_id].L1.cache_mem[curr_add % 8].status_bit = 'S';
				pros_list[p.proc_id].L1.cache_mem[curr_add % 8].dirtybit = 0;
				pp[p.proc_id] = 'S';

				pros_list[curr_pid].L1.cache_mem[curr_add % 8].status_bit = 'S';
				pros_list[curr_pid].L1.cache_mem[curr_add % 8].dirtybit = 0;
				pros_list[curr_pid].L1.cache_mem[curr_add % 8].data = p.L1.cache_mem[curr_add % 8].data;
				pp[curr_pid] = 'S';
				found=1;
				print_message(pp, "BusRd", to_string(p.proc_id), cycle);
				break;
			case 'M':
				pros_list[p.proc_id].L1.cache_mem[curr_add % 8].status_bit = 'S';
				pros_list[p.proc_id].L1.cache_mem[curr_add % 8].dirtybit = 0;
				Main_mem.mem[curr_add] = p.L1.cache_mem[curr_add % 8].data; //write back to memory
				pp[p.proc_id] = 'S';

				pros_list[curr_pid].L1.cache_mem[curr_add % 8].status_bit = 'S';
				pros_list[curr_pid].L1.cache_mem[curr_add % 8].dirtybit = 0;
				pros_list[curr_pid].L1.cache_mem[curr_add % 8].data = p.L1.cache_mem[curr_add % 8].data;
				pp[curr_pid] = 'S';
				found = 1;
				print_message(pp,"BusRd", to_string(p.proc_id), cycle);
				break;
			case 'S':  // no change in shared state
					pros_list[p.proc_id].L1.cache_mem[curr_add % 8].dirtybit = 0;

					pros_list[curr_pid].L1.cache_mem[curr_add % 8].status_bit = 'S';
					pros_list[curr_pid].L1.cache_mem[curr_add % 8].dirtybit = 0;
					pp[curr_pid] = 'S';

					pros_list[curr_pid].L1.cache_mem[curr_add % 8].data = p.L1.cache_mem[curr_add % 8].data;
					found = 1;
					print_message(pp, "BusRd", to_string(p.proc_id), cycle);
				
				break;
			default:
				break;
			}//end of switch
			
			if (found == 1 || c == 'E' || c == 'M') break;
		}   //end of if exixts
	}   //end of for
	cycle++; //for snooping
	if (found == 0) {
		//not in any cache so read from memory
		//cout << "read miss and found 0\n";
		pros_list[curr_pid].L1.cache_mem[curr_add % 8].data = Main_mem.mem[curr_add];
		cycle += 3;
		pros_list[curr_pid].L1.cache_mem[curr_add % 8].status_bit = 'E';
		pros_list[curr_pid].L1.cache_mem[curr_add % 8].dirtybit = 0;
		pp[curr_pid] = 'E';
		print_message(pp, "BusRd", "Mem", cycle);
	}
}

void simu_cache::print_message(char pp[4], string busrq, string datasup, int clks)
{
	char c = ' ';
	char ppp[8];
	for (int i = 0, k=0; i < 7; i++, k++) {
		ppp[i] = pp[k];
		i++;
		if(i<7) ppp[i] = c;
	}
	ppp[7] = '\0';
	mes+= ("  " +string(ppp) + "   " + busrq + "      p" + datasup + "       " + to_string(clks) + "\n");
	simu_que.push(mes);
	mes = "";
}

void simu_cache::runSimu()
{
	cout << "       processors\n";
	cout << "inst     0 1 2 3   Busreq       suplier    clk" << endl;
	while (!simu_que.empty())
	{
		cout << simu_que.front();
		simu_que.pop();
	}

}
