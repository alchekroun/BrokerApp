#define CATCH_CONFIG_MAIN

#include "catch.hpp"
#include "book_service.h"

TEST_CASE("top GIVEN 2 bid RETURN rigth one", "[unit]") {
    //INIT
    book<bid> book_bid;
    bid const o1{1,10,10};
    bid const o2{ 2, 9, 10 };

    book_bid.push(o1);
    book_bid.push(o2);

    REQUIRE(book_bid.top() == o1);
}

TEST_CASE("top GIVEN 2 ask RETURN rigth one", "[unit]") {
    // INIT
    book<ask> book_ask;
    ask const o1{ 1,10,10 };
    ask const o2{ 2, 9, 10 };

    book_ask.push(o1);
    book_ask.push(o2);

    REQUIRE(book_ask.top() == o2);
}

TEST_CASE("fill_with_orders GIVEN 1 ask 1 bid same price&quantity RETURN vector 1 bid", "[unit]") {
    // INIT
    book<ask> book_ask;
    ask const a{ 1,10,10 };
    book_ask.push(a);
    book<bid> book_bid;
    bid const b{ 2, 10, 10 };
    book_bid.push(b);

    std::vector<bid> const expect{ b };

    //
    const auto [fst, snd] = book_service::fill_with_orders(book_bid, book_ask);

    REQUIRE_FALSE(fst.empty());
    REQUIRE(fst[0]->get_id() == b.get_id());
    REQUIRE_FALSE(fst.empty());
    REQUIRE(snd[0]->get_id() == a.get_id());
}

TEST_CASE("fill_with_orders GIVEN 1 ask price +1 & 1 bid RETURN void", "[unit]") {
    // INIT
    book<ask> book_ask;
    ask const a{ 1,11,10 };
    book_ask.push(a);
    book<bid> book_bid;
    bid const b{ 2, 9, 10 };
    book_bid.push(b);

    std::vector<bid> const expect{};

    //
    const auto [fst, snd] = book_service::fill_with_orders(book_bid, book_ask);

    REQUIRE(fst.empty());
}

TEST_CASE("fill_with_orders GIVEN 1 ask & 2 bid RETURN vector 1 bid", "[unit]") {
    // INIT
    book<ask> book_ask;
    ask const a{ 1,10,10 };
    book_ask.push(a);
    book<bid> book_bid;
    bid const b1{ 2, 9, 10 };
    bid const b2{ 3, 10, 10 };
    book_bid.push(b1);
    book_bid.push(b2);

    std::vector<bid> const expect{b2};

    //
    const auto [fst, snd] = book_service::fill_with_orders(book_bid, book_ask);
    
    REQUIRE_FALSE(fst.empty());
    REQUIRE(fst[0]->get_id() == b2.get_id());
    REQUIRE_FALSE(fst.empty());
    REQUIRE(snd[0]->get_id() == a.get_id());
}

TEST_CASE("fill_with_orders GIVEN 2 ask & 2 bid RETURN vector 2 bid", "[unit]") {
    // INIT
    book<ask> book_ask;
    ask const a1{ 1,10,10 };
    ask const a2{ 2,9,10 };
    book_ask.push(a1);
    book_ask.push(a2);
    book<bid> book_bid;
    bid const b1{ 3, 9, 10 };
    bid const b2{ 4, 10, 10 };
    book_bid.push(b1);
    book_bid.push(b2);

    std::vector<bid> const expect{ b1, b2 };

    const auto [fst, snd] = book_service::fill_with_orders(book_bid, book_ask);

    REQUIRE_FALSE(fst.empty());
    REQUIRE(fst[0]->get_id() == b2.get_id());
    REQUIRE(fst[1]->get_id() == b1.get_id());
    REQUIRE_FALSE(fst.empty());
    REQUIRE(snd[0]->get_id() == a2.get_id());
}

