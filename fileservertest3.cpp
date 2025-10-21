#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <filesystem>  // برای مدیریت فایل‌ها و دایرکتوری‌ها

// برای استفاده از std::filesystem نیاز به C++17 یا بالاتر است
namespace fs = std::filesystem;
using namespace std;

const int PORT = 8080;
const int BACKLOG = 5;
// مسیر پوشه moon1 را بر اساس مسیر دسکتاپ لینوکس خود تنظیم کنید
const fs::path ROOT_DIR = "/home/sara/Desktop/moon1"; 
const int BUFFER_SIZE = 4096;

// تابع مدیریت خطا
void error(const string& msg) {
    perror(msg.c_str());
    // در اینجا نیازی به exit نیست، فقط گزارش خطا می‌دهیم
}

// تابع URL Decode برای مدیریت فاصله‌ها در نام فایل
string url_decode(const string& url_encoded) {
    string decoded;
    char hex_val[3];
    hex_val[2] = '\0';
    
    for (size_t i = 0; i < url_encoded.length(); ++i) {
        if (url_encoded[i] == '%') {
            if (i + 2 < url_encoded.length()) {
                hex_val[0] = url_encoded[i+1];
                hex_val[1] = url_encoded[i+2];
                // تبدیل هگزا دسیمال به کاراکتر
                decoded += (char)strtol(hex_val, nullptr, 16);
                i += 2;
            } else {
                // اگر % ناقص باشد، آن را نادیده می‌گیریم
                decoded += url_encoded[i];
            }
        } else if (url_encoded[i] == '+') {
            // تبدیل + به فاصله (طبق استاندارد)
            decoded += ' ';
        } else {
            decoded += url_encoded[i];
        }
    }
    return decoded;
}

// تابع ارسال پاسخ HTTP
void send_http_response(int socket, const string& status, const string& content_type, const string& content, bool is_file = false) {
    stringstream ss;
    
    // Status Line و هدرها
    ss << "HTTP/1.1 " << status << "\r\n";
    ss << "Content-Type: " << content_type << "\r\n";
    
    if (!is_file) {
        ss << "Content-Length: " << content.length() << "\r\n";
    }
    
    ss << "Connection: close\r\n";
    ss << "\r\n"; // خط خالی جداکننده هدر از بدنه

    string header = ss.str();
    send(socket, header.c_str(), header.length(), 0);

    if (!is_file) {
        send(socket, content.c_str(), content.length(), 0);
    }
}

