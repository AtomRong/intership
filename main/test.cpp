#include <iostream>
#include <fstream>
#include <map>

#include "lowlayer.h"
#include "highlayer.h"
#include "cmcu.h"

using namespace std;
/* //��������һ
const unsigned L1_num = 16;   //6.4w��������
const unsigned L2_num = 14;   //1.6w����������L1 L2 �Ĺ�ϣ���ڴ�һ����
const unsigned CMCUCF_num = 12; //4k*d����������CMCU�ı���L1 ��1.5�� 
								//d=3ʱ,�ܼ���Ҫ 3.5 ����L1�ڴ�, ԼΪ 28k��int
const unsigned CMCU_num = 9967; //10k*3����������30k��int���Դ���CMCU_CF�ڴ�����
*/

/*//�������ö�
const unsigned L1_num = 18;   //25.6w��������
const unsigned L2_num = 16;   //6.4w����������L1 L2 �Ĺ�ϣ���ڴ�һ����
const unsigned CMCUCF_num = 14; //1.6w*d����������CMCU�ı���L1 ��1.5��
								//d=3ʱ,�ܼ���Ҫ 3.5 ����L1�ڴ�, ԼΪ 11.2w��int
const unsigned CMCU_num = 38053; //3.8w*3����������11.4w��int���Դ���CMCU_CF�ڴ�����
*/

const unsigned L1_num = 20;   //100w��������
const unsigned L2_num = 18;   //25.6w����������L1 L2 �Ĺ�ϣ���ڴ�һ����
const unsigned CMCUCF_num = 16; //6.4w*d����������CMCU�ı���L1 ��1.5��
								//d=3ʱ,�ܼ���Ҫ 3.5 ����L1�ڴ�, ԼΪ 45w��int
const unsigned CMCU_num = 150053; //15w*3����������45w��int���Դ���CMCU_CF�ڴ�����
uint32_t L1_th = 15, L2_th = 4000;

lowlayer<L1_num, 2, 3>	L1(L1_th);	
highlayer<L2_num, 3>	L2(L2_th);	
Cmcu< 1<< CMCUCF_num, 3 >CMCU_CF;	
								

Cmcu<CMCU_num, 3>		CMCU;   


void insertCMCU_CF(uint32_t key)
{
	if (L1.update(key) < L1_th)  return ; //����ǰL1δ�����������
	if (L2.update(key) < L2_th)  return ;
	CMCU_CF.update( key );
}

uint32_t queryCMCU_CF(uint32_t key)
{
	uint32_t v1, v2, v3;
	v1 = L1.query(key);
	if ( v1 < L1_th) 
		return v1; //L1 δ��

	v2 = L2.query(key);
	if (v2 < L2_th) 
		return v1+v2;  // �����п���v1�ոպõ���ֵ������v2�Ǵ�����ײ������
	
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