TEST_CASE("top_match GIVEN 1 ask 1 bid same quantity RETURN pair 1 bid quantity", "[unit]") {
    // INIT
    std::vector<std::shared_ptr<ask>> ask_concerned;
    ask const a1{ 1,10,200 };
    ask_concerned.push_back(std::make_shared<ask>(a1));
    std::vector<std::shared_ptr<bid>> bid_to_fulfill;
    bid const b1{ 2, 10, 200 };
    bid_to_fulfill.push_back(std::make_shared<bid>(b1));

    book_service::total_quantity_available_ask = 200;
    book_service::total_quantity_wanted_bid = 200;

    const auto [bid_ptr, quantity] = book_service::top_match(bid_to_fulfill);

    REQUIRE_FALSE(bid_ptr == nullptr);
    REQUIRE(bid_to_fulfill[0] == bid_ptr);
    REQUIRE(quantity == a1.get_quantity());
    REQUIRE(quantity == b1.get_quantity());
    
}

TEST_CASE("top_match GIVEN 1 ask 1 bid quantity < 200 RETURN pair nullptr, 0", "[unit]") {
    // INIT
    std::vector<std::shared_ptr<ask>> ask_concerned;
    ask const a1{ 1,10,200 };
    ask_concerned.push_back(std::make_shared<ask>(a1));
    std::vector<std::shared_ptr<bid>> bid_to_fulfill;
    bid const b1{ 2, 10, 199 };
    bid_to_fulfill.push_back(std::make_shared<bid>(b1));

    book_service::total_quantity_available_ask = 200;
    book_service::total_quantity_wanted_bid = 199;

    const auto [bid_ptr, quantity] = book_service::top_match(bid_to_fulfill);

    REQUIRE(bid_ptr == nullptr);
    REQUIRE(quantity == 0);

}

TEST_CASE("top_match GIVEN 2 ask 1 bid RETURN pair 1 bid quantity", "[unit]") {
    // INIT
    std::vector<std::shared_ptr<ask>> ask_concerned;
    ask const a1{ 1,10,150 };
    ask const a2{ 2,10,50 };
    ask_concerned.push_back(std::make_shared<ask>(a1));
    ask_concerned.push_back(std::make_shared<ask>(a2));
    std::vector<std::shared_ptr<bid>> bid_to_fulfill;
    bid const b1{ 3, 10, 200 };
    bid_to_fulfill.push_back(std::make_shared<bid>(b1));

    book_service::total_quantity_available_ask = 200;
    book_service::total_quantity_wanted_bid = 200;

    const auto [bid_ptr, quantity] = book_service::top_match(bid_to_fulfill);

    REQUIRE_FALSE(bid_ptr == nullptr);
    CHECK(bid_ptr->get_id() == bid_to_fulfill[0]->get_id());
    REQUIRE(quantity == a1.get_quantity() + a2.get_quantity());
    REQUIRE(quantity == b1.get_quantity());

}

TEST_CASE("top_match GIVEN 2 ask 1 bid overquantity RETURN pair 1 bid quantity", "[unit]") {
    // INIT
    std::vector<std::shared_ptr<ask>> ask_concerned;
    ask const a1{ 1,10,150 };
    ask const a2{ 2,10,150 };
    ask_concerned.push_back(std::make_shared<ask>(a1));
    ask_concerned.push_back(std::make_shared<ask>(a2));
    std::vector<std::shared_ptr<bid>> bid_to_fulfill;
    bid const b1{ 3, 10, 200 };
    bid_to_fulfill.push_back(std::make_shared<bid>(b1));

    book_service::total_quantity_available_ask = 300;
    book_service::total_quantity_wanted_bid = 200;

    const auto [bid_ptr, quantity] = book_service::top_match(bid_to_fulfill);

    REQUIRE_FALSE(bid_ptr == nullptr);
    CHECK(bid_ptr->get_id() == bid_to_fulfill[0]->get_id());
    REQUIRE(quantity == b1.get_quantity());

}

