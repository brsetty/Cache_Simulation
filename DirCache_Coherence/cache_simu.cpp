#include "cache_simu.h"
int r;
void simu_cache::init_simu()
{
	for(int i=0; i<32; i++)
	Main_mem.mem[i] = i + 1;
	r = 0;
	inst_set.inst_list.push_back({ 'r', 1, 0 }); // op, addr, processor    
	inst_set.inst_list.push_back({'r', 1, 1});
	inst_set.inst_list.push_back({'r', 1, 2});
	inst_set.inst_list.push_back({ 'w', 1, 1 });
	inst_set.inst_list.push_back({ 'r', 1, 3 });
	//inst_set.inst_list.push_back({ 'w', 1, 3 });
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
	int ctt = 1;
	for(Inst& inst_addr : inst_set.inst_list)
	{
		mes += to_string(ctt)+"  "; ctt++;
		mes += inst_addr.out_string()+" "; // r 1 0 prints from here
		int curr_pid = inst_addr.pid;
		char op = inst_addr.op; int curr_addr = inst_addr.addr;
		switch(op)
		{
			
		case 'r':
			/*if (r == 0)
			{
				pros_list[curr_pid].readmiss++;
				d.set_Dirtybit(curr_addr);
				d.set_Directory(curr_addr, curr_pid); //have to update state how?
				pros_list[curr_pid].L1.cache_mem[curr_addr% 8].status_bit = 'E';
				pp[curr_pid] = 'E';
				r = 1;
				break;
			}*/
			if (d.is_set_pros(curr_pid, curr_addr))
			{   //Readhit
				cycle++;
			pros_list[curr_pid].readhit++;
			print_message( pp, "rdHit", to_string(curr_pid), cycle,d.dir[curr_addr]);
			
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
			if (d.is_set_pros(curr_pid, curr_addr))   //write hit
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
		
			d.set_Dirtybit(curr_add);   //as we are transforming from E to M the dir entry for this processor is already 1, so need to set it again
			pp[curr_pid] = 'M';
		   }
		else if (pros_list[curr_pid].L1.cache_mem[curr_add % 8].status_bit == 'S')  //if S, change the value, status is M and invalidate all other
		{
			pros_list[curr_pid].L1.cache_mem[curr_add % 8].status_bit = 'M';
			d.set_Dirtybit(curr_add);        //do we need to write back to mem???????
			
			pp[curr_pid] = 'M';
			//invalidate others, whose entry in dir is 1
			for (int i = 0; i < Assumed_Const().PROCS; i++)
			{
				if (i == curr_pid) { d.set_Directory(curr_add, i); continue; }    //set the current processor dir bit
				if (d.is_set_pros(i, curr_add)) {
					pros_list[i].L1.cache_mem[curr_add % 8].status_bit = 'I';
					d.clear_Directory(curr_add, i);
					pp[i] = 'I';
				}
			}
			cycle++;  //for snooping invalidate others
		}  
			print_message(pp, "BusRdX", "pros", cycle, d.dir[curr_add] );
			found = 1;
			break;
		
	case cache_access_res::WriteMiss:     //check if any other pros have this, then copy it and become owner(dr=1), invalidate the doner
		//for (processor p : pros_list) 
		for (int i = 0; i < Assumed_Const().PROCS; i++) 
		{
			if (i == curr_pid) continue;
			if (d.is_set_pros(i, curr_add))
			{
				char c = pros_list[i].L1.cache_mem[curr_add % 8].status_bit;
				switch (c) 
				{
				case 'E' : 
					pros_list[curr_pid].L1.cache_mem[curr_add % 8].data = pros_list[i].L1.cache_mem[curr_add % 8].data;
					pros_list[i].L1.cache_mem[curr_add % 8].status_bit = 'S';
					pp[i] = 'S';
					pros_list[curr_pid].L1.cache_mem[curr_add % 8].status_bit = 'S';
					
					d.clear_Dirtybit(curr_add);
					d.set_Directory(curr_add, curr_pid);

					pp[curr_pid] = 'S';
					
					found = 1;
					print_message(pp, "BusRdX", to_string(i), cycle, d.dir[curr_add]);
					break;
				case 'M':
					pros_list[curr_pid].L1.cache_mem[curr_add % 8].data = pros_list[i].L1.cache_mem[curr_add % 8].data;
					pros_list[i].L1.cache_mem[curr_add % 8].status_bit = 'S';
					pp[i] = 'S';

					pros_list[curr_pid].L1.cache_mem[curr_add % 8].status_bit = 'S';
					d.clear_Dirtybit(curr_add);
					d.set_Directory(curr_add, curr_pid);
					
					pp[curr_pid] = 'S';
					found = 1;
					print_message(pp, "BusRdX", to_string(i), cycle, d.dir[curr_add]);
					break;
				case 'S' : 
						pros_list[curr_pid].L1.cache_mem[curr_add % 8].data = pros_list[i].L1.cache_mem[curr_add % 8].data;
						
						d.clear_Dirtybit(curr_add);
						pros_list[curr_pid].L1.cache_mem[curr_add % 8].status_bit = 'M';
						d.set_Directory(curr_add, curr_pid);

						pp[curr_pid] = 'S';
						//make other as invalidate
						for (int i = 0; i < Assumed_Const().PROCS; i++)
						{
							if (i == curr_pid) { d.set_Directory(curr_add, i); continue; }    //set the current processor dir bit
							if (d.is_set_pros(i, curr_add)) {
								pros_list[i].L1.cache_mem[curr_add % 8].status_bit = 'I';
								d.clear_Directory(curr_add, i);
								pp[i] = 'I';
							}
						}
						found = 1;
						print_message(pp, "BusRdX", to_string(i), cycle, d.dir[curr_add]);
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
			d.set_Dirtybit(curr_add);
			d.set_Directory(curr_add, curr_pid);
			
			pp[curr_pid] = 'E';
			print_message(pp, "BusRdX", "memRd", cycle, d.dir[curr_add]);
		}
		break;
	default:
		break;
	}
}
// directoryRd
void simu_cache::BusRd(cache_access_res res, Inst inst_addr)
{
	int found = 0;//read miss
	int curr_add = inst_addr.addr;
	int curr_pid = inst_addr.pid;
	//cout << "read miss with " << curr_add << " " << curr_pid;
	cycle++;
	//for (processor p : pros_list)
	for(int i=0; i< Assumed_Const().PROCS; i++)
	{
		if (i == curr_pid) {
			continue;
		}
		//int c = d.check_Directoryentry(curr_add); // this returns 1 for proc 0 2 for proc 1 etc
		 char c = pros_list[i].L1.cache_mem[curr_add % 8].status_bit;
		 
		 if(d.is_set_pros(i, curr_add)) 
		 {
			found = 1; //for supplier found
			switch (c)
			{
			case '1':
				if(pros_list[1].L1.cache_mem[curr_add % 8].status_bit = 'I') {
					d.clear_Dirtybit(curr_add);
					pros_list[1].L1.cache_mem[curr_add % 8].status_bit = 'S';
					pp[curr_pid] = 'S'; // our state should be shared
					cout << "current state" << pp[curr_pid] << d.check_Directoryentry(curr_add) << endl;
				}
				else if (pros_list[1].L1.cache_mem[curr_add % 8].status_bit = 'S') {
					d.clear_Dirtybit(curr_add);
					pp[curr_pid] = 'S'; // our state should be shared
				}
				else if (pros_list[1].L1.cache_mem[curr_add % 8].status_bit = 'M') {
					pp[curr_pid] = 'S'; // our state should be shared
				}
				else
				{
				}
				break;
				/*---------------------------------------*/
			case 'E':
				pros_list[i].L1.cache_mem[curr_add % 8].status_bit = 'S';  //'M'  ???;
				d.clear_Dirtybit(curr_add);      //what shld be the dirty bit????
				pp[i] = 'S';

				pros_list[curr_pid].L1.cache_mem[curr_add % 8].status_bit = 'S';
				pros_list[curr_pid].L1.cache_mem[curr_add % 8].data = pros_list[i].L1.cache_mem[curr_add % 8].data;
				d.set_Directory(curr_add, curr_pid);

				pp[curr_pid] = 'S'; 
				found = 1;
				print_message(pp, "BusRd", to_string(i), cycle, d.dir[curr_add]);
				break;
			case 'M':
				pros_list[i].L1.cache_mem[curr_add % 8].status_bit = 'S';
				pp[i] = 'S';
				d.clear_Dirtybit(curr_add);       ///////

				Main_mem.mem[curr_add] = pros_list[i].L1.cache_mem[curr_add % 8].data; //write back to memory
				pros_list[curr_pid].L1.cache_mem[curr_add % 8].data = pros_list[i].L1.cache_mem[curr_add % 8].data;
				
				pros_list[curr_pid].L1.cache_mem[curr_add % 8].status_bit = 'S';
				d.set_Directory(curr_add, curr_pid);

				pp[curr_pid] = 'S';
				found = 1;
				print_message(pp, "BusRd WrBack", to_string(i), cycle, d.dir[curr_add]);
				break;
			case 'S':  // no change in shared state
				
				pros_list[curr_pid].L1.cache_mem[curr_add % 8].status_bit = 'S';
				pp[curr_pid] = 'S';
				d.set_Directory(curr_add, curr_pid);

				pros_list[curr_pid].L1.cache_mem[curr_add % 8].data = pros_list[i].L1.cache_mem[curr_add % 8].data;
				found = 1;
				print_message(pp, "BusRd", to_string(i), cycle, d.dir[curr_add]);

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
		
		pros_list[curr_pid].L1.cache_mem[curr_add % 8].data = Main_mem.mem[curr_add];
		cycle += 3;
		pros_list[curr_pid].L1.cache_mem[curr_add % 8].status_bit = 'E';
		d.set_Directory(curr_add, curr_pid);
		d.set_Dirtybit(curr_add);

		pp[curr_pid] = 'E';
		print_message(pp, "BusRd", "MemRd", cycle, d.dir[curr_add]);
	}
}

void simu_cache::print_message(char pp[4], string busrq, string datasup, int clks, int d)
{
	char c = ' ';
	char ppp[8];
	for (int i = 0, k=0; i < 7; i++, k++) {
		ppp[i] = pp[k];
		i++;
		if(i<7) ppp[i] = c;
	}
	ppp[7] = '\0';
	string tb = "     ";
	string tbb = "";
	string t = "";
	for (int i = 0; i < (10 - datasup.size()); i++) tbb += " ";
	for (int i = 0; i < (15 - busrq.size()); i++) t += " ";

	string dd = to_string(((d & (1 << 8)) > 0) ? 1 : 0)+"  "+	to_string(((d & 1) > 0) ? 1 : 0) + "  " + to_string(((d & (1 << 1)) > 0) ? 1 : 0) + "  " + to_string(((d & (1 << 2)) > 0) ? 1 : 0) + "  " +
		to_string(((d & (1 << 3)) > 0) ? 1 : 0);
	mes+= (tb +string(ppp) +tb + busrq+t + tb+ "p-" + datasup + tbb+ tb+ to_string(clks) + tb + dd+"\n");
	simu_que.push(mes);
	mes = "";
}

void simu_cache::runSimu()
{
	string tb = "     ";
	cout << tb<<tb<<"processors\n";
	cout << "inst"<<tb<<tb<<"0 1 2 3"<<tb<<"Busreq"<<tb<<tb<<tb<<"suplier"<<tb<<"clk" << tb<<"directory"<<endl;
	while (!simu_que.empty())
	{
		cout << simu_que.front();
		simu_que.pop();
	}

	cout << "read hits: ";
	for (int i = 0; i < Assumed_Const().PROCS; i++) {
			cout<< pros_list[i].readhit << "    ";
	}
	cout << endl;
	cout << "Write hits: ";
	for (int i = 0; i < Assumed_Const().PROCS; i++) {
		cout << pros_list[i].writehit << "    ";
	}
	cout << endl;
	cout << "Read Miss: ";
	for (int i = 0; i < Assumed_Const().PROCS; i++) {
		cout << pros_list[i].readmiss << "    ";
	}
	cout << endl;
	cout << "Write Miss: ";
	for (int i = 0; i < Assumed_Const().PROCS; i++) {
		cout << pros_list[i].writemiss << "    ";
	}
}
