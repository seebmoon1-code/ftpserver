import tkinter as tk
from tkinter import filedialog, ttk
import vlc
import sys
import os

# --- Main Application Class for the TS Player ---
class TSPlayerApp:
    def __init__(self, master):
        self.master = master
        master.title("TS Simple Player (Powered by VLC)")
        
        # Initialize VLC
        # The 'no-xlib' option is sometimes needed on Linux systems
        self.instance = vlc.Instance()
        self.player = self.instance.media_player_new()
        
        # Variables
        self.file_path = tk.StringVar(value="No file selected")
        self.current_media = None

        # Set minimum window size and make it resizable
        master.geometry("800x600")
        master.minsize(600, 450)

        # Build the user interface
        self.create_widgets()

        # Handle window closing to release VLC resources
        self.master.protocol("WM_DELETE_WINDOW", self.on_closing)

    def create_widgets(self):
        # Main Frame
        main_frame = ttk.Frame(self.master, padding="10")
        main_frame.pack(fill='both', expand=True)

        # --- File Path Display Section ---
        path_label = ttk.Label(main_frame, text="Current File:")
        path_label.pack(pady=5, padx=5, anchor='e')
        
        path_display = ttk.Label(main_frame, textvariable=self.file_path, foreground="#3498db")
        path_display.pack(fill='x', padx=5, pady=5)
        
        # --- Video Display Frame ---
        # This is the area where VLC will draw the video frames
        # Use a black background to clearly show the video area
        style = ttk.Style()
        style.configure('Black.TFrame', background='black')
        
        self.video_frame = ttk.Frame(main_frame, relief=tk.SUNKEN, borderwidth=1, style='Black.TFrame')
        self.video_frame.pack(fill='both', expand=True, pady=10)
        
        # --- Control Buttons Section ---
        button_frame = ttk.Frame(main_frame)
        button_frame.pack(pady=10)

        # Open File Button
        open_button = ttk.Button(button_frame, text="Open .ts File", command=self.open_file)
        open_button.pack(side='left', padx=10)

        # Play Button
        self.play_button = ttk.Button(button_frame, text="Play", command=self.play_media, state=tk.DISABLED)
        self.play_button.pack(side='left', padx=10)
        
        # Stop Button
        self.stop_button = ttk.Button(button_frame, text="Stop", command=self.stop_media, state=tk.DISABLED)
        self.stop_button.pack(side='left', padx=10)

    def open_file(self):
        # Open file dialog, filtering for .ts files
        filetypes = [('Transport Stream files', '*.ts'), ('All files', '*.*')]
        filename = filedialog.askopenfilename(defaultextension=".ts", filetypes=filetypes)
        
        if filename:
            # Stop any currently playing media
            self.stop_media()
            
            self.file_path.set(filename)
            self.current_media = self.instance.media_new(filename)
            self.player.set_media(self.current_media)
            
            # --- Embed the video into the tkinter frame ---
            if sys.platform.startswith('linux'): # Linux
                self.player.set_xwindow(self.video_frame.winfo_id())
            elif sys.platform == 'win32': # Windows
                self.player.set_hwnd(self.video_frame.winfo_id())
            # For macOS ('darwin'), embedding can be tricky; 
            # often requires advanced setup or letting VLC open an external window.

            self.play_button.config(state=tk.NORMAL, text="Play")
            self.stop_button.config(state=tk.DISABLED) # Will be enabled on play

    def play_media(self):
        if self.player.play() == -1:
            # -1 indicates an error (e.g., file not found or invalid format)
            self.file_path.set("ERROR: Could not play the file.")
            self.play_button.config(state=tk.NORMAL, text="Play")
            self.stop_button.config(state=tk.DISABLED)
        else:
            self.play_button.config(text="Playing (Pause/Resume)", state=tk.NORMAL)
            self.stop_button.config(state=tk.NORMAL)
            
            # Add a check for playing state to allow Pause/Resume functionality
            if self.player.is_playing():
                self.player.pause()
                self.play_button.config(text="Paused (Play/Resume)")
            else:
                self.player.play()
                self.play_button.config(text="Playing (Pause/Resume)")

    def stop_media(self):
        self.player.stop()
        self.play_button.config(text="Play", state=tk.NORMAL)
        self.stop_button.config(state=tk.DISABLED)

    def on_closing(self):
        # Release VLC resources before exiting the application
        self.player.release()
        self.instance.release()
        self.master.destroy()

# --- Application Entry Point ---
if __name__ == "__main__":
    try:
        root = tk.Tk()
        # Set a common language setting for display (optional, but good practice)
        root.option_add('*Font', 'Tahoma 10')
        
        app = TSPlayerApp(root)
        root.mainloop()
    except Exception as e:
        # In a real app, use tkinter.messagebox for errors
        print(f"An unexpected error occurred: {e}")
