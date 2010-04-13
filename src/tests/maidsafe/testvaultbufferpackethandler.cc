/*
* ============================================================================
*
* Copyright [2009] maidsafe.net limited
*
* Version:      1.0
* Created:      09/09/2008 12:14:35 PM
* Revision:     none
* Compiler:     gcc
* Author:       Team www.maidsafe.net
* Company:      maidsafe.net limited
*
* The following source code is property of maidsafe.net limited and is not
* meant for external use.  The use of this code is governed by the license
* file LICENSE.TXT found in the root of this directory and also on
* www.maidsafe.net.
*
* You are not free to copy, amend or otherwise use this source code without
* the explicit written permission of the board of directors of maidsafe.net.
*
* ============================================================================
*/
#include <gtest/gtest.h>
#include "maidsafe/client/packetfactory.h"
#include "maidsafe/vaultbufferpackethandler.h"
#include "tests/maidsafe/cached_keys.h"

class VaultBufferPacketHandlerTest : public testing::Test {
 public:
  VaultBufferPacketHandlerTest() : vbph_(),
                                   public_key_(),
                                   private_key_(),
                                   testuser_(),
                                   ser_bp_(),
                                   cry_obj_(),
                                   keys_() {}
 protected:
  void SetUp() {
    cry_obj_.set_symm_algorithm(crypto::AES_256);
    cry_obj_.set_hash_algorithm(crypto::SHA_512);
    testuser_ = "testuser";
    cached_keys::MakeKeys(2, &keys_);
    public_key_ = keys_.at(0).public_key();
    private_key_ = keys_.at(0).private_key();
    maidsafe::BufferPacketInfo bpi;
    bpi.set_owner("test bufferpacket");
    bpi.set_owner_publickey(public_key_);
    bpi.add_users(cry_obj_.Hash(testuser_, "", crypto::STRING_STRING, false));
    maidsafe::BufferPacket bp;
    maidsafe::GenericPacket *info = bp.add_owner_info();
    std::string ser_bpi;
    bpi.SerializeToString(&ser_bpi);
    info->set_data(ser_bpi);
    info->set_signature(cry_obj_.AsymSign(ser_bpi, "", private_key_,
                        crypto::STRING_STRING));
    bp.SerializeToString(&ser_bp_);
  }
  void TearDown() {
    ser_bp_ = "";
  }

  maidsafe::VaultBufferPacketHandler vbph_;
  std::string public_key_;
  std::string private_key_;
  std::string testuser_;
  std::string ser_bp_;
  crypto::Crypto cry_obj_;
  std::vector<crypto::RsaKeyPair> keys_;
};

TEST_F(VaultBufferPacketHandlerTest, BEH_MAID_ValidateOwnerSig) {
  ASSERT_TRUE(vbph_.ValidateOwnerSignature(public_key_, ser_bp_));
  ASSERT_FALSE(vbph_.ValidateOwnerSignature(keys_.at(1).public_key(), ser_bp_));
}

TEST_F(VaultBufferPacketHandlerTest, BEH_MAID_ChangeOwnerInfo) {
  maidsafe::BufferPacketInfo bpi;
  bpi.set_owner("test bufferpacket");
  bpi.set_owner_publickey(public_key_);
  bpi.add_users(testuser_);
  bpi.add_users("newuser");
  maidsafe::BufferPacket bp;
  maidsafe::GenericPacket *info = bp.add_owner_info();
  std::string ser_bpi;
  bpi.SerializeToString(&ser_bpi);
  info->set_data(ser_bpi);
  info->set_signature(cry_obj_.AsymSign(ser_bpi, "", private_key_,
                      crypto::STRING_STRING));
  std::string ser_gp;
  info->SerializeToString(&ser_gp);
  ASSERT_TRUE(vbph_.ChangeOwnerInfo(ser_gp, public_key_, &ser_bp_));
  maidsafe::BufferPacket bp_up;
  ASSERT_TRUE(bp_up.ParseFromString(ser_bp_));
  maidsafe::GenericPacket new_gp;
  new_gp = bp.owner_info(0);
  maidsafe::BufferPacketInfo bpi_up;
  bpi_up.ParseFromString(new_gp.data());
  ASSERT_EQ("test bufferpacket", bpi_up.owner());
  ASSERT_EQ(2, bpi_up.users_size());

  bpi.SerializeToString(&ser_bpi);
  info->set_data(ser_bpi);
  info->set_signature(cry_obj_.AsymSign(ser_bpi, "", private_key_,
                      crypto::STRING_STRING));
  info->SerializeToString(&ser_gp);
  ASSERT_TRUE(vbph_.ChangeOwnerInfo(ser_gp, public_key_, &ser_bp_));
  ASSERT_TRUE(bp_up.ParseFromString(ser_bp_));
  new_gp = bp.owner_info(0);
  bpi_up.ParseFromString(new_gp.data());
  ASSERT_EQ("test bufferpacket", bpi_up.owner());
  ASSERT_EQ(2, bpi_up.users_size());
}

