#include <bits/stdc++.h>
#include <time.h>

#define pb push_back
#define mp make_pair

using namespace std;

int M,V,P, MM_num_pages, VM_num_pages, pid_counter;
char* main_memory;
int* main_memory_pages;

map<int,map<int,int>> MM_page_table, VM_page_table;
set<int> free_MM_pages, free_VM_pages;
set<int> active_process_ids;
set<string> active_process_names;
map<int,int> process_size; // in bytes
map<int,string> process_name;
map<string, int> process_id;
stack<int> recently_run_process;

int getPhysicalAddress(int pid, int logAdd)
{
	if(logAdd >= process_size[pid])
	{
		printf("Invalid Memory Address %d specified for process id %d\n",logAdd,pid);
		return -1;
	}
	return (((MM_page_table[pid])[logAdd/P])*P + (logAdd%P));
}

void processInterpreter(int pid)
{
	string pname = process_name[pid];
	string command;
	int x,y,z,x1,y1,z1;
	char ch;
	bool possible = true;

	ifstream ifile(pname);
	ifile>>x;

	while((ifile>>command) && possible)
	{
		if(command == "add")
		{
			ifile>>x;ifile>>ch;ifile>>y;ifile>>ch;ifile>>z;
			x1 = getPhysicalAddress(pid,x);
			y1 = getPhysicalAddress(pid,y);
			z1 = getPhysicalAddress(pid,z);
			if(x1==-1 || y1==-1 || z1==-1)
				possible = false;
			else
			{
				main_memory[z1] = main_memory[x1] + main_memory[y1];
				printf("Command: add %d,%d,%d; Result: Value in addr %d = %d, addr %d = %d, addr %d = %d\n",x,y,z,x,main_memory[x1],y,main_memory[y1],z,main_memory[z1]);
			}
		}
		else if(command == "sub")
		{
			ifile>>x;ifile>>ch;ifile>>y;ifile>>ch;ifile>>z;
			x1 = getPhysicalAddress(pid,x);
			y1 = getPhysicalAddress(pid,y);
			z1 = getPhysicalAddress(pid,z);
			if(x1==-1 || y1==-1 || z1==-1)
				possible = false;
			else
			{
				main_memory[z1] = main_memory[x1] - main_memory[y1];
				printf("Command: sub %d,%d,%d; Result: Value in addr %d = %d, addr %d = %d, addr %d = %d\n",x,y,z,x,main_memory[x1],y,main_memory[y1],z,main_memory[z1]);
			}
		}
		else if(command == "print")
		{
			ifile>>x;
			x1 = getPhysicalAddress(pid,x);
			if(x1==-1)
				possible = false;
			else
				printf("Command: print %d; Result: Value in addr %d = %d\n",x,x,main_memory[x1]);
		}
		else if(command == "load")
		{
			ifile>>x;ifile>>ch;ifile>>y;
			y1 = getPhysicalAddress(pid,y);
			if(y1==-1)
				possible = false;
			else
			{
				main_memory[y1] = x;
				printf("Command: load %d,%d; Result: Value of %d is now stored in addr %d\n",x,y,x,y);
			}
		}
	}
	ifile.close();
}

void loadProcess(string filename)
{
	if(active_process_names.find(filename) != active_process_names.end())
		printf("Process %s is already loaded and is assigned id %d\n",filename.c_str() ,process_id[filename]);
	else
	{
		int filesize, pages_reqd, i;
		ifstream ifile(filename);

		if(!ifile.good())
		{
			printf("%s could not be loaded - file does not exist\n", filename.c_str());
			ifile.close();
			return;
		}

		ifile>>filesize;
		ifile.close();

		pages_reqd = ceil((filesize*1024.0)/P);
		if(free_VM_pages.size() >= pages_reqd)
		{
			pid_counter++;
			process_id[filename] = pid_counter;
			process_name[pid_counter] = filename;
			process_size[pid_counter] = filesize*1024;
			active_process_ids.insert(pid_counter);
			active_process_names.insert(filename);

			set<int> :: iterator it;
			i = 0;
			while(i<pages_reqd)
			{
				it = free_VM_pages.begin();
				(VM_page_table[pid_counter])[i] = *it;
				free_VM_pages.erase(it);
				i++;
			}

			if(free_MM_pages.size() >= pages_reqd)
			{
				i = 0;
				while(i<pages_reqd)
				{
					it = free_MM_pages.begin();
					(MM_page_table[pid_counter])[i] = *it;
					main_memory_pages[*it] = pid_counter;
					free_MM_pages.erase(it);
					i++;
				}
				printf("%s is loaded in main memory and is assigned process id %d\n",filename.c_str(), pid_counter);
			}
			else
			{
				for(i=0; i<pages_reqd; i++)
					(MM_page_table[pid_counter])[i] = -1;
				printf("%s is loaded in virtual memory and is assigned process id %d\n",filename.c_str(), pid_counter);
			}
		}
		else
			printf("%s could not be loaded - memory is full\n", filename.c_str());
	}
}

