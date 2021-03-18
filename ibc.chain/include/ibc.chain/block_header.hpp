/**
 *  @file
 *  @copyright defined in eos/LICENSE.txt
 */
#pragma once

#include <eosiolib/core/eosio/time.hpp>
#include <eosiolib/contracts/eosio/eosio.hpp>
#include <eosiolib/contracts/eosio/producer_schedule.hpp>
#include <eosiolib/contracts/eosio/transaction.hpp>
#include <ibc.chain/types.hpp>

namespace eosio {

   // typedef std::tuple<uint16_t, std::vector<char>> extension;
   // typedef std::vector<extension> extensions_type;

   struct block_header {
      block_timestamp                           timestamp;
      name                                      producer;
      uint16_t                                  confirmed;
      block_id_type                             previous;
      checksum256                          transaction_mroot;
      checksum256                          action_mroot;
      uint32_t                                  schedule_version;
      std::optional<eosio::producer_schedule>   new_producers;
      extensions_type                           header_extensions;

      checksum256     digest()const;
      block_id_type        id() const;
      uint32_t             block_num() const { return num_from_id(previous) + 1; }
      static uint32_t      num_from_id(const checksum256& id);
      std::optional<eosio::producer_schedule> get_ext_new_producers( uint16_t ext_id ) const;

      EOSLIB_SERIALIZE(block_header, (timestamp)(producer)(confirmed)(previous)(transaction_mroot)(action_mroot)
         (schedule_version)(new_producers)(header_extensions))
   };

   struct signed_block_header : public block_header {
      signature     producer_signature;

      EOSLIB_SERIALIZE_DERIVED( signed_block_header, block_header, (producer_signature) )
   };
/*
   struct key_weight {
      public_key        key;
      uint16_t          weight;
      EOSLIB_SERIALIZE( key_weight, (key)(weight) )
   };

   struct block_signing_authority_v0 {
      uint32_t                   threshold;
      std::vector<key_weight>    keys;
      EOSLIB_SERIALIZE( block_signing_authority_v0, (threshold)(keys) )
   };

   using block_signing_authority = std::variant<block_signing_authority_v0>;

   struct producer_authority {
      name                    producer_name;
      block_signing_authority authority;
      EOSLIB_SERIALIZE( producer_authority, (producer_name)(authority) )
   };
*/
   struct producer_authority_schedule {
      uint32_t                            version;
      std::vector<producer_authority>     producers;
      EOSLIB_SERIALIZE( producer_authority_schedule, (version)(producers) )
   };

   struct producer_schedule_change_extension : producer_authority_schedule {
      static constexpr uint16_t extension_id() { return 1; }
      EOSLIB_SERIALIZE_DERIVED( producer_schedule_change_extension, producer_authority_schedule, )
   };

} /// namespace eosio


