# 游戏登录器项目

这是一个完整的游戏登录器系统，包含加密工具、登录器生成器和成品登录器。

## 项目结构

```
game_launcher/
├── common/              # 公共模块
│   ├── crc32/          # CRC32校验
│   ├── encrypt/        # 异或加解密
│   └── file_pack/      # 文件打包
├── encrypt_tool/       # 控制台加密工具
├── launcher_generator/ # 登录器生成器(MFC)
├── game_launcher/      # 成品登录器(MFC)
├── plugin_dll/         # 外置扩展DLL
└── CMakeLists.txt
```

## 编译说明

### 使用CMake
```bash
mkdir build
cd build
cmake .. -A Win32
cmake --build . --config Release
```

### 使用Visual Studio
直接打开CMakeLists.txt或使用CMake生成VS解决方案。

## 功能说明

### encrypt_tool
- 单文件/目录打包
- XOR加密
- 命令行参数和交互式两种模式

### game_launcher
- 服务器选择
- 补丁自动更新
- 客户端启动
- 外置DLL动态加载

### launcher_generator
- 可视化配置
- 一键生成登录器
- UI样式配置(框架已预留)

## 公共模块

所有模块共用的底层功能：
- CRC32: 数据完整性校验
- encrypt: XOR加解密
- file_pack: 文件打包/解包

## 许可证

本项目仅供学习交流使用。
