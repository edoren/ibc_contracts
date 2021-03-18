/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */

#include <eosiolib/contracts/eosio/action.hpp>
#include <eosiolib/contracts/eosio/transaction.hpp>
#include <ibc.token/ibc.token.hpp>
#include <ibc.proxy/ibc.proxy.hpp>
#include "utils.cpp"

namespace eosio {

   proxy::proxy( name s, name code, datastream<const char*> ds ):
         contract( s, code, ds ),
         _global_state( _self, _self.value ),
         _proxytrxs( _self, _self.value ),
         _proxytrxs2( _self, _self.value )
   {
      _gstate = _global_state.exists() ? _global_state.get() : global_state{};
   }

   proxy::~proxy(){
      _global_state.set( _gstate, _self );
    }
   
   void proxy::setglobal(  name ibc_token_account ) {
      require_auth( _self );
      _gstate.ibc_token_account   = ibc_token_account;
   }

   void proxy::transfer_notify( name token_contract, name from, name to, asset quantity, string memo ){
      if( token_contract != _gstate.ibc_token_account ){
         eosio::check(token::token_contract_registered_in_accepts(_gstate.ibc_token_account,token_contract),
               "token contract not registered in table 'accepts' of ibc.token contract");
      }

      eosio::check( to == _self, "to must be the proxy contract account self");

      auto info = get_memo_info( memo );
      eosio::check( info.receiver != name(),"receiver not provide");
      eosio::check( info.peerchain != name(),"peerchain not provide");
      eosio::check( info.notes.length() <= 64,"memo string too long");

      auto id = _proxytrxs.available_primary_key();
      id = (id == 0 ? 1 : id);
      _proxytrxs.emplace( _self, [&]( auto& r ){
         r.id              = id;
         r.orig_trx_id     = get_trx_id();
         r.block_time_slot = get_block_time_slot();
         r.token_contract  = token_contract;
         r.orig_from       = from;
         r.to              = to;
         r.quantity        = quantity;
         r.orig_memo       = memo;
      });
   }

   void proxy::transfer( name from, name to, asset quantity, string memo ){

      eosio::check( from == _self, "from must be _self");

      string orig_trxid_str = get_value_str_by_key_str( memo, key_orig_trxid );
      eosio::check( ! orig_trxid_str.empty(), ("key: " + key_orig_trxid + " not exist in memo string").c_str());
      checksum256 orig_trx_id = string_to_checksum256( orig_trxid_str );

      auto idx = _proxytrxs.get_index<"trxid"_n>();
      const auto& trx_p = idx.find(fixed_bytes<32>(orig_trx_id));
      eosio::check( trx_p != idx.end(), "transaction not found.");
      eosio::check( quantity == trx_p->quantity, "quantity == trx_p->quantity assert failed");

      if( to == trx_p->orig_from ){
         eosio::check( get_block_time_slot() - trx_p->block_time_slot > 240, "you can't rollback proxy transaction within two minutes");
         string rollback_memo = "rollback transaction: " + checksum256_to_string(orig_trx_id);
         transfer_action_type action_data{ from, to, quantity, rollback_memo };
         action( permission_level{ _self, "active"_n }, trx_p->token_contract, "transfer"_n, action_data ).send();
         _proxytrxs.erase( *trx_p );
         return;
      }

      eosio::check( to == _gstate.ibc_token_account, "to must be ibc_token_account");

      name orig_from = name( get_value_str_by_key_str( memo, key_orig_from ));
      eosio::check( orig_from != name(), ("key: " + key_orig_from + " not exist in memo string").c_str());
      eosio::check( orig_from == trx_p->orig_from, "orig_from == trx_p->from assert failed");

      eosio::check( memo.find(trx_p->orig_memo) == 0, "memo sting must start with user's original memo string");

      string correct_memo_str = trx_p->orig_memo + " " +
            key_orig_trxid + "=" + checksum256_to_string(trx_p->orig_trx_id) + " " +
            key_orig_from + "=" + trx_p->orig_from.to_string();

      eosio::check( memo == correct_memo_str, "memo != correct_memo_str");

      transfer_action_type action_data{ from, to, quantity, memo };
      action( permission_level{ _self, "active"_n }, trx_p->token_contract, "transfer"_n, action_data ).send();

      _proxytrxs.erase( *trx_p );
   }

   void proxy::mvtotrash( transaction_id_type orig_trx_id ){

      auto idx = _proxytrxs.get_index<"trxid"_n>();
      const auto& trx_p = idx.find(fixed_bytes<32>(orig_trx_id));
      eosio::check( trx_p != idx.end(), "transaction not found in table proxytrx.");

      auto duration = 3600*2*12; // half a day
      eosio::check( get_block_time_slot() - trx_p->block_time_slot > duration, "you can't move this proxy transaction to trash within half a day");

      auto idx2 = _proxytrxs2.get_index<"trxid"_n>();
      const auto& trx_p2 = idx2.find(fixed_bytes<32>(orig_trx_id));
      eosio::check( trx_p2 == idx2.end(), "transaction already exist in table proxytrx2");

      _proxytrxs2.emplace( _self, [&]( auto& r ){ r = *trx_p; });
      _proxytrxs.erase( *trx_p );
   }

} /// namespace eosio

extern "C" {
   void apply( uint64_t receiver, uint64_t code, uint64_t action ) {
      if( code == receiver ) {
         switch( action ) {
            EOSIO_DISPATCH_HELPER( eosio::proxy, (setglobal)(transfer)(mvtotrash))
         }
         return;
      }
      if (code != receiver && action == eosio::name("transfer").value) {
         auto args = eosio::unpack_action_data<eosio::transfer_action_type>();
         if( args.to == eosio::name(receiver) && args.quantity.amount > 0 && args.memo.length() > 0 ){
            eosio::proxy thiscontract(eosio::name(receiver), eosio::name(code), eosio::datastream<const char*>(nullptr, 0));
            thiscontract.transfer_notify(eosio::name(code), args.from, args.to, args.quantity, args.memo);
         }
      }
   }
}

