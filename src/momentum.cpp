#include "momentum.h"
#include <boost/unordered_map.hpp>

namespace bts 
{
	#define MAX_MOMENTUM_NONCE  (1<<26)
	#define SEARCH_SPACE_BITS 50
	#define BIRTHDAYS_PER_HASH 8
	
	std::vector< std::pair<uint32_t,uint32_t> > momentum_search( uint256 midHash )
  {
      boost::unordered_map<uint64_t,uint32_t>  found;
      found.reserve( MAX_MOMENTUM_NONCE);
      std::vector< std::pair<uint32_t,uint32_t> > results;

      char  hash_tmp[sizeof(midHash)+4];
      memcpy((char*)&hash_tmp[4], (char*)&midHash, sizeof(midHash) );
      uint32_t* index = (uint32_t*)hash_tmp;

      for( uint32_t i = 0; i < MAX_MOMENTUM_NONCE;  )
      {
          *index = i;
          uint64_t  result_hash[8];
			    SHA512((unsigned char*)hash_tmp, sizeof(hash_tmp), (unsigned char*)result_hash);
        
          for( uint32_t x = 0; x < 8; ++x )
          {
              uint64_t birthday = result_hash[x] >> (64-SEARCH_SPACE_BITS);
              uint32_t nonce = i+x;
              boost::unordered_map<uint64_t,uint32_t>::const_iterator itr = found.find( birthday );
              if( itr != found.end() )
              {
                  results.push_back( std::make_pair( itr->second, nonce ) );
              }
              else
              {
                  found[birthday] = nonce;
              }
          }
          i += 8;
      }
      return results;
   }


	uint64_t getBirthdayHash(uint256 midHash, uint32_t a)
  {

    char  hash_tmp[sizeof(midHash)+4];
    memcpy((char*)&hash_tmp[4], (char*)&midHash, sizeof(midHash) );
    uint32_t* index = (uint32_t*)hash_tmp;
    *index = a-a%BIRTHDAYS_PER_HASH;

    uint64_t  result_hash[8];
		SHA512((unsigned char*)hash_tmp, sizeof(hash_tmp), (unsigned char*)result_hash);

    return result_hash[a%BIRTHDAYS_PER_HASH]>>(64-SEARCH_SPACE_BITS);
	}

	bool momentum_verify( uint256 midHash, uint32_t a, uint32_t b ){
		if( a == b ) return false;
		if( a > MAX_MOMENTUM_NONCE ) return false;
		if( b > MAX_MOMENTUM_NONCE ) return false;		
		return (getBirthdayHash(midHash,a)==getBirthdayHash(midHash,b));
	}

}
