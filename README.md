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

