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

class MainLayer : public Walnut::Layer {
public:
	MainLayer()
		: Walnut::Layer()
		, mViewport()
		, mImage()
		, mImageData(nullptr)
		, mLastRenderTime(-1.0f)
	{
		extern void UIStyle();
		UIStyle();

		SetWindowPos(GetConsoleWindow(), 0, 3832, 566, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		ShowWindow(GetConsoleWindow(), SW_SHOWMAXIMIZED);
	}

	void OnUIRender() override {
		if (ImGui::Begin("Settings")) {
			if (ImGui::Button("Render")) {
				std::cout << "Starting render..." << std::endl;
				
				this->RenderImage();
				
				std::cout << "Done!" << std::endl;
			}

			if (mLastRenderTime != -1.0f) {
				ImGui::Text("Frametime: %fms", mLastRenderTime);
			}
		} ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
		if (ImGui::Begin("Viewport")) {
			mViewport.x = static_cast<glm::u32>(ImGui::GetContentRegionAvail().x);
			mViewport.y = static_cast<glm::u32>(ImGui::GetContentRegionAvail().y);

			if (mImage) {
				ImGui::Image(mImage->GetDescriptorSet(), { static_cast<glm::f32>(mImage->GetWidth()), static_cast<glm::f32>(mImage->GetHeight()) });
			}
		} ImGui::End(); ImGui::PopStyleVar();
	}

	void RenderImage() {
		Walnut::Timer timer;

		if (!mImage || mViewport.x != mImage->GetWidth() || mViewport.y != mImage->GetHeight()) {
			mImage = std::make_shared<Walnut::Image>(mViewport.x, mViewport.y, Walnut::ImageFormat::RGBA);
			delete[] mImageData;
			mImageData = new glm::u32[mViewport.x * mViewport.y];
		}

		for (glm::u32 i = 0; i < mViewport.x * mViewport.y; i++) {
			mImageData[i] = Walnut::Random::UInt() | 0xFF000000;
		}

		mImage->SetData(mImageData);

		mLastRenderTime = timer.ElapsedMillis();
	}
	
private:
	glm::u32vec2 mViewport;
	std::shared_ptr<Walnut::Image> mImage;
	glm::u32* mImageData;
	glm::f32 mLastRenderTime;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv) {
	Walnut::ApplicationSpecification spec;
	spec.Name = "lumitracer";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<MainLayer>();

	return app;
}
