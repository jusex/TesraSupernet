







#ifndef CRYPTOPP_MERSENNE_TWISTER_H
#define CRYPTOPP_MERSENNE_TWISTER_H

#include "cryptlib.h"
#include "secblock.h"
#include "misc.h"

NAMESPACE_BEGIN(CryptoPP)













template <unsigned int K, unsigned int M, unsigned int N, unsigned int F, unsigned long S>
class MersenneTwister : public RandomNumberGenerator
{
public:
	
	
	
	
	MersenneTwister(unsigned long seed = S) : m_seed(seed), m_idx(N)
	{
		m_state[0] = seed;
		for (unsigned int i = 1; i < N+1; i++)
			m_state[i] = word32(F * (m_state[i-1] ^ (m_state[i-1] >> 30)) + i);
	}

	
	
	
	
	
	
	
	void GenerateBlock(byte *output, size_t size)
	{
		
		word32 temp;
		for (size_t i=0; i < size/4; i++, output += 4)
		{
#if defined(CRYPTOPP_ALLOW_UNALIGNED_DATA_ACCESS) && defined(IS_LITTLE_ENDIAN)
			*((word32*)output) = ByteReverse(NextMersenneWord());
#elif defined(CRYPTOPP_ALLOW_UNALIGNED_DATA_ACCESS)
			*((word32*)output) = NextMersenneWord();
#else
			temp = NextMersenneWord();
			output[3] = CRYPTOPP_GET_BYTE_AS_BYTE(temp, 0);
			output[2] = CRYPTOPP_GET_BYTE_AS_BYTE(temp, 1);
			output[1] = CRYPTOPP_GET_BYTE_AS_BYTE(temp, 2);
			output[0] = CRYPTOPP_GET_BYTE_AS_BYTE(temp, 3);
#endif
		}

		
		if (size%4 == 0)
		{
			
			*((volatile word32*)&temp) = 0;
			return;
		}

		
		temp = NextMersenneWord();
		switch (size%4)
		{
			case 3: output[2] = CRYPTOPP_GET_BYTE_AS_BYTE(temp, 1); 
			case 2: output[1] = CRYPTOPP_GET_BYTE_AS_BYTE(temp, 2); 
			case 1: output[0] = CRYPTOPP_GET_BYTE_AS_BYTE(temp, 3); break;

			default: CRYPTOPP_ASSERT(0); ;;
		}

		
		*((volatile word32*)&temp) = 0;
	}

	
	
	
	
	word32 GenerateWord32(word32 min=0, word32 max=0xffffffffL)
	{
		const word32 range = max-min;
		if (range == 0xffffffffL)
			return NextMersenneWord();

		const int maxBits = BitPrecision(range);
		word32 value;

		do{
			value = Crop(NextMersenneWord(), maxBits);
		} while (value > range);

		return value+min;
	}

	
	
	
	
	
	
	void DiscardBytes(size_t n)
	{
		for(size_t i=0; i < RoundUpToMultipleOf(n, 4U); i++)
			NextMersenneWord();
	}

protected:

	
	
	
	
	word32 NextMersenneWord()
	{
		if (m_idx >= N) { Twist(); }

		word32 temp = m_state[m_idx++];

		temp ^= (temp >> 11);
		temp ^= (temp << 7)  & 0x9D2C5680; 
		temp ^= (temp << 15) & 0xEFC60000; 

		return temp ^ (temp >> 18);
	}

	
	void Twist()
	{
		static const unsigned long magic[2]={0x0UL, K};
		word32 kk, temp;

		CRYPTOPP_ASSERT(N >= M);
		for (kk=0;kk<N-M;kk++)
		{
			temp = (m_state[kk] & 0x80000000)|(m_state[kk+1] & 0x7FFFFFFF);
			m_state[kk] = m_state[kk+M] ^ (temp >> 1) ^ magic[temp & 0x1UL];
		}

		for (;kk<N-1;kk++)
		{
			temp = (m_state[kk] & 0x80000000)|(m_state[kk+1] & 0x7FFFFFFF);
			m_state[kk] = m_state[kk+(M-N)] ^ (temp >> 1) ^ magic[temp & 0x1UL];
		}

		temp = (m_state[N-1] & 0x80000000)|(m_state[0] & 0x7FFFFFFF);
		m_state[N-1] = m_state[M-1] ^ (temp >> 1) ^ magic[temp & 0x1UL];

		
		m_idx = 0;

		
		*((volatile word32*)&temp) = 0;
	}

private:

	
	FixedSizeSecBlock<word32, N+1> m_state;
	
	unsigned int m_seed;
	
	unsigned int m_idx;
};







#if CRYPTOPP_DOXYGEN_PROCESSING
class MT19937 : public MersenneTwister<0x9908B0DF , 397, 624, 0x10DCD , 4537> {};
#else
typedef MersenneTwister<0x9908B0DF , 397, 624, 0x10DCD , 4537> MT19937;
#endif








#if CRYPTOPP_DOXYGEN_PROCESSING
class MT19937ar : public MersenneTwister<0x9908B0DF , 397, 624, 0x6C078965 , 5489> {};
#else
typedef MersenneTwister<0x9908B0DF , 397, 624, 0x6C078965 , 5489> MT19937ar;
#endif

NAMESPACE_END

#endif 

