if(NOT MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_DEBUG} -Werror -Wall -Wextra -Wpedantic")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Werror -Wall -Wextra -Wpedantic -g3 -O0")
endif()


if(MSVC_VERSION VERSION_GREATER_EQUAL 1920)
	# Make sure __cplusplus is set correctly depending on std
	string(APPEND CMAKE_CXX_FLAGS " /Zc:__cplusplus")
endif()

if("${HSM_CLANG_COVERAGE}")
  set(CMAKE_CXX_FLAGS "-fprofile-instr-generate -fcoverage-mapping")
endif()

if("${HSM_GCC_COVERAGE}")
  set(CMAKE_CXX_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
endif()

if("${HSM_ASAN}")
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
  set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
endif()

if("${HSM_UBSAN}")
  set(CMAKE_CXX_FLAGS "-fsanitize=undefined -fno-omit-frame-pointer -fno-sanitize-recover -fsanitize=float-divide-by-zero")
endif()

if("${HSM_TIME_TRACE}")
  set(CMAKE_CXX_FLAGS "-ftime-trace")
endif()

if("${HSM_TEMPLIGHT_PROFILER}")
  set(CMAKE_CXX_FLAGS "-Xtemplight -profiler -Xtemplight -ignore-system")
endif()
