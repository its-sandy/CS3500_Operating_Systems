#include <bits/stdc++.h>
#include <time.h>

#define pb push_back
#define mp make_pair

using namespace std;

#define NUM_DIR_BLOCKS 2

int disk_size;
string disk_file;
// first 2 blocks contain directory info - each one has 6 entries (6*(16+1+4) = 126)
// 3rd block contains free vector

map<string, int> file_size, file_read_pointer, file_write_pointer, inodes, index_blocks; // file size also contains directories
map<string, vector<int>> file_blocks;
bool free_vector[8*128];
int num_free_blocks;

int ceilADivB(int a, int b)
{
	if(a<=0)
		return 0;
	else if(a%b == 0)
		return a/b;
	else
		return a/b + 1;
}

vector<string> tokenizeBlock(string path, char ch)
{
	vector <string> tokens; 
    stringstream check1(path); 
    string intermediate; 
      
    // Tokenizing w.r.t. '/' or '*'
    while(getline(check1, intermediate, ch)) 
    { 
        tokens.push_back(intermediate); 
    }
    return tokens; // tokens[0] is empty
}

/////////////////////////////

int getPhysicalLocation(string filename, int loc)
{
	if(loc >= file_size[filename])
		return -1;
	else
		return (file_blocks[filename][loc/128])*128 + (loc%128);
}

int getIntegerAtLocation(int loc)
{
	//returns 4 byte integer starting from location loc
	if(disk_file[loc] == '*')
		return -1;
	return (disk_file[loc+3] + ((disk_file[loc+2])<<8) + ((disk_file[loc+1])<<16) + ((disk_file[loc])<<24));
}

void setIntegerAtLocation(int loc, int val)
{
	//sets 4 byte integer starting from location loc
	disk_file[loc] = (val>>24)%(1<<8);
	disk_file[loc+1] = (val>>16)%(1<<8);
	disk_file[loc+2] = (val>>8)%(1<<8);
	disk_file[loc+3] = (val)%(1<<8);
}

int getNextFreeBlock()
{
	for(int i=0; i<8*128; i++)
		if(free_vector[i])
		{
			free_vector[i] = false;
			num_free_blocks--;
			return i;
		}
	return -1;
}

void deleteBlock(int block)
{
	free_vector[block] = true;
	num_free_blocks++;
	memset(&disk_file[128*block], '*', 128);
}

int checkDirectory(string dir)
{
	// checks if directory exists ... gives number of files in it
	int count = -1;
	for(auto pp : file_size)
	{
		if((tokenizeBlock(pp.first, '/')	)[1] == dir)
			count++;
	}
	return count;
}

bool checkIfParent(string dir, string subdir)
{
	for(int i=0; i<dir.size(); i++)
		if(dir[i] != subdir[i])
			return false;
	return true;
}

////////////////

bool extendFileSize(string filename, int mem_reqd)
{
	if(mem_reqd <= 0)
		return true;
	else if(mem_reqd + file_size[filename] > 4480)
	{
		cout<<"Required file extension exceeds maximum file size of 4480\n";
		return false;
	}
	else if(mem_reqd > ((128 - file_size[filename]%128)%128 + 128*num_free_blocks))
	{
		cout<<"Required file extension exceeds disk file size\n";
		return false;
	}
	else
	{
		int new_mem_reqd = mem_reqd - (128 - file_size[filename]%128)%128;
		int blocks_reqd = ceilADivB(new_mem_reqd, 128);
		int inode = inodes[filename], index_block = index_blocks[filename];

		while(blocks_reqd > 0)
		{
			int block = getNextFreeBlock();
			file_blocks[filename].pb(block);
			blocks_reqd--;

			if(file_blocks[filename].size() == 1)
				setIntegerAtLocation(inode*128+69, block);
			else if(file_blocks[filename].size() == 2)
				setIntegerAtLocation(inode*128+73, block);
			else if(file_blocks[filename].size() == 3)
				setIntegerAtLocation(inode*128+77, block);
			else
				setIntegerAtLocation(index_block*128 + (file_blocks[filename].size() - 4)*4, block);
		}

		file_size[filename] = file_size[filename] + mem_reqd;
		setIntegerAtLocation(inode*128+17, file_size[filename]);

		// update inode
		// 0-15 -> name
		// 16 -> directory/ file indicator
		// 17-20 -> file size
		// 21-44 -> date created
		// 45-68 -> date modified
		// 69-80 -> 3 direct block addressses of 4 bytes each
		// 81-84 -> index table block address
		
		cout<<"File size of "<<filename<<" extended to "<<file_size[filename]<<" bytes\n";
		return true;
	}
}

