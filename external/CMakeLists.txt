set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(nlohmann_json)


add_library(external "")

target_link_libraries(external PRIVATE nlohmann_json::nlohmann_json)