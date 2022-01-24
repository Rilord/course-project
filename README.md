# Курсовая работа по компьютерной графике

В этом репозитории содержится исходный код курсовой работы по компьютерной графике. Тема курсовой работы: "Программа для генерации реалистичного изображения на основании алгоритма трассировки пути".


Трассировщие был разработан на основе кода из известного всем третьего издания [Physically Based Rendering: From Theory to Implementation, by Matt Pharr, Wenzel Jakob, and Greg Humphreys](http://pbrt.org/). 

Графический интерфейс был полностью написан при помощью библиотеки ImGui и Vulkan API.  

На данный момент поддерживаются две платформы: Linux (Xorg)  и MacOs. Сборка содержит косольный вариант приложения (pbrt), и с поддержкой графического интерфейса (pbrt_vk).

## Примеры работы

Пример генерации изображения из сцены [teapot-area-light.pbrt](https://github.com/Rilord/course-project/blob/master/ray-tracer/scenes/teapot-area-light.pbrt)


![https://github.com/Rilord/course-project/blob/master/ray-tracer/scenes/teapot-area-light.pbrt](http://cdn.kodors.co/images/courseproj/1.png)

Пример генерации изображения из сцены [killeroo-simple.pbrt](https://github.com/Rilord/course-project/blob/master/ray-tracer/scenes/killeroo-simple.pbrt)

![https://github.com/Rilord/course-project/blob/master/ray-tracer/scenes/killeroo-simple.pbrt](http://cdn.kodors.co/images/courseproj/2.png)


## Сборка проекта

Данный билд не использует каких-либо подмодулей git, и поэтому достаточно лишь клонировать данный репозиторий, т.е.
```
git clone https://github.com/Rilord/course-project.git
```

Далее при помощи cmake производится полная сборка проекта, результатом которой является несколько исполняемых файлов (pbrt, pbrt_vk, и.т.п). Сборка может занять более 10 минут. Пример сборки приложения с GUI при помощи cmake
```
mkdir pbrt_vk_build
cd pbrt_vk_build
cmake ../
cmake --build . --target pbrt_vk --use-stderr -- -j 8
```
