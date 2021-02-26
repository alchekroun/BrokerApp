#pragma once

#include <iostream>
#include <queue>
#include <numeric>

template <typename T>
class book
{
	std::priority_queue<T> orders_;

public:
	T top() const { return orders_.top(); }
	void pop() { orders_.pop(); }
	void push(T const& order) { orders_.push(std::move(order)); }
	bool empty() { return orders_.empty(); }

	void printout() {
		while (!orders_.empty()) {
			std::cout << top().get_price() << "\n";
			pop();
		}
	}
};

