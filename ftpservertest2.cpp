#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <unistd.h>      // POSIX functions
#include <sys/types.h>   // System data types
#include <sys/socket.h>  // Socket functions
#include <netinet/in.h>  // Internet address structures
#include <arpa/inet.h>   // Address conversion functions
#include <sstream>

using namespace std;

const int PORT = 8080;
const int BACKLOG = 5;

// تابع اصلی مدیریت خطای سوکت‌ها
void error(const string& msg) {
    perror(msg.c_str());
    exit(1);
}

// تابع کمکی برای ارسال پاسخ HTTP
void send_http_response(int socket, const string& content_type, const string& content) {
    stringstream ss;
    
    // هدر HTTP (Status Code 200 OK)
    ss << "HTTP/1.1 200 OK\r\n";
    ss << "Content-Type: " << content_type << "\r\n";
    ss << "Content-Length: " << content.length() << "\r\n";
    ss << "Connection: close\r\n";
    ss << "\r\n"; // خط خالی جداکننده هدر از بدنه
    ss << content;

    string response = ss.str();
    send(socket, response.c_str(), response.length(), 0);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;

    // 1. ساخت سوکت
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        error("خطا در ساخت سوکت");
    }
    
    // 2. تنظیم و Bind
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        error("خطا در bind کردن سوکت به پورت");
    }
    
    // 3. Listen
    if (listen(server_fd, BACKLOG) < 0) {
        error("خطا در listen کردن");
    }

    cout << "سرور HTTP در حال گوش دادن روی پورت " << PORT << " است." << endl;

    // 4. Accept (پذیرش اتصالات)
    while(true) {
        socklen_t addrlen = sizeof(address);
        
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
            perror("خطا در پذیرش اتصال");
            continue;
        }

        char buffer[1024] = {0};
        
        // 5. خواندن درخواست HTTP (بخش اول)
        read(new_socket, buffer, 1024);
        cout << "\nدرخواست دریافتی:\n" << buffer << endl;
        
        // 6. تعیین محتوای پاسخ
        string html_content = 
            "<html>"
            "<head><title>Moon1 Share</title></head>"
            "<body>"
            "<h1>Welcome to Moon1 File Share!</h1>"
            "<p>This is your simple C++ HTTP server.</p>"
            "</body>"
            "</html>";
        
        // 7. ارسال پاسخ HTTP
        send_http_response(new_socket, "text/html", html_content);

        // 8. بستن اتصال
        close(new_socket);
    }

    close(server_fd);
    return 0;
}