void listProcesses()
{
	printf("pid of processes in Main Memory :- ");
	for(auto pid : active_process_ids)
	{
		if(MM_page_table[pid][0] != -1)
			printf("%d ",pid);
	}
	printf("\npid of processes in Virtual Memory :- ");
	for(auto pid : active_process_ids)
		printf("%d ",pid);
	printf("\n");
}

void swapout(int pid)
{
	if(active_process_ids.find(pid) == active_process_ids.end())
		printf("Cannot swapout process - Process with pid %d is not loaded\n",pid);
	else if(MM_page_table[pid][0] == -1)
		printf("Process with pid %d is not in Main Memory\n",pid);
	else
	{
		for(auto pp : MM_page_table[pid])
		{
			main_memory_pages[pp.second] = 0;
			free_MM_pages.insert(pp.second);
			MM_page_table[pid][pp.first] = -1;
		}
		printf("Process with pid %d swapped out\n",pid);
	}
}

int getNextProcessToSwapout()
{
	int pid;
	while(!recently_run_process.empty())
	{
		pid = recently_run_process.top();
		recently_run_process.pop();
		if(active_process_ids.find(pid) != active_process_ids.end() && MM_page_table[pid][0] != -1)
			return pid;
	}

	for(int i=0; i<MM_num_pages; i++)
		if(main_memory_pages[i]>0)
			return main_memory_pages[i];
}

void swapin(int pid)
{
	if(active_process_ids.find(pid) == active_process_ids.end())
		printf("Cannot swapin process - Process with pid %d is not loaded\n",pid);
	else if(MM_page_table[pid][0] != -1)
		printf("Process with pid %d is already in Main Memory\n",pid);
	else
	{
		while(free_MM_pages.size() < VM_page_table[pid].size())
			swapout(getNextProcessToSwapout());

		set<int> :: iterator it;
		for(auto pp : MM_page_table[pid])
		{
			it = free_MM_pages.begin();
			MM_page_table[pid][pp.first] = *it;
			main_memory_pages[*it] = pid;
			free_MM_pages.erase(it);
		}
		printf("Process with pid %d swapped in\n",pid);
	}
}

void runProcess(int pid)
{
	if(active_process_ids.find(pid) == active_process_ids.end())
		printf("Cannot run process - Process with pid %d is not loaded\n",pid);
	else
	{
		swapin(pid);
		processInterpreter(pid);
		recently_run_process.push(pid);
	}
}

void killProcess(int pid)
{
	if(active_process_ids.find(pid) == active_process_ids.end())
		printf("Cannot kill process - Process with pid %d is not loaded\n",pid);
	else
	{
		string pname = process_name[pid];

		active_process_ids.erase(pid);
		active_process_names.erase(pname);
		process_name.erase(pid);
		process_id.erase(pname);
		process_size.erase(pid);

		for(auto pp : VM_page_table[pid])
		{
			if(pp.second != -1)
				free_VM_pages.insert(pp.second);
		}
		for(auto pp : MM_page_table[pid])
		{
			if(pp.second != -1)
			{
				main_memory_pages[pp.second] = 0;
				free_MM_pages.insert(pp.second);
				// no need to change page table as we are anyway going to erase the whole thing
			}
		}
		VM_page_table.erase(pid);
		MM_page_table.erase(pid);
		printf("Process with pid %d killed successfully\n",pid);
	}
}

