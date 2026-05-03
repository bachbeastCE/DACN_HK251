#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <iostream>
#include <sstream>
#include <curl/curl.h>
#include <chrono>
#include <thread>
#include <mutex>
#include "lora_api.h"
#include "micro_aes.h"
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <queue>
#include <condition_variable>

queue<string> wsQueue;
queue<std::string> fbQueue;

std::mutex mtx;
std::condition_variable cv;

bool isInternetAvailable = true;

#define PACKET_SIZE 88
#define PLAIN_TEXT_SIZE 64

typedef struct __attribute__((packed)) LOCATION_DATA_HEADER {
	uint64_t seq_num; // 8 bytes
}  LOC_DATA_HEADER;

typedef struct __attribute__((packed)) LOCATION_DATA {
	uint32_t device_id; // 4 bytes
	float gps_hdop;  // 4 bytes
    double loc_gps_lon; // 8 bytes
    double loc_gps_lat; // 8 bytes
    double loc_gps_alt; // 8 bytes
    double tag_gps_lon; // 8 bytes
    double tag_gps_lat; // 8 bytes
    double tag_gps_alt; // 8 bytes
    double tag_distance; // 8 bytes
}  LOC_DATA_PAYLOAD;

typedef struct __attribute__((packed)) LOC_RECEIVE_PACKET {
  LOC_DATA_HEADER loc_data_header;
  LOC_DATA_PAYLOAD loc_data_payload;
  uint8_t security_tag[16] ;
} LOC_RECEIVE_PACKET ;

LOC_RECEIVE_PACKET receive_packet;
uint8_t key[] = {0x2b, 0x7e, 0x15, 0x16,0x28, 0xae, 0xd2, 0xa6,0xab, 0xf7, 0x15, 0x88,0x09, 0xcf, 0x4f, 0x3c};
uint8_t gcm_nonce[GCM_NONCE_LEN];

LOC_DATA_PAYLOAD plain_text;
uint8_t decrypt_status;

// Hàm này nhận vào Sequence Number và con trỏ chứa Payload đã giải mã (plain_text)
void print_location_data(uint64_t seq_num, const LOC_DATA_PAYLOAD *payload) {
    // Dùng PRIu64 để in chuẩn xác uint64_t trên mọi hệ điều hành (32/64 bit)
    printf("------ After decrypt: -------------\n");
    printf("Packet order: %" PRIu64 "\n", seq_num);
    printf("------ RECEIVED DATA PACKET ---\n");
    
    // In Device ID (Dạng Hex 8 chữ số, có số 0 ở đầu)
    printf("Device ID:    0x%08X\n", payload->device_id);

    // In tọa độ GPS Local (%.6f để lấy 6 số thập phân, %.2f lấy 2 số)
    printf("Local GPS:    Lat: %.6f | Lon: %.6f | Alt: %.2f\n", 
           payload->loc_gps_lat, payload->loc_gps_lon, payload->loc_gps_alt);

    // In tọa độ GPS Tag
    printf("Tag GPS:      Lat: %.6f | Lon: %.6f | Alt: %.2f\n", 
           payload->tag_gps_lat, payload->tag_gps_lon, payload->tag_gps_alt);

    // In khoảng cách và sai số
    printf("Distance:     %.2f meters\n", payload->tag_distance);
    printf("GPS HDOP:     %.2f\n", payload->gps_hdop);
    
    printf("\n------------------------------------------------\n");
}

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;

// ================= Firebase =================
class FirebaseClient {
private:
    std::string url;

public:
    FirebaseClient(const std::string& firebase_url) : url(firebase_url) {}

    bool sendData(
        int id,
        double observer_lat,
        double observer_lon,
        double observer_alt,
        double target_lat,
        double target_lon,
        double target_alt,
        const std::string& time
    ) {
        CURL* curl = curl_easy_init();
        if (!curl) {
            std::cerr << "Curl init failed\n";
            return false;
        }

        std::stringstream json;
        json << "{"
             << "\"id\":" << id << ","
             << "\"observer_lat\":" << observer_lat << ","
             << "\"observer_lon\":" << observer_lon << ","
             << "\"observer_alt\":" << observer_alt << ","
             << "\"target_lat\":" << target_lat << ","
             << "\"target_lon\":" << target_lon << ","
             << "\"target_alt\":" << target_alt << ","
             << "\"time\":\"" << time << "\""
             << "}";

        std::string jsonStr = json.str();

        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonStr.c_str());

        std::cout << "Sending Firebase:\n" << jsonStr << std::endl;

        CURLcode res = curl_easy_perform(curl);

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        return res == CURLE_OK;
    }
};

// ================= WebSocket Server =================
class WSServer {
private:
    net::io_context ioc;
    tcp::acceptor acceptor_;
    std::vector<std::shared_ptr<websocket::stream<tcp::socket>>> clients;
    std::mutex mtx;

public:
    WSServer(int port)
        : ioc(1), acceptor_(ioc, {tcp::v4(), (unsigned short)port}) {}