void updateDateModified(string filename, bool modified)
{
	// modified = true -> change modified time, else change created time
	int offset;
	if(modified == true)
		offset = inodes[filename]*128 + 45;
	else
		offset = inodes[filename]*128 + 21;

	time_t result = time(nullptr);
    string time_string(asctime(localtime(&result)));

	for(int i=0; i<24; i++)
		disk_file[i+offset] = time_string[i];
}

void insertDirectoryStructureEntry(string name)
{
	for(int i=0; i<2; i++)
	{
		for(int j=0; j<6; j++)
		{
			if(disk_file[128*i + 21*j] == '*')
			{
				for(int k=0; k<name.size(); k++)
					disk_file[128*i + 21*j + k] = name[k];
				
				setIntegerAtLocation(128*i + 21*j + 17, inodes[name]);

				return;
			}
		}
	}
}

void deleteDirectoryStructureEntry(string name)
{
	for(int i=0; i<2; i++)
	{
		for(int j=0; j<6; j++)
		{
			string sub = disk_file.substr(128*i + 21*j, 21);
			vector<string> tokens = tokenizeBlock(sub, '*');

			if(tokens[0] == name)
			{
				memset(&disk_file[128*i + 21*j], '*', 21);
				return;
			}
		}
	}
}

bool createFileOrDir(string filename, int FileOrDir)
{
	if(index_blocks.find(filename) != index_blocks.end())
	{
		cout<<"File "<<filename<<" already exists\n";
		return false;
	}
	else if(inodes.find(filename) != inodes.end())
	{
		cout<<"ERROR: Directory "<<filename<<" already exists\n";
		return false;
	}
	if(inodes.size() == NUM_DIR_BLOCKS*6)
	{
		cout<<"ERROR: Blocks allotted for directory structure are full\n";
		return false;
	}
	else if(FileOrDir + num_free_blocks < 2)
	{
		cout<<"ERROR: Not enough free blocks available in disk file\n";
		return false;
	}
	else
	{
		// 0-15 -> name
		// 16 -> directory/ file indicator
		// 17-20 -> file size
		// 21-44 -> date created
		// 45-68 -> date modified
		// 69-80 -> 3 direct block addressses of 4 bytes each
		// 81-84 -> index table block address
		string newinode(128,'*');
		for(int i=0; i<filename.size(); i++)
			newinode[i] = filename[i];
		newinode[16] = FileOrDir+48;

		file_size[filename] = 0;
		newinode[17]=newinode[18]=newinode[19]=newinode[20]=0; // file size

		time_t result = time(nullptr);
    	string time_string(asctime(localtime(&result)));
		for(int i=0; i<24; i++)
			newinode[i+21] = newinode[i+45] = time_string[i];

		if(FileOrDir == 0)
		{
			int index_block = getNextFreeBlock();
			index_blocks[filename] = index_block;
			for(int i=0; i<4; i++)
				newinode[84-i] = (index_block>>(8*i))%(1<<8);

			file_blocks[filename].clear();
		}

		int inode_block = getNextFreeBlock();
		inodes[filename] = inode_block;
		insertDirectoryStructureEntry(filename);

		for(int i=0; i<128; i++)
			disk_file[inode_block*128 + i] = newinode[i]; 

		return true;
	}
}

/////////////////////

void openFile(string filename)
{
	if(index_blocks.find(filename) != index_blocks.end())
	{
		file_read_pointer[filename] = file_write_pointer[filename] = 0;
		cout<<"File "<<filename<<" already exists\n";
	}
	else
	{
		vector<string> path_tokens = tokenizeBlock(filename, '/');

		if(path_tokens.size() == 3 && checkDirectory(path_tokens[1]) == -1)
			cout<<"ERROR: Subdirectory /"<<path_tokens[1]<<" does not exist\n";
		else
		{
			if(createFileOrDir(filename, 0))
			{
				file_read_pointer[filename] = file_write_pointer[filename] = 0;
				cout<<"Created "<<filename<<"\n";
			}
		}
	}
}

void createDirectory(string filename)
{
	if(createFileOrDir(filename, 1))
		cout<<"Directory "<<filename<<" created\n";
}

