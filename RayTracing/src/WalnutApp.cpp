#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Timer.h"

#include "Renderer.h"
#include "Camera.h";
#ifndef GLM_FORCE_INTRINSICS
#define GLM_FORCE_INTRINSICS
#endif // !GLM_FORCE_INTRINSICS
#include <glm/gtc/type_ptr.hpp>

using namespace Walnut;


class ExampleLayer : public Walnut::Layer
{
public:
	ExampleLayer()
		: m_Camera(45.0f, 0.1f, 100.0f) 
	{
		{
			Sphere sphere;
			sphere.Position = { 0.0f, 0.0f, 0.0f };
			sphere.Radius = 1.0f;
			sphere.Mat.Albedo = { 1.0f, 0.0f, 1.0f };
			m_Scene.Spheres.push_back(sphere);
		}
		{
			Sphere sphere;
			sphere.Position = { 0.0f, -101.0f, 0.0f };
			sphere.Radius = 100.0f;
			sphere.Mat.Albedo = { 0.2f, 0.3f, 1.0f };
			m_Scene.Spheres.push_back(sphere);
		}
	}

	virtual void OnUpdate(float ts) override
	{
		m_Camera.OnUpdate(ts);
	}


	virtual void OnUIRender() override
	{
		ImGui::Begin("Info");
		ImGui::Text("Last render: %.3fms", m_LastRenderTime);
		ImGui::Text("Current FPS: %.3ffps", 1000.0f / m_LastRenderTime);
		ImGui::End();

		ImGui::Begin("Scene");

		for (size_t i = 0; i < m_Scene.Spheres.size(); i++)
		{
			ImGui::PushID(i);

			Sphere& sphere = m_Scene.Spheres[i];
			ImGui::DragFloat3("Position", glm::value_ptr(sphere.Position), 0.1f);
			ImGui::DragFloat("Radius", &sphere.Radius, 0.1f);
			ImGui::ColorEdit3("Albedo", glm::value_ptr(sphere.Mat.Albedo), 0.1f);
			ImGui::DragFloat("Roughness", &(sphere.Mat.Roughness), 0.1f);
			ImGui::DragFloat("Metallic", &(sphere.Mat.Metallic), 0.1f);

			ImGui::Separator();

			ImGui::PopID();
		}
		


		if (ImGui::SliderFloat3("Light Direction", &m_Renderer.lightDirProxy.x, -1.0f, 1.0f))
		{
			m_Renderer.lightDirUpdated();
		}

		ImGui::Checkbox("Do shading", &m_Renderer.doShading);
		ImGui::Separator();
		ImGui::Text("Add Sphere");
		if (ImGui::Button("Add to Scene"))
		{
			Sphere newSphere;
			newSphere.Position = m_newSpherePosition;
			newSphere.Radius = m_newSphereRadius;
			newSphere.Mat.Albedo = m_newSphereAlbedo;
			newSphere.Mat.Roughness = m_newSphereRoughness;
			newSphere.Mat.Metallic = m_newSphereMetallic;
			m_Scene.Spheres.push_back(newSphere);
		}
		ImGui::DragFloat3("Position", glm::value_ptr(m_newSpherePosition), 0.1f);
		ImGui::DragFloat("Radius", &m_newSphereRadius, 0.1f);
		ImGui::ColorEdit3("Albedo", glm::value_ptr(m_newSphereAlbedo), 0.1f);
		ImGui::DragFloat("Roughness", &(m_newSphereRoughness), 0.1f);
		ImGui::DragFloat("Metallic", &(m_newSphereMetallic), 0.1f);
		ImGui::End();


		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport");
		pm_ViewportWidth = m_ViewportWidth;
		pm_ViewportHeight = m_ViewportHeight;
		m_ViewportWidth = ImGui::GetContentRegionAvail().x;
		m_ViewportHeight = ImGui::GetContentRegionAvail().y;

		auto image = m_Renderer.GetFinalImage();
		if (image)
			ImGui::Image(image->GetDescriptorSet(), { (float)image->GetWidth(), (float)image->GetHeight() },
				ImVec2(0, 1), ImVec2(1, 0));

		ImGui::End();
		ImGui::PopStyleVar();

		Render();
	}

	void Render()
	{
		Timer timer;

		if (pm_ViewportHeight != m_ViewportHeight || pm_ViewportWidth != m_ViewportWidth)
		{
			m_Renderer.OnResize(m_ViewportWidth, m_ViewportHeight);
			m_Camera.OnResize(m_ViewportWidth, m_ViewportHeight);
		}
		
		
		m_Renderer.Render(m_Scene, m_Camera);

		m_LastRenderTime = timer.ElapsedMillis();
	}
private:
	Renderer m_Renderer;
	Camera m_Camera;
	Scene m_Scene;
	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
	uint32_t pm_ViewportWidth = -1, pm_ViewportHeight = -1;

	glm::vec3 m_newSpherePosition = glm::vec3(0.0f);
	glm::vec3 m_newSphereAlbedo = glm::vec3(0.0f);
	float m_newSphereRoughness = 1.0f;
	float m_newSphereMetallic = 0.0f;
	float m_newSphereRadius = 0.0f;

	float m_LastRenderTime = 0.0f;
};

Walnut::Application* Walnut::CreateApplication(int argc, char** argv)
{
	Walnut::ApplicationSpecification spec;
	spec.Name = "Ray Tracing";

	Walnut::Application* app = new Walnut::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}