TEST_CASE("top_match GIVEN 2 ask 1 bid underquantity RETURN pair 1 bid quantity", "[unit]") {
    // INIT
    std::vector<std::shared_ptr<ask>> ask_concerned;
    ask const a1{ 1,10,100 };
    ask const a2{ 2,10,50 };
    ask_concerned.push_back(std::make_shared<ask>(a1));
    ask_concerned.push_back(std::make_shared<ask>(a2));
    std::vector<std::shared_ptr<bid>> bid_to_fulfill;
    bid const b1{ 3, 10, 200 };
    bid_to_fulfill.push_back(std::make_shared<bid>(b1));

    book_service::total_quantity_available_ask = 150;
    book_service::total_quantity_wanted_bid = 200;


    const auto [bid_ptr, quantity] = book_service::top_match(bid_to_fulfill);

    REQUIRE_FALSE(bid_ptr == nullptr);
    CHECK(bid_ptr->get_id() == bid_to_fulfill[0]->get_id());
    REQUIRE(quantity == a1.get_quantity() + a2.get_quantity());

}

TEST_CASE("pro_rata GIVEN 1ask & 2bid , available == wanted RETURN map 2 pair with quantity", "[unit]") {
    // INIT
    std::vector<std::shared_ptr<ask>> ask_concerned;
    ask const a1{ 1,10,100 };
    auto const s_pt_a1 = std::make_shared<ask>(a1);
    ask_concerned.push_back(s_pt_a1);
    std::vector<std::shared_ptr<bid>> bid_to_fulfill;
    bid const b1{ 2, 10, 50 };
    bid const b2{ 3, 10, 50 };
    auto const s_pt_b1 = std::make_shared<bid>(b1);
    auto const s_pt_b2 = std::make_shared<bid>(b2);
    bid_to_fulfill.push_back(s_pt_b1);
    bid_to_fulfill.push_back(s_pt_b2);

    book_service::total_quantity_available_ask = 100;
    book_service::total_quantity_wanted_bid = 100;
    book_service::total_quantity_already_given = 0;

    std::unordered_map<std::shared_ptr<bid>, int> expect = {
        {s_pt_b1, b1.get_quantity()},
        {s_pt_b2, b2.get_quantity()}
    };

    const auto result = book_service::pro_rata(bid_to_fulfill);


    for_each(begin(result), end(result),
        [&](std::pair<std::shared_ptr<bid>, int> const& i) {
            CHECK(i.second == expect.at(i.first));
        });
}

TEST_CASE("pro_rata GIVEN CME EXEMPLE", "[unit]") {
    // INIT
    std::vector<std::shared_ptr<ask>> ask_concerned;
    ask const a1{ 1,10,50 };
    auto const s_pt_a1 = std::make_shared<ask>(a1);
    ask_concerned.push_back(s_pt_a1);
    std::vector<std::shared_ptr<bid>> bid_to_fulfill;
    bid const b1{ 2, 10, 25 };
    bid const b2{ 3, 10, 50 };
    bid const b3{ 4, 10, 10 };
    auto const s_pt_b1 = std::make_shared<bid>(b1);
    auto const s_pt_b2 = std::make_shared<bid>(b2);
    auto const s_pt_b3 = std::make_shared<bid>(b3);
    bid_to_fulfill.push_back(s_pt_b1);
    bid_to_fulfill.push_back(s_pt_b2);
    bid_to_fulfill.push_back(s_pt_b3);


    book_service::total_quantity_available_ask = 250;
    book_service::total_quantity_wanted_bid = 285;
    book_service::total_quantity_already_given = 200;

    std::unordered_map<std::shared_ptr<bid>, int> expect = {
        {s_pt_b1, 14},
        {s_pt_b2, 29},
        {s_pt_b3, 5}
    };

    const auto result = book_service::pro_rata(bid_to_fulfill);


    for_each(begin(result), end(result),
        [&](std::pair<std::shared_ptr<bid>, int> const& i) {
            CHECK(i.second == expect.at(i.first));
        });
}