void printMemLoc(int start, int length)
{
	if(start<0 || start+length >= M*1024)
		printf("Requested values out of bounds of main memory\n");
	for(int i=0; i<length; i++)
		printf("Value of %d: %d\n", start+i, main_memory[start+i]);
}

void printPageTable(int pid, string filename, bool printTime)
{
	FILE* ofile;
	ofile = fopen(filename.c_str(), "a");

	if(printTime)
	{
		time_t my_time = time(NULL);
		printf("%s\n", ctime(&my_time));
		fprintf(ofile,"%s\n", ctime(&my_time));
	}
	if(active_process_ids.find(pid) == active_process_ids.end())
	{
		printf("Process with pid %d is not loaded\n", pid);
		fprintf(ofile,"Process with pid %d is not loaded\n", pid);
	}
	else
	{
		printf("Page Table of process with pid %d (-1 indicates page not in Main Memory)\n", pid);
		fprintf(ofile,"Page Table of process with pid %d (-1 indicates page not in Main Memory)\n", pid);
		printf("Logical Page Number\tPhysical Page Number\n");
		fprintf(ofile,"Logical Page Number\tPhysical Page Number\n");
		for(auto pp : MM_page_table[pid])
		{	
			printf("%20d\t%20d\n",pp.first,pp.second);
			fprintf(ofile,"%20d\t%20d\n",pp.first,pp.second);
		}
	}
	fclose(ofile);
}

void printAllPageTables(string filename)
{
	time_t my_time = time(NULL);
	FILE* ofile;
	ofile = fopen(filename.c_str(), "a");
	fprintf(ofile,"%s\n", ctime(&my_time));
	printf("%s\n", ctime(&my_time));
	fclose(ofile);

	for(auto pid : active_process_ids)
		printPageTable(pid, filename, false);
}

int main(int argc, char** argv)
{
	int i,j,k;

	M = atoi(argv[2]);
	V = atoi(argv[4]);
	P = atoi(argv[6]);

	if(M>V)
	{
		printf("Virtual Memory Size cannot be smaller than Main Memory Size\n");
		return 0;
	}

	MM_num_pages = M*1024/P;
	VM_num_pages = V*1024/P;

	for(i=0; i<MM_num_pages; i++)
		free_MM_pages.insert(i);
	for(i=0; i<VM_num_pages; i++)
		free_VM_pages.insert(i);

	main_memory = (char *)calloc(M*1024, sizeof(char));
	main_memory_pages = (int *)calloc(MM_num_pages, sizeof(int));
	pid_counter = 0;

	while(true)
	{
		printf("\n<Command Please> ");
		string command, intermediate;

		getline(cin, command);
		stringstream check1(command);

		vector<string> tokens;
		while(getline(check1, intermediate, ' ')) 
	        tokens.push_back(intermediate); 

	    if(tokens[0] == "load")
	    {
	    	for(i=1; i<tokens.size(); i++)
	    		loadProcess(tokens[i]);
	    }
	    else if(tokens[0] == "run")
	    	runProcess(stoi(tokens[1]));
	    else if(tokens[0] == "kill")
	    	killProcess(stoi(tokens[1]));
	    else if(tokens[0] == "listpr")
	    	listProcesses();
	    else if(tokens[0] == "pte")
	    	printPageTable(stoi(tokens[1]), tokens[2], true);
	    else if(tokens[0] == "pteall")
	    	printAllPageTables(tokens[1]);
	    else if(tokens[0] == "swapout")
	    	swapout(stoi(tokens[1]));
	    else if(tokens[0] == "swapin")
	    	swapin(stoi(tokens[1]));
	    else if(tokens[0] == "print")
	    	printMemLoc(stoi(tokens[1]), stoi(tokens[2]));
	    else if(tokens[0] == "exit")
	    {
	    	printf("Goodbye\n");
	    	break;
	    }
	    else
	    	printf("Enter a valid command\n");
	}
	return 0;
}