#pragma once

#include "Order.h"

class ask final :
	public order {

public:
	ask(uint32_t const id_client, float const price, int const quantity) : order(id_client, price, quantity) {}
	ask(std::chrono::system_clock::time_point const timestamp, uint32_t const id_client, float const price, int const quantity) : order(timestamp, id_client, price, quantity) {}

	bool operator<(ask const& x) const {
		return this->price_ > x.price_;
	}

	bool operator()(ask const& x) const {
		return this->price_ > x.price_;
	}
};