add_executable(Checkers Core/Core.h Core/Core.cpp Renderer/Renderer.h Renderer/Renderer.cpp Renderer/RendererImpl.h Renderer/RendererImpl.cpp Renderer/Utils.h Renderer/Utils.cpp Renderer/Resources.h Position.h Position.cpp Controllers/Controller.h Controllers/ComputerController.h Controllers/ComputerController.cpp Controllers/PlayerController.h Controllers/PlayerController.cpp Controllers/MCTS.h Controllers/MCTS.cpp Controllers/Simulator.h Controllers/Simulator.cu Controllers/DeviceSimulator.cu Controllers/DeviceSimulator.h Controllers/HostSimulator.h Controllers/HostSimulator.cpp Game.h Game.cpp Window.h Window.cpp GraphicsCardConfig.h GraphicsCardConfig.cu main.cpp)

target_include_directories(Checkers PRIVATE ${CMAKE_SOURCE_DIR}/Checkers)
target_include_directories(Checkers PRIVATE ${CMAKE_SOURCE_DIR}/vendor/imgui/imgui/backends)
target_include_directories(Checkers PRIVATE ${CMAKE_SOURCE_DIR}/vendor/stb/stb)
target_link_libraries(Checkers glfw glad imgui glm stb)