#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Random.h"
#include "Walnut/Timer.h"
#include "Walnut/Image.h"

#include "glm/glm.hpp"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <memory>
#include <iostream>

#include "Renderer.h"
#include "Camera.h"

class MainLayer : public Walnut::Layer {
public:
	MainLayer()
		: Walnut::Layer()
		, mViewport()
		, mLastRenderTime(-1.0f)
		, mCamera(45.0f, 0.1f, 200.0f)
		, mRenderer()
	{
		extern void UIStyle();
		UIStyle();

		SetWindowPos(GetConsoleWindow(), 0, 3832, 566, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		ShowWindow(GetConsoleWindow(), SW_SHOWMAXIMIZED);

		mCamera.SetSensitivity(0.004f);
	}

	void OnUpdate(glm::f32 ts) override {
		mCamera.OnUpdate(ts);
	}

	void OnUIRender() override {
		this->RenderImage();

		if (ImGui::Begin("Settings")) {
			if (mLastRenderTime != -1.0f) {
				ImGui::Text("Frametime: %fms", mLastRenderTime);
			}
		} ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
		if (ImGui::Begin("Viewport")) {
			mViewport.x = static_cast<glm::u32>(ImGui::GetContentRegionAvail().x);
			mViewport.y = static_cast<glm::u32>(ImGui::GetContentRegionAvail().y);

			if (mRenderer.GetFinalImage()) {
				ImGui::Image(mRenderer.GetFinalImage()->GetDescriptorSet(), { static_cast<glm::f32>(mViewport.x), static_cast<glm::f32>(mViewport.y) }, { 0, 1 }, { 1, 0 });
			}
		} ImGui::End(); ImGui::PopStyleVar();
	}

	void RenderImage() {
		Walnut::Timer timer;

		mCamera.OnResize(mViewport.x, mViewport.y);
		mRenderer.Render(mViewport, mCamera);

		mLastRenderTime = timer.ElapsedMillis();
	}
	
private:
	glm::u32vec2 mViewport;
	glm::f32 mLastRenderTime;
	Camera mCamera;
	Renderer mRenderer;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv) {
	Walnut::ApplicationSpecification spec;
	spec.Name = "lumitracer";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<MainLayer>();

	return app;
}
