#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <cinttypes>
#include <cassert>


#include <algorithm>
#include "bobhash.h"

// 随便试试头文件，原来c标准库有支持很多新东西了，
//例如科学计算/统计的函数、类型， tgamma fma erf  cbrt  complex
//原子类型+线程库
#include <cstdalign>
#include <atomic>


//2^M counters, a counter contains 2^cb bit, d hashfunc
template< unsigned M, unsigned cb, unsigned d >
class lowlayer {
public:
	unsigned threshold; //counter的阈值,必须小于 2^ (2^cb) 
	uint64_t table[1ull << (M + cb - 6)];   // 哈希表，每个桶 2^cb 位，按64位分组，一组有 2^(6-cb)个counter。 总共2^M个counter 
	
	const uint32_t	sub_hash_bit = 6-cb;	// 从哈希值高位的sub_hash_bit个位，以定位64位字中的counter
	const uint32_t	sub_hash_mask = (1 << (6 - cb)) - 1;

	const uint32_t	hash_index_mask = ((1 << (M + cb - 6)) - 1); // hash值的低 M+cb-6位的掩码
	const uint64_t	counter_mask = (1 << (1 << cb)) - 1;

	lowlayer(unsigned th) : threshold(th)
	{
		assert((1 << (1 << cb)) > th); // 阈值不能比counter最大值还大 
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

	void setCounter(uint64_t& word, uint32_t shift, uint64_t val )// val 必须在阈值内
	{
		assert(val <= threshold);
		clearCounter( word, shift );
		word |=  val << shift;
	}

	uint32_t query(unsigned kid)
	{
		uint32_t hval = bobhash(kid);
		//给哈希值分段，低位部分用于确定哈希表中的哪个64位字， 高位部分用于定位字中的d个counter
		uint32_t index = hval & hash_index_mask;
		uint32_t segs = hval >> (32 - d * sub_hash_bit); //哈希值的高位分为d段
		//printf("%u hash code %x index=%d segs=%x\n", kid, hval, index, segs);
		uint32_t counter_id = 0;  //字内的counter 编号从左至右全1到全0
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

	uint32_t update(unsigned kid) // 插入item，更新表，返回插入前的计数 
	{
		uint32_t hval = bobhash(kid);
		//给哈希值分段，低位部分用于确定哈希表中的哪个64位字， 高位部分用于定位字中的d个counter
		uint32_t index = hval & hash_index_mask;
		uint32_t segs = hval >> ( 32 - d*sub_hash_bit); //哈希值的高位分为d段
		//printf("%u hash code %x index=%d segs=%x\n", kid, hval, index, segs);
		uint32_t counter_id = 0;  //字内的counter 编号从左至右全1到全0
		uint32_t counter_val[d] = { 0 };
		uint32_t counter_shift[d] = { 0 };
		for (int i = 0; i < d; ++i, segs>>=sub_hash_bit)
		{
			counter_id = segs & sub_hash_mask;
			counter_shift[i] = counter_id * (1 << cb);
			counter_val[i] = (table[index] & (counter_mask << counter_shift[i])) >> counter_shift[i];
			//printf("counter %d id=%u value=%u\n", i, counter_id,  counter_val[i]);
		}

		uint32_t V = 0;  // d个counter中的最小值
		V = *std::min_element(counter_val, counter_val + d);
		
		if (V < threshold)
		{
			for (int i = 0; i < d; i++)
			{
				//printf("word %llu\n", table[index]);
				if (V == counter_val[i])
				{
					setCounter( table[index], counter_shift[i], V+1 ); 
					//即使这几个counter重复了，也只是重复set成V+1
					//但是在这种情况下，这个item对应实际counter 少于d个，会令该item的计数更易受碰撞影响，导致计数更高
				}
			}
		}
		return V;
	}

};


