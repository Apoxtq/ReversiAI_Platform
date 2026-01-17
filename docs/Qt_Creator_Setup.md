# Qt Creator 配置指南

## 环境要求

- **Qt Creator**: 18.0.1 (Community)
- **Qt版本**: 6.5.0 或 5.15.x (推荐6.5.0)
- **编译器**: MinGW 11.2.0 (64-bit) 或 MSVC 2019+
- **CMake**: 3.16+

## 安装步骤

### 1. 下载并安装Qt

访问 [Qt官网](https://www.qt.io/download) 下载Qt Creator:

1. 选择 "Qt Creator 18.0.1" Community版本
2. 在安装程序中选择以下组件:
   - ✅ Qt 6.5.0 > MinGW 11.2.0 (64-bit)
   - ✅ Qt 6.5.0 > MSVC 2019 (如果使用VS)
   - ✅ Qt Creator 18.0.1
   - ✅ CMake Tools (可选)

### 2. 配置Qt Creator

#### 方法A: 使用.pro文件 (推荐)

1. **打开项目**:
   - 启动Qt Creator
   - File > Open File or Project
   - 选择 `WBChessQT.pro` (位于项目根目录)

2. **配置构建套件**:
   - Projects > Build Settings
   - 选择合适的Kit (MinGW或MSVC)
   - 确保Qt版本正确 (6.5.0)

3. **构建项目**:
   - 点击左下角的锤子图标 (Build)
   - 等待编译完成

#### 方法B: 使用CMake

1. **配置CMake**:
   ```bash
   cd build_qt
   cmake .. -DBUILD_QT_GUI=ON -DCMAKE_PREFIX_PATH="C:/Qt/6.5.0/mingw_64"
   ```

2. **在Qt Creator中打开**:
   - File > Open File or Project
   - 选择 `CMakeLists.txt`
   - 选择构建目录为 `build_qt`

## 项目结构说明

```
ReversiAI_Platform/
├── WBChessQT.pro          # Qt Creator项目文件
├── main.cpp              # Qt应用程序入口
├── include/              # 头文件
│   ├── mainwindow.h      # 主窗口类
│   ├── MCTS.h           # MCTS AI算法
│   └── core/BitBoard.h   # 位棋盘核心
├── ui/                   # Qt界面实现
│   └── mainwindow.cpp
├── ai/                   # AI算法
│   └── MCTS.cpp
├── src/                  # 源文件和资源
│   ├── core/BitBoard.cpp
│   ├── mainwindow.ui     # Qt Designer界面
│   └── rsc.qrc          # 资源文件
└── build_qt/            # 构建目录
```

## 常见问题解决

### 问题1: "Qt5/Qt6 not found"

**解决方案**:
1. 检查Qt安装路径
2. 在.pro文件中添加:
   ```
   QT_DIR = C:/Qt/6.5.0/mingw_64
   ```

### 问题2: 编译器路径错误

**解决方案**:
- Tools > Options > Kits
- 检查Compiler路径是否正确
- 确保MinGW路径包含在PATH中

### 问题3: 无法找到头文件

**解决方案**:
- 检查.pro文件中的INCLUDEPATH
- 确保相对路径正确

### 问题4: 资源文件加载失败

**解决方案**:
- 检查rsc.qrc文件中的路径
- 确保图片文件在src/目录下

## 调试和运行

1. **设置断点**: 点击行号左侧
2. **启动调试**: F5 或 Debug按钮
3. **查看变量**: 在调试模式下悬停鼠标
4. **控制台输出**: 使用qDebug()函数

## 学习资源

- [Qt Creator 官方文档](https://doc.qt.io/qtcreator/)
- [Qt6 编程指南](https://doc.qt.io/qt-6/)
- [C++ GUI Programming with Qt](https://www.qt.io/product/qt6/qml-book)

## 项目特色功能

- 🎯 **BitBoard位运算**: 高效的黑白棋逻辑
- 🤖 **MCTS AI**: 蒙特卡洛树搜索算法
- 🎮 **Qt GUI**: 现代图形界面
- 📊 **性能监控**: 实时算法性能显示
- 🎓 **学术标准**: 完整的引用和文档

---

**提示**: 如果遇到问题，请检查Qt Creator的编译输出窗口，通常会有详细的错误信息帮助诊断问题。
