import tkinter as tk
from tkinter import filedialog, messagebox, scrolledtext, Frame, Label, Button, Radiobutton, StringVar, ttk
import subprocess
import os


class FileSyncUI(tk.Tk):
    def __init__(self):
        super().__init__()

        self.title("File Sync Tool")
        self.geometry("750x450")
        self.last_log_position = 0
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

        self.browse_button = Button(self.frame_top, text="Browse", command=self.browse_folder)
        self.browse_button.grid(row=0, column=2, padx=5, pady=5)

        # 选择模式
        self.mode_frame = Frame(self.frame_top, padx=5, pady=5)
        self.mode_frame.grid(row=1, column=0, columnspan=3, sticky=tk.W)

        self.mode_label = Label(self.mode_frame, text="Select Mode:")
        self.mode_label.pack(side=tk.LEFT, padx=5, pady=5)

        self.mode_var = StringVar(value="local")
        self.local_radio = Radiobutton(self.mode_frame, text="Local", variable=self.mode_var, value="local", command=self.toggle_ip_port)
        self.local_radio.pack(side=tk.LEFT, padx=5, pady=5)

        self.remote_radio = Radiobutton(self.mode_frame, text="Remote", variable=self.mode_var, value="remote", command=self.toggle_ip_port)
        self.remote_radio.pack(side=tk.LEFT, padx=5, pady=5)

        # 选择角色
        self.role_frame = Frame(self.frame_top, padx=5, pady=5)
        self.role_frame.grid(row=2, column=0, columnspan=3, sticky=tk.W)

        self.role_label = Label(self.role_frame, text="Select Role:")
        self.role_label.pack(side=tk.LEFT, padx=5, pady=5)

        self.role_var = StringVar(value="client")
        self.client_radio = Radiobutton(self.role_frame, text="Client", variable=self.role_var, value="client")
        self.client_radio.pack(side=tk.LEFT, padx=5, pady=5)

        self.server_radio = Radiobutton(self.role_frame, text="Server", variable=self.role_var, value="server")
        self.server_radio.pack(side=tk.LEFT, padx=5, pady=5)

        # IP地址输入框
        self.ip_label = Label(self.frame_top, text="IP Address:")
        self.ip_label.grid(row=3, column=0, padx=5, pady=5, sticky=tk.W)

        self.ip_entry = tk.Entry(self.frame_top, width=50)
        self.ip_entry.grid(row=3, column=1, padx=5, pady=5, sticky=tk.W)
        self.ip_entry.insert(0, "127.0.0.1")  # Default IP address

        # 端口输入框
        self.port_label = Label(self.frame_top, text="Port:")
        self.port_label.grid(row=4, column=0, padx=5, pady=5, sticky=tk.W)

        self.port_entry = tk.Entry(self.frame_top, width=50)
        self.port_entry.grid(row=4, column=1, padx=5, pady=5, sticky=tk.W)
        self.port_entry.insert(0, "12345")  # Default port

        # 启动按钮
        self.start_button = ttk.Button(self.frame_bottom, text="Start", command=self.start_sync)
        self.start_button.pack(pady=20)

        # 日志显示
        self.log_label = Label(self.frame_bottom, text="Log Output:")
        self.log_label.pack(side=tk.LEFT, padx=5, pady=5)

        self.log_output = scrolledtext.ScrolledText(self.frame_bottom, wrap=tk.WORD, width=60, height=10)
        self.log_output.pack(side=tk.LEFT, padx=5, pady=5, fill=tk.BOTH, expand=True)

        # 初始化IP和端口输入框的可用性
        self.toggle_ip_port()

    def toggle_ip_port(self):
        # 根据模式启用或禁用IP和端口输入框
        if self.mode_var.get() == "local":
            self.ip_entry.config(state=tk.DISABLED)
        else:
            self.ip_entry.config(state=tk.NORMAL)
            self.port_entry.config(state=tk.NORMAL)

    def browse_folder(self):
        folder_selected = filedialog.askdirectory()
        if folder_selected:
            self.folder_entry.delete(0, tk.END)
            self.folder_entry.insert(0, folder_selected)

    def start_sync(self):
        folder_to_sync = self.folder_entry.get()
        mode = self.mode_var.get()
        role = self.role_var.get()
        ip_address = self.ip_entry.get()
        port = self.port_entry.get()

        if not folder_to_sync:
            messagebox.showerror("Error", "Please select a folder to sync.")
            return

        if not os.path.exists(folder_to_sync):
            messagebox.showerror("Error", "Selected folder does not exist.")
            return

        try:
            self.log_file = os.path.join(folder_to_sync, "../build/synctool.log")
            self.log_output.insert(tk.END, f"Starting {role} in {mode} mode...\n")


            if mode == "local":
                command = ["../build/FileSyncTool", "local", role]
            else:  # remote mode
                command = ["../build/FileSyncTool", "remote", role, ip_address, port]

            process = subprocess.Popen(command, cwd=folder_to_sync, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

            stdout, stderr = process.communicate()

            self.log_output.insert(tk.END, stdout.decode() + "\n")
            if stderr:
                self.log_output.insert(tk.END, stderr.decode() + "\n")

            self.check_log_updates()
            self.log_output.insert(tk.END, f"{role.capitalize()} stopped.\n")
        except Exception as e:
            messagebox.showerror("Error", str(e))


    def check_log_updates(self):
        try:
            with open(self.log_file, 'r') as a:
                a.seek(self.last_log_position)
                
                new_lines = a.readlines()
                
                if new_lines:
                    for line in new_lines:
                        self.log_output.insert(tk.END, line)
                    self.log_output.yview(tk.END)  # Scroll to the end of the output

                self.last_log_position = a.tell()

        except FileNotFoundError:
            pass
        
        self.after(1000, self.check_log_updates)

if __name__ == "__main__":
    app = FileSyncUI()
    app.mainloop()
