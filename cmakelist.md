### 1. 设置 CMake 最低版本和项目名称

```
cmake复制代码cmake_minimum_required(VERSION 3.0)
project(ai_backgammon)
set(CMAKE_C_STANDARD 99)
```

- **`cmake_minimum_required(VERSION 3.0)`**: 指定项目所需的最低 CMake 版本为 3.0。
- **`project(ai_backgammon)`**: 定义项目名称为 `ai_backgammon`。
- **`set(CMAKE_C_STANDARD 99)`**: 设置项目使用的 C 语言标准为 C99。

### 2. 添加 Raylib 库

```
cmake复制代码# Adding Raylib
# License: zlib https://github.com/raysan5/raylib/blob/master/LICENSE
include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE) # don't build the supplied example games

FetchContent_Declare(
    raylib
    GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
    GIT_TAG "master"
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(raylib)
```

- **注释部分**: 说明 Raylib 的许可证为 zlib，并提供了 Raylib 的 GitHub 地址。
- **`include(FetchContent)`**: 引入 CMake 的 `FetchContent` 模块，用于下载和包含外部项目。
- **`set(FETCHCONTENT_QUIET FALSE)`**: 设置 `FetchContent` 的输出不静默，便于调试。
- **`set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)`** 和 **`set(BUILD_GAMES OFF CACHE BOOL "" FORCE)`**: 禁用 Raylib 自带的示例和游戏的构建，以减少不必要的编译。
- **`FetchContent_Declare`**: 声明要下载的 Raylib 库，指定其 Git 仓库地址和分支（master）。
- **`FetchContent_MakeAvailable(raylib)`**: 下载并将 Raylib 库包含到项目中。

### 3. 添加 Raygui 库

```
cmake复制代码# Adding Raygui
# License: zlib https://github.com/raysan5/raygui/blob/master/LICENSE
FetchContent_Declare(
    raygui
    GIT_REPOSITORY "https://github.com/raysan5/raygui.git"
    GIT_TAG "master"
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(raygui)
```

- **注释部分**: 说明 Raygui 的许可证为 zlib，并提供了 Raygui 的 GitHub 地址。
- **`FetchContent_Declare`**: 声明要下载的 Raygui 库，指定其 Git 仓库地址和分支（master）。
- **`FetchContent_MakeAvailable(raygui)`**: 下载并将 Raygui 库包含到项目中。

### 4. 声明可执行文件

```
cmake复制代码# Declaring our executable
add_executable(${PROJECT_NAME})
```

- **`add_executable(${PROJECT_NAME})`**: 创建一个名为 `ai_backgammon` 的可执行文件目标。

### 5. 设置 Raygui 的包含目录

```
cmake复制代码# Specify the raygui include directory
target_include_directories(${PROJECT_NAME} PRIVATE ${raygui_SOURCE_DIR}/src)
```

- **`target_include_directories`**: 为 `ai_backgammon` 目标添加 Raygui 的头文件目录，确保在编译时可以找到 Raygui 的头文件。

### 6. 添加源文件

```
cmake复制代码# Adding our source files
file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_LIST_DIR}/sources/*.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/sources/Object/*.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/sources/ChessCore/*.cpp"
        "${CMAKE_CURRENT_LIST_DIR}/sources/General/*.cpp") # Define PROJECT_SOURCES as a list of all source files
set(PROJECT_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/sources/") # Define PROJECT_INCLUDE to be the path to the include directory of the project
```

- **`file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS ...)`**: 递归搜索 `sources` 目录及其子目录下的所有 `.cpp` 文件，并将它们加入到 `PROJECT_SOURCES` 变量中。这些文件将被编译到可执行文件中。
- **`set(PROJECT_INCLUDE "${CMAKE_CURRENT_LIST_DIR}/sources/")`**: 定义 `PROJECT_INCLUDE` 变量，指向项目的头文件目录 `sources`。

### 7. 设置编译选项

```
cmake复制代码# Adding our compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
```

- **`set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")`**: 为 C++ 编译器添加 `-Wall` 标志，启用所有警告信息，有助于发现潜在的问题。

### 8. 关联源文件和包含目录，并链接 Raylib 库

```
cmake复制代码target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE})
target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
```

- **`target_sources(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCES})`**: 将之前搜集到的源文件添加到 `ai_backgammon` 目标中。
- **`target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDE})`**: 添加项目的头文件目录到 `ai_backgammon` 目标中，以便编译时找到项目的头文件。
- **`target_link_libraries(${PROJECT_NAME} PRIVATE raylib)`**: 将 Raylib 库链接到 `ai_backgammon` 目标中，使其能够使用 Raylib 提供的功能。

### 9. 设置 `ASSETS_PATH` 宏

```
cmake复制代码# Setting ASSETS_PATH
target_compile_definitions(${PROJECT_NAME} PUBLIC ASSETS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/assets/") # Set the asset path macro to the absolute path on the dev machine
#target_compile_definitions(${PROJECT_NAME} PUBLIC ASSETS_PATH="./assets") # Set the asset path macro in release mode to a relative path that assumes the assets folder is in the same directory as the game executable
```

- `target_compile_definitions`

  : 为 

  ```
  ai_backgammon
  ```

   目标定义编译时宏 

  ```
  ASSETS_PATH
  ```

  。

  - 第一行定义 `ASSETS_PATH` 为项目源代码目录下的 `assets` 文件夹的绝对路径，适用于开发阶段。
  - 第二行被注释掉的代码示例展示了在发布模式下如何将 `ASSETS_PATH` 设置为相对路径 `./assets`，假设资产文件夹与可执行文件位于同一目录。

### 总结

整个 `CMakeLists.txt` 文件的主要功能是：

1. **项目配置**: 设置项目名称、C 语言标准及最低 CMake 版本。
2. **依赖管理**: 使用 `FetchContent` 下载并包含 Raylib 和 Raygui 库，同时禁用它们的示例和游戏构建。
3. **构建目标**: 定义可执行文件 `ai_backgammon`，添加源文件和包含目录，设置编译选项和链接库。
4. **宏定义**: 根据开发或发布阶段设置资源路径宏 `ASSETS_PATH`，以便在代码中方便地引用资产文件。