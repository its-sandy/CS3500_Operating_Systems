#include <bits/stdc++.h>
using namespace std;

int main()
{
    string str(20,'a');
    str[4] = 0;str[8] = 0;
    cout<<str<<endl;

    cout<<str[4]<<endl;
    cout<<str[5]<<endl;
    cout<<str[8]<<endl;

    ofstream out("output.txt");
    out << str;
    out.close();

    string str2;
    ifstream in("output.txt");
    in >> str2;
    in.close();

    cout<<str2<<endl;

    cout<<str2[4]<<endl;
    cout<<str2[5]<<endl;
    cout<<str2[8]<<endl;

    cout<<str.size()<<" "<<str2.size()<<endl;
    return 0;
}