void seekRead(string filename, int pos)
{
	if(index_blocks.find(filename) == index_blocks.end())
		cout<<"ERROR: File does not exist\n";
	else if(pos >= file_size[filename])
		cout<<"ERROR: File length exceeded\n";
	else
	{
		file_read_pointer[filename] = pos;
		cout<<"Read pointer set to byte "<<pos<<"\n";
	}
}

void seekWrite(string filename, int pos)
{
	if(index_blocks.find(filename) == index_blocks.end())
		cout<<"ERROR: File does not exist\n";
	else
	{
		file_write_pointer[filename] = min(file_size[filename], pos);
		cout<<"Write pointer set to byte "<<file_write_pointer[filename]<<"\n";
	}
}

void seekWriteEnd(string filename)
{
	seekWrite(filename, file_size[filename]);
}

void fileRead(string filename, int length)
{
	if(index_blocks.find(filename) == index_blocks.end())
		cout<<"ERROR: File does not exist\n";
	else if(file_read_pointer.find(filename) == file_read_pointer.end())
		cout<<"ERROR: File Read Pointer not initialised yet\n";
	else
	{
		int start = file_read_pointer[filename];
		int end = min(file_size[filename]-1, start + length - 1);

		if(start > end)
			cout<<"ERROR: Read pointer at end of file, read not possilbe\n";
		else
		{
			cout<<"\n***************\n";
			cout<<"Filename: "<<filename<<"; Bytes read: "<<start<<" - "<<end;
			cout<<"\nString stored: ";
			for(int i=start; i<=end; i++)
				cout<<disk_file[getPhysicalLocation(filename, i)];
			cout<<"\n***************\n";
			file_read_pointer[filename] = end + 1;
		}
	}
}

void fileWrite(string filename, string text)
{
	if(index_blocks.find(filename) == index_blocks.end())
		cout<<"ERROR: File does not exist\n";
	else if(file_write_pointer.find(filename) == file_write_pointer.end())
		cout<<"ERROR: File Write Pointer not initialised yet\n";
	else
	{
		int space_reqd = file_write_pointer[filename] + text.length() - file_size[filename];
		if(extendFileSize(filename, space_reqd))
		{
			for(auto ch : text)
			{
				disk_file[getPhysicalLocation(filename, file_write_pointer[filename])] = ch;
				file_write_pointer[filename]++;
			}
			cout<<"Write performed. New File Length = "<<file_size[filename]<<"\n";
			updateDateModified(filename, true);
		}
		else
			cout<<"ERROR: Write could not be performed\n";
	}
}

void deleteFile(string filename)
{
	if(index_blocks.find(filename) == index_blocks.end())
		cout<<"ERROR: File does not exist\n";
	else
	{
		file_size.erase(filename);
		file_read_pointer.erase(filename);
		file_write_pointer.erase(filename);

		for(auto b : file_blocks[filename])
		{
			deleteBlock(b);
		}
		file_blocks.erase(filename);

		deleteBlock(inodes[filename]);
		inodes.erase(filename);

		deleteBlock(index_blocks[filename]);
		index_blocks.erase(filename);

		deleteDirectoryStructureEntry(filename);
		cout<<"File deleted\n";
	}
}

void deleteDirectory(string dir)
{
	if(index_blocks.find(dir) != index_blocks.end())
		cout<<"ERROR: Given name is that of a file\n";
	else
	{
		vector<string> tokens = tokenizeBlock(dir, '/');

		if(tokens.size() == 1)
			cout<<"ERROR: Root Directory cannot be deleted\n";
		else
		{
			int count = checkDirectory(tokens[1]);
			if(count == -1)
				cout<<"ERROR: Directory does not exist\n";
			else if(count >= 1)
				cout<<"ERROR: Directory "<<dir<<" has files, cannot be deleted\n";
			else
			{
				file_size.erase(dir);

				deleteBlock(inodes[dir]);
				inodes.erase(dir);
				deleteDirectoryStructureEntry(dir);
				cout<<"Directory deleted\n";
			}
		}
	}
}

void listDirectory(string dir)
{
	if(index_blocks.find(dir) != index_blocks.end())
		cout<<"ERROR: Given name is that of a file\n";
	else
	{
		vector<string> tokens = tokenizeBlock(dir, '/');

		if(tokens.size() == 2)
		{
			int count = checkDirectory(tokens[1]);
			if(count == -1)
			{
				cout<<"ERROR: Directory does not exist\n";
				return;
			}
			else
				dir = dir + string("/");
		}

		cout<<"\n*************\n";
		cout<<"Directories:\n";
		for(auto pp : inodes)
		{
			if(index_blocks.find(pp.first) == index_blocks.end() && checkIfParent(dir,pp.first))
				cout<<pp.first<<endl;
		}
		cout<<"Files:\n";
		for(auto pp : index_blocks)
		{
			if(checkIfParent(dir,pp.first))
				cout<<pp.first<<"\t"<<file_size[pp.first]<<endl;
		}
		cout<<"*************\n";
	}
}

