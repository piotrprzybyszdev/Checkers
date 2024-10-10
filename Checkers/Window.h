#pragma once

#include <GLFW/glfw3.h>
#include <imgui.h>

#include <cstdint>

namespace Checkers
{

class Window
{
public:
	Window(int width, int height, const char *title, bool vsync);
	virtual ~Window();

	bool ShouldClose();
	void WaitEvents();
	void OnUpdate();
	void OnRender();

	GLFWwindow *GetHandle() const;
	void Refresh() const;

	void SetTextOverlay(const char *text);

private:
	int m_Width, m_Height;
	bool m_Vsync;

	GLFWwindow *m_Handle;
	ImGuiIO *m_Io;

	ImVec2 m_ViewportPos;
	ImVec2 m_ViewportSize;

	int m_DesiredFontSize;
	int m_FontSize;

	bool m_ShouldFlip;
	bool m_Flipped;

	ImFont *m_OverlayFont = nullptr;
	const char *m_TextOverlay = nullptr;

	void SetupDocking(ImGuiID dockspaceId);
};

}
