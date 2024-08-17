## 编译

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