#include <iostream>
#include <olc_net.h>
#include <queue>

#include "book_service.h"

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

class custom_server : public olc::net::server_interface<custom_msg_types>
{
public:
	custom_server(uint16_t nPort) : olc::net::server_interface<custom_msg_types>(nPort)
	{

	}

	// Force server to respond to incoming messages
	void Update(size_t nMaxMessages = -1, bool bWait = false)
	{
		if (bWait) m_qMessagesIn.wait();

		// Process as many messages as you can up to the value
		// specified
		size_t nMessageCount = 0;
		while (nMessageCount < nMaxMessages && !m_qMessagesIn.empty())
		{
			// Grab the front message
			auto msg = m_qMessagesIn.pop_front();

			// Pass to message handler
			OnMessage(msg.remote, msg.msg);

			nMessageCount++;
		}
	}

protected:
	virtual bool OnClientConnect(std::shared_ptr<olc::net::connection<custom_msg_types>> client)
	{
		olc::net::message<custom_msg_types> msg;
		msg.header.id = custom_msg_types::server_accept;
		client->Send(msg);
		return true;
	}

	// Called when a client appears to have disconnected
	virtual void OnClientDisconnect(std::shared_ptr<olc::net::connection<custom_msg_types>> client)
	{
		std::cout << "Removing client [" << client->GetID() << "]\n";
	}

	// Called when a message arrives
	virtual void OnMessage(std::shared_ptr<olc::net::connection<custom_msg_types>> client, olc::net::message<custom_msg_types>& msg)
	{
		switch (msg.header.id)
		{

		case custom_msg_types::ask_info: {
			int id_asked;
			msg >> id_asked;
			olc::net::message<custom_msg_types> msg1;
			if(book_bid_.empty() || book_ask_.empty()) {
				msg1.header.id = custom_msg_types::error;
				msg1 << 404;
				MessageClient(client, msg1);
			} else {
				msg1.header.id = custom_msg_types::ask_info;
				auto const o_bid = book_bid_.top();
				auto const o_ask = book_ask_.top();
				msg1 << o_bid.get_price() << o_ask.get_price();
				client->Send(msg1);
			}
		} break;

		case custom_msg_types::place_bid: {
			float price;
			int quantity;
			msg >> quantity >> price;
			
			bid const bid_client{client->GetID(), price, quantity};
			book_bid_.push(bid_client);

			olc::net::message<custom_msg_types> msg1;
			msg1.header.id = custom_msg_types::place_bid;
			msg1 << price << quantity;
			client->Send(msg1);
		} break;

		case custom_msg_types::place_ask: {
			float price;
			int quantity;
			msg >> quantity >> price;

			ask const ask_client{ client->GetID(), price, quantity };
			book_ask_.push(ask_client);

			olc::net::message<custom_msg_types> msg1;
			msg1.header.id = custom_msg_types::place_ask;
			msg1 << price << quantity;
			client->Send(msg1);

		} break;

		case custom_msg_types::test: {

			auto const [bids, asks] = book_service::update_books(book_bid_, book_ask_);

			std::for_each(begin(bids), end(bids),
				[&](std::pair<std::shared_ptr<bid>, int> const& i) {
					if (i.second != 0) {
						olc::net::message<custom_msg_types> msg_to_send;
						msg_to_send.header.id = custom_msg_types::success_bid;
						msg_to_send << i.second << i.first->get_price();
						message_client_by_id(i.first->get_id(), msg_to_send);
					}
				});

			std::for_each(begin(asks), end(asks),
				[&](std::pair<std::shared_ptr<ask>, int> const& i) {
					if (i.second != 0) {
						olc::net::message<custom_msg_types> msg_to_send;
						msg_to_send.header.id = custom_msg_types::success_ask;
						msg_to_send << i.second << i.first->get_price();
						message_client_by_id(i.first->get_id(), msg_to_send);
					}
				});

		} break;

        }
	}

	void message_client_by_id(uint32_t const id, const olc::net::message<custom_msg_types>& msg)
	{
		auto b_invalid_client_exists = false;

		// Iterate through all clients in container
		for (auto& client : m_deqConnections)
		{
			if (client->GetID() == id) {
				// Check client is connected...
				if (client && client->IsConnected())
				{
					client->Send(msg);
					break;
				}
				// The client couldnt be contacted, so assume it has
				// disconnected.
				OnClientDisconnect(client);
				client.reset();

				// Set this flag to then remove dead clients from container
				b_invalid_client_exists = true;
			}
			
		}

		// Remove dead clients, all in one go - this way, we dont invalidate the
		// container as we iterated through it.
		if (b_invalid_client_exists)
			m_deqConnections.erase(
				std::remove(m_deqConnections.begin(), m_deqConnections.end(), nullptr), m_deqConnections.end());
	}

private:
	book<bid> book_bid_;
	book<ask> book_ask_;

};

int main() {

	custom_server server(60000);
	server.Start();


	while (true) {
		server.Update(-1, true);
	}
}