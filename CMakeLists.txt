cmake_minimum_required(VERSION 3.16)
project(MyCad VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置源文件编码为UTF-8（解决中文乱码问题）
if(MSVC)
    add_compile_options(/utf-8)
    # 同时设置执行字符集为UTF-8
    add_compile_definitions(_UTF8_SOURCE)
endif()

# OpenCascade路径
set(OCCT_DIR "F:/OpenCasCAD/opencascade-7.9.2/occt-vc14-64")
set(OCCT_INCLUDE_DIR "${OCCT_DIR}/inc")
set(OCCT_LIB_DIR "${OCCT_DIR}/win64/vc14/lib")
set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${OCCT_DIR}/cmake")

# Qt路径
set(Qt5_DIR "D:/Qt5.15.2/5.15.2/msvc2019_64/lib/cmake/Qt5")

# 查找Qt5组件
find_package(Qt5 REQUIRED COMPONENTS Core Widgets OpenGL)

# 查找OpenCascade
# 默认使用手动配置以避免VTK依赖问题
# 如果需要使用OpenCascade的CMake配置，可以设置 USE_OCCT_CMAKE=ON
option(USE_OCCT_CMAKE "Use OpenCascade CMake configuration (may require VTK)" OFF)

if(USE_OCCT_CMAKE)
    # 尝试使用OpenCascade的CMake配置
    set(CMAKE_DISABLE_FIND_PACKAGE_VTK TRUE)
    find_package(OpenCASCADE QUIET)
    if(OpenCASCADE_FOUND)
        set(USE_MANUAL_OCCT_CONFIG FALSE)
        message(STATUS "Using OpenCascade CMake configuration")
    else()
        set(USE_MANUAL_OCCT_CONFIG TRUE)
        message(WARNING "OpenCASCADE CMake package not found, falling back to manual configuration")
    endif()
else()
    # 使用手动配置（默认，避免VTK依赖问题）
    set(USE_MANUAL_OCCT_CONFIG TRUE)
    message(STATUS "Using manual OpenCascade configuration (to avoid VTK dependency)")
endif()

# 设置包含目录
if(USE_MANUAL_OCCT_CONFIG)
    set(OpenCASCADE_INCLUDE_DIR ${OCCT_INCLUDE_DIR})
endif()

# 包含目录
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OpenCASCADE_INCLUDE_DIR}
)

# 源文件
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/View3D.cpp
    src/Document.cpp
    src/FileIO.cpp
    src/Modeling.cpp
    src/SelectionManager.cpp
    src/TransformManager.cpp
    src/ParameterDialog.cpp
)

# 头文件
set(HEADERS
    include/MainWindow.h
    include/View3D.h
    include/Document.h
    include/FileIO.h
    include/Modeling.h
    include/SelectionManager.h
    include/TransformManager.h
    include/ParameterDialog.h
)

# 资源文件
set(RESOURCES
    resources/resources.qrc
)

# 生成资源
qt5_add_resources(RESOURCE_FILES ${RESOURCES})

# 使用Qt的MOC自动处理Q_OBJECT类
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 创建可执行文件
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${RESOURCE_FILES}
)

# 链接库
target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Widgets
    Qt5::OpenGL
)

