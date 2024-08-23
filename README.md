# FileSyncTool

## 可监听本地或者在本地修改服务器中文件的增删改查操作进行记录与备份

## 使用C++编译完成命令行进行使用，或者使用Python tkinter打包的简易版GUI使用

使用步骤如下：
```bash
Docker构建:
    docker build -t filesync-tool .
    docker run -it filesync-tool

本地构建项目:
    cd build
    cmake ..
    make

使用方式有:
    1.
        local: (本地)
        ./FileSyncTool local


        remote: (服务器)
            ./FileSyncTool remote server <ip: default: 127.0.0.1> <port>
        local:
            ./FileSyncTool remote client <remote ip> <port>
    2.
        python_ui/gui.py运行
```
