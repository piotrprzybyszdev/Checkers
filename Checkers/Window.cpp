#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_internal.h>
#include <imgui.h>

#include <iostream>
#include <string>

#include "Core/Core.h"
#include "Renderer/Renderer.h"

#include "Game.h"
#include "Window.h"

namespace Checkers
{

static void GlfwErrorCallback(int error, const char *description)
{
	throw std::exception(std::format("GLFW error {} {}", error, description).c_str());
}

Window::Window(int width, int height, const char *title, bool vsync)
	: m_Width(width), m_Height(height), m_Vsync(vsync), m_FontSize(13),
	m_DesiredFontSize(13), m_Flipped(false), m_ShouldFlip(false)
{
	int result = glfwInit();

#ifndef NDEBUG
	if (result == GLFW_FALSE)
		throw new std::exception("Glfw initialization failed!");
#endif

	glfwSetErrorCallback(GlfwErrorCallback);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

	m_Handle = glfwCreateWindow(width, height, title, nullptr, nullptr);
	
#ifndef NDEBUG
	if (m_Handle == nullptr)
		throw new std::exception("Window creation failed!");
#endif

	glfwMakeContextCurrent(m_Handle);
	glfwSwapInterval(vsync);

	result = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

#ifndef NDEBUG
	if (result == GLFW_FALSE)
		throw new std::exception("glad initalization failed!");
#endif

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	m_Io = &ImGui::GetIO();
	m_Io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	m_Io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;
	m_Io->IniFilename = nullptr;
	
	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(m_Handle, true);
	ImGui_ImplOpenGL3_Init("#version 460");
}

Window::~Window()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(m_Handle);
	glfwTerminate();
}

GLFWwindow *Window::GetHandle() const
{
	return m_Handle;
}

void Window::Refresh() const
{
	int width, height;
	glfwGetWindowSize(m_Handle, &width, &height);
	glfwSetWindowSize(m_Handle, width, height);
}

void Window::SetTextOverlay(const char *text)
{
	m_TextOverlay = text;
}

bool Window::ShouldClose()
{
	return glfwWindowShouldClose(m_Handle);
}

void Window::WaitEvents()
{
	glfwWaitEvents();
}

void Window::OnUpdate()
{
	if (m_FontSize != m_DesiredFontSize)
	{
		m_FontSize = m_DesiredFontSize;

		ImFontConfig cfg;
		cfg.SizePixels = m_FontSize;
		m_Io->Fonts->Clear();
		m_Io->Fonts->AddFontDefault(&cfg);
		m_Io->Fonts->Build();

		ImGui_ImplOpenGL3_DestroyFontsTexture();
	}

	if (m_Flipped != m_ShouldFlip)
	{
		m_Flipped = !m_Flipped;
		Renderer::Flip();
	}

	glfwPollEvents();

	if (glfwGetWindowAttrib(m_Handle, GLFW_ICONIFIED) != 0)
	{
		ImGui_ImplGlfw_Sleep(10);
		return;
	}
}

