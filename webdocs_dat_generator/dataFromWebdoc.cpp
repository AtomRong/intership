#include <fstream>
#include <string>

using namespace std;

#define BUFSIZE (512*1024)
unsigned char buf[BUFSIZE];


struct args {
	unsigned item_per_part;
	unsigned parts;
	args(unsigned n, unsigned p = 3): item_per_part(n), parts(p){}
};

args settings[] =
{
	{ 10000  },
	/*{ 50000, },
	{ 100000, },
	{ 200000, },
	{ 400000, },
	{ 800000, },
	{ 1600000, },
	{ 3200000, },
	{ 4800000, },
	{ 6400000, },
	{ 10000000, },
	{ 20000000, },
	{ 30000000, },*/
};

// 从fin 读至少num个数到fout，默认fin无限，读不完的
void RW( ifstream& fin, ofstream& fout, int num ) 
{
	unsigned outbuf[10000];
	int n = 0;
	while (n < num)
	{
		for (int i = 0; i < 10000 && fin; i++)
		{
			fin >> outbuf[i];
			//printf("%d ", outbuf[i]);
			//fout << outbuf[i];
		}
		fout.write( (char*)outbuf, sizeof(outbuf) );
		n += 10000;
	}
	
}
int main()
{
	ifstream fin("../data/webdocs.dat");
	fin.sync_with_stdio(false);
	for (unsigned i = 0; i < sizeof(settings) / sizeof(settings[0]); i++)
	{
		string fpath("../data/");
		for (unsigned j = 0; j < settings[i].parts; ++j)
		{
			string fname = fpath + ("web_") ;
			fname += to_string( settings[i].item_per_part / 10000);
			fname += string("w_part_") + to_string(j);
			ofstream fout(fname, ofstream::binary|ofstream::trunc);
			RW( fin, fout, settings[i].item_per_part );  
			fout.close();
		}

	}
	system("pause");
	return 0;
}

/*
od -N 128  --endian=little -tx4 web_1w_part_0
od -N 1000 --endian=little -td4 web_1w_part_2
*/