# 链接OpenCascade库
if(USE_MANUAL_OCCT_CONFIG)
    # 手动配置模式：直接链接库文件
    message(STATUS "Using manual OpenCascade configuration")
    message(STATUS "OpenCascade library directory: ${OCCT_LIB_DIR}")
    
    # 设置库目录
    link_directories(${OCCT_LIB_DIR})
    
    # 链接库（使用完整路径以确保找到）
    # 注意：如果编译Debug版本，库文件名可能是TKerneld.lib，需要相应调整
    target_link_libraries(${PROJECT_NAME}
        ${OCCT_LIB_DIR}/TKernel.lib
        ${OCCT_LIB_DIR}/TKMath.lib
        ${OCCT_LIB_DIR}/TKG2d.lib
        ${OCCT_LIB_DIR}/TKG3d.lib
        ${OCCT_LIB_DIR}/TKGeomBase.lib
        ${OCCT_LIB_DIR}/TKBRep.lib
        ${OCCT_LIB_DIR}/TKGeomAlgo.lib
        ${OCCT_LIB_DIR}/TKTopAlgo.lib
        ${OCCT_LIB_DIR}/TKPrim.lib
        ${OCCT_LIB_DIR}/TKBO.lib
        ${OCCT_LIB_DIR}/TKBool.lib
        ${OCCT_LIB_DIR}/TKShHealing.lib
        ${OCCT_LIB_DIR}/TKHLR.lib
        ${OCCT_LIB_DIR}/TKMesh.lib
        ${OCCT_LIB_DIR}/TKDESTEP.lib
        ${OCCT_LIB_DIR}/TKDEIGES.lib
        ${OCCT_LIB_DIR}/TKDESTL.lib
        ${OCCT_LIB_DIR}/TKService.lib
        ${OCCT_LIB_DIR}/TKXSBase.lib
        ${OCCT_LIB_DIR}/TKXSDRAWSTEP.lib
        ${OCCT_LIB_DIR}/TKXSDRAWIGES.lib
        ${OCCT_LIB_DIR}/TKV3d.lib
        ${OCCT_LIB_DIR}/TKMeshVS.lib
        ${OCCT_LIB_DIR}/TKOpenGl.lib
        ${OCCT_LIB_DIR}/TKFillet.lib
        ${OCCT_LIB_DIR}/TKOffset.lib
        ${OCCT_LIB_DIR}/TKFeat.lib
        ${OCCT_LIB_DIR}/TKXCAF.lib
        ${OCCT_LIB_DIR}/TKCAF.lib
        ${OCCT_LIB_DIR}/TKLCAF.lib
    )
elseif(OpenCASCADE_LIBRARIES)
    # 使用OpenCascade CMake配置提供的库
    target_link_libraries(${PROJECT_NAME} ${OpenCASCADE_LIBRARIES})
else()
    # 回退到手动配置
    message(WARNING "OpenCASCADE_LIBRARIES not found, using manual configuration")
    link_directories(${OCCT_LIB_DIR})
    target_link_libraries(${PROJECT_NAME}
        ${OCCT_LIB_DIR}/TKernel.lib
        ${OCCT_LIB_DIR}/TKMath.lib
        ${OCCT_LIB_DIR}/TKG2d.lib
        ${OCCT_LIB_DIR}/TKG3d.lib
        ${OCCT_LIB_DIR}/TKGeomBase.lib
        ${OCCT_LIB_DIR}/TKBRep.lib
        ${OCCT_LIB_DIR}/TKGeomAlgo.lib
        ${OCCT_LIB_DIR}/TKTopAlgo.lib
        ${OCCT_LIB_DIR}/TKPrim.lib
        ${OCCT_LIB_DIR}/TKBO.lib
        ${OCCT_LIB_DIR}/TKBool.lib
        ${OCCT_LIB_DIR}/TKShHealing.lib
        ${OCCT_LIB_DIR}/TKHLR.lib
        ${OCCT_LIB_DIR}/TKMesh.lib
        ${OCCT_LIB_DIR}/TKDESTEP.lib
        ${OCCT_LIB_DIR}/TKDEIGES.lib
        ${OCCT_LIB_DIR}/TKDESTL.lib
        ${OCCT_LIB_DIR}/TKService.lib
        ${OCCT_LIB_DIR}/TKXSBase.lib
        ${OCCT_LIB_DIR}/TKXSDRAWSTEP.lib
        ${OCCT_LIB_DIR}/TKXSDRAWIGES.lib
        ${OCCT_LIB_DIR}/TKV3d.lib
        ${OCCT_LIB_DIR}/TKMeshVS.lib
        ${OCCT_LIB_DIR}/TKOpenGl.lib
        ${OCCT_LIB_DIR}/TKFillet.lib
        ${OCCT_LIB_DIR}/TKOffset.lib
        ${OCCT_LIB_DIR}/TKFeat.lib
        ${OCCT_LIB_DIR}/TKXCAF.lib
        ${OCCT_LIB_DIR}/TKCAF.lib
        ${OCCT_LIB_DIR}/TKLCAF.lib
    )
endif()

# 包含OpenCascade头文件目录
target_include_directories(${PROJECT_NAME} PRIVATE
    ${OpenCASCADE_INCLUDE_DIR}
)

# Windows特定设置
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()

