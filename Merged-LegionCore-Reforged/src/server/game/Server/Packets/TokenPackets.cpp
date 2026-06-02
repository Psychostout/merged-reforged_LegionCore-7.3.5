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

#include "TokenPackets.h"

void WorldPackets::Token::UpdateListedAuctionableTokens::Read()
{
    _worldPacket >> Type;
}

WorldPacket const* WorldPackets::Token::UpdateListedAuctionableTokensResponse::Write()
{
    _worldPacket << UnkInt;
    _worldPacket << Result;
    _worldPacket << static_cast<uint32>(AuctionableTokenAuctionableList.size());
    for (auto const& v : AuctionableTokenAuctionableList)
    {
        _worldPacket << v.DistributionID;
        _worldPacket << v.DateCreated;
        _worldPacket << v.Owner;
        _worldPacket << v.BuyoutPrice;
        _worldPacket << v.EndTime;
    }

    return &_worldPacket;
}

void WorldPackets::Token::RequestWowTokenMarketPrice::Read()
{
    _worldPacket >> UnkInt;
}

WorldPacket const* WorldPackets::Token::WowTokenMarketPriceResponse::Write()
{
    _worldPacket << CurrentMarketPrice;
    _worldPacket << UnkInt;
    _worldPacket << Result;
    _worldPacket << UnkInt2;

    return &_worldPacket;
}

void WorldPackets::Token::CheckVeteranTokenEligibility::Read()
{
    _worldPacket >> UnkInt;
}

WorldPacket const* WorldPackets::Token::WowTokenCanVeteranBuyResult::Write()
{
    _worldPacket << UnkLong;
    _worldPacket << UnkInt;
    _worldPacket << UnkInt2;

    return &_worldPacket;
}

// --- Buy Token ---

void WorldPackets::Token::BuyWowTokenStart::Read()
{
    _worldPacket >> UnkInt;
    _worldPacket >> Buyer;
    _worldPacket >> CurrentMarketPrice;
}

void WorldPackets::Token::BuyWowTokenConfirm::Read()
{
    _worldPacket >> UnkInt;
    _worldPacket >> PendingBuyConfirmations;
    _worldPacket >> GuaranteedPrice;
    Confirmed = _worldPacket.ReadBit();
}

WorldPacket const* WorldPackets::Token::WowTokenBuyRequestConfirmation::Write()
{
    _worldPacket << CurrentMarketPrice;
    _worldPacket << TokenCount;
    _worldPacket << Result;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Token::WowTokenBuyResultConfirmation::Write()
{
    _worldPacket << UnkInt;
    _worldPacket << Result;

    return &_worldPacket;
}

// --- Sell Token ---

void WorldPackets::Token::SellWowTokenStart::Read()
{
    _worldPacket >> UnkInt64;
    _worldPacket >> CurrentMarketPrice;
    _worldPacket >> UnkInt;
}

void WorldPackets::Token::SellWowTokenConfirm::Read()
{
    _worldPacket >> TokenGuid;
    _worldPacket >> UnkInt;
    _worldPacket >> PendingBuyConfirmations;
    _worldPacket >> GuaranteedPrice;
    Confirmed = _worldPacket.ReadBit();
}

WorldPacket const* WorldPackets::Token::WowTokenSellRequestConfirmation::Write()
{
    _worldPacket << GuaranteedPrice;
    _worldPacket << UnkInt;
    _worldPacket << Result;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Token::WowTokenSellResultConfirmation::Write()
{
    _worldPacket << UnkInt;
    _worldPacket << Result;

    return &_worldPacket;
}

// --- Redeem Token ---

void WorldPackets::Token::RedeemWowTokenStart::Read()
{
    _worldPacket >> Count;
    _worldPacket >> UnkInt;
    _worldPacket >> UnkInt2;
}

void WorldPackets::Token::RedeemWowTokenConfirm::Read()
{
    _worldPacket >> UnkInt;
    _worldPacket >> Count;
    _worldPacket >> TokenGuid;
    _worldPacket >> UnkInt2;
    Confirm = _worldPacket.ReadBit();
}

WorldPacket const* WorldPackets::Token::WowTokenRedeemRequestConfirmation::Write()
{
    _worldPacket << Count;
    _worldPacket << UnkInt;
    _worldPacket << Result;

    return &_worldPacket;
}

WorldPacket const* WorldPackets::Token::WowTokenRedeemResult::Write()
{
    _worldPacket << UnkInt;
    _worldPacket << Result;

    return &_worldPacket;
}

// --- Update Token Count ---

void WorldPackets::Token::UpdateWowTokenCount::Read()
{
    _worldPacket >> UnkInt;
}

WorldPacket const* WorldPackets::Token::WowTokenDistributionGlueUpdate::Write()
{
    _worldPacket << UnkInt;
    _worldPacket << UnkInt2;
    _worldPacket << DistributionCount1;
    _worldPacket << DistributionCount2;
    for (int32 i = 0; i < DistributionCount1; ++i)
        _worldPacket << DistributionIDs1[i];
    for (int32 i = 0; i < DistributionCount2; ++i)
        _worldPacket << DistributionIDs2[i];

    return &_worldPacket;
}

// --- Can Redeem For Balance ---

void WorldPackets::Token::CanRedeemWowTokenForBalance::Read()
{
    _worldPacket >> UnkInt;
}

WorldPacket const* WorldPackets::Token::WowTokenCanRedeemForBalanceResult::Write()
{
    _worldPacket << UnkInt;
    _worldPacket << Result;

    return &_worldPacket;
}
