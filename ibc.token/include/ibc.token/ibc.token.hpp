/**
 *  @file
 *  @copyright defined in bos/LICENSE.txt
 */
#pragma once

#include <eosiolib/core/eosio/asset.hpp>
#include <eosiolib/contracts/eosio/eosio.hpp>
#include <eosiolib/contracts/eosio/singleton.hpp>
#include <ibc.token/types.hpp>
#include <ibc.proxy/ibc.proxy.hpp>

namespace eosio {

   struct transfer_action_type {
      name    from;
      name    to;
      asset   quantity;
      string  memo;

      EOSLIB_SERIALIZE( transfer_action_type, (from)(to)(quantity)(memo) )
   };

   struct transfer_action_info {
      name    contract;
      name    from;
      asset   quantity;

      EOSLIB_SERIALIZE( transfer_action_info, (contract)(from)(quantity) )
   };

   struct cash_action_type {
      uint64_t                               seq_num;
      name                                   from_chain;
      transaction_id_type                    orig_trx_id;
      std::vector<char>                      orig_trx_packed_trx_receipt;
      std::vector<checksum256>          orig_trx_merkle_path;
      uint32_t                               orig_trx_block_num;
      std::vector<char>                      orig_trx_block_header_data;
      std::vector<checksum256>          orig_trx_block_id_merkle_path;
      uint32_t                               anchor_block_num;
      name                                   to;
      asset                                  quantity;
      string                                 memo;
      name                                   relay;

      EOSLIB_SERIALIZE( cash_action_type, (seq_num)(from_chain)(orig_trx_id)(orig_trx_packed_trx_receipt)
                        (orig_trx_merkle_path)(orig_trx_block_num)(orig_trx_block_header_data)
                        (orig_trx_block_id_merkle_path)(anchor_block_num)(to)(quantity)(memo)(relay) )
   };

   const static uint32_t default_max_trxs_per_minute_per_token = 100;

   class [[eosio::contract("ibc.token")]] token : public contract {
      public:
      token( name s, name code, datastream<const char*> ds );
      ~token();

      [[eosio::action]]
      void setglobal( name this_chain, bool active );

      [[eosio::action]]
      void setgactive( bool value );

      [[eosio::action]]
      void setadmin( name admin );

      [[eosio::action]]
      void setproxy( name proxy );

      [[eosio::action]]
      void setprchproxy( name peerchain_name, name proxy_account);

      [[eosio::action]]
      void regpeerchain( name           peerchain_name,
                         string         peerchain_info,
                         name           peerchain_ibc_token_contract,
                         name           thischain_ibc_chain_contract,
                         name           thischain_free_account,
                         uint32_t       max_original_trxs_per_block,
                         uint32_t       max_origtrxs_table_records,
                         uint32_t       cache_cashtrxs_table_records,
                         bool           active );

      [[eosio::action]]
      void setchainbool( name peerchain_name, string which, bool value );

      [[eosio::action]]
      void regacpttoken( name        original_contract,
                         asset       max_accept,
                         asset       min_once_transfer,
                         asset       max_once_transfer,
                         asset       max_daily_transfer,
                         uint32_t    max_tfs_per_minute, // 0 means the default value defined by default_max_trxs_per_minute_per_token
                         string      organization,
                         string      website,
                         name        administrator,
                         name        service_fee_mode,
                         asset       service_fee_fixed,
                         double      service_fee_ratio,
                         asset       failed_fee,
                         bool        active );  // when non active, transfer not allowed, but cash trigger by peerchain withdraw can still execute

      [[eosio::action]]
      void setacptasset( symbol_code symcode, string which, asset quantity );

      [[eosio::action]]
      void setacptstr( symbol_code symcode, string which, string value );

      [[eosio::action]]
      void setacptint( symbol_code symcode, string which, uint64_t value );

      [[eosio::action]]
      void setacptbool( symbol_code symcode, string which, bool value );

      [[eosio::action]]
      void setacptfee( symbol_code symcode,
                       name   kind,   // "success"_n or "failed"_n
                       name   fee_mode,
                       asset  fee_fixed,
                       double fee_ratio );

