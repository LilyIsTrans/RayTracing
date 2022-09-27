#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Timer.h"

#include "Renderer.h"
#include "Camera.h";

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
			sphere.Radius = 0.5f;
			sphere.Albedo = { 1.0f, 0.0f, 1.0f };
			m_Scene.Spheres.push_back(sphere);
		}
		{
			Sphere sphere;
			sphere.Position = { 1.0f, 0.0f, -5.0f };
			sphere.Radius = 1.5f;
			sphere.Albedo = { 0.2f, 0.3f, 1.0f };
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
		/*
			ImGui::SliderFloat3("Sphere Origin", &m_Renderer.SphereOrigin.x, -1.0f, 1.0f);
			ImGui::SliderFloat("Sphere Radius", &m_Renderer.radius, 0.0f, 2.0f);
			if (ImGui::SliderFloat3("Light Direction", &m_Renderer.lightDirProxy.x, -1.0f, 1.0f))
			{
				m_Renderer.lightDirUpdated();
			}
			ImGui::Checkbox("Do shading", &m_Renderer.doShading);
		*/

		for (size_t i = 0; i < m_Scene.Spheres.size(); i++)
		{
			ImGui::PushID(i);

			Sphere& sphere = m_Scene.Spheres[i];
			ImGui::DragFloat3("Position", glm::value_ptr(sphere.Position), 0.1f);
			ImGui::DragFloat("Radius", &sphere.Radius, 0.1f);
			ImGui::ColorEdit3("Albedo", glm::value_ptr(sphere.Albedo), 0.1f);

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
			newSphere.Albedo = m_newSphereAlbedo;
			m_Scene.Spheres.push_back(newSphere);
		}
		ImGui::DragFloat3("Position", glm::value_ptr(m_newSpherePosition), 0.1f);
		ImGui::DragFloat("Radius", &m_newSphereRadius, 0.1f);
		ImGui::ColorEdit3("Albedo", glm::value_ptr(m_newSphereAlbedo), 0.1f);
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
			m_Renderer.OnResize(m_ViewportWidth, m_ViewportHeight, m_SSAALevel);
			m_Camera.OnResize(m_ViewportWidth, m_ViewportHeight, m_SSAALevel);
		}
		
		
		m_Renderer.Render(m_Scene, m_Camera);

		m_LastRenderTime = timer.ElapsedMillis();
	}
private:
	Renderer m_Renderer;
	Camera m_Camera;
	Scene m_Scene;
	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0, m_SSAALevel = 1;
	uint32_t pm_ViewportWidth = -1, pm_ViewportHeight = -1;

	glm::vec3 m_newSpherePosition = glm::vec3(0.0f);
	glm::vec3 m_newSphereAlbedo = glm::vec3(0.0f);
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