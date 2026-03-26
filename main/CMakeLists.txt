set(SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/main.c"

    "${CMAKE_CURRENT_SOURCE_DIR}/ota/manifest.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/ota/ota.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/ota/webpanel.c"

    "${CMAKE_CURRENT_SOURCE_DIR}/util/sntp.c"

    "${CMAKE_CURRENT_SOURCE_DIR}/webpanel/httpd.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/webpanel/partition.c"

    "${CMAKE_CURRENT_SOURCE_DIR}/wifi/station.c"
)

idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "."

    REQUIRES
        spiffs
        esp_wifi
        esp_netif
        nvs_flash
        esp_http_client
        esp_http_server
        app_update
        esp_https_ota
)

find_program(NPM_EXECUTABLE npm REQUIRED)

execute_process(
    COMMAND ${NPM_EXECUTABLE} install
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    RESULT_VARIABLE NPM_INSTALL_RESULT
)

if(NOT NPM_INSTALL_RESULT EQUAL 0)
    message(FATAL_ERROR "npm install failed with exit code ${NPM_INSTALL_RESULT}")
endif()

add_custom_target(webpanel_build
    COMMAND ${NPM_EXECUTABLE} run build:release
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    USES_TERMINAL
    VERBATIM
)

spiffs_create_partition_image(
    webpanel
    ../webpanel/dist
    FLASH_IN_PROJECT
    DEPENDS webpanel_build
)
