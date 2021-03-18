/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#pragma once

#include <string>
#include <eosiolib/core/eosio/varint.hpp>
#include <eosiolib/contracts/eosio/privileged.hpp>

namespace eosio {

   using std::string;

   typedef checksum256   digest_type;
   typedef checksum256   block_id_type;
   typedef checksum256   chain_id_type;
   typedef checksum256   transaction_id_type;
   typedef signature     signature_type;

   template<typename T>
   void push(T&){}

   template<typename Stream, typename T, typename ... Types>
   void push(Stream &s, T arg, Types ... args){
      s << arg;
      push(s, args...);
   }

   template<class ... Types> checksum256 get_checksum256(const Types & ... args ){
      datastream <size_t> ps;
      push(ps, args...);
      size_t size = ps.tellp();

      std::vector<char> result;
      result.resize(size);

      datastream<char *> ds(result.data(), result.size());
      push(ds, args...);
      checksum256 digest;
      assert_sha256(result.data(), result.size(), digest);
      return digest;
   }

   inline bool is_equal_checksum256( checksum256 a, checksum256 b ){
      return a == b;
   }
}
