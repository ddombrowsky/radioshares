#include "momentum.h"

namespace bts 
{
	#define MAX_MOMENTUM_NONCE  (1<<26)
	#define SEARCH_SPACE_BITS 50
	#define BIRTHDAYS_PER_HASH 8
	
	std::vector< std::pair<uint32_t,uint32_t> > momentum_search( uint256 midHash ){
		
		std::vector< std::pair<uint32_t,uint32_t> > results;
		
		//Note - unordered_map with pre-allocated should be faster
		std::map<uint64_t,uint32_t > collisionFinder;
		for(uint32_t i=0;i<MAX_MOMENTUM_NONCE;i=i+BIRTHDAYS_PER_HASH){
			//Note - There must be a faster way to create the SHA512 hash
			char midHashPlus[100];
			sprintf(midHashPlus, "%s-%u", midHash.GetHex().c_str(), i);
			string midHashPlusString = string(midHashPlus);
			vector<unsigned char> midHashPlusVector(midHashPlusString.begin(), midHashPlusString.end());
			unsigned char sha512hash[SHA512_DIGEST_LENGTH];
			SHA512(&midHashPlusVector[0], midHashPlusVector.size(), (unsigned char*)&sha512hash);
			//printf("SHA512 %s\n", sha512hash);
	
			//Each SHA512 hash is treated as 8 separate birthday hash results.  	
			for( uint32_t x = 0; x < BIRTHDAYS_PER_HASH; x++ ){
				uint64_t birthday;
				memcpy (&birthday, &sha512hash[x*4], 8);
				//Some bits are discarded to reduce the 64 bit birthday to required number of bits
				birthday = birthday >> (64-SEARCH_SPACE_BITS);
				//printf("birthday hash %llu\n", birthday);
				uint32_t nonce = i+x;
				
				//check for matching birthday hashes
				if(collisionFinder[birthday]!=0){
					//printf("matching birthdays! %u,%u share the birthday %llu\n", collisionFinder[birthday],nonce,birthday);
					results.push_back( std::make_pair( collisionFinder[birthday], nonce ) );
				}
				else{
					collisionFinder[birthday] = nonce;
				}
			}
		}
		return results;
	}

	uint64_t getBirthdayHash(uint256 midHash, uint32_t a){
		//Note - Should be a faster way to create the SHA512 hash
		char midHashPlus[100];
		sprintf(midHashPlus, "%s-%u", midHash.GetHex().c_str(), (a/BIRTHDAYS_PER_HASH)*BIRTHDAYS_PER_HASH);
		string midHashPlusString = string(midHashPlus);
		vector<unsigned char> midHashPlusVector(midHashPlusString.begin(), midHashPlusString.end());
		unsigned char sha512hash[SHA512_DIGEST_LENGTH];
		SHA512(&midHashPlusVector[0], midHashPlusVector.size(), (unsigned char*)&sha512hash);
		uint64_t birthdayHash;
		memcpy (&birthdayHash, &sha512hash[(a%BIRTHDAYS_PER_HASH)*4], 8);
		birthdayHash = birthdayHash >>  (64-SEARCH_SPACE_BITS);
		printf("birthday hash %llu\n", birthdayHash);
		return birthdayHash;
	}

	bool momentum_verify( uint256 midHash, uint32_t a, uint32_t b ){
		if( a == b ) return false;
		if( a > MAX_MOMENTUM_NONCE ) return false;
		if( b > MAX_MOMENTUM_NONCE ) return false;		
		return (getBirthdayHash(midHash,a)==getBirthdayHash(midHash,b));
	}

}