#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cinttypes>
#include <cassert>


#include <algorithm>
#include "bobhash.h"

// �������ͷ�ļ���ԭ��c��׼����֧�ֺܶ��¶����ˣ�
//�����ѧ����/ͳ�Ƶĺ��������ͣ� tgamma fma erf  cbrt  complex
//ԭ������+�߳̿�
#include <cstdalign>
#include <atomic>


//2^M counters, a counter contains 2^cb bit, d hashfunc
template< unsigned M, unsigned cb, unsigned d >
class lowlayer {
public:
	unsigned threshold; //counter����ֵ,����С�� 2^ (2^cb) 
	uint64_t table[1ull << (M + cb - 6)];   // ��ϣ��ÿ��Ͱ 2^cb λ����64λ���飬һ���� 2^(6-cb)��counter�� �ܹ�2^M��counter 
	
	const uint32_t	sub_hash_bit = 6-cb;	// �ӹ�ϣֵ��λ��sub_hash_bit��λ���Զ�λ64λ���е�counter
	const uint32_t	sub_hash_mask = (1 << (6 - cb)) - 1;

	const uint32_t	hash_index_mask = ((1 << (M + cb - 6)) - 1); // hashֵ�ĵ� M+cb-6λ������
	const uint64_t	counter_mask = (1 << (1 << cb)) - 1;

	lowlayer(unsigned th) : threshold(th)
	{
		assert((1 << (1 << cb)) > th); // ��ֵ���ܱ�counter���ֵ���� 
		memset(table, 0, sizeof(table));
		//printf("configure: sub_hash_mask=%x, hash_index_mask=%x, counter_mask=%llx\n", sub_hash_mask, hash_index_mask, counter_mask);
	}
	void clear() {
		memset(table, 0, sizeof(table));
	}

	void clearCounter(uint64_t& word, uint32_t shift)
	{
		word &= (~(counter_mask << shift));
	}

	void setCounter(uint64_t& word, uint32_t shift, uint64_t val )// val ��������ֵ��
	{
		assert(val <= threshold);
		clearCounter( word, shift );
		word |=  val << shift;
	}

	uint32_t query(unsigned kid)
	{
		uint32_t hval = bobhash(kid);
		//����ϣֵ�ֶΣ���λ��������ȷ����ϣ���е��ĸ�64λ�֣� ��λ�������ڶ�λ���е�d��counter
		uint32_t index = hval & hash_index_mask;
		uint32_t segs = hval >> (32 - d * sub_hash_bit); //��ϣֵ�ĸ�λ��Ϊd��
		//printf("%u hash code %x index=%d segs=%x\n", kid, hval, index, segs);
		uint32_t counter_id = 0;  //���ڵ�counter ��Ŵ�������ȫ1��ȫ0
		uint32_t counter_val[d] = { 0 };
		uint32_t counter_shift[d] = { 0 };
		for (int i = 0; i < d; ++i, segs >>= sub_hash_bit)
		{
			counter_id = segs & sub_hash_mask;
			counter_shift[i] = counter_id * (1 << cb);
			counter_val[i] = (table[index] & (counter_mask << counter_shift[i])) >> counter_shift[i];
			//printf("counter %d id=%u value=%u\n", i, counter_id, counter_val[i]);
		}

		return*std::min_element(counter_val, counter_val + d);
	}

	uint32_t update(unsigned kid) // ����item�����±����ز���ǰ�ļ��� 
	{
		uint32_t hval = bobhash(kid);
		//����ϣֵ�ֶΣ���λ��������ȷ����ϣ���е��ĸ�64λ�֣� ��λ�������ڶ�λ���е�d��counter
		uint32_t index = hval & hash_index_mask;
		uint32_t segs = hval >> ( 32 - d*sub_hash_bit); //��ϣֵ�ĸ�λ��Ϊd��
		//printf("%u hash code %x index=%d segs=%x\n", kid, hval, index, segs);
		uint32_t counter_id = 0;  //���ڵ�counter ��Ŵ�������ȫ1��ȫ0
		uint32_t counter_val[d] = { 0 };
		uint32_t counter_shift[d] = { 0 };
		for (int i = 0; i < d; ++i, segs>>=sub_hash_bit)
		{
			counter_id = segs & sub_hash_mask;
			counter_shift[i] = counter_id * (1 << cb);
			counter_val[i] = (table[index] & (counter_mask << counter_shift[i])) >> counter_shift[i];
			//printf("counter %d id=%u value=%u\n", i, counter_id,  counter_val[i]);
		}

		uint32_t V = 0;  // d��counter�е���Сֵ
		V = *std::min_element(counter_val, counter_val + d);
		
		if (V < threshold)
		{
			for (int i = 0; i < d; i++)
			{
				//printf("word %llu\n", table[index]);
				if (V == counter_val[i])
				{
					setCounter( table[index], counter_shift[i], V+1 ); 
					//��ʹ�⼸��counter�ظ��ˣ�Ҳֻ���ظ�set��V+1
					//��������������£����item��Ӧʵ��counter ����d���������item�ļ�����������ײӰ�죬���¼�������
				}
			}
		}
		return V;
	}

};