      [[eosio::action]]
      void regpegtoken( name        peerchain_name,
                        name        peerchain_contract,  // the original token contract on peer chain
                        asset       max_supply,
                        asset       min_once_withdraw,
                        asset       max_once_withdraw,
                        asset       max_daily_withdraw,
                        uint32_t    max_wds_per_minute,  // 0 means the default value defined by default_max_trxs_per_minute_per_token
                        name        administrator,
                        asset       failed_fee,
                        bool        active );   // when non active, withdraw not allowed, but cash which trigger by peerchain transfer can still execute

#ifdef HUB
      [[eosio::action]]
      void regpegtoken2(name        peerchain_name,
                        name        peerchain_contract,  // the original token contract on peer chain
                        asset       max_supply,
                        asset       min_once_transfer,
                        asset       max_once_transfer,
                        asset       max_daily_transfer,
                        uint32_t    max_tfs_per_minute,  // 0 means the default value defined by default_max_trxs_per_minute_per_token
                        string      organization,
                        string      website,
                        name        administrator,
                        name        service_fee_mode,
                        asset       service_fee_fixed,
                        double      service_fee_ratio,
                        asset       failed_fee,
                        bool        active );   // when non active, withdraw not allowed, but cash which trigger by peerchain transfer can still execute
#endif

      [[eosio::action]]
      void setpegasset( symbol_code symcode, string which, asset quantity );

      [[eosio::action]]
      void setpegint( symbol_code symcode, string which, uint64_t value );

      [[eosio::action]]
      void setpegbool( symbol_code symcode, string which, bool value );

      [[eosio::action]]
      void setpegtkfee( symbol_code symcode,
                        name   kind,
                        name   fee_mode,
                        asset  fee_fixed,
                        double fee_ratio );

      [[eosio::action]]
      void unregtoken( name table, symbol_code sym_code );

      // called in C apply function
      void transfer_notify( name    code,
                            name    from,
                            name    to,
                            asset   quantity,
                            string  memo );

      [[eosio::action]]
      void transfer( name    from,
                     name    to,
                     asset   quantity,
                     string  memo );

      // called by ibc plugin
      [[eosio::action]]
      void cash( const uint64_t&                        seq_num,
                 const name&                            from_chain,
                 const transaction_id_type&             orig_trx_id,          // redundant, facilitate indexing and checking
                 const std::vector<char>&               orig_trx_packed_trx_receipt,
                 const std::vector<checksum256>&   orig_trx_merkle_path,
                 const uint32_t&                        orig_trx_block_num,   // redundant, facilitate indexing and checking
                 const std::vector<char>&               orig_trx_block_header,
                 const std::vector<checksum256>&   orig_trx_block_id_merkle_path,
                 const uint32_t&                        anchor_block_num,
                 const name&                            to,                   // redundant, facilitate indexing and checking
                 const asset&                           quantity,             // redundant, facilitate indexing and checking
                 const string&                          memo,
                 const name&                            relay );

      // called by ibc plugin
      [[eosio::action]]
      void cashconfirm( const name&                            from_chain,
                        const transaction_id_type&             cash_trx_id,            // redundant, facilitate indexing and checking
                        const std::vector<char>&               cash_trx_packed_trx_receipt,
                        const std::vector<checksum256>&   cash_trx_merkle_path,
                        const uint32_t&                        cash_trx_block_num,     // redundant, facilitate indexing and checking
                        const std::vector<char>&               cash_trx_block_header,
                        const std::vector<checksum256>&   cash_trx_block_id_merkle_path,
                        const uint32_t&                        anchor_block_num,
                        const transaction_id_type&             orig_trx_id );          // redundant, facilitate indexing and checking

      // called by ibc plugin repeatedly
      [[eosio::action]]
      void rollback( name peerchain_name, const transaction_id_type trx_id, name relay );   // check if any orignal transactions should be rollback, rollback them if have

      // called by ibc plugin repeatedly when there are unrollbackable original transactions
      [[eosio::action]]
      void rmunablerb( name peerchain_name, const transaction_id_type trx_id, name relay );   // force to remove unrollbackable transaction

      // this action maybe needed when repairing the ibc system manually
      [[eosio::action]]
      void fcrollback( name peerchain_name, const std::vector<transaction_id_type> trxs );   // force rollback

      // this action maybe needed when can not rollback (because eosio account can refuse transfer token to it)
      [[eosio::action]]
      void fcrmorigtrx( name peerchain_name, const std::vector<transaction_id_type> trxs );   // force remove original transaction records, the parameter must be trx_id, in order to query the original transaction conveniently in the later period.

      [[eosio::action]]
      void lockall();   // when locked, ibc-transfer and withdraw will not allowed to execute for all token

