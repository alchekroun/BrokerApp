#pragma once
#include <iostream>
#include <queue>
#include <numeric>
#include <cmath>
#include <vector>
#include <unordered_map>

#include "Ask.h"
#include "Bid.h"
#include "Book.h"

class book_service
{
    static int const TOP_PRICE_MINIMUM = 200;

public:
	inline static int total_quantity_available_ask;
	inline static int total_quantity_wanted_bid;
	inline static int total_quantity_already_given = 0;
	
	/**
	 * @param bid_to_fulfill vector of bid on the same level as ask
	 * @result return pair of one shared ptr to the top bid and an int referring to the quantity allowed to this bid
	 */
	static std::pair<std::shared_ptr<bid>, int> top_match(std::vector<std::shared_ptr<bid>> const& bid_to_fulfill) {
		std::pair<std::shared_ptr<bid>, int> output;

		std::shared_ptr<bid> top_order1 = nullptr;
        for (auto const& element : bid_to_fulfill) {
            if(element->get_quantity() >= TOP_PRICE_MINIMUM) {
				top_order1 = element;
				break;
            }
        }
		if(top_order1){
			output.first = top_order1;

			if(total_quantity_available_ask >= output.first->get_quantity()) {
				output.second = output.first->get_quantity();
			} else if (total_quantity_available_ask < output.first->get_quantity()) {
				output.second = total_quantity_available_ask;
			}
			total_quantity_already_given += output.second;
		}
		return output;
	}

	/**
	 * @param bid_to_fulfill vector of bid on the same level as ask
	 * @result map of bid and quantity allowed to buy
	 */
	static std::unordered_map<std::shared_ptr<bid>, int> pro_rata(std::vector<std::shared_ptr<bid>> const& bid_to_fulfill) {
		std::unordered_map<std::shared_ptr<bid>, int> output;

		std::vector<int> nb_to_order;

		auto const actual_quantity_bid = total_quantity_wanted_bid - total_quantity_already_given;
		auto const actual_quantity_ask = total_quantity_available_ask - total_quantity_already_given;

        std::for_each(begin(bid_to_fulfill), end(bid_to_fulfill),
                      [&actual_quantity_ask, &nb_to_order, &actual_quantity_bid](std::shared_ptr<bid> const& i) {
                          auto value = static_cast<int>(std::floor(static_cast<float>(i->get_quantity()) / static_cast<float>(actual_quantity_bid)
                              * static_cast<float>(actual_quantity_ask)));
                          value = value < 2 ? 0 : value;
						  nb_to_order.push_back(static_cast<int>(value));
						  total_quantity_already_given += value;
                      });
		
		auto k = 0;
		std::for_each(begin(bid_to_fulfill), end(bid_to_fulfill),
			[&output, &nb_to_order, &k](std::shared_ptr<bid> const& i) {
				output[i] = nb_to_order[k++];
			});

		return output;
	}

	/**
	 * @param bids bid to fulfill 
	 */
	static void fifo(std::unordered_map<std::shared_ptr<bid>, int>& bids) {

		std::vector<std::shared_ptr<bid>> v_to_be_sorted;
		std::for_each(begin(bids), end(bids),
			[&v_to_be_sorted](std::pair<std::shared_ptr<bid>, int> const& i) {
				v_to_be_sorted.push_back(i.first);
			});
		sort(begin(v_to_be_sorted), end(v_to_be_sorted),
		    [](std::shared_ptr<bid> const& a, std::shared_ptr<bid> const& b) {
				return a->get_time().time_since_epoch().count() < b->get_time().time_since_epoch().count();
		});

        for (auto const& i : v_to_be_sorted) {
			auto const real_available = total_quantity_available_ask - total_quantity_already_given;
			if (real_available > 0) {
				auto const need = i->get_quantity() - bids.at(i);
				if (need > 0) {
					auto const value = need - real_available > 0 ? real_available : real_available - need;
					bids[i] += value;
				    total_quantity_already_given += value;
				}
			} else {
				break;
			}
        }
	}

	/**
	 * @param bid_to_fulfill vector of bid on the same level as ask
	 * @result map of bid and quantity allowed to buy
	 */
	static std::unordered_map<std::shared_ptr<bid>, int> fill_bids(std::vector<std::shared_ptr<bid>> const& bid_to_fulfill) {
		std::unordered_map<std::shared_ptr<bid>, int> output;

		std::for_each(begin(bid_to_fulfill), end(bid_to_fulfill), 
			[&output](std::shared_ptr<bid> const& i) {
				output[i] = i->get_quantity();
				total_quantity_already_given += i->get_quantity();
			});

		return output;
	}

	/**
	 * @param b_bid book of all bid
	 * @param b_ask book of all ask
	 * @result pair of vector each contains bid/ask at the matching price level.
	 */
	static std::pair<std::vector<std::shared_ptr<bid>>, std::vector<std::shared_ptr<ask>>> fill_with_orders(book<bid>& b_bid, book<ask>& b_ask) {
		std::pair<std::vector<std::shared_ptr<bid>>, std::vector<std::shared_ptr<ask>>> output;

		// The lowest ask price is always the price reference.
		auto const price_reference = b_ask.top().get_price();

		while (!b_ask.empty()) {
		    if(std::fabs(b_ask.top().get_price() - price_reference) < static_cast<float>(0.0001) )  {
				output.second.push_back(std::make_shared<ask>(b_ask.top()));
				b_ask.pop();
		    } else {
				break;
		    }
		}

		while (!b_bid.empty()) {
			if (price_reference <= b_bid.top().get_price()) {
				output.first.push_back(std::make_shared<bid>(b_bid.top()));
				b_bid.pop();
			}
			else {
				break;
			}
		}

		return output;
	}