TEST_CASE("fill_bids GIVEN 5 bids wanted RETURN map 5 pair with quantity", "[unit]") {
    // INIT
    std::vector<std::shared_ptr<bid>> bid_to_fulfill;
    bid const b1{ 2, 10, 25 };
    bid const b2{ 3, 10, 50 };
    bid const b3{ 4, 10, 10 };
    bid const b4{ 4, 10, 100 };
    bid const b5{ 4, 10, 500 };
    auto const s_pt_b1 = std::make_shared<bid>(b1);
    auto const s_pt_b2 = std::make_shared<bid>(b2);
    auto const s_pt_b3 = std::make_shared<bid>(b3);
    auto const s_pt_b4 = std::make_shared<bid>(b4);
    auto const s_pt_b5 = std::make_shared<bid>(b5);
    bid_to_fulfill.push_back(s_pt_b1);
    bid_to_fulfill.push_back(s_pt_b2);
    bid_to_fulfill.push_back(s_pt_b3);
    bid_to_fulfill.push_back(s_pt_b4);
    bid_to_fulfill.push_back(s_pt_b5);
    
    book_service::total_quantity_already_given = 0;

    std::unordered_map<std::shared_ptr<bid>, int> expect = {
        {s_pt_b1, 25},
        {s_pt_b2, 50},
        {s_pt_b3, 10},
        {s_pt_b4, 100},
        {s_pt_b5, 500}
    };

    const auto result = book_service::fill_bids(bid_to_fulfill);
    
    for_each(begin(result), end(result),
        [&](std::pair<std::shared_ptr<bid>, int> const& i) {
            CHECK(i.second == expect.at(i.first));
        });
}

TEST_CASE("fifo GIVEN CME EXEMPLE", "[unit]") {
    // INIT
    std::vector<std::shared_ptr<ask>> ask_concerned;
    ask const a1{ 1,10,50 };
    auto const s_pt_a1 = std::make_shared<ask>(a1);
    ask_concerned.push_back(s_pt_a1);
    std::vector<std::shared_ptr<bid>> bid_to_fulfill;
    bid const b1{ 2, 10, 25 };
    bid const b2{ 3, 10, 50 };
    bid const b3{ 4, 10, 10 };
    auto const s_pt_b1 = std::make_shared<bid>(b1);
    auto const s_pt_b2 = std::make_shared<bid>(b2);
    auto const s_pt_b3 = std::make_shared<bid>(b3);
    bid_to_fulfill.push_back(s_pt_b1);
    bid_to_fulfill.push_back(s_pt_b2);
    bid_to_fulfill.push_back(s_pt_b3);


    book_service::total_quantity_available_ask = 250;
    book_service::total_quantity_wanted_bid = 285;
    book_service::total_quantity_already_given = 200;

    std::unordered_map<std::shared_ptr<bid>, int> expect = {
        {s_pt_b1, 16},
        {s_pt_b2, 29},
        {s_pt_b3, 5}
    };

    auto result = book_service::pro_rata(bid_to_fulfill);
    book_service::fifo(result);

    for_each(begin(result), end(result),
        [&](std::pair<std::shared_ptr<bid>, int> const& i) {
            CHECK(i.second == expect.at(i.first));
        });    
}

TEST_CASE("matching_orders GIVEN CME EXEMPLE", "[unit]") {
    // INIT
    std::vector<std::shared_ptr<ask>> ask_concerned;
    ask const a1{ 1,10,150 };
    ask const a2{ 2,10,100 };
    ask_concerned.push_back(std::make_shared<ask>(a1));
    ask_concerned.push_back(std::make_shared<ask>(a2));
    std::vector<std::shared_ptr<bid>> bid_to_fulfill;
    bid const b1{ 3, 10, 200 };
    bid const b2{ 4, 10, 25 };
    bid const b3{ 5, 10, 50 };
    bid const b4{ 6, 10, 10 };
    auto const s_pt_b1 = std::make_shared<bid>(b1);
    auto const s_pt_b2 = std::make_shared<bid>(b2);
    auto const s_pt_b3 = std::make_shared<bid>(b3);
    auto const s_pt_b4 = std::make_shared<bid>(b4);
    bid_to_fulfill.push_back(s_pt_b1);
    bid_to_fulfill.push_back(s_pt_b2);
    bid_to_fulfill.push_back(s_pt_b3);
    bid_to_fulfill.push_back(s_pt_b4);

    std::unordered_map<std::shared_ptr<bid>, int> expect = {
        {s_pt_b1, 200},
        {s_pt_b2, 16},
        {s_pt_b3, 29},
        {s_pt_b4, 5},
    };

    auto const result = book_service::matching_orders(bid_to_fulfill, ask_concerned);
    
    std::for_each(begin(result), end(result), 
        [&](std::pair<std::shared_ptr<bid>, int> const& i) {
            REQUIRE(i.second == expect.at(i.first));
    });

}

