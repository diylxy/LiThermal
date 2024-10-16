# LiThermal 热成像相机
![封面](https://github.com/user-attachments/assets/4fc6f390-e887-44be-a5c8-28915dc0e16e)

# 相关链接
**[Bilibili 演示视频](https://oshwhub.com/lxu0423/lithermal-thermal-imaging-camera)**  
**[立创开源平台（开源硬件工程）](https://oshwhub.com/lxu0423/lithermal-thermal-imaging-camera)**   
**[教程中用到的相关工具](https://github.com/diylxy/LiThermal_Tools)**  
**[编译工具链](https://github.com/diylxy/LiThermal_Compiler)**  

# 复刻教程
## 复刻前你必须要了解的
复刻本作品需要你能够焊接0402元件和0.3毫米间距的QFP引脚，需要有一定Linux系统使用经验和计算机网络基础。**会使用搜索引擎解决常见问题**。  
### **复刻难度很大，如果你不符合上述情况，请勿轻易尝试。**  
本作品为**开源项目**，原理图、PCB和部分复刻教程位于**立创开源平台**。  
本作品用到的热成像模组：**海康威视TB4117**，如果是UVC模式还需要一个88179芯片的网卡来改，或者让卖家帮改NCM模式。  
视频发布时海康TB4117参考价格：290元。  

## 复刻前你需要准备
耗材：  
BOM表里的元器件  
海康威视TB4117模组  
导线  
焊锡  
助焊剂  
3D打印耗材（如果你准备自己打印）  

工具：  
电烙铁  
热风枪（或加热台）  
吸锡带  
万用表  
镊子  
热熔胶枪及配套胶棒
CH340 USB转串口（或其它USB转串口，安装好驱动）  
88179芯片的网卡（如果你的TB4117是UVC模式）  
3D打印机（如果你准备自己打印）  
其它用于焊接的工具  

## 硬件焊接
**除非你有十足把握电源电路没有虚焊或短路，否则务必最后焊接T113-S3。电源部分如有虚焊导致输出电压不对，会直接烧毁T113-S3芯片。**  
焊接完成后，用USB转串口连接PCB上STC8单片机旁边的测试点（R-TX，T-RX，GND-任意GND网络上的焊点），用USB线连接电脑和PCB的USB口。打开STC-ISP（可在“教程中用到的相关工具”里找到，下文用到的文件都在这里），用默认设置烧录固件`51单片机电源管理芯片/Objects/power.hex`。  
提示：51单片机烧录时需要重新上电，具体请百度：`STC单片机烧录教程`  
烧录完成后，重新上电，测量EA3036三路电源是否均无输出；按住`SW1`三秒，测量三路电源电压是否与原理图中网络名相同。  
如一切正常，焊接T113-S3，并检查是否有连锡。  

## 软件
硬件完成后，解压`PhoenixCard.zip`并打开其中的`PhoenixCard.exe`，打开`tina_t113-100ask_uart1.img`，选择“启动卡”，点击“烧卡”。  
烧录完成后，将TF卡插入PCB上的TF卡槽，连接屏幕，并重新通过USB上电。如果一切正常，屏幕将亮起并显示开机画面，否则检查你的焊接是否正常。  
到[LiThermal_Compiler](https://github.com/diylxy/LiThermal_Compiler/actions)这里找到最新的构建结果（Artifacts），下载并按照[“编译后的操作”](https://github.com/diylxy/LiThermal_Compiler?tab=readme-ov-file#%E7%BC%96%E8%AF%91%E5%90%8E%E7%9A%84%E6%93%8D%E4%BD%9C)将文件复制到MTP设备，随后通过adb shell sync刷新写入缓存，并通过adb shell reboot重启设备。  
如果一切正常，此处会有开机动画并卡在第一阶段，否则重复上一步。

## 摄像头设置
密码：`Ab123456`  
如果你的海康4117是UVC模式，可以[参考这个](https://www.bilibili.com/read/cv25703139)中关于88179的描述，改回NCM模式并设置IP地址，并使用Linux虚拟机连接，设置管理员密码。 
如需重置密码，需要到海康官网“密码重置”注册一个账号，并使用“设备网络搜索”通过导出特征文件的方式重置密码。  
下面是摄像头相关设置，建议按照图片顺序操作，其中IP地址可以在“设备网络搜索”修改，管理员密码改不了可以先设置一个强密码，然后到4117的web里改。  
视频编码：  
![QQ图片20241015215140](https://github.com/user-attachments/assets/1d4b7760-b291-4e4c-89d0-b77a35bd5b06)  
Web认证：  
![QQ图片20241015215129](https://github.com/user-attachments/assets/e9c40654-4035-4ac9-8116-c74eacb13f62)  
视频旋转（其中视频制式没有显示就不用改）：  
![QQ图片20241015215125](https://github.com/user-attachments/assets/8088bac8-444c-41eb-b19e-8d9cda9f1f3d)  
NCM模式：  
![QQ图片20241015215132](https://github.com/user-attachments/assets/e9527ab0-d80f-4045-90ac-9bd451e288bd)
IP地址（也可以在web里改）：  
![QQ图片20241015215136](https://github.com/user-attachments/assets/0a004d42-08c2-4440-b0da-c663becd8c56)

## 摄像头连接
通过USB连接摄像头到PCB（VCC <-> VCC, D- <-> D-, D+ <-> D+, GND <-> CGND）【图待补充】。再次上电，如果一切正常，屏幕会显示热成像画面。  

## 外壳组装
4117模组拆开后按照下图组装，注意安装方向  
第一步安装热成像镜头部分，从外往里（此时需要断开排线）：  
![DSC_8540](https://github.com/user-attachments/assets/0edbf785-3393-4aa7-a6d8-651abf95b338)  
第二步连接好主板排线，向右上方翻折，注意排线方向：  
![DSC_8541](https://github.com/user-attachments/assets/d806ed85-da94-4f88-8ed7-2d209df5b4eb)  

# 编译教程
[LiThermal_Compiler](https://github.com/diylxy/LiThermal_Compiler)  
可以Fork这个仓库和本仓库，修改`LiThermal_Compiler/.gitmodules`中子模块路径为自己的仓库链接，再用vscode.dev修改代码，并利用Github Actions直接编译（理论可行，还未测试，建议用自己的系统编译）。  

# 常见问题
以下已知问题来自网友，供大家参考。  
### 照片黑屏、图像显示样式没有反应、获取不到温度最大值最小值……
按照本文“摄像头设置-图-Web认证”设置，记得点保存。  
### 视频黑屏
按照本文“摄像头设置-图-视频编码”设置。  
### 视频花屏
按照本文“摄像头设置-图-视频旋转”设置。  
### 用设备网络搜索修改密码时，不让修改成Ab123456，说是弱口令，咋办？
先改一个强密码，然后进web里面修改。  
### 主菜单卡死
更新最新LiThermal固件，位于[LiThermal_Compiler](https://github.com/diylxy/LiThermal_Compiler)中的GitHub Actions   
### 电脑上面一直不出MTP设备，但adb shell 能正常用
MTP只支持Windows，如需在其它系统下复制，**参考**以下命令。  
```bash
adb push LiThermal /mnt/UDISK
adb push thermalcamera.sh /mnt/UDISK
adb push BSOD /mnt/UDISK
adb shell sync
adb shell reboot
```
### 最大值显示去不掉，或者和4117内置的最值显示重叠了
### 中心值不在中心，或者希望自定义中心值显示
### 图像太糊，希望调整滤波算法，或者调节亮度对比度
如果已经焊接，可以[配置端口转发](https://github.com/diylxy/LiThermal_Tools/blob/master/%E9%85%8D%E7%BD%AE%E7%AB%AF%E5%8F%A3%E8%BD%AC%E5%8F%91.md)后进入海康后台修改。  
### IP地址忘改了，又不想重新拆焊
参考[配置端口转发](https://github.com/diylxy/LiThermal_Tools/blob/master/%E9%85%8D%E7%BD%AE%E7%AB%AF%E5%8F%A3%E8%BD%AC%E5%8F%91.md)，使用ifconfig配置usb0网卡静态IP到海康4117网段，然后利用ping和curl找到4117的IP，并重定向它的80端口到电脑，在浏览器web后台修改IP。这需要一些Linux基础。  
### T113-S3识别不到4117，或者4117识别不到88179网卡
建议改用短线或屏蔽线。或尝试用干扰较小的电源，如充电宝、可调电源等为4117供电。  
*贡献者：`风雪里悠悠长白`*   

### 其它问题，比如白屏、黑屏、花屏、屏幕出现绿色条纹、开机没反应
检查焊接吧。  

## 参考资料
PCB设计参考：
[全志T113-S3智能家居86屏](https://oshwhub.com/fanhuacloud/t113-s3-86panel)
