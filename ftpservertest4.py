from flask import Flask, request, render_template_string, redirect, url_for
from werkzeug.utils import secure_filename
import os

# تعیین مسیر ریشه برای ذخیره فایل‌ها
# توجه: این مسیر باید در سیستم شما معتبر باشد.
UPLOAD_FOLDER = '/home/sara/Desktop/moon1/uploads'
ALLOWED_EXTENSIONS = {'txt', 'pdf', 'png', 'jpg', 'jpeg', 'gif', 'mp4', 'mkv', 'zip', 'rar'} # انواع مجاز فایل

app = Flask(__name__)
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER

# اطمینان از وجود پوشه آپلود
if not os.path.exists(UPLOAD_FOLDER):
    os.makedirs(UPLOAD_FOLDER)

def allowed_file(filename):
    """بررسی می‌کند که پسوند فایل مجاز است یا خیر."""
    return '.' in filename and \
           filename.rsplit('.', 1)[1].lower() in ALLOWED_EXTENSIONS

# مسیر اصلی: نمایش لیست فایل‌ها و فرم آپلود
@app.route('/', methods=['GET', 'POST'])
def upload_file():
    message = None
    
    if request.method == 'POST':
        # بررسی وجود فایل در درخواست
        if 'file' not in request.files:
            message = 'فایلی انتخاب نشده است!'
            return redirect(request.url)

        file = request.files['file']
        
        # اگر کاربر فایلی انتخاب نکرد
        if file.filename == '':
            message = 'لطفاً یک فایل را انتخاب کنید.'
        
        # اگر فایل وجود دارد و پسوند آن مجاز است
        elif file and allowed_file(file.filename):
            # برای جلوگیری از مشکلات امنیتی، نام فایل را ایمن‌سازی می‌کنیم
            filename = secure_filename(file.filename)
            
            # ذخیره فایل در مسیر مشخص شده
            file.save(os.path.join(app.config['UPLOAD_FOLDER'], filename))
            message = f'✅ فایل "{filename}" با موفقیت آپلود شد!'
        
        else:
            message = '🚫 نوع فایل مجاز نیست.'

    # لیست کردن فایل‌ها
    files = os.listdir(app.config['UPLOAD_FOLDER'])
    
    # HTML پاسخ
    html = """
    <!doctype html>
    <title>آپلود و دانلود فایل (پایتون)</title>
    <style>
        body { font-family: Tahoma; direction: rtl; text-align: right; }
        h1 { color: #333; }
        .message { color: green; font-weight: bold; }
        .error { color: red; font-weight: bold; }
        ul { list-style-type: none; padding: 0; }
        a { text-decoration: none; color: #007bff; }
    </style>
    <body>
        <h1>سیستم اشتراک گذاری فایل Moon1</h1>
        
        {% if message %}
            <p class="message">{{ message }}</p>
        {% endif %}
        
        <h2>آپلود فایل جدید:</h2>
        <form method=post enctype=multipart/form-data>
            <input type=file name=file required>
            <input type=submit value=آپلود>
        </form>
        
        <hr>
        
        <h2>فایل‌های موجود:</h2>
        <ul>
        {% for filename in files %}
            <li>
                <a href="{{ url_for('download_file', name=filename) }}">{{ filename }}</a> 
            </li>
        {% endfor %}
        </ul>
    </body>
    </html>
    """
    
    return render_template_string(html, files=files, message=message)

# مسیر دانلود فایل‌ها
@app.route('/downloads/<name>')
def download_file(name):
    # برای جلوگیری از دسترسی به فایل‌های خارج از پوشه (امنیت)
    try:
        return send_from_directory(app.config["UPLOAD_FOLDER"], name, as_attachment=True)
    except FileNotFoundError:
        return "فایل مورد نظر پیدا نشد.", 404

from flask import send_from_directory
    
if __name__ == '__main__':
    # تنظیمات سرور برای گوش دادن به تمام IPها در پورت 8080
    app.run(host='0.0.0.0', port=8080, debug=True)
