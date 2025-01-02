#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"
#include "Walnut/Random.h"
#include "Walnut/Timer.h"
#include "Walnut/Image.h"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <memory>
#include <iostream>

#include "Renderer.h"
#include "Camera.h"
#include "Scene.h"

class MainLayer : public Walnut::Layer {
public:
	MainLayer()
		: Walnut::Layer()
		, mViewport()
		, mLastRenderTime(-1.0f)
		, mCamera(45.0f, 0.1f, 200.0f)
		, mRenderer()
		, mScene()
	{
		extern void UIStyle();
		UIStyle();

		SetWindowPos(GetConsoleWindow(), 0, 3832, 566, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		ShowWindow(GetConsoleWindow(), SW_SHOWMAXIMIZED);

		mCamera.SetSensitivity(0.004f);

		mScene.materials.push_back({
			.albedo = glm::vec3(0.0f),
			.roughness = 1.0f,
			.metallic = 0.0f
		});

		mScene.materials.push_back({
			.albedo = glm::vec3(1.0f, 0.0f, 0.0f),
			.roughness = 1.0f,
			.metallic = 0.0f
		});

		mScene.spheres.push_back({
			.position = glm::vec3(0.0f, -101.0f, 0.0f),
			.radius = 100.0f,
			.materialIndex = 0
		});

		mScene.spheres.push_back({
			.position = glm::vec3(2.0f, -0.5f, -5.0f),
			.radius = 0.75f,
			.materialIndex = 1
		});
	}

	void OnUpdate(glm::f32 ts) override {
		bool cameraMoved = mCamera.OnUpdate(ts);

		if (cameraMoved) {
			mRenderer.ResetAccumulationFrames();
		}
	}

	void OnUIRender() override {
		if (ImGui::Begin("Overview")) {
			if (mLastRenderTime != -1.0f) {
				ImGui::Text("Frametime: %fms", mLastRenderTime);
			}

			ImGui::Text("Accumulation: %i frames", mRenderer.GetAccumulationFrames());
			ImGui::SameLine();
			if (ImGui::Button("Reset")) {
				mRenderer.ResetAccumulationFrames();
			}

			ImGui::Text("Viewport: %i pixels", mViewport.x * mViewport.y);
		} ImGui::End();

		if (ImGui::Begin("Scene")) {
			ImGui::Text("Spheres:");
			ImGui::Indent();
			for (int i = 0; i < mScene.spheres.size(); i++) {
				ImGui::PushID(i);

				Sphere& sphere = mScene.spheres[i];
				ImGui::DragFloat3("Position", glm::value_ptr(sphere.position), 0.1f);
				ImGui::DragFloat("Radius", &sphere.radius, 0.1f);
				ImGui::InputInt("Material", &sphere.materialIndex, 1, 1);
				
				if (i != mScene.spheres.size() - 1) {
					ImGui::Separator();
				}

				ImGui::PopID();
			}
			ImGui::Unindent();
			ImGui::Text("Materials:");
			ImGui::Indent();
			for (int i = 0; i < mScene.materials.size(); i++) {
				ImGui::PushID(i);

				Material& material = mScene.materials[i];
				ImGui::ColorEdit3("Albedo", glm::value_ptr(material.albedo));
				ImGui::SliderFloat("Roughness", &material.roughness, 0.0f, 1.0f);
				ImGui::SliderFloat("Metallic", &material.metallic, 0.0f, 1.0f);

				if (i != mScene.materials.size() - 1) {
					ImGui::Separator();
				}

				ImGui::PopID();
			}
			ImGui::Unindent();
		} ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0.0f, 0.0f });
		if (ImGui::Begin("Viewport")) {
			mViewport.x = static_cast<glm::u32>(ImGui::GetContentRegionAvail().x);
			mViewport.y = static_cast<glm::u32>(ImGui::GetContentRegionAvail().y);

			if (mRenderer.GetFinalImage()) {
				ImGui::Image(mRenderer.GetFinalImage()->GetDescriptorSet(), { static_cast<glm::f32>(mViewport.x), static_cast<glm::f32>(mViewport.y) }, { 0, 1 }, { 1, 0 });
			}
		} ImGui::End(); ImGui::PopStyleVar();

		this->RenderImage();
	}

	void RenderImage() {
		Walnut::Timer timer;

		mCamera.Resize(mViewport.x, mViewport.y);
		mRenderer.Render(mScene, mCamera);

		mLastRenderTime = timer.ElapsedMillis();
	}
	
private:
	glm::u32vec2 mViewport;
	glm::f32 mLastRenderTime;
	Camera mCamera;
	Renderer mRenderer;
	Scene mScene;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv) {
	Walnut::ApplicationSpecification spec;
	spec.Name = "lumitracer";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<MainLayer>();

	return app;
}
