/*
	This file is part of cpp-ethereum.

	cpp-ethereum is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	cpp-ethereum is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with cpp-ethereum.  If not, see <http:
*/
/** @file EthashClient.h
 * @author Gav Wood <i@gavwood.com>
 * @date 2014
 */

#pragma once

#include <tuple>
#include <libethereum/Client.h>

namespace dev
{
namespace eth
{

class Ethash;

DEV_SIMPLE_EXCEPTION(InvalidSealEngine);

class EthashClient: public Client
{
public:
	
	EthashClient(
		ChainParams const& _params,
		int _networkID,
		p2p::Host* _host,
		std::shared_ptr<GasPricer> _gpForAdoption,
		std::string const& _dbPath = std::string(),
		WithExisting _forceAction = WithExisting::Trust,
		TransactionQueue::Limits const& _l = TransactionQueue::Limits{1024, 1024}
	);

	Ethash* ethash() const;

	
	void setShouldPrecomputeDAG(bool _precompute);

	
	bool isMining() const;

	
	u256 hashrate() const;

	
	WorkingProgress miningProgress() const;

	
	bool shouldServeWork() const { return m_bq.items().first == 0 && (isMining() || remoteActive()); }

	
	
	
	std::tuple<h256, h256, h256> getEthashWork();

	/** @brief Submit the proof for the proof-of-work.
	 * @param _s A valid solution.
	 * @return true if the solution was indeed valid and accepted.
	 */
	bool submitEthashWork(h256 const& _mixHash, h64 const& _nonce);

	void submitExternalHashrate(u256 const& _rate, h256 const& _id);

protected:
	u256 externalHashrate() const;

	
	mutable std::unordered_map<h256, std::pair<u256, std::chrono::steady_clock::time_point>> m_externalRates;
	mutable SharedMutex x_externalRates;
};

EthashClient& asEthashClient(Interface& _c);
EthashClient* asEthashClient(Interface* _c);

}
}
