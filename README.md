# Simple Renderer

> 这是基于开源项目 tinyrenderer 开发的一款软光栅渲染器。在保留原项目特色(0图形API)的基础上，采用了更加modern的面向对象设计模式，并借鉴了OpenGL的上下文绑定、资源池化等设计理念。有计划在未来补上AO以及PBR渲染的内容～



## 部署

```shell
cmake -B build -DCMAKE_BUILD_TYPE=release 
cmake --build build -j
build/release/tinyrenderer <scene_name> [-r]|[-v]
```

其中，`<scene_name>`请参考configs目录下的scene.json，亦可自行对scene.json文件进行编辑以支持新模型/新场景。



## 效果图展示

### Bresenham Line Algorithm
<img width="600" height="600" alt="bresenham" src="https://github.com/user-attachments/assets/695f3519-ec51-4011-b89d-d92101efd836" />

### Triangle Rasterization
<img width="600" height="600" alt="triangle_rasterization" src="https://github.com/user-attachments/assets/a3da1ef8-3efb-4b88-98fa-d293b07b980f" />

### Flat Shading
<img width="600" height="600" alt="african_head_flat" src="https://github.com/user-attachments/assets/2e3aa1f6-bd4c-4265-a4d1-956e17ff1968" />

<img width="600" height="600" alt="diablo_flat" src="https://github.com/user-attachments/assets/ee97fc77-06f7-4ae1-bf2f-cc335f02d168" />

<img width="600" height="600" alt="Astarion_flat" src="https://github.com/user-attachments/assets/82e44bdc-01f2-431c-abcb-68e131f1c7b9" />

<img width="600" height="600" alt="Darius_flat" src="https://github.com/user-attachments/assets/d7039ced-865c-4f47-b5d8-6396157be357" />

### Bling-Phong Shading
<img width="600" height="600" alt="framebuffer_20260117_202035" src="https://github.com/user-attachments/assets/958d6ef3-c443-43c4-9d02-3a6dc647b63b" />

<img width="600" height="600" alt="framebuffer_20260113_202512" src="https://github.com/user-attachments/assets/dab175ec-a777-4c57-9f3b-6522c963cc5d" />

<img width="600" height="600" alt="framebuffer_20260114_202250" src="https://github.com/user-attachments/assets/db64b423-731a-4588-8cbd-15452ac181fc" />

<img width="600" height="600" alt="Darius" src="https://github.com/user-attachments/assets/aded15de-2348-416e-b3e4-a7020444858d" />


### Shadow Map (Soft Shadow with PCSS)
<img width="600" height="600" alt="framebuffer_20260121_160935" src="https://github.com/user-attachments/assets/e10efbb8-9788-43b9-afb3-531c5a7b68fb" />

https://github.com/user-attachments/assets/9c731873-e253-4fe3-bde2-6d5b0b02c1bf


### 双光源
<img width="600" height="600" alt="framebuffer_20260119_222740" src="https://github.com/user-attachments/assets/c727a5d8-f64a-405e-b897-74ec50749eef" />

https://github.com/user-attachments/assets/ee6ee050-19f5-4189-8086-86ba09bcb5be

## Credits
This project uses the following 3D models from Sketchfab:
* "Mario obj" (https://skfb.ly/6X8o8) by MatiasH290 is licensed under Creative Commons Attribution (http://creativecommons.org/licenses/by/4.0/).
* "Darius" (https://skfb.ly/C8SQ) by kraken is licensed under Creative Commons Attribution (http://creativecommons.org/licenses/by/4.0/).
* "Chibi Astarion - Baldurs Gate 3" () by Turbo Topology is licensed under Creative Commons Attribution (http://creativecommons.org/licenses/by/4.0/).