	/**
	 * @param bid_to_fulfill bid to fulfill
	 * @param ask_concerned  ask concerned
	 * @result unordered_map containing a shared_ptr of bid and the quantity given to it
	 */
	static std::unordered_map<std::shared_ptr<bid>, int> matching_orders(std::vector<std::shared_ptr<bid>>& bid_to_fulfill, std::vector<std::shared_ptr<ask>>& ask_concerned) {

		std::unordered_map<std::shared_ptr<bid>, int> output;

			if (!bid_to_fulfill.empty()) {

				total_quantity_available_ask = std::accumulate(begin(ask_concerned), end(ask_concerned), 0,
					[](int const x, std::shared_ptr<ask> const& i) {
						return x + i->get_quantity();
					});

				total_quantity_wanted_bid = std::accumulate(begin(bid_to_fulfill), end(bid_to_fulfill), 0,
					[](int const x, std::shared_ptr<bid> const& i) {
						return x + i->get_quantity();
					});

				total_quantity_already_given = 0;

				// If everybody can't have what they ask, we launch the top/pro-rata/fifo allocation
				if (total_quantity_available_ask < total_quantity_wanted_bid) {

					const auto top_order = top_match(bid_to_fulfill);

					if (top_order.first && top_order.second > 0) {

						// Top order filled thus we can add him on the output
						output.insert(top_order);
						// ... and remove him from bid to fulfill
						bid_to_fulfill.erase(find(begin(bid_to_fulfill), end(bid_to_fulfill), top_order.first));
					}

					// If the top order didn't take everything => pro-rata
					if (top_order.second < total_quantity_available_ask) {

						auto pro_rated = pro_rata(bid_to_fulfill);
						
						// If there is any leftover you pass them with fifo
						if(total_quantity_available_ask - total_quantity_already_given > 0) {
							fifo(pro_rated);
						}

						output.merge(pro_rated);
					} else {
						std::unordered_map<std::shared_ptr<bid>, int> to_merge;
						std::for_each(begin(bid_to_fulfill), end(bid_to_fulfill),
							[&to_merge](std::shared_ptr<bid> const& i) {
								to_merge[i] = 0;
							});
						output.merge(to_merge);
					}
				} else {
					auto bid_fulfilled = fill_bids(bid_to_fulfill);
					output.merge(bid_fulfilled);
				}
			}
		return output;
	}

	static std::unordered_map<std::shared_ptr<ask>, int> remove_quantity_from_ask(std::unordered_map<std::shared_ptr<bid>, int> const& orders, std::vector<std::shared_ptr<ask>> const& ask_concerned) {
		std::unordered_map<std::shared_ptr<ask>, int> output(ask_concerned.size());
		auto k = 0, qte_placed = 0, qte_placed_this_ask = 0;
        for (const auto& [ptr, qte] : orders) {
			while (qte_placed < qte) {
				auto const real_qte = qte - qte_placed;
				auto const real_ask_qte = ask_concerned[k]->get_quantity() - qte_placed_this_ask;
				auto const diff = real_ask_qte - real_qte;
				if (diff > 0) {
					output[ask_concerned[k]] += real_qte;
					qte_placed += real_qte;
					qte_placed_this_ask += real_qte;
				}
				else if (diff == 0) {
					output[ask_concerned[k]] += real_qte;
					qte_placed += real_qte;
					qte_placed_this_ask = 0;
					k++;
				}
				else {
					output[ask_concerned[k]] += real_ask_qte;
					qte_placed += real_ask_qte;
					qte_placed_this_ask = 0;
					k++;
				}
			}
			qte_placed = 0;
        }

		return output;
	}

	static void affect_residual_bids(book<bid>& b_bid, std::unordered_map<std::shared_ptr<bid>, int> const& bids) {

        for (const auto& [ptr, qte] : bids) {
            if(ptr->get_quantity() > qte) {
				bid const new_bid{ptr->get_time(), ptr->get_id(), ptr->get_price(), ptr->get_quantity() - qte};
				b_bid.push(new_bid);
            }
        }

	}

	static void affect_residual_asks(book<ask>& b_ask, std::unordered_map<std::shared_ptr<ask>, int> const& asks) {

		for (const auto& [ptr, qte] : asks) {
			if (ptr->get_quantity() > qte) {
				ask const new_bid{ ptr->get_time(), ptr->get_id(), ptr->get_price(), ptr->get_quantity() - qte };
				b_ask.push(new_bid);
			}
		}

	}

	static void affect_residual_orders(book<bid>& b_bid, book<ask>& b_ask, std::unordered_map<std::shared_ptr<bid>, int> const& bids, std::unordered_map<std::shared_ptr<ask>, int> const& asks) {

		affect_residual_bids(b_bid, bids);

		affect_residual_asks(b_ask, asks);

	}


	static std::pair<std::unordered_map<std::shared_ptr<bid>, int>, std::unordered_map<std::shared_ptr<ask>, int>> update_books(book<bid>& b_bid, book<ask>& b_ask) {

		std::pair<std::unordered_map<std::shared_ptr<bid>, int>, std::unordered_map<std::shared_ptr<ask>, int>> output;

		if (!b_bid.empty() && !b_ask.empty()) {

			auto [bid_to_fulfill, ask_concerned] = fill_with_orders(b_bid, b_ask);

			auto const matched_bid = matching_orders(bid_to_fulfill, ask_concerned);

			output.first = matched_bid;

			auto const matched_ask = remove_quantity_from_ask(matched_bid, ask_concerned);

			output.second = matched_ask;

			affect_residual_orders(b_bid, b_ask, matched_bid, matched_ask);

		}

		return output;
	    
	}
};

