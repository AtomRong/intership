#include <cinttypes>
#include <cstring>
#include <cassert>

#include <algorithm>
#include "bobhash.h"

template<unsigned M, unsigned d=3>
class highlayer {
public:
	unsigned threshold;
	uint16_t table[1<<M];  // cf�ļ������̶� 16 bit
	Bob hashfunc[d];
	highlayer( unsigned th ): threshold(th)
	{
		assert( th <= UINT_FAST16_MAX);
		memset(table, 0, sizeof(table));
		for (unsigned i = 0; i < d; i++) //����d����ͬ�Ĺ�ϣ����
		{
			hashfunc[i].initval = Bob::arbitrary[i];
		}
	}
	void clear() {
		memset(table, 0, sizeof(table));
	}
	uint32_t update(unsigned kid) //���ز���ǰ�ļ���
	{
		uint16_t V = UINT16_MAX;
		uint32_t index[d] = { 0 };

		for (unsigned i = 0; i < d; i++)
		{
			uint32_t hval = hashfunc[i](kid) & ( (1<<M)-1 );  //ȡBobֵ�ĵ�Mλ��Ϊ��ϣ�±�
			index[i] = hval;
			V = std::min( V, table[hval] );
		}
		if (V < threshold) //���¼���
		for (unsigned i = 0; i < d; i++)
		{
			table[index[i]] = std::max<uint16_t>( V+1, table[index[i]] );
		}
		return V;
	}

	uint32_t query(unsigned kid)
	{
		uint16_t V = UINT16_MAX;

		for (unsigned i = 0; i < d; i++)
		{
			uint32_t hval = hashfunc[i](kid) & ((1 << M) - 1);  //ȡBobֵ�ĵ�Mλ��Ϊ��ϣ�±�
			V = std::min(V, table[hval]);
		}
		return V;
	}
};
