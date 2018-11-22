#include <bits/stdc++.h>

#define pb push_back
#define mp make_pair

using namespace std;


int main(int argc, char** argv)
{
	string f1(argv[1]), f2(argv[2]);
	string disk_file;

	ifstream in(f1);
	in>>disk_file;
	in.close();

	cout<<disk_file.size()<<endl;
	ofstream out(f2);
	for(int i=0; i<disk_file.size(); i++)
	{
		out<<(int)disk_file[i]<<" ";
		if(i%128 == 127)
			out<<endl;
	}
	out.close();

	return 0;
}