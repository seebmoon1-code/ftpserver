import cv2
import sys

def play_ts_file(file_path):
    # 1. مطمئن شوید که مسیر فایل وارد شده است
    if not file_path:
        print("لطفاً مسیر فایل .ts را به عنوان آرگومان وارد کنید.")
        return

    # 2. شیء VideoCapture را برای باز کردن فایل ایجاد کنید
    # OpenCV از FFmpeg برای دیکد کردن فرمت .ts استفاده می‌کند
    cap = cv2.VideoCapture(file_path)

    # 3. بررسی کنید که آیا فایل با موفقیت باز شده است یا خیر
    if not cap.isOpened():
        print(f"خطا: نمی‌توان فایل '{file_path}' را باز کرد. فرمت پشتیبانی نمی‌شود یا فایل وجود ندارد.")
        return

    # 4. یک پنجره برای نمایش ویدیو ایجاد کنید
    window_name = "TS Player - Press 'q' to exit"
    cv2.namedWindow(window_name, cv2.WINDOW_AUTOSIZE)

    print(f"شروع پخش فایل: {file_path}")

    while True:
        # فریم به فریم ویدیو را بخوانید
        ret, frame = cap.read()

        # اگر ret برابر False باشد، یعنی به انتهای ویدیو رسیده‌ایم
        if not ret:
            print("پخش به پایان رسید.")
            break

        # فریم را در پنجره نمایش دهید
        cv2.imshow(window_name, frame)

        # 5. کنترل خروج: اگر کلید 'q' یا ESC (27) فشرده شود، حلقه را متوقف کنید
        # '1' نشان‌دهنده یک میلی‌ثانیه تأخیر است که سرعت پخش را کنترل می‌کند
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    # 6. پاکسازی منابع
    cap.release()
    cv2.destroyAllWindows()

if __name__ == "__main__":
    # فایل .ts را از آرگومان‌های خط فرمان می‌خوانیم
    if len(sys.argv) > 1:
        play_ts_file(sys.argv[1])
    else:
        # اگر کاربر هیچ فایلی نداد، می‌توانیم یک مسیر پیش‌فرض بدهیم
        print("لطفاً مسیر فایل را هنگام اجرای برنامه مشخص کنید. مثال: python player.py my_video.ts")
