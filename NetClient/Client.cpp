#include <iostream>
#include <olc_net.h>


enum class custom_msg_types : uint32_t
{
	ask_info,
	place_bid,
	place_ask,
	success_ask,
	success_bid,
	fail,
	test,
	error,
	server_accept,
};

struct order {
	order(float const price, int const quantity) : price(price), quantity(quantity) {}
	std::chrono::system_clock::time_point timestamp = std::chrono::system_clock::now();
	float price;
	int quantity;
};

class custom_client : public olc::net::client_interface<custom_msg_types>
{
public:

	void bid(order const o) {
		olc::net::message<custom_msg_types> msg;
		msg.header.id = custom_msg_types::place_bid;
		msg << o.price << o.quantity;
		Send(msg);
	}

	void ask(order const o) {
		olc::net::message<custom_msg_types> msg;
		msg.header.id = custom_msg_types::place_ask;
		msg << o.price << o.quantity;
		Send(msg);
	}

	void ask_stock_value() {
		olc::net::message<custom_msg_types> msg;
		msg.header.id = custom_msg_types::ask_info;
		auto const stock = 1;
		msg << stock;
		Send(msg);
	}

	void testo() {
		olc::net::message<custom_msg_types> msg;
		msg.header.id = custom_msg_types::test;
		auto const stock = 1;
		msg << stock;
		Send(msg);
	    
	}
};

int main()
{
	custom_client c;
	c.Connect("127.0.0.1", 60000);

	bool key[4] = { false, false, false, false};
	bool old_key[4] = { false, false, false, false};

    auto b_quit = false;
	while (!b_quit)
	{
		if (GetForegroundWindow() == GetConsoleWindow())
		{
			key[0] = GetAsyncKeyState('1') & 0x8000;
			key[1] = GetAsyncKeyState('2') & 0x8000;
			key[2] = GetAsyncKeyState('3') & 0x8000;
			key[3] = GetAsyncKeyState('4') & 0x8000;
		}

		if (key[0] && !old_key[0]) {
			float price = 10;
			//std::cin >> price;
			int qte;
		    std::cin >> qte;
			c.bid(order{ price, qte });
		}
		if (key[1] && !old_key[1]) {
			float price = 10;
			//std::cin >> price;
			int qte;
			std::cin >> qte;
		    c.ask(order{ price, qte});
		}
		if (key[2] && !old_key[2]) c.ask_stock_value();
		if (key[3] && !old_key[3]) c.testo();

		for (auto i = 0; i < 4; i++) old_key[i] = key[i];

		if (c.IsConnected())
		{
			if (!c.Incoming().empty())
			{


				auto msg = c.Incoming().pop_front().msg;

				switch (msg.header.id)
				{
				case custom_msg_types::server_accept:
				{
					// Server has responded to a ping request				
					std::cout << "Server Accepted Connection\n";
				} break;

				case custom_msg_types::ask_info: {
					float o_bid, o_ask;
					msg >> o_ask >> o_bid;
					std::cout << "best ask : " << o_ask << " $\tbest bid : " << o_bid << " $\n";
				} break;

				case custom_msg_types::place_bid: {
					float price;
					int quantity;
					msg >> quantity >> price;
					std::cout << "You successfuly placed a bid order @ " << price << " $ over " << quantity << " shares\n";
				} break;
										
				case custom_msg_types::place_ask: {
					float price;
					int quantity;
					msg >> quantity >> price;
					std::cout << "You successfuly placed a ask order @ " << price << " $ over " << quantity << " shares\n";
				} break;

				case custom_msg_types::error: {
					int ec;
					msg >> ec;
					std::cout << "Error : " << ec << "\n";
				} break;

				case custom_msg_types::success_bid: {
					int nb;
					float price;
					msg >> price >> nb;
					std::cout << "Your order has been executed and you have obtained " << nb << " shares @ " << price << " $\nAny residual orders are still in the book\n";
				} break;

				case custom_msg_types::success_ask: {
					int nb;
					float price;
					msg >> price >> nb;
					std::cout << "Your order has been executed and you have sold " << nb << " shares @ " << price << " $\nAny residual orders are still in the book\n";
				} break;

				}
			}
		} else {
			std::cout << "Server Down\n";
			b_quit = true;
		}
	}
	return 0;
}