TEST_F(VaultBufferPacketHandlerTest, BEH_MAID_AddGetClearMessages) {
  std::string ser_msg("Invalid msg format");
  std::string sender;
  maidsafe::MessageType type;
  ASSERT_FALSE(vbph_.CheckMsgStructure(ser_msg, &sender, &type));
  maidsafe::GenericPacket gp_msg;
  maidsafe::BufferPacketMessage bp_msg;
  bp_msg.set_sender_id("non authuser");
  std::string signed_public_key = cry_obj_.AsymSign(keys_.at(1).public_key(),
                                  "", keys_.at(1).private_key(),
                                  crypto::STRING_STRING);
  std::string enc_key = cry_obj_.AsymEncrypt("key", "", public_key_,
    crypto::STRING_STRING);
  bp_msg.set_rsaenc_key(enc_key);
  std::string enc_msg = cry_obj_.SymmEncrypt("msj tonto", "",
                        crypto::STRING_STRING, "key");
  bp_msg.set_aesenc_message(enc_msg);
  bp_msg.set_type(maidsafe::INSTANT_MSG);
  std::string ser_bp_msg;
  bp_msg.SerializeToString(&ser_bp_msg);
  gp_msg.set_data(ser_bp_msg);
  gp_msg.set_signature(cry_obj_.AsymSign(ser_bp_msg, "",
      keys_.at(1).private_key(), crypto::STRING_STRING));
  gp_msg.SerializeToString(&ser_msg);
  ASSERT_TRUE(vbph_.CheckMsgStructure(ser_msg, &sender, &type));
  ASSERT_EQ("non authuser", sender);
  ASSERT_EQ(maidsafe::INSTANT_MSG, type);

  std::string sender_id = cry_obj_.Hash(keys_.at(1).public_key() +
      signed_public_key, "", crypto::STRING_STRING, false);
  std::string hashed_sender_id = cry_obj_.Hash(sender_id, "",
                                 crypto::STRING_STRING, false);
  bp_msg.set_sender_id(sender_id);
  bp_msg.SerializeToString(&ser_bp_msg);
  bp_msg.set_sender_public_key(keys_.at(1).public_key());
  gp_msg.set_data(ser_bp_msg);
  gp_msg.set_signature(cry_obj_.AsymSign(ser_bp_msg, "",
      keys_.at(1).private_key(), crypto::STRING_STRING));
  gp_msg.SerializeToString(&ser_msg);
  ASSERT_TRUE(vbph_.CheckMsgStructure(ser_msg, &sender, &type));
  ASSERT_EQ(sender_id, sender);
  ASSERT_EQ(maidsafe::INSTANT_MSG, type);
  std::string ser_bp_updated;
  ASSERT_FALSE(vbph_.AddMessage(ser_bp_, ser_msg, signed_public_key,
               &ser_bp_updated));

  // Adding the user
  maidsafe::BufferPacket bp;
  bp.ParseFromString(ser_bp_);
  maidsafe::GenericPacket bp_info = bp.owner_info(0);
  maidsafe::BufferPacketInfo bpi;
  bpi.ParseFromString(bp_info.data());
  bpi.add_users(hashed_sender_id);
  bp.clear_owner_info();
  maidsafe::GenericPacket *bp_info_up = bp.add_owner_info();
  std::string new_ser_bpi;
  bpi.SerializeToString(&new_ser_bpi);
  bp_info_up->set_data(new_ser_bpi);
  bp_info_up->set_signature(cry_obj_.AsymSign(new_ser_bpi, "", private_key_,
                            crypto::STRING_STRING));
  bp.SerializeToString(&ser_bp_);

  bp_msg.set_sender_public_key(keys_.at(1).public_key());
  bp_msg.SerializeToString(&ser_bp_msg);
  gp_msg.set_data(ser_bp_msg);
  gp_msg.set_signature(cry_obj_.AsymSign(ser_bp_msg, "",
      keys_.at(1).private_key(), crypto::STRING_STRING));
  gp_msg.SerializeToString(&ser_msg);

  ASSERT_TRUE(vbph_.AddMessage(ser_bp_, ser_msg, signed_public_key,
              &ser_bp_updated));

  maidsafe::BufferPacket bp_updated;
  ASSERT_TRUE(bp_updated.ParseFromString(ser_bp_updated));
  ASSERT_EQ(boost::int32_t(1), bp_updated.messages_size());
  std::vector<std::string> msgs, msgs1;
  ASSERT_TRUE(vbph_.IsOwner("test bufferpacket", bp_updated.owner_info(0)));
  ASSERT_TRUE(vbph_.GetMessages(&ser_bp_updated, &msgs));
  ASSERT_EQ(size_t(1), msgs.size());
  ASSERT_TRUE(vbph_.ClearMessages(&ser_bp_updated));
  ASSERT_TRUE(vbph_.GetMessages(&ser_bp_updated, &msgs1));
  ASSERT_EQ(size_t(0), msgs1.size());
}