TEST_CASE("matching_orders GIVEN over quantity asked", "[unit]") {
    // INIT
    std::vector<std::shared_ptr<ask>> ask_concerned;
    ask const a1{ 1,10,150 };
    ask const a2{ 2,10,200 };
    ask_concerned.push_back(std::make_shared<ask>(a1));
    ask_concerned.push_back(std::make_shared<ask>(a2));
    std::vector<std::shared_ptr<bid>> bid_to_fulfill;
    bid const b1{ 3, 10, 200 };
    bid const b2{ 4, 10, 25 };
    bid const b3{ 5, 10, 50 };
    bid const b4{ 6, 10, 10 };
    auto const s_pt_b1 = std::make_shared<bid>(b1);
    auto const s_pt_b2 = std::make_shared<bid>(b2);
    auto const s_pt_b3 = std::make_shared<bid>(b3);
    auto const s_pt_b4 = std::make_shared<bid>(b4);
    bid_to_fulfill.push_back(s_pt_b1);
    bid_to_fulfill.push_back(s_pt_b2);
    bid_to_fulfill.push_back(s_pt_b3);
    bid_to_fulfill.push_back(s_pt_b4);

    std::unordered_map<std::shared_ptr<bid>, int> expect = {
        {s_pt_b1, 200},
        {s_pt_b2, 25},
        {s_pt_b3, 50},
        {s_pt_b4, 10},
    };

    auto const result = book_service::matching_orders(bid_to_fulfill, ask_concerned);

    for_each(begin(result), end(result),
        [&](std::pair<std::shared_ptr<bid>, int> const& i) {
            REQUIRE(i.second == expect.at(i.first));
        });

}

TEST_CASE("matching_orders GIVEN top order took everything", "[unit]") {
    // INIT
    std::vector<std::shared_ptr<ask>> ask_concerned;
    ask const a1{ 1,10,200 };
    ask_concerned.push_back(std::make_shared<ask>(a1));
    std::vector<std::shared_ptr<bid>> bid_to_fulfill;
    bid const b1{ 3, 10, 200 };
    bid const b2{ 4, 10, 25 };
    bid const b3{ 5, 10, 50 };
    bid const b4{ 6, 10, 10 };
    auto const s_pt_b1 = std::make_shared<bid>(b1);
    auto const s_pt_b2 = std::make_shared<bid>(b2);
    auto const s_pt_b3 = std::make_shared<bid>(b3);
    auto const s_pt_b4 = std::make_shared<bid>(b4);
    bid_to_fulfill.push_back(s_pt_b1);
    bid_to_fulfill.push_back(s_pt_b2);
    bid_to_fulfill.push_back(s_pt_b3);
    bid_to_fulfill.push_back(s_pt_b4);

    std::unordered_map<std::shared_ptr<bid>, int> expect = {
        {s_pt_b1, 200},
        {s_pt_b2, 0},
        {s_pt_b3, 0},
        {s_pt_b4, 0},
    };

    auto const result = book_service::matching_orders(bid_to_fulfill, ask_concerned);

    for_each(begin(result), end(result),
        [&](std::pair<std::shared_ptr<bid>, int> const& i) {
            REQUIRE(i.second == expect.at(i.first));
        });

}

