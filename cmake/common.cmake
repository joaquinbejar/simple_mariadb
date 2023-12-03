include(FetchContent)
FetchContent_Declare(common
        GIT_REPOSITORY https://github.com/joaquinbejar/common_cpp.git
        GIT_TAG dev
)
FetchContent_MakeAvailable(common)

set(COMMON_INCLUDE ${common_SOURCE_DIR}/include CACHE INTERNAL "")