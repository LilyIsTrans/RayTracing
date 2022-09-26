#include "Walnut/Application.h"
#include "Walnut/EntryPoint.h"

#include "Walnut/Image.h"
#include "Walnut/Timer.h"

#include "Renderer.h"

using namespace Walnut;


class ExampleLayer : public Walnut::Layer
{
public:
	virtual void OnUIRender() override
	{
		ImGui::Begin("Settings");
		ImGui::Text("Last render: %.3fms", m_LastRenderTime);
		ImGui::Text("Current FPS: %.3ffps", 1000.0f / m_LastRenderTime);
		ImGui::SliderFloat3("Sphere Origin", &m_Renderer.SphereOrigin.x, -1.0f, 1.0f);
		ImGui::SliderFloat("Sphere Radius", &m_Renderer.radius, 0.0f, 2.0f);
		if (ImGui::SliderFloat3("Light Direction", &m_Renderer.lightDirProxy.x, -1.0f, 1.0f))
		{
			m_Renderer.lightDirUpdated();
		}
		ImGui::Checkbox("Do shading", &m_Renderer.doShading);
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
		}
		
		
		m_Renderer.Render();

		m_LastRenderTime = timer.ElapsedMillis();
	}
private:
	Renderer m_Renderer;
	uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
	uint32_t pm_ViewportWidth = -1, pm_ViewportHeight = -1;

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