      [[eosio::action]]
      void unlockall();   // when unlocked, the restrictions caused by execute lockall function will be removed

      // this action maybe needed when repairing the ibc system manually
      [[eosio::action]]
      void forceinit( name peerchain_name ); //force init

      [[eosio::action]]
      void open( name owner, const symbol_code& symcode, name ram_payer );

      [[eosio::action]]
      void close( name owner, const symbol_code& symcode );

      [[eosio::action]]
      void setfreeacnt( name peerchain_name, name account );

      [[eosio::action]]
      void mvunrtotbl2( name peerchain_name, uint64_t id, const transfer_action_info transfer_para );

      [[eosio::action]]
      void rbkunrbktrx( const transaction_id_type trx_id );

#ifdef HUB
      [[eosio::action]]
      void hubinit( name hub_account );

      // inline action
      [[eosio::action]] /// used to support blockchain browser display detail information
      void feetransfer( name from, name to, asset quantity, string memo );

      [[eosio::action]]
      void rbkdiehubtrx( const transaction_id_type& hub_trx_id );
#endif
      static asset get_supply( name token_contract_account, symbol_code sym_code )
      {
         stats statstable( token_contract_account, token_contract_account.value );
         const auto& st = statstable.get( sym_code.raw() );
         return st.supply;
      }

      static asset get_balance( name token_contract_account, name owner, symbol_code sym_code )
      {
         accounts accountstable( token_contract_account, owner.value );
         const auto& ac = accountstable.get( sym_code.raw() );
         return ac.balance;
      }

      struct [[eosio::table("globals")]] global_state {
         global_state(){}

         name              this_chain;
         bool              active = true; // use as global locks

         // explicit serialization macro is necessary, without this, error "Exceeded call depth maximum" will occur when call state_singleton.set(state)
         EOSLIB_SERIALIZE( global_state, (this_chain)(active))
      };

      struct [[eosio::table("admin")]] admin_struct {
         name              admin;
         EOSLIB_SERIALIZE( admin_struct, (admin))
      };

      struct [[eosio::table("proxy")]] proxy_struct {
         name              proxy;
         EOSLIB_SERIALIZE( proxy_struct, (proxy))
      };

      // code,scope (_self,_self)
      struct [[eosio::table]] currency_accept {
         name        original_contract;
         asset       accept;
         asset       max_accept;
         asset       min_once_transfer;
         asset       max_once_transfer;
         asset       max_daily_transfer;
         uint32_t    max_tfs_per_minute;  // max transfer transactions per minute
         string      organization;
         string      website;
         name        administrator;
         name        service_fee_mode;    // "fixed"_n or "ratio"_n
         asset       service_fee_fixed;
         double      service_fee_ratio;
         asset       failed_fee;
         asset       total_transfer;
         uint64_t    total_transfer_times;
         asset       total_cash;
         uint64_t    total_cash_times;
         bool        active;

         struct currency_accept_mutables {
            uint32_t    minute_trx_start;
            uint32_t    minute_trxs;
            uint32_t    daily_tf_start;
            asset       daily_tf_sum;
            uint32_t    daily_wd_start;
            asset       daily_wd_sum;
         } mutables;

         uint64_t  primary_key()const { return accept.symbol.code().raw(); } /// by token symbol
         uint64_t  by_original_contract()const { return original_contract.value; }
      };
      typedef eosio::multi_index< "accepts"_n, currency_accept,
            indexed_by<"origcontract"_n, const_mem_fun<currency_accept, uint64_t, &currency_accept::by_original_contract> >
      > accepts_table;

      static bool token_contract_registered_in_accepts( name ibc_token_account, name token_contract ) {
         auto _accepts = accepts_table( ibc_token_account, ibc_token_account.value );
         auto idx = _accepts.get_index<"origcontract"_n>();
         auto itr = idx.find( token_contract.value );
         return itr == idx.end() ? false : true;
      }

   private:
      eosio::singleton< "globals"_n, global_state >   _global_state;
      global_state                                    _gstate;
      eosio::singleton< "admin"_n, admin_struct >     _admin_sg;
      admin_struct                                    _admin_st;
      eosio::singleton< "proxy"_n, proxy_struct >     _proxy_sg;
      proxy_struct                                    _proxy_st;

      // code,scope (_self,_self)
      struct [[eosio::table("freeaccount")]] peer_chain_free_account {
         name     peerchain_name;
         name     peerchain_account;

