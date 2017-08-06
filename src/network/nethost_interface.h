/*
 * Copyright (C) 2008-2017 by the Widelands Development Team
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef WL_NETWORK_NETHOST_INTERFACE_H
#define WL_NETWORK_NETHOST_INTERFACE_H

#include "network/network.h"

/**
 * A NetHost manages the client connections of a network game in
 * which this computer participates as a server.
 * This class provides the interface all NetHost implementation have to follow.
 * Currently two implementations exists: A "real" NetHost for local games and a
 * NetHostProxy which relays commands over a relay server.
 */
class NetHostInterface {
public:
	/// IDs used to enumerate the clients.
	using ConnectionId = uint32_t;

	/**
	 * Closes the server.
	 */
	virtual ~NetHostInterface() {
	}

	/**
	 * Returns whether the given client is connected.
	 * \param The id of the client to check.
	 * \return \c true if the connection is open, \c false otherwise.
	 */
	virtual bool is_connected(ConnectionId id) const = 0;

	/**
	 * Closes the connection to the given client.
	 * \param id The id of the client to close the connection to.
	 */
	virtual void close(ConnectionId id) = 0;

	/**
	 * Tries to accept a new client.
	 * \param new_id The connection id of the new client will be stored here.
	 * \return \c true if a client has connected, \c false otherwise.
	 *   The given id is only modified when \c true is returned.
	 *   Calling this on a closed server will return false.
	 *   The returned id is always greater than 0.
	 */
	virtual bool try_accept(ConnectionId* new_id) = 0;

	/**
	 * Tries to receive a packet.
	 * \param id The connection id of the client that should be received.
	 * \param packet A packet that should be overwritten with the received data.
	 * \return \c true if a packet is available, \c false otherwise.
	 *   The given packet is only modified when \c true is returned.
	 *   Calling this on a closed connection will return false.
	 */
	virtual bool try_receive(ConnectionId id, RecvPacket* packet) = 0;

	/**
	 * Sends a packet.
	 * Calling this on a closed connection will silently fail.
	 * \param id The connection id of the client that should be sent to.
	 * \param packet The packet to send.
	 */
	virtual void send(ConnectionId id, const SendPacket& packet) = 0;
};

#endif  // end of include guard: WL_NETWORK_NETHOST_INTERFACE_H
