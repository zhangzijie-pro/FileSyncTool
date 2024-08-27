# FileSyncTool

## 可监听本地或者在本地修改服务器中文件的增删改查操作进行记录与备份

## 使用C++编译完成命令行进行使用，或者使用Python tkinter打包的简易版GUI使用
### 目前支持Unix系统运行，对于windows后续更新exe内容


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
        ./FileSyncTool local <path: full path or relative path>


        remote: (服务器)
            ./FileSyncTool remote server <path: full path or relative path> <ip: default: 127.0.0.1> <port>
        local:
            ./FileSyncTool remote client <path: full path or relative path> <remote ip> <port>
    2.
        python_ui/gui.py运行
```
