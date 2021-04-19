#include "math.h"
#include <chrono>

namespace vio{


	// find a library to do this! Maybe processor have a build-in thing for this which is faster.
	u32 nChoosek(u32 n, u32 k){
		vassert(k <= n);
		if (k * 2 > n) k = n-k;
		if (k == 0) return 1;

		u32 result = n;
		for(u32 i = 2; i <= k; ++i ) {
			result *= (n-i+1);
			result /= i;
		}
		return result;
	}

	i32 modPow(i32 base,i32 exponent,i32 modulus){
	    if(modulus == 1) return 0;
	    i32 result = 1;
	    base = mod(base,modulus);
	    while(exponent > 0){
	        if(mod(exponent,2) == 1){
	            result = mod(result * base,modulus);
	        }
	        exponent = exponent >> 1;
	        base = mod(base * base,modulus);
	    }
	    return result;
	}

	struct Fasthash{
		i32 a,b,c;
		i32 hash(i32 x){
			uint16_t low = (uint16_t)x;
			uint16_t high = x >> 16;
			return (low * a + high * b + c);
		};
		i32 hash(i32 x,i32 y,i32 z){
			return this->hash(this->hash(this->hash(this->hash(x) ^ y) ^ z) ^ x);
		}
		double randAtInt(i32 x,i32 y,i32 z){
			return ((double)this->hash(x,y,z)) / 2. / (INT_MAX);
		}
	};
	static inline double fade(double t) noexcept{
		return t * t * t * (t * (t * 6 - 15) + 10);
	}
	static inline double lerp(double t, double a, double b) noexcept{
		return a + t * (b - a);
	}
	static inline i32 fastfloor(float a){
	    i32 ai = (i32) a;
	    return (a < ai) ? ai-1 : ai;
	}
	double perlin(double x, double y, double z){
		const int32_t X = fastfloor(x);
		const int32_t Y = fastfloor(y);
		const int32_t Z = fastfloor(z);

		x -= X;
		y -= Y;
		z -= Z;

		const double u = fade(x); // add option for fade?
		const double v = fade(y);
		const double w = fade(z);

		Fasthash fh = Fasthash();
		fh.a = 413441;
		fh.b = 134134;
		fh.c = 431231;

		const double XYZ = fh.randAtInt(X,Y,Z);
		const double XmYZ = fh.randAtInt(X+1,Y,Z);
		const double XYmZ = fh.randAtInt(X,Y+1,Z);
		const double XmYmZ = fh.randAtInt(X+1,Y+1,Z);

		const double XYZm = fh.randAtInt(X,Y,Z+1);
		const double XmYZm = fh.randAtInt(X+1,Y,Z+1);
		const double XYmZm = fh.randAtInt(X,Y+1,Z+1);
		const double XmYmZm = fh.randAtInt(X+1,Y+1,Z+1);

		return lerp(w, 
			lerp(v, lerp(u, XYZ, XmYZ), lerp(u, XYmZ, XmYmZ)),
			lerp(v, lerp(u, XYZm,XmYZm),lerp(u, XYmZm,XmYmZm)))
		*2 - 1;
	}
	double isoperlin(double x,double y,double z){
		return (
				perlin(x,y,z) +
				perlin((y+z)/2,(x+z)/2,(x+y)/2) +
				perlin((y-z)/2,(x-z)/2,(x-y)/2)
		) / 3.;
	}

	// Randomness implementation.
	static constexpr size_t SIZE   = 624;
	static constexpr size_t PERIOD = 397;
	static constexpr size_t DIFF   = SIZE - PERIOD;
	static constexpr u32 MAGIC = 0x9908b0df;

	struct MTState {
		u32 MT[SIZE];
		u32 MT_TEMPERED[SIZE];
		size_t index = SIZE;
	};

	static MTState state;
	static bool seeded = false;

	#define M32(x) (0x80000000 & x) // 32nd MSB
	#define L31(x) (0x7FFFFFFF & x) // 31 LSBs

