
#include <iostream>
#include <boost/beast.hpp>


template <typename WS>
void DoRecv(WS& ws, boost::system::error_code& ec)
{
    boost::beast::flat_buffer flat_buffer;
    ec.clear();
    auto result = ws.read(flat_buffer, ec);
    if (!ec)
    {
        std::cout << "Received " << result << " bytes" << std::endl;
        char* data = (char*)(flat_buffer.data().data());
        size_t len = flat_buffer.data().size();
        std::string str(data, len);
        std::cout << str << std::endl;

        return;
    }
    std::cout << "Error on recv" << ec << std::endl;

    return;
}

int main()
{
    boost::asio::io_context ctx(1);
    boost::asio::ip::tcp::endpoint localhost(boost::asio::ip::make_address_v4("127.0.0.1"), 9876);
    boost::asio::ip::tcp::acceptor acc(ctx, localhost);
    boost::system::error_code ec;

    while (true)
    {
        try
        {
            auto sock = acc.accept(ec);

            boost::beast::flat_buffer flat_buffer;
            boost::beast::http::request<boost::beast::http::string_body> req;
            boost::beast::http::read(sock, flat_buffer, req, ec);

            if (boost::beast::websocket::is_upgrade(req))
            {
                std::cout << "Detected Websocket upgrade request" << std::endl;
                boost::beast::websocket::stream<boost::asio::ip::tcp::socket> ws(std::move(sock));

                ws.accept(req);

                std::cout << "Websocket Accept completed" << std::endl;

                while (!ec)
                {
                    ec.clear();
                    DoRecv(ws, ec);
                }
            }
            else
            {
                boost::beast::http::response<boost::beast::http::string_body> resposne;

                std::cout << "incoming request was not a websocket upgrade" << std::endl;
                std::string response_str;
                boost::beast::http::response< boost::beast::http::string_body> response;
                response_str = "<html><body><h1>404 Not Found</h1></body></html>";
                response.body() = response_str;
                response.result(boost::beast::http::status::not_found);
                response.set("Content-Type", "text/html");
                response.prepare_payload();
                boost::beast::http::write(sock, response);
            }
        }
        catch (std::exception&)
        {
            std::cout << "Exception caught" << std::endl;
        }
    }
};


