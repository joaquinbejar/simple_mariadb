include(FetchContent)
FetchContent_Declare(simple_config
        GIT_REPOSITORY https://github.com/joaquinbejar/simple_config.git
        GIT_TAG dev
        )
FetchContent_MakeAvailable(simple_config)

set(SIMPLE_CONFIG_INCLUDE ${simple_config_SOURCE_DIR}/include CACHE INTERNAL "")