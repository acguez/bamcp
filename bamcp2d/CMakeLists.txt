
#set(CMAKE_C_COMPILER gcc-mp-4.6)
#set(CMAKE_CXX_COMPILER  g++-mp-4.6)

cmake_minimum_required(VERSION 2.6)

SET(gx 1)
IF(gx)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -DGX -mfpmath=sse -msse -msse2 -msse3 -mtune=native -ftree-vectorize -ffast-math -Wall")
ELSE(gx)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -static-libgcc -mfpmath=sse -msse -msse2 -msse3 -mtune=native -ftree-vectorize -ffast-math -Wall")
ENDIF(gx)

set(CMAKE_SOURCE_DIR "src/")
SET(BASEPATH "${CMAKE_SOURCE_DIR}")
INCLUDE_DIRECTORIES("${BASEPATH}")

PROJECT(brl)
SET(brl_src
	src/envs/flat/simulator2D.cpp
	src/envs/flat/sq2Dgrid.cpp
	src/envs/flat/lazySq2Dgrid.cpp
	src/envs/flat/inf2Dgrid.cpp
	src/envs/flat/lazyInfGrid.cpp
	src/utils/rng.cpp
	src/utils/utils.cpp
	src/utils/utils2.cpp
	src/utils/anyoption.cpp
	src/utils/hr_time.cpp
	src/planners/mcp/experiment.cpp
	src/planners/mcp/bamcp2D/bamcp2D.cpp
	src/planners/mcp/bamcp2D/node2D.cpp
	src/planners/MDPutils.cpp
	src/planners/boss/boss/boss.cpp
	src/planners/boss/experiment_boss.cpp
	src/planners/experiment_baselines.cpp
	src/planners/mcp/bfs3/bfs3.cpp
	src/planners/mcp/bfs3/node.cpp
	src/planners/mcp/experiment_bfs3.cpp
	src/samplers/flat/M1Sampler.cpp
	src/samplers/flat/M1SamplerFactory.cpp
	src/samplers/flat/M2Sampler.cpp
	src/samplers/flat/M2SamplerFactory.cpp
	src/samplers/flat/MixtureSampler.cpp
	src/samplers/flat/MixtureSamplerFactory.cpp
	src/samplers/flat/LazyMixtureSampler.cpp
	src/samplers/flat/InfGridSamplerFactory.cpp
	src/samplers/flat/LazyInfGridSampler.cpp
	src/samplers/flat/dummyFactory.cpp
	src/viz/interface.cpp)

IF(gx)
	SET(brl_src ${brl_src} 
	src/viz/graphics2d.cpp
	src/viz/font.cpp)
ENDIF(gx)

SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "/opt/local/include/")

SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "/usr/local/include/")

set(Boost_USE_STATIC_LIBS ON)
FIND_PACKAGE(Boost COMPONENTS system filesystem REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
INCLUDE_DIRECTORIES("/usr/local/include/")
MESSAGE("${Boost_INCLUDE_DIRS}")

IF(gx)
	FIND_PACKAGE(GLU) #define GLU_INCLUDE_DIR + GLU_LIBRARY
	INCLUDE_DIRECTORIES(${GLU_INCLUDE_DIR})
ENDIF(gx)

ADD_EXECUTABLE(brl src/main.cpp ${brl_src})

If(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	FIND_LIBRARY(COCOA_LIBRARY Cocoa)
	FIND_LIBRARY(APPLICATION_SERVICES_LIBRARY ApplicationServices)
IF(COCOA_LIBRARY)
	target_link_libraries(brl glfw ${GLU_LIBRARY} ${COCOA_LIBRARY} ${APPLICATION_SERVICES_LIBRARY} ${Boost_LIBRARIES} ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
ELSE(COCOA_LIBRARY)
	FIND_LIBRARY(CARBON_LIBRARY Carbon)
target_link_libraries(brl glfw ${GLU_LIBRARY} ${CARBON_LIBRARY} ${APPLICATION_SERVICES_LIBRARY} ${Boost_LIBRARIES} ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
ENDIF(COCOA_LIBRARY)
else(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	IF(gx)
		TARGET_LINK_LIBRARIES(brl glfw ${GLU_LIBRARY} ${Boost_LIBRARIES} ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
	ELSE(gx)
		TARGET_LINK_LIBRARIES(brl ${Boost_LIBRARIES} ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY})
	ENDIF(gx)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

