#include <cinttypes>
#include <cstring>
#include <cassert>

#include <algorithm>
#include "bobhash.h"

template<unsigned M,  unsigned d=3>
class Cmcu { // count min with conservative update
public:
	uint32_t table[d][M];
	Bob hashfunc[d];
	Cmcu()
	{
		memset( table, 0, sizeof(table) );
		for (unsigned i = 0; i < d; i++) //����d����ͬ�Ĺ�ϣ����
		{
			hashfunc[i].initval = Bob::arbitrary[3+i];
		}
	}
	void clear() {
		memset(table, 0, sizeof(table));
	}
	uint32_t update(unsigned kid)
	{
		uint32_t V = UINT_FAST32_MAX, index[d] = { 0 };

		for (unsigned i = 0; i < d; i++)
		{
			uint32_t hval = hashfunc[i](kid) % M;
			index[i] = hval;
			V = std::min(V, table[i][hval]);
		}
		
		for (unsigned i = 0; i < d; i++)
		{
			table[i][index[i]] = std::max(V + 1, table[i][index[i]]);
		}
		return V+1; //���ظ��º�ļ���
	}

	uint32_t query(unsigned kid)
	{
		uint32_t V = UINT_FAST32_MAX;

		for (unsigned i = 0; i < d; i++)
		{
			uint32_t hval = hashfunc[i](kid) % M;
			V = std::min(V, table[i][hval]);
		}
		return V;
	}
};