TEST_CASE("matching orders GIVEN CME EXEMPLE V2", "[unit]") {
    // INIT
    std::vector<std::shared_ptr<ask>> ask_concerned;
    ask const a1{ 1,10,100 };
    ask_concerned.push_back(std::make_shared<ask>(a1));
    std::vector<std::shared_ptr<bid>> bid_to_fulfill;
    bid const b1{ 3, 10, 100 };
    bid const b2{ 4, 10, 5 };
    bid const b3{ 5, 10, 150 };
    auto const s_pt_b1 = std::make_shared<bid>(b1);
    auto const s_pt_b2 = std::make_shared<bid>(b2);
    auto const s_pt_b3 = std::make_shared<bid>(b3);
    bid_to_fulfill.push_back(s_pt_b1);
    bid_to_fulfill.push_back(s_pt_b2);
    bid_to_fulfill.push_back(s_pt_b3);

    std::unordered_map<std::shared_ptr<bid>, int> expect = {
        {s_pt_b1, 42},
        {s_pt_b2, 0},
        {s_pt_b3, 58},
    };

    auto const result = book_service::matching_orders(bid_to_fulfill, ask_concerned);

    for_each(begin(result), end(result),
        [&](std::pair<std::shared_ptr<bid>, int> const& i) {
            REQUIRE(i.second == expect.at(i.first));
        });
}

TEST_CASE("matching orders GIVEN 1ask 1bid", "[unit]") {
    // INIT
    std::vector<std::shared_ptr<ask>> ask_concerned;
    ask const a1{ 1,10,50 };
    ask_concerned.push_back(std::make_shared<ask>(a1));
    std::vector<std::shared_ptr<bid>> bid_to_fulfill;
    bid const b1{ 3, 10, 100 };
    auto const s_pt_b1 = std::make_shared<bid>(b1);
    bid_to_fulfill.push_back(s_pt_b1);

    std::unordered_map<std::shared_ptr<bid>, int> expect = {
        {s_pt_b1, 50},
    };

    auto const result = book_service::matching_orders(bid_to_fulfill, ask_concerned);

    for_each(begin(result), end(result),
        [&](std::pair<std::shared_ptr<bid>, int> const& i) {
            REQUIRE(i.second == expect.at(i.first));
        });
}

TEST_CASE("remove_quantity_from_ask GIVEN 1 ask 1 bid", "[unit]") {
    // INIT
    std::vector<std::shared_ptr<ask>> ask_concerned;
    ask const a1{ 1,10,50 };
    auto const s_pt_a1 = std::make_shared<ask>(a1);
    ask_concerned.push_back(s_pt_a1);
    std::vector<std::shared_ptr<bid>> bid_to_fulfill;
    bid const b1{ 3, 10, 100 };
    auto const s_pt_b1 = std::make_shared<bid>(b1);
    bid_to_fulfill.push_back(s_pt_b1);

    std::unordered_map<std::shared_ptr<bid>, int> const orders = {
        {s_pt_b1, 50},
    };

    std::unordered_map<std::shared_ptr<ask>, int> const expect = {
        {s_pt_a1, 50}
    };

    auto const result = book_service::remove_quantity_from_ask(orders, ask_concerned);

    for_each(begin(result), end(result),
        [&](std::pair<std::shared_ptr<ask>, int> const& i) {
            REQUIRE(i.second == expect.at(i.first));
        });
    
}

TEST_CASE("remove_quantity_from_ask GIVEN CME EXEMPLE", "[unit]") {
    // INIT
    std::vector<std::shared_ptr<ask>> ask_concerned;
    ask const a1{ 1,10,150 };
    ask const a2{ 2,10,100 };
    auto const s_pt_a1 = std::make_shared<ask>(a1);
    auto const s_pt_a2 = std::make_shared<ask>(a2);
    ask_concerned.push_back(s_pt_a1);
    ask_concerned.push_back(s_pt_a2);
    std::vector<std::shared_ptr<bid>> bid_to_fulfill;
    bid const b1{ 3, 10, 200 };
    bid const b2{ 4, 10, 25 };
    bid const b3{ 5, 10, 50 };
    bid const b4{ 6, 10, 10 };
    auto const s_pt_b1 = std::make_shared<bid>(b1);
    auto const s_pt_b2 = std::make_shared<bid>(b2);
    auto const s_pt_b3 = std::make_shared<bid>(b3);
    auto const s_pt_b4 = std::make_shared<bid>(b4);
    bid_to_fulfill.push_back(s_pt_b1);
    bid_to_fulfill.push_back(s_pt_b2);
    bid_to_fulfill.push_back(s_pt_b3);
    bid_to_fulfill.push_back(s_pt_b4);

    std::unordered_map<std::shared_ptr<bid>, int> const orders = {
        {s_pt_b1, 200},
        {s_pt_b2, 16},
        {s_pt_b3, 29},
        {s_pt_b4, 5},
    };

    std::unordered_map<std::shared_ptr<ask>, int> const expect = {
        {s_pt_a1, 150},
        {s_pt_a2, 100}
    };

    auto const result = book_service::remove_quantity_from_ask(orders, ask_concerned);

    std::for_each(begin(result), end(result),
        [&](std::pair<std::shared_ptr<ask>, int> const& i) {
            REQUIRE(i.second == expect.at(i.first));
        });

}

