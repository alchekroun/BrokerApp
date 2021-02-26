#pragma once

#include "Order.h"

class bid final :
    public order {

public:
	bid(uint32_t const id_client, float const price, int const quantity) : order(id_client, price, quantity) {}
	bid(std::chrono::system_clock::time_point const timestamp, uint32_t const id_client, float const price, int const quantity) : order(timestamp, id_client, price, quantity) {}

	bool operator<(bid const& x) const {
		return this->price_ < x.price_;
	}

	bool operator()(bid const& x) const {
		return this->price_ < x.price_;
	}
};