void Window::OnRender()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGuiID dockspaceId = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport(), ImGuiDockNodeFlags_AutoHideTabBar);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
	ImGui::Begin("Viewport");

	const ImVec2 avail = ImGui::GetContentRegionAvail();
	const float size = std::min(avail.x, avail.y);
	m_ViewportSize = { size, size };
	m_ViewportPos = { (avail.x - size) / 2.0f, (avail.y - size) / 2.0f };
	ImGui::SetCursorPos(m_ViewportPos);
	ImGui::Image((ImTextureID)(uintptr_t)Renderer::GetTextureId(), m_ViewportSize,
		m_Flipped ? ImVec2(0, 1) : ImVec2(1, 0), m_Flipped ? ImVec2(1, 0) : ImVec2(0, 1));
	
	if (ImGui::IsItemClicked())
	{
		ImVec2 mousePos = ImGui::GetMousePos();
		Game::HandleClick(
			m_Flipped ? (mousePos.x - m_ViewportPos.x) / m_ViewportSize.x : 1.0f - (mousePos.x - m_ViewportPos.x) / m_ViewportSize.x,
			m_Flipped ? 1.0f - ((mousePos.y - m_ViewportPos.y) / m_ViewportSize.y) : ((mousePos.y - m_ViewportPos.y) / m_ViewportSize.y)
		);
	}

	if (m_TextOverlay)
	{
		ImGui::SetWindowFontScale(3.0f);
		const ImVec2 textSize = ImGui::CalcTextSize(m_TextOverlay);
		const ImVec2 textPos = { m_ViewportPos.x + (m_ViewportSize.x - textSize.x) / 2.0f, m_ViewportPos.y + (m_ViewportSize.y - textSize.y) / 2.0f };
		ImGui::GetWindowDrawList()->AddText(textPos, ImColor(0.0f, 0.0f, 0.0f, 1.0f), m_TextOverlay);
	}

	ImGui::End();
	ImGui::PopStyleVar();

	ImGui::Begin("Settings");

	ImGui::Dummy(ImVec2(0.0f, 5.0f));
	ImGui::InputInt("font size", &m_DesiredFontSize);

	ImGui::Dummy(ImVec2(0.0f, 5.0f));

	ImGui::PushID(0);
	ImGui::Text("Black Player:");
	if (ImGui::RadioButton("Human", Game::GetBlackPlayerType() == ControllerType::PlayerController))
		Game::SelectBlackPlayer(ControllerType::PlayerController);
	if (ImGui::RadioButton("Computer (CPU)", Game::GetBlackPlayerType() == ControllerType::ComputerHostController))
		Game::SelectBlackPlayer(ControllerType::ComputerHostController);
	if (ImGui::RadioButton("Computer (GPU)", Game::GetBlackPlayerType() == ControllerType::ComputerDeviceController))
		Game::SelectBlackPlayer(ControllerType::ComputerDeviceController);
	ImGui::PopID();

	ImGui::PushID(1);
	ImGui::Text("White Player:");
	if (ImGui::RadioButton("Human", Game::GetWhitePlayerType() == ControllerType::PlayerController))
		Game::SelectWhitePlayer(ControllerType::PlayerController);
	if (ImGui::RadioButton("Computer (CPU)", Game::GetWhitePlayerType() == ControllerType::ComputerHostController))
		Game::SelectWhitePlayer(ControllerType::ComputerHostController);
	if (ImGui::RadioButton("Computer (GPU)", Game::GetWhitePlayerType() == ControllerType::ComputerDeviceController))
		Game::SelectWhitePlayer(ControllerType::ComputerDeviceController);
	ImGui::PopID();

	ImGui::Dummy(ImVec2(0.0f, 15.0f));
	if (ImGui::Button("Flip board"))
		m_ShouldFlip = !m_ShouldFlip;

	ImGui::Dummy(ImVec2(0.0f, 15.0f));
	if (ImGui::Button("Restart"))
	{
		Game::End();
		Game::Start();
	}

	ImGui::End();

	ImGui::Begin("Info");

	for (const auto &stat : Stats::GetStats())
		ImGui::Text(stat.second.c_str());

	ImGui::End();

	static bool setup = true;
	if (setup)
	{
		setup = false;
		SetupDocking(dockspaceId);
	}

	ImGui::Render();
	Renderer::Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
	Renderer::Render();

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	glfwSwapBuffers(m_Handle);
}

void Window::SetupDocking(ImGuiID dockspaceId)
{
	ImGuiID viewportId, settingsId;
	ImGui::DockBuilderSplitNode(dockspaceId, ImGuiDir_Right, 0.2f, &settingsId, &viewportId);
	ImGui::DockBuilderSetNodeSize(settingsId, { std::min(400.0f, m_Width / 4.0f), m_Height / 2.0f });
	ImGui::DockBuilderDockWindow("Settings", settingsId);
	ImGui::DockBuilderDockWindow("Viewport", viewportId);

	ImGuiID infoId = ImGui::DockBuilderSplitNode(settingsId, ImGuiDir_Down, 0.2f, nullptr, nullptr);
	ImGui::DockBuilderSetNodeSize(infoId, { std::min(400.0f, m_Width / 4.0f), m_Height / 2.0f });
	ImGui::DockBuilderDockWindow("Info", infoId);

	ImGui::DockBuilderFinish(dockspaceId);
}

}