         uint64_t primary_key()const { return peerchain_name.value; }
         EOSLIB_SERIALIZE( peer_chain_free_account, (peerchain_name)(peerchain_account))
      };
      eosio::multi_index< "freeaccount"_n, peer_chain_free_account > _freeaccount;

      // code,scope (_self,_self)
      struct [[eosio::table("peerchains")]] peer_chain_state {
         name           peerchain_name;
         string         peerchain_info;
         name           peerchain_ibc_token_contract;
         name           thischain_ibc_chain_contract;
         name           thischain_free_account; // used by IBC monitor system, transactions which transfer token from or to this account have no charge
         uint32_t       max_original_trxs_per_block = 0;
         uint32_t       max_origtrxs_table_records = 0;
         uint32_t       cache_cashtrxs_table_records = 0;
         bool           active = true;

         uint64_t primary_key()const { return peerchain_name.value; }
         EOSLIB_SERIALIZE( peer_chain_state, (peerchain_name)(peerchain_info)(peerchain_ibc_token_contract)
                           (thischain_ibc_chain_contract)(thischain_free_account)(max_original_trxs_per_block)
                           (max_origtrxs_table_records)(cache_cashtrxs_table_records)(active))
      };
      eosio::multi_index< "peerchains"_n, peer_chain_state > _peerchains;

      // code,scope (_self,_self)
      struct [[eosio::table("peerchains2")]] peer_chain_state2 {
         name           peerchain_name;
         name           proxy_account;

         uint64_t primary_key()const { return peerchain_name.value; }
         EOSLIB_SERIALIZE( peer_chain_state2, (peerchain_name)(proxy_account))
      };
      eosio::multi_index< "peerchains2"_n, peer_chain_state2 > _peerchains2;


      // code,scope(_self,peerchain_name.value)
      struct [[eosio::table("chainassets")]] peer_chain_asset {
         asset    balance;

         uint64_t primary_key()const { return balance.symbol.code().raw(); }
      };
      typedef eosio::multi_index< "chainassets"_n, peer_chain_asset > chainassets_table;


      // code,scope (_self,_self)
      struct [[eosio::table("peerchainm")]] peer_chain_mutable {
         peer_chain_mutable(){}

         name        peerchain_name;
         uint64_t    cash_seq_num = 0;    // set by seq_num in cash action from cashconfirm action, and must be increase one by one, and start from one
         uint32_t    last_confirmed_orig_trx_block_time_slot = 0; // used to determine which failed original transactions should be rolled back
         uint32_t    current_block_time_slot = 0;
         uint32_t    current_block_trxs = 0;
         uint64_t    origtrxs_tb_next_id = 1; // used to retain an incremental id for table origtrxs

         uint64_t primary_key()const { return peerchain_name.value; }
         EOSLIB_SERIALIZE( peer_chain_mutable, (peerchain_name)(cash_seq_num)(last_confirmed_orig_trx_block_time_slot)
                           (current_block_time_slot)(current_block_trxs)(origtrxs_tb_next_id) )
      };
      eosio::multi_index< "peerchainm"_n, peer_chain_mutable > _peerchainm;


      accepts_table     _accepts;
      const currency_accept& get_currency_accept( symbol_code symcode );

      // code,scope (_self,_self)
      struct [[eosio::table]] currency_stats {
         name        peerchain_name;
         name        peerchain_contract;
         asset       supply;
         asset       max_supply;
         asset       min_once_withdraw;
         asset       max_once_withdraw;
         asset       max_daily_withdraw;
         uint32_t    max_wds_per_minute;  // max withdraw transactions per minute
         string      organization;
         string      website;
         name        administrator;
         name        service_fee_mode;    // "fixed"_n or "ratio"_n
         asset       service_fee_fixed;
         double      service_fee_ratio = 0;
         asset       failed_fee;
         asset       total_issue;
         uint64_t    total_issue_times;
         asset       total_withdraw;
         uint64_t    total_withdraw_times;
         bool        active;
         bool        hub_enable = false;

         struct currency_stats_mutables {
            uint32_t    minute_trx_start;
            uint32_t    minute_trxs;
            uint32_t    daily_isu_start;
            asset       daily_isu_sum;
            uint32_t    daily_wd_start;
            asset       daily_wd_sum;
         } mutables;