/////////////////////

void initialise(string disk_file_name, int given_disk_size)
{
	ifstream in(disk_file_name);
	if(!in.good())
	{
		in.close();
		disk_size = given_disk_size*1024;
		disk_file = string(disk_size,'*');

		num_free_blocks = (disk_size/128) - 3;
		memset(&free_vector, 0, sizeof(bool) * 8*128);
		memset(&free_vector[3], 1, sizeof(bool) * num_free_blocks);
	}
	else
	{
		// in>>disk_file;
		getline(in, disk_file);
		in.close();
		disk_size = disk_file.size();
		cout<<"disk_size = "<<disk_size<<" bytes\n";

		for(int i=0; i<2; i++)
		{
			for(int j=0; j<6; j++)
			{
				string sub = disk_file.substr(128*i + 21*j, 21);
				vector<string> tokens = tokenizeBlock(sub, '*');

				if(tokens[0].length() != 0)
				{
					// string inode_str = tokens[tokens.size()-1];
					// inodes[tokens[0]] = inode_str[3] + ((inode_str[2])<<8) + ((inode_str[1])<<16) + ((inode_str[0])<<24);
					inodes[tokens[0]] = getIntegerAtLocation(128*i + 21*j + 17);
				}
			}
		}

		for(auto pp : inodes)
		{
			int inode = pp.second;
			string name = pp.first;
			file_size[name] = getIntegerAtLocation(inode*128 + 17);

			if(disk_file[inode*128 + 16] == '0')
			{// it is a file
				index_blocks[name] = getIntegerAtLocation(inode*128 + 81);
				int index_block = index_blocks[name];

				if(file_size[name] > 0)
					file_blocks[name].pb(getIntegerAtLocation(inode*128 + 69));
				if(file_size[name] > 128)
					file_blocks[name].pb(getIntegerAtLocation(inode*128 + 73));
				if(file_size[name] > 256)
					file_blocks[name].pb(getIntegerAtLocation(inode*128 + 77));

				int num_index_entries = ceilADivB(file_size[name],128) - 3;
				for(int i=0; i<num_index_entries; i++)
				{
					file_blocks[name].pb(getIntegerAtLocation(128*index_block + 4*i));
				}
			}
		}

		//free vector
		string free_vector_str = disk_file.substr(128*2, 128);
		num_free_blocks = 0;
		for(int i=0; i<128; i++)
		{
			int val = free_vector_str[i];
			for(int j=0; j<8; j++)
			{
				if((val>>j)%2 == 0)
					free_vector[i*8 + 7 - j] = false;
				else
				{
					free_vector[i*8 + 7 - j] = true;
					num_free_blocks++;
				}
			}
		}
		// 0-15 -> name
		// 16 -> directory/ file indicator
		// 17-20 -> file size
		// 21-44 -> date created
		// 45-68 -> date modified
		// 69-80 -> 3 direct block addressses of 4 bytes each
		// 81-84 -> index table block address


// 		map<string, int> file_size, file_read_pointer, file_write_pointer, inodes, index_blocks; // file size also contains directories
// map<string, vector<int>> file_blocks;
// bool free_vector[8*128];
// int num_free_blocks;
	}
}

void updateDiskFile(string disk_file_name)
{
	for(int i=0; i<128; i++)
	{
		int val = 0;
		for(int j=0; j<8; j++)
		{
			if(free_vector[i*8 + 7 - j] == true);
				val += (1<<j);
		}
		disk_file[128*2 + i] = val;
	}

	ofstream out(disk_file_name);
	out<<disk_file;
	out.close();
}