TEST_CASE("remove_quantity_from_ask GIVEN CME EXEMPLE V2", "[unit]") {
    // INIT
    std::vector<std::shared_ptr<ask>> ask_concerned;
    ask const a1{ 1,10,75 };
    ask const a2{ 1,10,15 };
    ask const a3{ 1,10,9 };
    ask const a4{ 1,10,1 };
    auto const s_pt_a1 = std::make_shared<ask>(a1);
    auto const s_pt_a2 = std::make_shared<ask>(a2);
    auto const s_pt_a3 = std::make_shared<ask>(a3);
    auto const s_pt_a4 = std::make_shared<ask>(a4);
    ask_concerned.push_back(s_pt_a1);
    ask_concerned.push_back(s_pt_a2);
    ask_concerned.push_back(s_pt_a3);
    ask_concerned.push_back(s_pt_a4);
    std::vector<std::shared_ptr<bid>> bid_to_fulfill;
    bid const b1{ 3, 10, 100 };
    bid const b2{ 4, 10, 5 };
    bid const b3{ 5, 10, 150 };
    auto const s_pt_b1 = std::make_shared<bid>(b1);
    auto const s_pt_b2 = std::make_shared<bid>(b2);
    auto const s_pt_b3 = std::make_shared<bid>(b3);
    bid_to_fulfill.push_back(s_pt_b1);
    bid_to_fulfill.push_back(s_pt_b2);
    bid_to_fulfill.push_back(s_pt_b3);

    std::unordered_map<std::shared_ptr<bid>, int> const orders = {
        {s_pt_b1, 42},
        {s_pt_b2, 0},
        {s_pt_b3, 58},
    };

    std::unordered_map<std::shared_ptr<ask>, int> const expect = {
       {s_pt_a1, 75},
       {s_pt_a2, 15},
       {s_pt_a3, 9},
       {s_pt_a4, 1},
    };

    auto const result = book_service::remove_quantity_from_ask(orders, ask_concerned);

    std::for_each(begin(result), end(result),
        [&](std::pair<std::shared_ptr<ask>, int> const& i) {
            REQUIRE(i.second == expect.at(i.first));
        });
}

TEST_CASE("remove_quantity_from_ask GIVEN CME EXEMPLE V2 with leftover", "[unit]") {
    // INIT
    std::vector<std::shared_ptr<ask>> ask_concerned;
    ask const a1{ 1,10,100 };
    ask const a2{ 1,10,25 };
    ask const a3{ 1,10,75 };
    ask const a4{ 1,10,200 };
    auto const s_pt_a1 = std::make_shared<ask>(a1);
    auto const s_pt_a2 = std::make_shared<ask>(a2);
    auto const s_pt_a3 = std::make_shared<ask>(a3);
    auto const s_pt_a4 = std::make_shared<ask>(a4);
    ask_concerned.push_back(s_pt_a1);
    ask_concerned.push_back(s_pt_a2);
    ask_concerned.push_back(s_pt_a3);
    ask_concerned.push_back(s_pt_a4);
    std::vector<std::shared_ptr<bid>> bid_to_fulfill;
    bid const b1{ 3, 10, 100 };
    bid const b2{ 4, 10, 5 };
    bid const b3{ 5, 10, 150 };
    auto const s_pt_b1 = std::make_shared<bid>(b1);
    auto const s_pt_b2 = std::make_shared<bid>(b2);
    auto const s_pt_b3 = std::make_shared<bid>(b3);
    bid_to_fulfill.push_back(s_pt_b1);
    bid_to_fulfill.push_back(s_pt_b2);
    bid_to_fulfill.push_back(s_pt_b3);

    std::unordered_map<std::shared_ptr<bid>, int> const orders = {
        {s_pt_b1, 100},
        {s_pt_b2, 5},
        {s_pt_b3, 150},
    };

    std::unordered_map<std::shared_ptr<ask>, int> const expect = {
       {s_pt_a1, 100},
       {s_pt_a2, 25},
       {s_pt_a3, 75},
       {s_pt_a4, 55},
    };

    auto const result = book_service::remove_quantity_from_ask(orders, ask_concerned);

    std::for_each(begin(result), end(result),
        [&](std::pair<std::shared_ptr<ask>, int> const& i) {
            REQUIRE(i.second == expect.at(i.first));
        });
}

