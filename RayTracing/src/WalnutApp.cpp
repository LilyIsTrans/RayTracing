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
		Material& pinkSphere = m_Scene.Materials.emplace_back();
		pinkSphere.Albedo = { 1.0f, 0.0f, 1.0f };
		pinkSphere.Roughness = 0.0f;
		Material& blueSphere = m_Scene.Materials.emplace_back();
		blueSphere.Albedo = { 0.2f, 0.3f, 1.0f };
		blueSphere.Roughness = 0.1f;


		{
			Sphere sphere;
			sphere.Position = { 0.0f, 0.0f, 0.0f };
			sphere.Radius = 1.0f;
			sphere.MaterialIndex = 0;
			m_Scene.Spheres.push_back(sphere);
		}
		{
			Sphere sphere;
			sphere.Position = { 0.0f, -101.0f, 0.0f };
			sphere.Radius = 100.0f;
			sphere.MaterialIndex = 1;
			m_Scene.Spheres.push_back(sphere);
		}
	}

	virtual void OnUpdate(float ts) override
	{
		if (m_Camera.OnUpdate(ts))
			m_Renderer.ResetFrameIndex();
	}


	virtual void OnUIRender() override
	{
		ImGui::Begin("Info");
		ImGui::Text("Last render: %.3fms", m_LastRenderTime);
		ImGui::Text("Current FPS: %.3ffps", 1000.0f / m_LastRenderTime);
		ImGui::End();

		ImGui::Begin("Controls");
		if (ImGui::Button("Render"))
		{
			Render();
		}

		ImGui::Checkbox("Accumulate", &m_Renderer.GetSettings().Accumulate);

		if (ImGui::Button("Reset"))
		{
			m_Renderer.ResetFrameIndex();
		}
		ImGui::End();

		ImGui::Begin("Scene");
		ImGui::Text("Objects");
		for (size_t i = 0; i < m_Scene.Spheres.size(); i++)
		{
			ImGui::PushID(i);

			Sphere& sphere = m_Scene.Spheres[i];
			ImGui::DragFloat3("Position", glm::value_ptr(sphere.Position), 0.1f);
			ImGui::DragFloat("Radius", &sphere.Radius, 0.1f);
			ImGui::DragInt("Material", &sphere.MaterialIndex, 1.0f, 0, (int)m_Scene.Materials.size() - 1);
			

			ImGui::Separator();

			ImGui::PopID();
		}
		
		for (size_t i = m_Scene.Spheres.size(); i < m_Scene.Materials.size() + m_Scene.Spheres.size(); i++)
		{
			ImGui::PushID(i);

			Material& material = m_Scene.Materials[i - m_Scene.Spheres.size()];

			ImGui::ColorEdit3("Albedo", glm::value_ptr(material.Albedo), 0.1f);
			ImGui::SliderFloat("Roughness", &(material.Roughness), 0.0f, 1.0f);
			ImGui::SliderFloat("Metallic", &(material.Metallic), 0.0f, 1.0f);

			ImGui::Separator();

			ImGui::PopID();
		}

		if (ImGui::SliderFloat3("Light Direction", &m_Renderer.lightDirProxy.x, -1.0f, 1.0f))
		{
			m_Renderer.lightDirUpdated();
		}

		//ImGui::Checkbox("Do shading", &m_Renderer.doShading);
		ImGui::Separator();
		ImGui::Text("Add Sphere");
		if (ImGui::Button("Add Sphere to Scene"))
		{
			Sphere newSphere;
			newSphere.Position = m_newSpherePosition;
			newSphere.Radius = m_newSphereRadius;
			newSphere.MaterialIndex = m_newSphereMaterialIndex;
			m_Scene.Spheres.push_back(newSphere);
		}
		ImGui::DragFloat3("Position", glm::value_ptr(m_newSpherePosition), 0.1f);
		ImGui::DragFloat("Radius", &m_newSphereRadius, 0.1f);
		ImGui::DragInt("Material", &m_newSphereMaterialIndex, 1.0f, 0, (int)m_Scene.Materials.size() - 1);
		
		

		ImGui::Separator();
		ImGui::Text("Add Material");
		if (ImGui::Button("Add Material to Scene"))
		{
			Material newMaterial;
			newMaterial.Albedo = m_newMaterialAlbedo;
			newMaterial.Roughness = m_newMaterialRoughness;
			newMaterial.Metallic = m_newMaterialMetallic;
			m_Scene.Materials.push_back(newMaterial);
		}
		ImGui::ColorEdit3("Albedo", glm::value_ptr(m_newMaterialAlbedo), 0.1f);
		ImGui::SliderFloat("Roughness", &(m_newMaterialRoughness), 0.0f, 1.0f);
		ImGui::SliderFloat("Metallic", &(m_newMaterialMetallic), 0.0f, 1.0f);

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
	glm::vec3 m_newMaterialAlbedo = glm::vec3(0.0f);
	float m_newMaterialRoughness = 1.0f;
	float m_newMaterialMetallic = 0.0f;
	float m_newSphereRadius = 0.0f;
	int m_newSphereMaterialIndex = 0;

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