void printInode(string filename)
{
	// inode contents are as follows 
	// 0-15 -> name
	// 16 -> directory/ file indicator
	// 17-20 -> file size
	// 21-44 -> date created
	// 45-68 -> date modified
	// 69-80 -> 3 direct block addressses of 4 bytes each
	// 81-84 -> index table block address

	if(inodes.find(filename) == inodes.end())
	{
		cout<<"file does not exist\n";
		return;
	}

	int inode = inodes[filename], index_block = index_blocks[filename];
	cout<<"***************\n";
	cout<<"Filename:\t"<<filename<<endl;
	cout<<"Size:\t"<<getIntegerAtLocation(inode*128 + 17)<<" bytes\n";
	cout<<"Date Created:\t"<<disk_file.substr(inode*128 + 21,24)<<endl;
	cout<<"Date Last Modified:\t"<<disk_file.substr(inode*128 + 45,24)<<endl;
	cout<<"Direct Block Values:\t"<<getIntegerAtLocation(inode*128 + 69)<<" "<<getIntegerAtLocation(inode*128 + 73)<<" "<<getIntegerAtLocation(inode*128 + 77)<<endl;
	cout<<"Index Block is stored in:\t"<<getIntegerAtLocation(inode*128 + 81)<<endl;
	
	cout<<"Index Block Contents:\t";
	for(int i=0; i<32 && getIntegerAtLocation(index_block*128 + 4*i) != -1; i++)
		cout<<getIntegerAtLocation(index_block*128 + 4*i)<<" ";
	cout<<"\n***************\n";
	
}

void printFreeVector()
{
	cout<<"num_free_blocks = "<<num_free_blocks<<endl;
	for(int i=0; i<disk_size/128; i++)
		cout<<free_vector[i];
	cout<<endl;
}

void loadFile(string pname)
{
	string command, fname, content;
	int pos;

	ifstream ifile(pname);

	while(ifile>>command)
	{
		if(command == "open")
		{
			ifile>>fname;
			cout<<"COMMAND: create "<<fname<<" ; RESULT: ";
			openFile(fname);
		}
		else if(command == "createdir")
		{
			ifile>>fname;
			cout<<"COMMAND: createdir "<<fname<<" ; RESULT: ";
			createDirectory(fname);
		}
		else if(command == "seekread")
		{
			ifile>>fname>>pos;
			cout<<"COMMAND: seekread "<<fname<<" "<<pos<<" ; RESULT: ";
			seekRead(fname, pos);
		}
		else if(command == "seekwrite")
		{
			ifile>>fname>>pos;
			cout<<"COMMAND: seekwrite "<<fname<<" "<<pos<<" ; RESULT: ";
			seekWrite(fname, pos);
		}
		else if(command == "seekwriteend")
		{
			ifile>>fname;
			cout<<"COMMAND: seekwriteend "<<fname<<" ; RESULT: ";
			seekWriteEnd(fname);
		}
		else if(command == "read")
		{
			ifile>>fname>>pos;
			cout<<"COMMAND: read "<<fname<<" "<<pos<<" ; RESULT: ";
			fileRead(fname, pos);
		}
		else if(command == "write")
		{
			ifile>>fname>>content;
			cout<<"COMMAND: write "<<fname<<" "<<content<<" ; RESULT: ";
			fileWrite(fname, content);
		}
		else if(command == "delete")
		{
			ifile>>fname;
			cout<<"COMMAND: delete "<<fname<<" ; RESULT: ";
			deleteFile(fname);
		}
		else if(command == "deletedir")
		{
			ifile>>fname;
			cout<<"COMMAND: deletedir "<<fname<<" ; RESULT: ";
			deleteDirectory(fname);
		}
		else if(command == "ls")
		{
			ifile>>fname;
			cout<<"COMMAND: ls "<<fname<<" ; RESULT: ";
			listDirectory(fname);
		}
		else if(command == "printfreevector")
		{
			printFreeVector();
		}
		else
			cout<<"Invalid file system command\n";

		cout<<endl;
	}
	ifile.close();
}

int main(int argc, char** argv)
{
	if(argc != 4)
	{
		cout<<"Enter correct number of arguments\n";
		return 0;
	}

	string disk_file_name(argv[1]);
	int given_disk_size = atoi(argv[3]);

	initialise(disk_file_name, given_disk_size);

	while(true)
	{
		cout<<"\n<Command Please> ";
		string command, intermediate;

		getline(cin, command);
		stringstream check1(command);

		vector<string> tokens;
		while(getline(check1, intermediate, ' ')) 
	        tokens.push_back(intermediate); 

	    if(tokens[0] == "load")
	    	loadFile(tokens[1]);
	    else if(tokens[0] == "printinode")
	    	printInode(tokens[1]);
	    else if(tokens[0] == "exit")
	    {
	    	printf("Goodbye\n");
	    	break;
	    }
	    else
	    	printf("Enter a valid command\n");
	}

	updateDiskFile(disk_file_name);
	return 0;
}