         uint64_t primary_key()const { return supply.symbol.code().raw(); }
      };
      typedef eosio::multi_index< "stats"_n, currency_stats > stats;
      stats _stats;
      const currency_stats& get_currency_stats( symbol_code symcode );

      struct [[eosio::table]] account {
         asset    balance;

         uint64_t primary_key()const { return balance.symbol.code().raw(); }
      };
      typedef eosio::multi_index< "accounts"_n, account > accounts;

      // table stat is just used for command 'cleos get currency stats ...' compatibility
      // code,scope (_self,sym_code)
      struct [[eosio::table]] currency_stats2 {
         asset    supply;
         asset    max_supply;
         name     issuer;

         uint64_t primary_key()const { return supply.symbol.code().raw(); }
      };
      typedef eosio::multi_index< "stat"_n, currency_stats2 > stats2;

      void update_stats2( symbol_code sym_code );

      // use to record accepted transfer and withdraw transactions
      // code,scope(_self,peerchain_name.value)
      struct [[eosio::table]] original_trx_info {
         uint64_t                id; // auto-increment
         uint64_t                block_time_slot; // new record must not decrease time slot,
         transaction_id_type     trx_id;
         transfer_action_info    action; // very important infomation, used when execute rollback


         uint64_t primary_key()const { return id; }
         uint64_t by_time_slot()const { return block_time_slot; }
         fixed_bytes<32> by_trx_id()const { return fixed_bytes<32>(trx_id); }
      };
      typedef eosio::multi_index< "origtrxs"_n, original_trx_info,
         indexed_by<"tslot"_n, const_mem_fun<original_trx_info, uint64_t,        &original_trx_info::by_time_slot> >,  // used by ibc plugin
         indexed_by<"trxid"_n, const_mem_fun<original_trx_info, fixed_bytes<32>, &original_trx_info::by_trx_id> >
      >  origtrxs_table;

      void origtrxs_emplace( name peerchain_name, transfer_action_info action, transaction_id_type trx_id );
      void rollback_trx( name peerchain_name, transaction_id_type trx_id );
      transfer_action_info get_orignal_action_by_trx_id( name peerchain_name, transaction_id_type trx_id );
      void erase_record_in_origtrxs_tb_by_trx_id_for_confirmed( name peerchain_name, transaction_id_type trx_id );


      /**
       * Note:
       * "orig_trx_block_num" is a very important parameter, in order to prevent replay attacks:
       * first, new record's block_num must not less then the highest block_num in the table,
       *        (so, the ibc plugin is required to take a mechanism to ensure that the original transaction is sent to this contract in the order in which it occured)
       * second, when delete old records, it is important to ensure that the records of heighest two block number must retain.
       * The above two features must be satisfied at the same time. Breaking any one of them will lead to serious replay attacks.
       * set cache_cashtrxs_table_records parameter, when above feature satified, this parameter will take effect
       */
      // code,scope(_self,peerchain_name.value)
      struct [[eosio::table]] cash_trx_info {
         uint64_t              seq_num; // set by seq_num in cash action, and must be increase one by one, and start from 1
         uint64_t              block_time_slot;
         checksum256      trx_id;
         transfer_action_type  action;                // redundant, facilitate indexing and checking
         checksum256      orig_trx_id;           // redundant, facilitate indexing and checking
         uint64_t              orig_trx_block_num;    // very important!

         uint64_t primary_key()const { return seq_num; }
         uint64_t by_time_slot()const { return block_time_slot; }
         fixed_bytes<32> by_orig_trx_id()const { return fixed_bytes<32>(orig_trx_id); }
         uint64_t by_orig_trx_block_num()const { return orig_trx_block_num; }
      };
      typedef eosio::multi_index< "cashtrxs"_n, cash_trx_info,
         indexed_by<"tslot"_n,    const_mem_fun<cash_trx_info, uint64_t,        &cash_trx_info::by_time_slot> >,  // used by ibc plugin
         indexed_by<"trxid"_n,    const_mem_fun<cash_trx_info, fixed_bytes<32>, &cash_trx_info::by_orig_trx_id> >,
         indexed_by<"blocknum"_n, const_mem_fun<cash_trx_info, uint64_t,        &cash_trx_info::by_orig_trx_block_num> >
      > cashtrxs_table;

      void trim_cashtrxs_table_or_not( name peerchain_name );
      uint64_t get_cashtrxs_tb_max_seq_num( name peerchain_name );
      uint64_t get_cashtrxs_tb_min_orig_trx_block_num( name peerchain_name );
      uint64_t get_cashtrxs_tb_max_orig_trx_block_num( name peerchain_name );
      bool is_orig_trx_id_exist_in_cashtrxs_tb( name peerchain_name, transaction_id_type orig_trx_id );

