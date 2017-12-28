///////////////////////////////////////////////////////////////////////////////
// Copyright (c) Lewis Baker
// Licenced under MIT license. See LICENSE.txt for details.
///////////////////////////////////////////////////////////////////////////////

#include "socket_helpers.hpp"

#include <cppcoro/net/ip_endpoint.hpp>

#if CPPCORO_OS_WINNT
#include <cstring>
#include <cassert>

#include <WinSock2.h>
#include <WS2tcpip.h>
#include <MSWSock.h>
#include <Windows.h>


cppcoro::net::ip_endpoint
cppcoro::net::detail::sockaddr_to_ip_endpoint(const sockaddr& address) noexcept
{
	if (address.sa_family == AF_INET)
	{
		const auto& ipv4Address = *reinterpret_cast<const sockaddr_in*>(&address);

		std::uint8_t addressBytes[4];
		std::memcpy(addressBytes, &ipv4Address.sin_addr, 4);

		return ipv4_endpoint{
			ipv4_address{ addressBytes },
			ipv4Address.sin_port
		};
	}
	else
	{
		assert(address.sa_family == AF_INET6);

		const auto& ipv6Address = *reinterpret_cast<const sockaddr_in6*>(&address);

		return ipv6_endpoint{
			ipv6_address{ ipv6Address.sin6_addr.u.Byte },
			ipv6Address.sin6_port
		};
	}
}

int cppcoro::net::detail::ip_endpoint_to_sockaddr(
	const ip_endpoint& endPoint,
	std::reference_wrapper<sockaddr_storage> address) noexcept
{
	if (endPoint.is_ipv4())
	{
		const auto& ipv4EndPoint = endPoint.to_ipv4();
		auto& ipv4Address = *reinterpret_cast<SOCKADDR_IN*>(&address.get());
		ipv4Address.sin_family = AF_INET;
		std::memcpy(&ipv4Address.sin_addr, ipv4EndPoint.address().bytes(), 4);
		ipv4Address.sin_port = ipv4EndPoint.port();
		std::memset(&ipv4Address.sin_zero, 0, sizeof(ipv4Address.sin_zero));
		return sizeof(SOCKADDR_IN);
	}
	else
	{
		const auto& ipv6EndPoint = endPoint.to_ipv6();
		auto& ipv6Address = *reinterpret_cast<SOCKADDR_IN6*>(&address.get());
		ipv6Address.sin6_family = AF_INET6;
		std::memcpy(&ipv6Address.sin6_addr, ipv6EndPoint.address().bytes(), 16);
		ipv6Address.sin6_port = ipv6EndPoint.port();
		ipv6Address.sin6_flowinfo = 0;
		ipv6Address.sin6_scope_struct = SCOPEID_UNSPECIFIED_INIT;
		return sizeof(SOCKADDR_IN6);
	}
}

#endif // CPPCORO_OS_WINNT