TEST_CASE("affect residual bids GIVEN", "[unit]") {
    // INIT
    book<bid> b_bid;
    std::vector<std::shared_ptr<bid>> bid_to_fulfill;
    bid const b1{ 3, 10, 100 };
    bid const b2{ 4, 10, 5 };
    bid const b3{ 5, 10, 150 };
    auto const s_pt_b1 = std::make_shared<bid>(b1);
    auto const s_pt_b2 = std::make_shared<bid>(b2);
    auto const s_pt_b3 = std::make_shared<bid>(b3);
    bid_to_fulfill.push_back(s_pt_b1);
    bid_to_fulfill.push_back(s_pt_b2);
    bid_to_fulfill.push_back(s_pt_b3);

    std::unordered_map<std::shared_ptr<bid>, int> const orders = {
        {s_pt_b1, 42},
        {s_pt_b2, 0},
        {s_pt_b3, 58},
    };

    std::unordered_map<int, std::shared_ptr<bid>> const expect = {
       {s_pt_b1->get_id(), s_pt_b1},
       {s_pt_b2->get_id(), s_pt_b2},
       {s_pt_b3->get_id(), s_pt_b3}
    };
    book_service::affect_residual_bids(b_bid, orders);

    while(!b_bid.empty()) {
        auto const curr_b = b_bid.top();
        auto const curr_ptr_b = expect.at(curr_b.get_id());

        REQUIRE(curr_b.get_quantity() == curr_ptr_b->get_quantity() - orders.at(curr_ptr_b));
        CHECK(curr_b.get_time().time_since_epoch().count() == curr_ptr_b->get_time().time_since_epoch().count());

        b_bid.pop();
    }
}

TEST_CASE("affect residual asks GIVEN", "[unit]") {
    // INIT
    book<ask> b_ask;
    std::vector<std::shared_ptr<ask>> ask_to_fulfill;
    ask const a1{ 3, 10, 100 };
    ask const a2{ 4, 10, 5 };
    ask const a3{ 5, 10, 150 };
    auto const s_pt_a1 = std::make_shared<ask>(a1);
    auto const s_pt_a2 = std::make_shared<ask>(a2);
    auto const s_pt_a3 = std::make_shared<ask>(a3);
    ask_to_fulfill.push_back(s_pt_a1);
    ask_to_fulfill.push_back(s_pt_a2);
    ask_to_fulfill.push_back(s_pt_a3);

    std::unordered_map<std::shared_ptr<ask>, int> const orders = {
        {s_pt_a1, 42},
        {s_pt_a2, 0},
        {s_pt_a3, 58},
    };

    std::unordered_map<int, std::shared_ptr<ask>> const expect = {
       {s_pt_a1->get_id(), s_pt_a1},
       {s_pt_a2->get_id(), s_pt_a2},
       {s_pt_a3->get_id(), s_pt_a3}
    };
    book_service::affect_residual_asks(b_ask, orders);

    while (!b_ask.empty()) {
        auto const curr_a = b_ask.top();
        auto const curr_ptr_a = expect.at(curr_a.get_id());

        REQUIRE(curr_a.get_quantity() == curr_ptr_a->get_quantity() - orders.at(curr_ptr_a));
        CHECK(curr_a.get_time().time_since_epoch().count() == curr_ptr_a->get_time().time_since_epoch().count());

        b_ask.pop();
    }
    
}