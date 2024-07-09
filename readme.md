# ZeroLaunch-CPP

[国内项目地址](https://gitee.com/ghost-him/ZeroLaunch-CPP)，[国外项目地址](https://github.com/ghost-him/ZeroLaunch-CPP)

## 介绍

`ZeroLaunch` 是一个使用 `qt` 开发的运行在 `windows` 环境下的用于快速启动应用程序的软件。

软件特点：

* 内存占用极低
* 纯本地运行，不联网
* 开源，永久免费
* 优秀的匹配算法

该软件因个人需要而开发，因此会长期维护！

## 软件运行界面

软件的运行界面：

![image-20240707234443465](https://raw.githubusercontent.com/ghost-him/ZeroLaunch-CPP/main/assets/image-20240707234443465.png)

![image-20240707234510649](https://raw.githubusercontent.com/ghost-him/ZeroLaunch-CPP/main/assets/image-20240707234510649.png)

软件的设置界面：

![image-20240707234524868](https://raw.githubusercontent.com/ghost-him/ZeroLaunch-CPP/main/assets/image-20240707234524868.png)

## 快速使用

双击`ZeroLaunch.exe`即可运行程序。程序运行在系统的托盘中，右键图标可以打开二级菜单。

基本操作：

* `Alt + space`：打开搜索栏
* `Enter`：选择要启动的应用。
* 方向键上：选择上一个选项
* 方向键下：选择下一个选择
* `Ctrl + j`：等于方向键下
* `Ctrl + k`：等于方向键上
* `ESC`：当搜索栏中有文字时，则清屏；没有文字时，则隐藏搜索栏。

**如果要删除软件，请先去程序的设置中将开机自启关闭！**

## 安装方法

### 下载预编译包

去`release`下载最新的版本，然后解压，点击`ZeroLaunch.exe`即可运行。

### 编译使用

1. 克隆`github`项目。
2. 使用`qt creator`打开该项目。
3. 编译，编译后的程序存放在`${CMAKE_BINARY_DIR}/program`目录下。

## 软件特点

### 搜索算法

本软件采用的搜索算法为融合算法，可以实现 使用很少的字符匹配长字符串 + 模糊查找。支持直接搜索，拼音搜索，拼音首字母搜索，且支持模糊匹配。

如果要搜索 `网易云音乐` ，则可以直接搜索 `网易云音乐`，也可搜索 `wang yi yun yin yue`，也可搜索 `wyyyy`。由于搜索算法支持模糊匹配，所以输入`wangyiyunyinyue`时，也可以将该程序排在队首。

由于存在多音字的情况，而软件中只考虑了单音字，因此会存在错标拼音的情况（`乐`，可以读做 `yue` ，也可读做 `le`）。由于可以通过模糊匹配来弥补拼音的不足，所以这里暂时不考虑解决该情况。如果该情况产生的问题比较严重，可以通过issue来通知我改进程序。

详细的搜索算法介绍可点此链接查看：[github](https://github.com/ghost-him/ZeroLaunch-CPP/blob/main/doc/%E6%90%9C%E7%B4%A2%E7%AE%97%E6%B3%95%E4%BB%8B%E7%BB%8D.md)

### 自定义功能

#### 软件选项颜色

软件选项的颜色会随着 `Windows` 的主题的变化而变化。

#### 自定义搜索的路径与不搜索的路径

* 对于自定义搜索路径

自定义搜索路径的文件夹搜索深度为1：会搜索给定的文件夹下的所有的文件与其下一层的子文件夹下的所有的文件，如果该子文件夹中还有文件夹，则不会再搜索。

程序只会索引 `.exe` ， `.url` ，`.lnk` 三类文件。

例如有以下文件夹结构

```
C:\users\ghost\desktop\root folder
└─folder 1
    └─folder 2
```

在自定义文件中写入 `C:\users\ghost\desktop\root folder` 后，文件会搜索 `root folder` 与 `folder 1` 中的所有内容，而不搜索 `folder 2`的 内容。

* 对于不搜索路径

如果不想要搜索某一文件夹，则可以将该文件夹的路径写入。不搜索路径要求：搜索路径的前缀与不搜索路径完全匹配。

以上例为例：如果写入了 `C:\users\ghost\desktop`，则不会遍历该路径，而写入了 `C:\users\ghost\desktop\root folder\folder 1` 时，只会遍历 `root folder` 下的所有文件与除了 `folder 1` 之外的所有的子文件夹下的所有的文件。

#### 自定义搜索栏提示文字

默认为 `Hello, ZeroLaunch!`

#### 自定义搜索结果空白提示

默认为 `当前搜索无结果`

#### 自定义搜索结果显示个数

默认为 4，最大为10，推荐为4或3。匹配算法可以确保使得你的目标程序优先显示出来。

## 未来规划

| 规划               | 重要度         | 备注                                           |
| ------------------ | -------------- | ---------------------------------------------- |
| 重构代码           | 中             | 当前版本的代码的耦合度尚可                     |
| 优化搜索界面       | 低             | 暂时不考虑                                     |
| 优化搜索算法       | 低（长期优化） | 慢慢优化吧，目前还是很满意的                   |
| 添加文件索引       | 中等           | 考虑是否使用Everything的服务来完成对文件的搜索 |
| bug修复            | 重要           | 如果有bug，我会第一时间修复的                  |
| 将程序打包成安装包 | 中等           | 暂时没必要，等正式版时再考虑                   |

对于功能的添加，由于当前主要是为我个人使用，因此如果是我暂时用不到的功能，可能不会优先添加。只有当我也有了相应的需求时，才会考虑加入。不过，非常欢迎各位大佬来完善和改进这个软件。

## 安全性

[![Security Status](https://www.murphysec.com/platform3/v31/badge/1806375627855749120.svg)](https://www.murphysec.com/console/report/1806375613305708544/1806375627855749120)

## 使用的开源项目

* [qt6](https://www.qt.io/product/qt6)：程序的UI界面与文件读写，json解析等部分使用qt6编写。
* [chinese-xinhua](https://github.com/pwxcoo/chinese-xinhua)：程序使用该项目提供的字典（经过处理）来完成中文转拼音。

* [Bootstrap](https://icons.bootcss.com/)：提供了免费使用的高质量图标。

* [LaunchyQt](https://github.com/samsonwang/LaunchyQt)：提供了索引 UWP 应用程序的代码。



