#include <iostream>
#include <fstream>
#include <map>

#include "lowlayer.h"
#include "highlayer.h"
#include "cmcu.h"

using namespace std;
/* //测试配置一
const unsigned L1_num = 16;   //6.4w个计数器
const unsigned L2_num = 14;   //1.6w个计数器，L1 L2 的哈希表内存一样多
const unsigned CMCUCF_num = 12; //4k*d个计数器。CMCU的表是L1 的1.5倍 
								//d=3时,总计需要 3.5 倍的L1内存, 约为 28k个int
const unsigned CMCU_num = 9967; //10k*3个计数器。30k个int，略大于CMCU_CF内存用量
*/

/*//测试配置二
const unsigned L1_num = 18;   //25.6w个计数器
const unsigned L2_num = 16;   //6.4w个计数器，L1 L2 的哈希表内存一样多
const unsigned CMCUCF_num = 14; //1.6w*d个计数器。CMCU的表是L1 的1.5倍
								//d=3时,总计需要 3.5 倍的L1内存, 约为 11.2w个int
const unsigned CMCU_num = 38053; //3.8w*3个计数器。11.4w个int，略大于CMCU_CF内存用量
*/

const unsigned L1_num = 20;   //100w个计数器
const unsigned L2_num = 18;   //25.6w个计数器，L1 L2 的哈希表内存一样多
const unsigned CMCUCF_num = 16; //6.4w*d个计数器。CMCU的表是L1 的1.5倍
								//d=3时,总计需要 3.5 倍的L1内存, 约为 45w个int
const unsigned CMCU_num = 150053; //15w*3个计数器。45w个int，略大于CMCU_CF内存用量
uint32_t L1_th = 15, L2_th = 4000;

lowlayer<L1_num, 2, 3>	L1(L1_th);	
highlayer<L2_num, 3>	L2(L2_th);	
Cmcu< 1<< CMCUCF_num, 3 >CMCU_CF;	
								

Cmcu<CMCU_num, 3>		CMCU;   


void insertCMCU_CF(uint32_t key)
{
	if (L1.update(key) < L1_th)  return ; //插入前L1未满，插入完成
	if (L2.update(key) < L2_th)  return ;
	CMCU_CF.update( key );
}

uint32_t queryCMCU_CF(uint32_t key)
{
	uint32_t v1, v2, v3;
	v1 = L1.query(key);
	if ( v1 < L1_th) 
		return v1; //L1 未满

	v2 = L2.query(key);
	if (v2 < L2_th) 
		return v1+v2;  // 这里有可能v1刚刚好到阈值，但是v2是纯粹碰撞出来的
	
	v3 = CMCU_CF.query(key);
	return v1 + v2 + v3;
}

void runtest( string fname)
{
	map<uint32_t, uint32_t> real_count;
	
	FILE *fp = fopen(fname.c_str(), "rb");

	uint32_t x, total = 0;
	while (fread(&x, 4, 1, fp) > 0)
	{
		insertCMCU_CF(x);
		CMCU.update(x);
		++real_count[x];
		++total;
	}

	double AAE1 = 0, AAE2 = 0;
	int missA = 0, missB = 0;
	for (auto it = real_count.begin(); it != real_count.end(); ++it)
	{
		uint32_t a, b;
		a = queryCMCU_CF(it->first);
		AAE1 += a - it->second;
		missA += (a - it->second) > 0 ? 1 : 0;

		b = CMCU.query(it->first);
		AAE2 += b - it->second;
		missB += (b - it->second) > 0 ? 1 : 0;
		//printf("key=%d freq=%8u\t%8u\t%8u\n", it->first, it->second, a, b );
	}
	printf("%s:unique item: %d, total=%d\n", fname.c_str(), real_count.size(), total);
	printf("L1 memory %u\t counters num %u threshold %u\n", sizeof(L1), 1 << L1_num, L1_th);
	printf("L2 memory %u\t counters num %u threshold %u\n", sizeof(L2), 1 << L2_num, L2_th);
	printf("CMCUCF memory %u\t counters num %u\n", sizeof(CMCU_CF), 1 << CMCUCF_num);
	printf("CMCU memory %u\t counters num %u\n", sizeof(CMCU), CMCU_num);

	printf("AAE with CMCUCF %lf, miss num %u\n", AAE1 / real_count.size(), missA);
	printf("AAE with CMCU   %lf, miss num %u\n\n", AAE2 / real_count.size(), missB);
	printf("==================================================\n");
	fclose(fp);
	L1.clear();
	L2.clear();
	CMCU.clear();
	CMCU_CF.clear();
}
int main()
{
	int arr[] = { 1,5,10,20,40,80,160,320, 480, 640, 1000, 2000, 3000 };
	const char format[] = "../data/web_%dw_part_%d";
	char fname[1024];
	for (int i = 0; i < sizeof(arr) / sizeof(arr[0]); ++i)
	{
		for (int part = 0; part < 3; part++)
		{
			sprintf(fname, format, arr[i], part);
			runtest(fname);
		}
	}
	system("pause");
	return 0;
}
