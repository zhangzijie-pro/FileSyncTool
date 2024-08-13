import tkinter as tk
from tkinter import filedialog, messagebox, scrolledtext, Frame, Label, Button, Radiobutton, StringVar, ttk
import subprocess
import os


class FileSyncUI(tk.Tk):
    def __init__(self):
        super().__init__()

        self.title("File Sync Tool")
        self.geometry("600x400")

        self.create_widgets()

    def create_widgets(self):
        # 使用Frame来组织界面
        self.frame_top = Frame(self, padx=10, pady=10)
        self.frame_top.pack(fill=tk.X, expand=True)

        self.frame_bottom = Frame(self, padx=10, pady=10)
        self.frame_bottom.pack(fill=tk.X, expand=True)

        # 文件夹选择
        self.folder_label = Label(self.frame_top, text="Select Folder to Sync:")
        self.folder_label.grid(row=0, column=0, padx=5, pady=5, sticky=tk.W)

        self.folder_entry = tk.Entry(self.frame_top, width=50)
        self.folder_entry.grid(row=0, column=1, padx=5, pady=5, sticky=tk.W)
        self.folder_entry.insert(0, "../build/sync_folder")  # Default folder path

        self.browse_button = Button(self.frame_top, text="Browse", command=self.browse_folder)
        self.browse_button.grid(row=0, column=2, padx=5, pady=5)

        # 选择角色
        self.role_frame = Frame(self.frame_top, padx=5, pady=5)
        self.role_frame.grid(row=1, column=0, columnspan=3, sticky=tk.W)

        self.role_label = Label(self.role_frame, text="Select Role:")
        self.role_label.pack(side=tk.LEFT, padx=5, pady=5)

        self.role_var = StringVar(value="client")
        self.client_radio = Radiobutton(self.role_frame, text="Client", variable=self.role_var, value="client")
        self.client_radio.pack(side=tk.LEFT, padx=5, pady=5)

        self.server_radio = Radiobutton(self.role_frame, text="Server", variable=self.role_var, value="server")
        self.server_radio.pack(side=tk.LEFT, padx=5, pady=5)

        # 启动按钮
        self.start_button = ttk.Button(self.frame_bottom, text="Start", command=self.start_sync)
        self.start_button.pack(pady=20)

        # 日志显示
        self.log_label = Label(self.frame_bottom, text="Log Output:")
        self.log_label.pack(side=tk.LEFT, padx=5, pady=5)

        self.log_output = scrolledtext.ScrolledText(self.frame_bottom, wrap=tk.WORD, width=60, height=10)
        self.log_output.pack(side=tk.LEFT, padx=5, pady=5, fill=tk.BOTH, expand=True)

    def browse_folder(self):
        folder_selected = filedialog.askdirectory()
        if folder_selected:
            self.folder_entry.delete(0, tk.END)
            self.folder_entry.insert(0, folder_selected)

    def start_sync(self):
        folder_to_sync = self.folder_entry.get()
        role = self.role_var.get()

        if not folder_to_sync:
            messagebox.showerror("Error", "Please select a folder to sync.")
            return

        if not os.path.exists(folder_to_sync):
            messagebox.showerror("Error", "Selected folder does not exist.")
            return

        try:
            log_file = os.path.join(folder_to_sync, "../build/synctool.log")
            self.log_output.insert(tk.END, f"Starting {role}...\n")

            command = ["../build/FileSyncTool", role]
            process = subprocess.Popen(command, cwd=folder_to_sync, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

            stdout, stderr = process.communicate()

            self.log_output.insert(tk.END, stdout.decode() + "\n")
            if stderr:
                self.log_output.insert(tk.END, stderr.decode() + "\n")

            self.log_output.insert(tk.END, f"{role.capitalize()} stopped.\n")
        except Exception as e:
            messagebox.showerror("Error", str(e))

if __name__ == "__main__":
    app = FileSyncUI()
    app.mainloop()