TEST_F(VaultBufferPacketHandlerTest, BEH_MAID_AddGetRequestMessages) {
  std::string ser_msg;
  std::string sender;
  maidsafe::MessageType type;
  maidsafe::GenericPacket gp_msg;
  maidsafe::BufferPacketMessage bp_msg;
  bp_msg.set_sender_id("non authuser");
  std::string enc_key = cry_obj_.AsymEncrypt("key", "", public_key_,
    crypto::STRING_STRING);
  bp_msg.set_rsaenc_key(enc_key);
  std::string enc_msg = cry_obj_.SymmEncrypt("msj tonto auth req", "",
                        crypto::STRING_STRING, "key");
  bp_msg.set_aesenc_message(enc_msg);
  bp_msg.set_type(maidsafe::ADD_CONTACT_RQST);

  std::string str_bp_updated;
  bp_msg.set_sender_public_key("invalid_pubkey");
  std::string ser_bp_msg;
  bp_msg.SerializeToString(&ser_bp_msg);
  gp_msg.set_data(ser_bp_msg);
  gp_msg.set_signature(cry_obj_.AsymSign(ser_bp_msg, "",
                       keys_.at(1).private_key(), crypto::STRING_STRING));
  gp_msg.SerializeToString(&ser_msg);
  ASSERT_TRUE(vbph_.CheckMsgStructure(ser_msg, &sender, &type));
  ASSERT_EQ("non authuser", sender);
  std::string signed_public_key = cry_obj_.AsymSign(keys_.at(1).public_key(),
                                  "", keys_.at(1).private_key(),
                                  crypto::STRING_STRING);
  ASSERT_EQ(maidsafe::ADD_CONTACT_RQST, type);
  ASSERT_FALSE(vbph_.AddMessage(ser_bp_, ser_msg, signed_public_key,
                               &str_bp_updated));
  bp_msg.set_sender_public_key(keys_.at(1).public_key());
  bp_msg.SerializeToString(&ser_bp_msg);
  gp_msg.Clear();
  gp_msg.set_data(ser_bp_msg);
  gp_msg.set_signature(cry_obj_.AsymSign(ser_bp_msg, "",
      keys_.at(1).private_key(), crypto::STRING_STRING));
  gp_msg.SerializeToString(&ser_msg);

  maidsafe::GenericPacket sig_packet;
  sig_packet.set_data(keys_.at(1).public_key());
  sig_packet.set_signature(signed_public_key);
  std::string ser_sig_packet;
  sig_packet.SerializeToString(&ser_sig_packet);
  maidsafe::BufferPacket bp_updated;
  ASSERT_TRUE(vbph_.AddMessage(ser_bp_, ser_msg, signed_public_key,
              &str_bp_updated));
  ASSERT_TRUE(bp_updated.ParseFromString(str_bp_updated));
  ASSERT_EQ(1, bp_updated.messages_size());
  std::vector<std::string> msgs;
  ASSERT_TRUE(vbph_.IsOwner("test bufferpacket", bp_updated.owner_info(0)));
  ASSERT_TRUE(vbph_.GetMessages(&str_bp_updated, &msgs));
  ASSERT_EQ(size_t(1), msgs.size());
}

