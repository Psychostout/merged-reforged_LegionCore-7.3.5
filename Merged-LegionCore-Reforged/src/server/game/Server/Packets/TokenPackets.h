/*
 * Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TokenPackets_h__
#define TokenPackets_h__

#include "Packet.h"
#include "ObjectGuid.h"

namespace WorldPackets
{
    namespace Token
    {
        class UpdateListedAuctionableTokens final : public ClientPacket
        {
        public:
            UpdateListedAuctionableTokens(WorldPacket&& packet) : ClientPacket(CMSG_UPDATE_WOW_TOKEN_AUCTIONABLE_LIST, std::move(packet)) { }

            void Read() override;

            uint32 Type = 0;
        };

        class UpdateListedAuctionableTokensResponse final : public ServerPacket
        {
        public:
            UpdateListedAuctionableTokensResponse() : ServerPacket(SMSG_WOW_TOKEN_UPDATE_AUCTIONABLE_LIST_RESPONSE, 12) { }

            WorldPacket const* Write() override;

            struct AuctionableTokenAuctionable
            {
                uint64 DistributionID = 0;
                uint32 DateCreated = 0;
                uint32 Owner = 0;
                uint64 BuyoutPrice = 0;
                uint32 EndTime = 0;
            };

            uint32 UnkInt = 0; // send CMSG_UPDATE_WOW_TOKEN_AUCTIONABLE_LIST
            uint32 Result = 0;
            std::vector<AuctionableTokenAuctionable> AuctionableTokenAuctionableList;
        };

        class RequestWowTokenMarketPrice final : public ClientPacket
        {
        public:
            RequestWowTokenMarketPrice(WorldPacket&& packet) : ClientPacket(CMSG_REQUEST_WOW_TOKEN_MARKET_PRICE, std::move(packet)) { }

            void Read() override;

            uint32 UnkInt = 0;
        };

        class WowTokenMarketPriceResponse final : public ServerPacket
        {
        public:
            WowTokenMarketPriceResponse() : ServerPacket(SMSG_WOW_TOKEN_MARKET_PRICE_RESPONSE, 8 + 4 * 3) { }

            WorldPacket const* Write() override;

            uint64 CurrentMarketPrice = 0;
            uint32 UnkInt = 0;
            uint32 Result = 0;
            uint32 UnkInt2 = 0;
        };
        
        class CheckVeteranTokenEligibility final : public ClientPacket
        {
        public:
            CheckVeteranTokenEligibility(WorldPacket&& packet) : ClientPacket(CMSG_CHECK_WOW_TOKEN_VETERAN_ELIGIBILITY, std::move(packet)) { }

            void Read() override;

            uint32 UnkInt = 0;
        };

        class WowTokenCanVeteranBuyResult final : public ServerPacket
        {
        public:
            WowTokenCanVeteranBuyResult() : ServerPacket(SMSG_WOW_TOKEN_CAN_VETERAN_BUY_RESULT, 8 + 4 * 2) { }

            WorldPacket const* Write() override;

            uint64 UnkLong = 0;
            uint32 UnkInt = 0;
            uint32 UnkInt2 = 0;
        };

        // --- Buy Token (with gold) ---

        class BuyWowTokenStart final : public ClientPacket
        {
        public:
            BuyWowTokenStart(WorldPacket&& packet) : ClientPacket(CMSG_BUY_WOW_TOKEN_START, std::move(packet)) { }

            void Read() override;

            uint32 UnkInt = 0;
            ObjectGuid Buyer;
            uint64 CurrentMarketPrice = 0;
        };

        class BuyWowTokenConfirm final : public ClientPacket
        {
        public:
            BuyWowTokenConfirm(WorldPacket&& packet) : ClientPacket(CMSG_BUY_WOW_TOKEN_CONFIRM, std::move(packet)) { }

            void Read() override;

            uint32 UnkInt = 0;
            uint32 PendingBuyConfirmations = 0;
            uint64 GuaranteedPrice = 0;
            bool Confirmed = false;
        };

        class WowTokenBuyRequestConfirmation final : public ServerPacket
        {
        public:
            WowTokenBuyRequestConfirmation() : ServerPacket(SMSG_WOW_TOKEN_BUY_REQUEST_CONFIRMATION, 8 + 4 + 4) { }

            WorldPacket const* Write() override;

            uint64 CurrentMarketPrice = 0;
            uint32 TokenCount = 0;
            uint32 Result = 0;
        };

        class WowTokenBuyResultConfirmation final : public ServerPacket
        {
        public:
            WowTokenBuyResultConfirmation() : ServerPacket(SMSG_WOW_TOKEN_BUY_RESULT_CONFIRMATION, 4 + 4) { }

            WorldPacket const* Write() override;

            uint32 UnkInt = 0;
            uint32 Result = 0;
        };

        // --- Sell Token (for gold) ---

        class SellWowTokenStart final : public ClientPacket
        {
        public:
            SellWowTokenStart(WorldPacket&& packet) : ClientPacket(CMSG_SELL_WOW_TOKEN_START, std::move(packet)) { }

            void Read() override;

            uint64 UnkInt64 = 0;
            uint64 CurrentMarketPrice = 0;
            uint32 UnkInt = 0;
        };

        class SellWowTokenConfirm final : public ClientPacket
        {
        public:
            SellWowTokenConfirm(WorldPacket&& packet) : ClientPacket(CMSG_SELL_WOW_TOKEN_CONFIRM, std::move(packet)) { }

            void Read() override;

            ObjectGuid TokenGuid;
            uint32 UnkInt = 0;
            uint32 PendingBuyConfirmations = 0;
            uint64 GuaranteedPrice = 0;
            bool Confirmed = false;
        };

        class WowTokenSellRequestConfirmation final : public ServerPacket
        {
        public:
            WowTokenSellRequestConfirmation() : ServerPacket(SMSG_WOW_TOKEN_SELL_REQUEST_CONFIRMATION, 8 + 4 + 4) { }

            WorldPacket const* Write() override;

            uint64 GuaranteedPrice = 0;
            uint32 UnkInt = 0;
            uint32 Result = 0;
        };

        class WowTokenSellResultConfirmation final : public ServerPacket
        {
        public:
            WowTokenSellResultConfirmation() : ServerPacket(SMSG_WOW_TOKEN_SELL_RESULT_CONFIRMATION, 4 + 4) { }

            WorldPacket const* Write() override;

            uint32 UnkInt = 0;
            uint32 Result = 0;
        };

        // --- Redeem Token (for balance) ---

        class RedeemWowTokenStart final : public ClientPacket
        {
        public:
            RedeemWowTokenStart(WorldPacket&& packet) : ClientPacket(CMSG_REDEEM_WOW_TOKEN_START, std::move(packet)) { }

            void Read() override;

            uint64 Count = 0;
            uint32 UnkInt = 0;
            uint32 UnkInt2 = 0;
        };

        class RedeemWowTokenConfirm final : public ClientPacket
        {
        public:
            RedeemWowTokenConfirm(WorldPacket&& packet) : ClientPacket(CMSG_REDEEM_WOW_TOKEN_CONFIRM, std::move(packet)) { }

            void Read() override;

            uint32 UnkInt = 0;
            uint64 Count = 0;
            ObjectGuid TokenGuid;
            uint32 UnkInt2 = 0;
            bool Confirm = false;
        };

        class WowTokenRedeemRequestConfirmation final : public ServerPacket
        {
        public:
            WowTokenRedeemRequestConfirmation() : ServerPacket(SMSG_WOW_TOKEN_REDEEM_REQUEST_CONFIRMATION, 8 + 4 + 4) { }

            WorldPacket const* Write() override;

            uint64 Count = 0;
            uint32 UnkInt = 0;
            uint32 Result = 0;
        };

        class WowTokenRedeemResult final : public ServerPacket
        {
        public:
            WowTokenRedeemResult() : ServerPacket(SMSG_WOW_TOKEN_REDEEM_RESULT, 4 + 4) { }

            WorldPacket const* Write() override;

            uint32 UnkInt = 0;
            uint32 Result = 0;
        };

        // --- Update Token Count ---

        class UpdateWowTokenCount final : public ClientPacket
        {
        public:
            UpdateWowTokenCount(WorldPacket&& packet) : ClientPacket(CMSG_UPDATE_WOW_TOKEN_COUNT, std::move(packet)) { }

            void Read() override;

            uint32 UnkInt = 0;
        };

        class WowTokenDistributionGlueUpdate final : public ServerPacket
        {
        public:
            WowTokenDistributionGlueUpdate() : ServerPacket(SMSG_WOW_TOKEN_DISTRIBUTION_GLUE_UPDATE, 4 + 4 + 4 + 4) { }

            WorldPacket const* Write() override;

            uint32 UnkInt = 0;
            uint32 UnkInt2 = 0;
            int32 DistributionCount1 = 0;
            int32 DistributionCount2 = 0;
            std::vector<int64> DistributionIDs1;
            std::vector<int64> DistributionIDs2;
        };

        // --- Can Redeem For Balance ---

        class CanRedeemWowTokenForBalance final : public ClientPacket
        {
        public:
            CanRedeemWowTokenForBalance(WorldPacket&& packet) : ClientPacket(CMSG_CAN_REDEEM_WOW_TOKEN_FOR_BALANCE, std::move(packet)) { }

            void Read() override;

            uint32 UnkInt = 0;
        };

        class WowTokenCanRedeemForBalanceResult final : public ServerPacket
        {
        public:
            WowTokenCanRedeemForBalanceResult() : ServerPacket(SMSG_WOW_TOKEN_CAN_REDEEM_FOR_BALANCE_RESULT, 4 + 4) { }

            WorldPacket const* Write() override;

            uint32 UnkInt = 0;
            uint32 Result = 0;
        };
    }
}

#endif // TokenPackets_h__
