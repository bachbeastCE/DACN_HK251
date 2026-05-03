// #include <boost/beast/core.hpp>
// #include <boost/beast/websocket.hpp>
// #include <boost/asio/ip/tcp.hpp>
// #include <iostream>
// #include <sstream>
// #include <thread>
// #include <chrono>

// namespace beast = boost::beast;
// namespace websocket = beast::websocket;
// namespace net = boost::asio;
// using tcp = net::ip::tcp;

// int main() {
//     try {
//         net::io_context ioc;

//         tcp::resolver resolver{ioc};
//         websocket::stream<tcp::socket> ws{ioc};

//         std::cout << "Connecting to server...\n";

//         auto const results = resolver.resolve("127.0.0.1", "9001");
//         net::connect(ws.next_layer(), results.begin(), results.end());

//         ws.handshake("127.0.0.1", "/");

//         std::cout << "✅ Connected!\n";

//         int id = 1;
//         double lat = 11.5, lon = 106.9;

//         while (true) {
//             // tạo JSON
//             std::stringstream json;
//             json << "{"
//                  << "\"id\":" << id << ","
//                  << "\"lat\":" << lat << ","
//                  << "\"lon\":" << lon
//                  << "}";

//             std::string msg = json.str();

//             // gửi
//             ws.write(net::buffer(msg));
//             std::cout << "📤 Sent: " << msg << std::endl;

//             // nhận echo từ server
//             beast::flat_buffer buffer;
//             ws.read(buffer);
//             std::cout << "📥 Received: "
//                       << beast::make_printable(buffer.data())
//                       << std::endl;

//             id++;
//             lat += 0.0001;
//             lon += 0.0001;

//             std::this_thread::sleep_for(std::chrono::seconds(2));
//         }

//     } catch (std::exception const& e) {
//         std::cerr << "❌ Error: " << e.what() << std::endl;
//     }
// }
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <iostream>

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;

int main() {
    try {
        net::io_context ioc;

        tcp::resolver resolver{ioc};
        websocket::stream<tcp::socket> ws{ioc};

        std::cout << "Connecting to server...\n";

        auto const results = resolver.resolve("127.0.0.1", "9001");
        net::connect(ws.next_layer(), results.begin(), results.end());

        ws.handshake("127.0.0.1", "/");

        std::cout << "✅ Connected! Listening for broadcast...\n";

        // 🔥 chỉ nhận dữ liệu từ server
        while (true) {
            beast::flat_buffer buffer;

            ws.read(buffer);  // block chờ server gửi

            std::string msg = beast::buffers_to_string(buffer.data());

            std::cout << "📥 Broadcast: " << msg << std::endl;
        }

    } catch (std::exception const& e) {
        std::cerr << "❌ Error: " << e.what() << std::endl;
    }

    return 0;
}