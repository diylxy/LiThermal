# LiThermal 热成像相机
一种基于全志T113和海康威视4117的热成像相机设计
## 硬件
原理图、PCB工程位于assets文件夹。需要用嘉立创EDA专业版的文件菜单导入。
## 组装、配置教程整理中

## 相关编译工具和头文件整理中
## 手动编译固件

```bash
export STAGING_DIR=/home/lxy/Desktop/tina-sdk/out/t113-100ask/staging_dir/target        # 替换为实际staging目录位置
mkdir build && cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../toolchain.cmake
make -j32
```

编译独立C语言程序使用：  
```bash
~/Desktop/tina-sdk/prebuilt/gcc/linux-x86/arm/toolchain-sunxi-musl/toolchain/bin/arm-openwrt-linux-gcc test.c
```