TEST_F(VaultBufferPacketHandlerTest, BEH_MAID_AddGetPresence) {
  maidsafe::BufferPacket bp;
  bp.ParseFromString(ser_bp_);
  maidsafe::GenericPacket gp_info = bp.owner_info(0);
  maidsafe::BufferPacketInfo bpi;
  bpi.ParseFromString(gp_info.data());
  std::string user("el usuario");
  bpi.add_users(cry_obj_.Hash(user, "", crypto::STRING_STRING, false));
  gp_info.set_data(bpi.SerializeAsString());
  gp_info.set_signature(cry_obj_.AsymSign(gp_info.data(), "", private_key_,
                        crypto::STRING_STRING));
  ASSERT_TRUE(vbph_.ChangeOwnerInfo(gp_info.SerializeAsString(), public_key_,
              &ser_bp_));

  maidsafe::LivePresence lp;
  lp.set_contact_id(user);
  maidsafe::EndPoint ep;
  for (int n = 0; n < 3; ++n) {
    ep.add_ip(base::itos(n));
    ep.add_port(n);
  }
  lp.set_end_point(cry_obj_.AsymEncrypt(ep.SerializeAsString(), "", public_key_,
                   crypto::STRING_STRING));
  maidsafe::GenericPacket lp_gp;
  lp_gp.set_data(lp.SerializeAsString());
  lp_gp.set_signature(cry_obj_.AsymSign(lp_gp.data(), "",
                      keys_[1].private_key(), crypto::STRING_STRING));
  ASSERT_TRUE(vbph_.AddPresence(lp_gp.SerializeAsString(), &ser_bp_));

  std::vector<std::string> msgs;
  ASSERT_TRUE(vbph_.GetPresence(&ser_bp_, &msgs));
  ASSERT_EQ(size_t(1), msgs.size());
  lp_gp.Clear();
  ASSERT_TRUE(lp_gp.ParseFromString(msgs[0]));
  ASSERT_TRUE(cry_obj_.AsymCheckSig(lp_gp.data(), lp_gp.signature(),
              keys_[1].public_key(), crypto::STRING_STRING));
  lp.Clear();
  ASSERT_TRUE(lp.ParseFromString(lp_gp.data()));
  ASSERT_EQ(user, lp.contact_id());
  std::string dec_ep(cry_obj_.AsymDecrypt(lp.end_point(), "", private_key_,
                     crypto::STRING_STRING));
  ep.Clear();
  ASSERT_TRUE(ep.ParseFromString(dec_ep));
  for (int a = 0; a < 3; ++a) {
    ASSERT_EQ(base::itos(a), ep.ip(a));
    ASSERT_EQ(a, ep.port(a));
  }
  msgs.clear();
  ASSERT_TRUE(vbph_.GetPresence(&ser_bp_, &msgs));
  ASSERT_EQ(size_t(0), msgs.size());

  lp.Clear();
  lp.set_contact_id("el rey mazorca");
  ep.Clear();
  for (int n = 0; n < 3; ++n) {
    ep.add_ip(base::itos(n));
    ep.add_port(n);
  }
  lp.set_end_point(cry_obj_.AsymEncrypt(ep.SerializeAsString(), "", public_key_,
                   crypto::STRING_STRING));
  lp_gp.Clear();
  lp_gp.set_data(lp.SerializeAsString());
  lp_gp.set_signature(cry_obj_.AsymSign(lp_gp.data(), "",
                      keys_[1].private_key(), crypto::STRING_STRING));
  ASSERT_FALSE(vbph_.AddPresence(lp_gp.SerializeAsString(), &ser_bp_));
  msgs.clear();
  ASSERT_TRUE(vbph_.GetPresence(&ser_bp_, &msgs));
  ASSERT_EQ(size_t(0), msgs.size());

  for (int a = 0; a < 10; ++a) {
    lp.Clear();
    lp.set_contact_id(user);
    ep.Clear();
    for (int n = 0; n < 3; ++n) {
      ep.add_ip(base::itos(n));
      ep.add_port(n);
    }
    lp.set_end_point(cry_obj_.AsymEncrypt(ep.SerializeAsString(), "",
                     public_key_, crypto::STRING_STRING));
    lp_gp.Clear();
    lp_gp.set_data(lp.SerializeAsString());
    lp_gp.set_signature(cry_obj_.AsymSign(lp_gp.data(), "",
                        keys_[1].private_key(), crypto::STRING_STRING));
    ASSERT_TRUE(vbph_.AddPresence(lp_gp.SerializeAsString(), &ser_bp_));
  }
  ASSERT_TRUE(vbph_.GetPresence(&ser_bp_, &msgs));
  ASSERT_EQ(size_t(10), msgs.size());
    for (size_t y = 0; y < msgs.size(); ++y) {
    ASSERT_TRUE(lp_gp.ParseFromString(msgs[0]));
    ASSERT_TRUE(cry_obj_.AsymCheckSig(lp_gp.data(), lp_gp.signature(),
                keys_[1].public_key(), crypto::STRING_STRING));
    lp.Clear();
    ASSERT_TRUE(lp.ParseFromString(lp_gp.data()));
    ASSERT_EQ(user, lp.contact_id());
    std::string dec_ep(cry_obj_.AsymDecrypt(lp.end_point(), "", private_key_,
                       crypto::STRING_STRING));
    ep.Clear();
    ASSERT_TRUE(ep.ParseFromString(dec_ep));
    for (int a = 0; a < 3; ++a) {
      ASSERT_EQ(base::itos(a), ep.ip(a));
      ASSERT_EQ(a, ep.port(a));
    }
  }
  ASSERT_TRUE(vbph_.GetPresence(&ser_bp_, &msgs));
  ASSERT_EQ(size_t(0), msgs.size());
}
