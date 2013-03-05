
#set(CMAKE_C_COMPILER gcc-mp-4.6)
#set(CMAKE_CXX_COMPILER  g++-mp-4.6)

cmake_minimum_required(VERSION 2.6)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=gnu++0x -mfpmath=sse -msse -msse2 -msse3 -mtune=native -ftree-vectorize -ffast-math -Wall")

set(CMAKE_SOURCE_DIR "src/")
SET(BASEPATH "${CMAKE_SOURCE_DIR}")
INCLUDE_DIRECTORIES("${BASEPATH}")

PROJECT(brl)
SET(brl_src
	src/envs/simulator.cpp
	src/envs/bandit.cpp
	src/envs/basicMDP.cpp
	src/envs/banditSim.cpp
	src/envs/grid.cpp
	src/envs/doubleloop.cpp
	src/envs/maze.cpp
	src/envs/lazyMDP.cpp
	src/envs/mazedoor.cpp
	src/utils/rng.cpp
	src/utils/utils.cpp
	src/utils/utils2.cpp
	src/utils/anyoption.cpp
	src/utils/hr_time.cpp
	src/planners/experiment_rand.cpp
	src/planners/mcp/experiment.cpp
	src/planners/mcp/bamcp/bamcp.cpp
	src/planners/mcp/bamcp/bauct.cpp
	src/planners/mcp/experiment_bauct.cpp
	src/planners/mcp/bamcp/node.cpp
	src/planners/mcp/experiment_bfs3.cpp	
	src/planners/mcp/bfs3/bfs3.cpp
	src/planners/mcp/bfs3/node.cpp
	src/planners/boss/boss/boss.cpp
	src/planners/boss/experiment.cpp
	src/planners/boss/sboss/sboss.cpp
	src/planners/boss/experiment_sboss.cpp
	src/planners/beb/beb.cpp
	src/planners/beb/experiment.cpp
	src/planners/MDPutils.cpp
	src/samplers/FDMsamplerFactory.cpp
	src/samplers/FDMTransitionSampler.cpp
	src/samplers/sampler.h
	src/samplers/SFDMsamplerFactory.cpp
	src/samplers/SFDMTransitionSampler.cpp
	src/samplers/LazySamplerFactory.cpp
	src/samplers/LazyMDPSampler.cpp
	src/samplers/DoorSampler.cpp
	src/samplers/DoorSamplerFactory.cpp
)

# Note: boost is not essential, dependency could be removed easily
SET(CMAKE_INCLUDE_PATH ${CMAKE_INCLUDE_PATH} "/opt/local/include/")
FIND_PACKAGE(Boost COMPONENTS system filesystem REQUIRED)
INCLUDE_DIRECTORIES(${Boost_INCLUDE_DIRS})
MESSAGE("${Boost_INCLUDE_DIRS}")
ADD_EXECUTABLE(brl src/main.cpp ${brl_src})

target_link_libraries(brl ${Boost_LIBRARIES}
	${Boost_FILESYSTEM_LIBRARY}
	${Boost_SYSTEM_LIBRARY}
) 