// تابع اصلی پردازش یک درخواست HTTP
void handle_request(int new_socket) {
    char buffer[BUFFER_SIZE] = {0};
    
    // خواندن درخواست HTTP (فقط خط اول مهم است)
    if (read(new_socket, buffer, BUFFER_SIZE - 1) <= 0) {
        close(new_socket);
        return;
    }

    stringstream request_ss(buffer);
    string method, path_encoded, protocol;
    request_ss >> method >> path_encoded >> protocol;
    
    // فقط متدهای GET پشتیبانی می‌شوند
    if (method != "GET") {
        send_http_response(new_socket, "405 Method Not Allowed", "text/html", "<h1>405 - Method Not Allowed</h1>");
        close(new_socket);
        return;
    }
    
    // دیکد کردن مسیر (مثلاً تبدیل %20 به فاصله)
    string path = url_decode(path_encoded);

    // ساخت مسیر کامل فایل (با جلوگیری از خروج از دایرکتوری moon1)
    fs::path requested_path = ROOT_DIR / path.substr(1); // حذف / اول

    // ----------------------------------------------------
    // منطق اصلی: دانلود فایل یا لیست کردن دایرکتوری
    // ----------------------------------------------------

    if (path == "/" || fs::is_directory(requested_path)) {
        // اگر مسیر اصلی (/) یا یک دایرکتوری درخواست شده
        stringstream html_body;
        html_body << "<html><head><title>Moon1 Shared Files</title><style>body{font-family:Tahoma; direction:rtl;} a{text-decoration:none;}</style></head><body>";
        html_body << "<h1>پوشه اشتراکی moon1</h1><ul>";

        try {
            // لیست کردن محتویات پوشه
            for (const auto& entry : fs::directory_iterator(ROOT_DIR)) {
                string filename = entry.path().filename().string();
                string display_name = filename;
                
                // برای فایل‌هایی که نام فارسی یا کاراکترهای خاص دارند، این بخش پیچیده‌تر است
                if (fs::is_directory(entry.path())) {
                    display_name = "[پوشه] " + display_name;
                }
                
                // ایجاد لینک برای دانلود
                html_body << "<li><a href=\"/" << filename << "\">" << display_name << "</a></li>";
            }
        } catch (const fs::filesystem_error& e) {
             html_body << "<li>خطا در خواندن دایرکتوری: " << e.what() << "</li>";
        }
        
        html_body << "</ul></body></html>";
        send_http_response(new_socket, "200 OK", "text/html; charset=utf-8", html_body.str());
        
    } else if (fs::exists(requested_path) && fs::is_regular_file(requested_path)) {
        // اگر یک فایل درخواست شده (برای دانلود)
        
        ifstream file(requested_path, ios::binary | ios::ate);
        if (file.is_open()) {
            size_t file_size = file.tellg();
            file.seekg(0, ios::beg);

            // ارسال هدر پاسخ برای دانلود
            stringstream header_ss;
            header_ss << "HTTP/1.1 200 OK\r\n";
            header_ss << "Content-Type: application/octet-stream\r\n"; // نوع محتوای باینری
            header_ss << "Content-Disposition: attachment; filename=\"" << requested_path.filename().string() << "\"\r\n";
            header_ss << "Content-Length: " << file_size << "\r\n";
            header_ss << "Connection: close\r\n";
            header_ss << "\r\n"; 
            
            string header = header_ss.str();
            send(new_socket, header.c_str(), header.length(), 0);

            // ارسال بدنه فایل
            char file_buffer[BUFFER_SIZE];
            while (file.read(file_buffer, BUFFER_SIZE) || file.gcount() > 0) {
                send(new_socket, file_buffer, file.gcount(), 0);
            }
            file.close();
        } else {
            // خطا در باز کردن فایل
            send_http_response(new_socket, "500 Internal Server Error", "text/html", "<h1>500 - Could not open file</h1>");
        }
        
    } else {
        // فایل یا پوشه پیدا نشد
        send_http_response(new_socket, "404 Not Found", "text/html", "<h1>404 - File or Directory Not Found</h1>");
    }

    close(new_socket);
}


// **********************************
// تابع main
// **********************************
int main() {
    // ایجاد پوشه‌ی moon1 در صورت عدم وجود
    if (!fs::exists(ROOT_DIR)) {
        try {
            fs::create_directories(ROOT_DIR);
            cout << "پوشه moon1 ایجاد شد.\n";
        } catch (...) {
            cerr << "خطا: امکان ایجاد پوشه moon1 وجود ندارد.\n";
            return 1;
        }
    }
    
    // ----------------------------------------------------
    // تنظیمات سوکت سرور
    // ----------------------------------------------------
    int server_fd;
    struct sockaddr_in address;

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        error("خطا در ساخت سوکت");
    }
    
    // **حل مشکل "Address already in use"**
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        error("خطا در setsockopt");
    }
    
    // تنظیم و Bind
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        error("خطا در bind کردن سوکت به پورت");
    }
    
    // Listen
    if (listen(server_fd, BACKLOG) < 0) {
        error("خطا در listen کردن");
    }

    cout << "سرور HTTP در حال گوش دادن روی پورت " << PORT << " است.\n";
    cout << "مسیر Root: " << ROOT_DIR << "\n";
    cout << "برای متوقف کردن، کلیدهای Ctrl+C را فشار دهید.\n";

    // ----------------------------------------------------
    // حلقه پذیرش اتصالات
    // ----------------------------------------------------
    while(true) {
        int new_socket;
        socklen_t addrlen = sizeof(address);
        
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
            perror("خطا در پذیرش اتصال");
            continue;
        }
        
        cout << "\nاتصال جدید از: " 
             << inet_ntoa(address.sin_addr) 
             << ":" << ntohs(address.sin_port) 
             << endl;

        // مدیریت درخواست در یک تابع جداگانه
        handle_request(new_socket);
    }

    close(server_fd);
    return 0;
}