    void start() {
        std::cout << "WebSocket Server running on port 9001\n";

        while (true) {
            tcp::socket socket{ioc};
            acceptor_.accept(socket);

            auto ws = std::make_shared<websocket::stream<tcp::socket>>(std::move(socket));
            ws->accept();

            {
                std::lock_guard<std::mutex> lock(mtx);
                clients.push_back(ws);
            }

            std::cout << "Client connected\n";

            // thread read msg
            std::thread([this, ws]() {
                try {
                    while (true) {
                        beast::flat_buffer buffer;
                        ws->read(buffer);

                        std::string msg = beast::buffers_to_string(buffer.data());
                        std::cout << "Received: " << msg << std::endl;
                    }
                } catch (...) {
                    std::cout << "Client disconnected\n";
                }
            }).detach();
        }
    }

    void broadcast(const std::string& message) {
        std::lock_guard<std::mutex> lock(mtx);

        for (auto& ws : clients) {
            try {
                ws->write(net::buffer(message));
            } catch (...) {
                // not use error client
            }
        }
    }
};

// ================= MAIN =================
int main() {
    printf("--- LoRa User-Space Test App ---\n");

    // ===== LoRa init =====
    int fd = lora_open("/dev/sx1278");
    if (fd < 0) {
        std::cout << "LoRa open failed\n";
        return -1;
    }

    // ===== READ CONFIG =====
    LoRa_config_t config;
    if (lora_get_config(fd, &config) == 0) {
        printf("    Frequency : %d Hz\n", config.frequency);
        printf("    SF        : %d\n", config.spreadingFactor);
        printf("    Bandwidth : %d\n", config.bandWidth);
        printf("    Sync Word : 0x%02X\n", config.syncWord);
    }

    // ===== SET CONFIG =====
    LoRa_config_t set_cfg{};
    set_cfg.frequency       = 434000000;
    set_cfg.spreadingFactor = 7;
    set_cfg.bandWidth       = 125000;
    set_cfg.crcRate         = 5;
    set_cfg.preamble        = 10;
    set_cfg.syncWord        = 0xF1;
    set_cfg.enableCrc       = 1;
    set_cfg.power           = 20;

    lora_set_config(fd, &set_cfg);

    // ===== VERIFY =====
    LoRa_config_t read_cfg;
    lora_get_config(fd, &read_cfg);
    printf("Sync Word (HW): 0x%02X\n", read_cfg.syncWord);

    // ===== WebSocket =====
    WSServer server(9001);
    std::thread ws_thread([&]() {
        server.start();
    });

    // ===== Firebase =====
    FirebaseClient firebase(
        "https://binnocular-default-rtdb.asia-southeast1.firebasedatabase.app/tracking_history.json"
    );

    int rssi, snr;

    printf("\n[+] Waiting for incoming data...\n");

    // ==========================================================
    // LOOP CHÍNH (LoRa receive + thêm WS/Firebase)
    // ==========================================================
    while (1) {
        memset(&receive_packet, 0, sizeof(receive_packet));

        int rx_len = lora_receive(fd, (uint8_t*)&receive_packet, sizeof(receive_packet));
        if (rx_len <= 0) continue;

        lora_get_pkt_rssi(fd, &rssi);
        lora_get_snr(fd, &snr);

        printf("\nReceived %d bytes | RSSI: %d | SNR: %d\n", rx_len, rssi, snr);

        // ===== HEX DUMP (giữ nguyên debug) =====
        uint8_t* byte_ptr = (uint8_t*)&receive_packet;
        for (int i = 0; i < rx_len; i++) {
            printf("%02X ", byte_ptr[i]);
            if ((i + 1) % 16 == 0) printf("\n");
        }
        printf("\n");

        // ===== DECRYPT =====
        memset(gcm_nonce, 0, GCM_NONCE_LEN);
        memcpy(gcm_nonce, &receive_packet.loc_data_header, sizeof(LOC_DATA_HEADER));

        decrypt_status = AES_GCM_decrypt(
            key,
            gcm_nonce,
            &(receive_packet.loc_data_header.seq_num),
            sizeof(receive_packet.loc_data_header.seq_num),
            &(receive_packet.loc_data_payload),
            sizeof(receive_packet.loc_data_payload),
            &plain_text
        );

        if (decrypt_status != 0) {
            printf("Decrypt failed\n");
            continue;
        }

        // ===== PRINT (giữ nguyên) =====
        print_location_data(receive_packet.loc_data_header.seq_num, &plain_text);

        // ===== JSON =====
        std::stringstream json;
        json << "{"
             << "\"id\":" << receive_packet.loc_data_header.seq_num  << ","
             << "\"observer_lat\":" << plain_text.loc_gps_lat << ","
             << "\"observer_lon\":" << plain_text.loc_gps_lon << ","
             << "\"observer_alt\":" << plain_text.loc_gps_alt << ","
             << "\"target_lat\":" << plain_text.tag_gps_lat << ","
             << "\"target_lon\":" << plain_text.tag_gps_lon << ","
             << "\"target_alt\":" << plain_text.tag_gps_alt << ","
             << "\"distance\":" << plain_text.tag_distance
             << "}";

        std::string jsonStr = json.str();

        // ===== WebSocket =====
        server.broadcast(jsonStr);

        // ===== Firebase =====
        firebase.sendData(
            receive_packet.loc_data_header.seq_num,
            plain_text.loc_gps_lat,
            plain_text.loc_gps_lon,
            plain_text.loc_gps_alt,
            plain_text.tag_gps_lat,
            plain_text.tag_gps_lon,
            plain_text.tag_gps_alt,
            "2026-04-27"
        );

        printf("Sent JSON\n");
    }

    ws_thread.join();
    lora_close(fd);
    return 0;
}