      // use to record removed unrollbackable transactions
      // code,scope(_self,peerchain_name.value)
      struct [[eosio::table]] deleted_unrollbackable_trx_info {
         uint64_t                id; // auto-increment
         transaction_id_type     trx_id;

         uint64_t primary_key()const { return id; }
      };
      typedef eosio::multi_index< "rmdunrbs"_n, deleted_unrollbackable_trx_info>  rmdunrbs_table;

      // use to record removed unrollbackable transactions
      // code,scope(_self,_self.value)
      struct [[eosio::table]] deleted_unrollbackable_trx_info2 {
         uint64_t                id; // auto-increment
         name                    peerchain;
         transaction_id_type     trx_id;
         transfer_action_info    action; // used when execute rollback

         uint64_t primary_key()const { return id; }
         fixed_bytes<32> by_trx_id()const { return fixed_bytes<32>(trx_id); }
      };
      typedef eosio::multi_index< "rmdunrbs2"_n, deleted_unrollbackable_trx_info2,
          indexed_by<"trxid"_n, const_mem_fun<deleted_unrollbackable_trx_info2, fixed_bytes<32>, &deleted_unrollbackable_trx_info2::by_trx_id> >
      >  rmdunrbs_table2;

      void withdraw( name from, name peerchain_name, name peerchain_receiver, asset quantity, string memo );
      void sub_balance( name owner, asset value );
      void add_balance( name owner, asset value, name ram_payer );
      void verify_merkle_path( const std::vector<checksum256>& merkle_path, digest_type check );

#ifdef HUB
      /**
       *  ibc-hub related macros, structs and functions
       */
      #define max_hub_unfinished_trxs 1000

      struct [[eosio::table("hubgs")]] hub_globals {
         hub_globals(){}
         bool                 is_open = false;
         name                 hub_account;
         uint64_t             unfinished_trxs = 0;
         EOSLIB_SERIALIZE( hub_globals, (is_open)(hub_account)(unfinished_trxs))
      };
      eosio::singleton< "hubgs"_n, hub_globals >   _hub_globals;
      hub_globals                                  _hubgs;
      
      // code,scope(_self,_self.value)
      struct [[eosio::table]] hub_trx_info {
         uint64_t              id; // set by seq_num in cash action
         uint64_t              cash_time_slot;
         name                  from_chain;
         name                  from_account;
         asset                 from_quantity;
         asset                 mini_to_quantity; /// minimum transfer amount
         checksum256      orig_trx_id;
         name                  to_chain;
         name                  to_account;
         string                orig_pure_memo;
         asset                 to_quantity;
         name                  fee_receiver;
         checksum256      hub_trx_id;
         uint64_t              hub_trx_time_slot;
         uint8_t               forward_times;
         uint8_t               backward_times;

         uint64_t primary_key()const { return id; }
         fixed_bytes<32> by_orig_trx_id()const { return fixed_bytes<32>(orig_trx_id.hash); }
         fixed_bytes<32> by_hub_trx_id()const { return fixed_bytes<32>(hub_trx_id.hash); }
      };
      typedef eosio::multi_index< "hubtrxs"_n, hub_trx_info,
      indexed_by<"origtrxid"_n,   const_mem_fun<hub_trx_info, fixed_bytes<32>, &hub_trx_info::by_orig_trx_id> >,
      indexed_by<"hubtrxid"_n,    const_mem_fun<hub_trx_info, fixed_bytes<32>, &hub_trx_info::by_hub_trx_id> >
      > hubtrxs_table;

      void ibc_cash_to_hub( const uint64_t&                 cash_seq_num,
                            const name&                     from_chain,
                            const name&                     from_account,
                            const transaction_id_type&      orig_trx_id,
                            const asset&                    quantity,
                            const string&                   memo,
                            bool                            from_free_account);
      void ibc_transfer_from_hub( const name& to, const asset& quantity, const string& memo );
      void delete_by_hub_trx_id( const transaction_id_type& hub_trx_id );     // when successfully completed
      void rollback_hub_trx( const transaction_id_type& hub_trx_id, asset quantity );   // when ibc transmit fails
#endif

      void check_admin_auth();
   };

} /// namespace eosio