	#define UNROLL(expr) \
		y = M32(state.MT[i]) | L31(state.MT[i+1]); \
		state.MT[i] = state.MT[expr] ^ (y >> 1) ^ (((i32(y) << 31) >> 31) & MAGIC); \
		++i;

	static void generate_numbers(){
		if(!seeded){seed(342);seeded = true;}
		size_t i = 0;
		u32 y;
		while ( i < DIFF ) {
			UNROLL(i+PERIOD);
		}
		while ( i < SIZE -1 ) {
			UNROLL(i-DIFF);
		}

		{
			// i = 623, last step rolls over
			y = M32(state.MT[SIZE-1]) | L31(state.MT[0]);
			state.MT[SIZE-1] = state.MT[PERIOD-1] ^ (y >> 1) ^ (((i32(y) << 31) >>
						31) & MAGIC);
		}

		// Temper all numbers in a batch
		for (size_t i = 0; i < SIZE; ++i) {
			y = state.MT[i];
			y ^= y >> 11;
			y ^= y << 7  & 0x9d2c5680;
			y ^= y << 15 & 0xefc60000;
			y ^= y >> 18;
			state.MT_TEMPERED[i] = y;
		}

		state.index = 0;
	}

	void seed(u32 value){
		seeded = true;
		state.MT[0] = value;
		// but all the other values of MT to 0 to reset the entropy.
		for(u32 i = 1;i < sizeof(state.MT);i++){
			state.MT[1] = 0;
		}
		state.index = SIZE;

		for ( uint_fast32_t i = 1; i < SIZE; ++i) // shuffle
			state.MT[i] = 0x6c078965*(state.MT[i-1] ^ state.MT[i-1]>>30) + i;
	}
	// provide more entropy for the generator
	void seed(u32 value1,u32 value2){
		seeded = true;
		state.MT[0] = value1;
		state.MT[1] = value2;
		for(u32 i = 2;i < sizeof(state.MT);i++){
			state.MT[1] = 0;
		}
		for (uint_fast32_t i = 2; i < SIZE; ++i) // shuffle
			state.MT[i] = 0x6c078965*(state.MT[i-1] ^ state.MT[i-1]>>30) + i;
		state.index = SIZE;
	}

	u32 unique_random(){
		unsigned long ms = std::chrono::duration_cast<std::chrono::milliseconds>(
			    std::chrono::system_clock::now().time_since_epoch()
			).count();
		// wow, so random
		return ms;
	}
	void seed(){
		seed(unique_random());
	}

	u32 rand_u32(){ // private
		if (state.index == SIZE) {
			generate_numbers();
			state.index = 0;
		}
		return state.MT_TEMPERED[state.index++];
	}

	// Already defined.
	// #define UINT32_MAX  ((u32)-1)

	float randomFloat(){
		u32 i = rand_u32();
		return ((double)i) / UINT32_MAX;
	}
	float random(){
		return randomFloat();
	}

	u32 randomU32(const u32 max){ // from 0 to max included. 100% uniform.
		// compute the next highest power of 2 of 32-bit v (2,3 -> 4, 4,5,6,7 -> 8 etc..)
		u32 p2 = max;
		p2 |= p2 >> 1;
		p2 |= p2 >> 2;
		p2 |= p2 >> 4;
		p2 |= p2 >> 8;
		p2 |= p2 >> 16;

		u32 rnd;
		do{
			rnd = rand_u32() & p2; // fast modulo
		}while(rnd > max);
		return rnd;
	}
	i32 randomI32(const i32 min,const i32 max){
		vassert(max >= min);
		return randomU32(max - min) + min;
	}

	double randAtInt(i32 x,i32 y,i32 z){
		Fasthash fh = Fasthash();
		fh.a = 483401;
		fh.b = 134734;
		fh.c = 931281;
		return fh.randAtInt(x,y,z);
	}
}
