cmake_minimum_required(VERSION 3.16)
project(PBL2_Clinic_Management LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
include(CTest)

# Tự động xử lý Signal/Slot, file Giao diện .ui và Resource .qrc
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# Tự tìm Qt6 hoặc Qt5 tùy theo máy cài bản nào
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

# Tự động gom tất cả file C++, file Header, file UI và file Resource .qrc
file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS "src/*.cpp")
file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS "include/*.h")
file(GLOB_RECURSE UI_FILES CONFIGURE_DEPENDS "src/ui/*.ui")
file(GLOB_RECURSE QRC_FILES CONFIGURE_DEPENDS "*.qrc")

# Khởi tạo ứng dụng
add_executable(PBL2_App ${SRC_FILES} ${HEADER_FILES} ${UI_FILES} ${QRC_FILES})

# Liên kết thư viện
target_link_libraries(PBL2_App PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_include_directories(PBL2_App PRIVATE include)

if(BUILD_TESTING)
    add_executable(PBL2_AlgorithmTests tests/algorithm_smoke.cpp)
    target_include_directories(PBL2_AlgorithmTests PRIVATE include)
    add_test(NAME PBL2_AlgorithmTests COMMAND PBL2_AlgorithmTests)

    file(GLOB_RECURSE MODEL_SOURCE_FILES CONFIGURE_DEPENDS "src/models/*.cpp")
    add_executable(PBL2_WorkflowTests
        tests/workflow_smoke.cpp
        src/services/Database.cpp
        src/services/DatabaseManagement.cpp
        src/services/DatabaseWorkflow.cpp
        ${MODEL_SOURCE_FILES}
    )
    target_include_directories(PBL2_WorkflowTests PRIVATE include)
    add_test(NAME PBL2_WorkflowTests COMMAND PBL2_WorkflowTests)
endif()

# Tự động copy thư mục "data" từ thư mục gốc vào thư mục chứa file chạy (.exe) mỗi khi Build
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})