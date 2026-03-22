#include <iostream>
#include <sstream>
#include <curl/curl.h>
#include <chrono>
#include <thread>
class FirebaseClient {
private:
    std::string url;

public:
    FirebaseClient(const std::string& firebase_url) : url(firebase_url) {}

    bool sendData(
        double observer_lat,
        double observer_lon,
        double observer_alt,
        double target_lat,
        double target_lon,
        double target_alt,
        const std::string& time
    ) {
        CURL* curl;
        CURLcode res;

        curl = curl_easy_init();
        if (!curl) {
            std::cerr << "Curl init failed\n";
            return false;
        }

        //create json
        std::stringstream json;
        json << "{"
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

        std::cout << "Sending:\n" << jsonStr << std::endl;

        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "Request failed: " << curl_easy_strerror(res) << std::endl;
        } else {
            std::cout << "Data sent successfully!\n";
        }

        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);

        return res == CURLE_OK;
    }
};

int main() {
    std::string firebase_url = "https://binnocular-default-rtdb.asia-southeast1.firebasedatabase.app/tracking_history.json";

    FirebaseClient firebase(firebase_url);

    double observer_lat = 11.584135;
    double observer_lon = 106.911602;
    double observer_alt = 502.9;

    double target_lat = 11.508544;
    double target_lon = 106.932658;
    double target_alt = 427;

    while (true) {
        firebase.sendData(
            observer_lat,
            observer_lon,
            observer_alt,
            target_lat,
            target_lon,
            target_alt,
            "2026-03-22T10:00:00"
        );

        // generate data
        target_lat += 0.0001;
        target_lon += 0.0001;

        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    return 0;
}
