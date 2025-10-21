#include <iostream>
#include <string>
#include <cstdlib>
#include <cstring>
#include <unistd.h>      // توابع POSIX (مثل close)
#include <sys/types.h>   // انواع داده سیستمی
#include <sys/socket.h>  // توابع سوکت
#include <netinet/in.h>  // ساختارهای آدرس اینترنتی
#include <arpa/inet.h>   // توابع تبدیل آدرس (مثل inet_ntoa)

using namespace std;

// پورت سرور (پورت استاندارد FTP 21 است، اما اینجا برای تست از 8080 استفاده می‌کنیم تا تداخلی نداشته باشد)
const int PORT = 8080;
// حداکثر تعداد اتصالات در صف انتظار
const int BACKLOG = 5;

// تابع اصلی مدیریت خطای سوکت‌ها
void error(const string& msg) {
    perror(msg.c_str());
    exit(1);
}

int main() {
    // 1. تعریف متغیرهای سوکت
    int server_fd;            // File Descriptor سوکت سرور
    int new_socket;           // File Descriptor سوکت اتصال جدید
    struct sockaddr_in address; // ساختار نگهداری آدرس‌ها

    // 2. ساخت سوکت (IPv4, TCP)
    // AF_INET: خانواده آدرس IPv4
    // SOCK_STREAM: سوکت جریانی (TCP)
    // 0: پروتکل پیش فرض (IP)
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        error("خطا در ساخت سوکت");
    }
    
    // 3. تنظیم آدرس سرور
    // پاک کردن ساختار حافظه
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;           // IPv4
    address.sin_addr.s_addr = INADDR_ANY;   // گوش دادن به تمام اینترفیس‌ها
    address.sin_port = htons(PORT);         // تبدیل پورت به ترتیب بایت‌های شبکه
    
    // 4. Bind (بستن سوکت به آدرس و پورت)
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        error("خطا در bind کردن سوکت به پورت");
    }
    
    // 5. Listen (آماده گوش دادن برای اتصالات ورودی)
    if (listen(server_fd, BACKLOG) < 0) {
        error("خطا در listen کردن");
    }

    cout << "سرور TCP در حال گوش دادن روی پورت " << PORT << " است..." << endl;
    cout << "برای متوقف کردن، کلیدهای Ctrl+C را فشار دهید." << endl;

    // 6. Accept (پذیرش اتصالات به صورت بی نهایت)
    while(true) {
        socklen_t addrlen = sizeof(address);
        
        // منتظر می‌ماند تا یک کلاینت متصل شود
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen)) < 0) {
            // اگر خطای پذیرش (accept) رخ داد، ادامه می‌دهد و منتظر اتصال بعدی می‌ماند
            perror("خطا در پذیرش اتصال");
            continue;
        }

        // نمایش اطلاعات کلاینت متصل شده
        cout << "\nاتصال جدید از: " 
             << inet_ntoa(address.sin_addr) 
             << ":" << ntohs(address.sin_port) 
             << endl;

        // شبیه‌سازی خوش آمد گویی FTP (کد 220)
        string welcome_msg = "220 Welcome to the Simple C++ Server.\r\n";
        
        // ارسال پیام به کلاینت
        send(new_socket, welcome_msg.c_str(), welcome_msg.length(), 0);
        
        cout << "پیام خوش آمد گویی ارسال شد." << endl;

        // بستن سوکت اتصال کلاینت
        close(new_socket);
        cout << "اتصال کلاینت بسته شد." << endl;
    }

    // 7. بستن سوکت سرور (عملا با Ctrl+C انجام می‌شود)
    close(server_fd);
    return 0;
}
