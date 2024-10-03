# LiThermal 热成像相机
![封面](https://github.com/user-attachments/assets/4fc6f390-e887-44be-a5c8-28915dc0e16e)
## 相关链接
**[Bilibili 演示视频](https://oshwhub.com/lxu0423/lithermal-thermal-imaging-camera)**  
**[硬件设计](https://oshwhub.com/lxu0423/lithermal-thermal-imaging-camera)**   
**[教程中用到的相关工具](https://github.com/diylxy/LiThermal_Tools)**  
**[自行编译固件](https://github.com/diylxy/LiThermal_Compiler)**  

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
烧录完成后，重新上电，测量EA3036三路电源是否均无输出；按住`SW1`三秒，测量三路电源电压是否与原理图中网络名相同。  
如一切正常，焊接T113-S3，并检查是否有连锡。

## 软件
硬件完成后，解压`PhoenixCard.zip`并打开其中的`PhoenixCard.exe`，打开`tina_t113-100ask_uart1.img	`，选择“启动卡”，点击“烧卡”。  
烧录完成后，将TF卡插入PCB上的TF卡槽，连接屏幕，并重新通过USB上电。如果一切正常，屏幕将亮起并显示开机画面，否则检查你的焊接是否正常。  
到[LiThermal_Compiler](https://github.com/diylxy/LiThermal_Compiler/actions)这里找到最新的构建结果（Artifacts），下载并按照[“编译后的操作”](https://github.com/diylxy/LiThermal_Compiler?tab=readme-ov-file#%E7%BC%96%E8%AF%91%E5%90%8E%E7%9A%84%E6%93%8D%E4%BD%9C)将文件复制到MTP设备，随后通过adb shell sync刷新写入缓存，并通过adb shell reboot重启设备。  
如果一切正常，此处会有开机动画并卡在第一阶段，否则重复上一步。

## 摄像头设置
如果你的海康4117是UVC模式，可以[参考这个](https://www.bilibili.com/read/cv25703139)中关于88179的描述，改回NCM模式并设置IP地址，并使用Linux虚拟机连接，设置管理员密码。  

## 摄像头连接
通过USB连接摄像头到PCB（VCC <-> VCC, D- <-> D-, D+ <-> D+, GND <-> CGND）【图待补充】。再次上电，如果一切正常，屏幕会显示热成像画面。  

## 外壳组装
【待补充】  

## 编译教程
[LiThermal_Compiler](https://github.com/diylxy/LiThermal_Compiler)  
可以Fork这个仓库和本仓库，修改`LiThermal_Compiler/.gitmodules`中子模块路径为自己的仓库链接，再用vscode.dev修改代码，并利用Github Actions直接编译（理论可行，还未测试，建议用自己的系统编译）。  

## 参考资料
PCB设计参考：
[全志T113-S3智能家居86屏](https://oshwhub.com/fanhuacloud/t113-s3-86panel)
