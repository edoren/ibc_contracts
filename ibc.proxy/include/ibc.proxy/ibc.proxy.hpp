/**
 *  @file
 *  @copyright defined in bos/LICENSE.txt
 */
#pragma once

#include <eosiolib/core/eosio/asset.hpp>
#include <eosiolib/contracts/eosio/eosio.hpp>
#include <eosiolib/contracts/eosio/singleton.hpp>
#include <ibc.chain/types.hpp>
#include <ibc.token/ibc.token.hpp>

namespace eosio {

   static const string key_orig_from = "orig_from";
   static const string key_orig_trxid = "orig_trxid";

   class [[eosio::contract("ibc.proxy")]] proxy : public contract {
      public:
      proxy( name s, name code, datastream<const char*> ds );
      ~proxy();


      [[eosio::action]]
      void setglobal( name ibc_token_account );

      // called in C apply function
      void transfer_notify( name    code,
                            name    from,
                            name    to,
                            asset   quantity,
                            string  memo );

      [[eosio::action]]
      void transfer( name from, name to, asset quantity, string memo );

      [[eosio::action]]
      void mvtotrash( transaction_id_type orig_trx_id );

      struct [[eosio::table("globals")]] global_state {
         global_state(){}
         name              ibc_token_account;
         EOSLIB_SERIALIZE( global_state, (ibc_token_account))
      };

   private:
      eosio::singleton< "globals"_n, global_state >   _global_state;
      global_state                                    _gstate;

      // use to record accepted ibc transactions
      // code,scope(_self,_self.value)
      struct [[eosio::table]] proxy_trx_info {
         uint64_t                id; // auto-increment
         transaction_id_type     orig_trx_id;
         uint64_t                block_time_slot;
         name                    token_contract;
         name                    orig_from;
         name                    to; // always be self
         asset                   quantity;
         string                  orig_memo;

         uint64_t primary_key()const { return id; }
         fixed_bytes<32> by_trx_id()const { return fixed_bytes<32>(orig_trx_id); }
      };
      eosio::multi_index< "proxytrxs"_n, proxy_trx_info,
      indexed_by<"trxid"_n, const_mem_fun<proxy_trx_info, fixed_bytes<32>, &proxy_trx_info::by_trx_id> >
      > _proxytrxs;

      eosio::multi_index< "proxytrxs2"_n, proxy_trx_info,
      indexed_by<"trxid"_n, const_mem_fun<proxy_trx_info, fixed_bytes<32>, &proxy_trx_info::by_trx_id> >
      > _proxytrxs2;

   };

} /// namespace eosio
