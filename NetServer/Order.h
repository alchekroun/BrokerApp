#pragma once
#include <chrono>
#include <cmath>

class order
{
protected:
	std::chrono::system_clock::time_point timestamp_ = std::chrono::system_clock::now();
	uint32_t id_client_;
	float price_;
	int quantity_;

public:

	order(uint32_t const id_client, float const price, int const quantity) :
        id_client_(id_client), price_(price), quantity_(quantity) {}

    order(std::chrono::system_clock::time_point const timestamp , uint32_t const id_client, float const price, int const quantity) :
        timestamp_(timestamp), id_client_(id_client), price_(price), quantity_(quantity) {}

	virtual ~order() = default;

	order& operator=(order const& x) = default;
	bool operator==(order const& x) const {
        if (this->timestamp_ == x.timestamp_ && this->quantity_ == x.quantity_ && this->id_client_ == x.id_client_)
            if (std::fabs(this->price_ - x.price_) < static_cast<float>(0.001)) return true;

        return false;
    }

    bool operator !=(order const& x) const {
        if (this->timestamp_ != x.timestamp_ || this->quantity_ != x.quantity_ || this->id_client_ != x.id_client_)
            if (std::fabs(this->price_ - x.price_) >= static_cast<float>(0.001)) return false;

        return true;
	}

    [[nodiscard]] int get_quantity() const { return quantity_; }
    [[nodiscard]] uint32_t get_id() const { return id_client_; }
    [[nodiscard]] float get_price() const { return price_; }
    [[nodiscard]] std::chrono::system_clock::time_point get_time() const { return timestamp_; }

};

