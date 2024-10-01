# LiThermal 热成像相机
![封面](https://github.com/user-attachments/assets/4fc6f390-e887-44be-a5c8-28915dc0e16e)
## 硬件
原理图、PCB工程位于assets文件夹。需要用嘉立创EDA专业版的文件菜单导入。  
**[演示视频链接点我](https://oshwhub.com/lxu0423/lithermal-thermal-imaging-camera)**  
**[立创开源平台链接点我](https://oshwhub.com/lxu0423/lithermal-thermal-imaging-camera)**   
复刻教程在开源平台，元器件照着BOM表买就没问题。这个作品涉及到较多0402元件和0.3毫米间距的引脚焊接，难度稍大，不建议纯小白复刻。希望复刻的小伙伴们尽快找一家便宜的海康4117购买吧，免得再涨价（视频发布时参考价格：290元，相对2周前已涨价50元。注意：如果是UVC模式还需要一个88179芯片的网卡来改，或者让卖家帮改NCM模式）  
## 组装、配置教程未完善，仍在整理中
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
