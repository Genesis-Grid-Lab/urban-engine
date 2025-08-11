#include "uepch.h"
#include "Panels/ContentBrowserPanel.h"

#include <imgui.h>

namespace UE {
  static std::string to_utf8(const std::filesystem::path& p) {
#ifdef _WIN32
    auto u8 = p.u8string();                    // std::u8string
    return std::string(u8.begin(), u8.end());  // to std::string (UTF-8)
#else
    return p.string();                         // already UTF-8 on POSIX
#endif
}

	// Once we have projects, change this
	extern const std::filesystem::path g_AssetPath = "Resources";

	ContentBrowserPanel::ContentBrowserPanel()
		: m_CurrentDirectory(g_AssetPath)
	{
		UE_PROFILE_FUNCTION();
		m_DirectoryIcon = Texture2D::Create("Resources/Icons/ContentBrowser/DirectoryIcon.png");
		m_FileIcon = Texture2D::Create("Resources/Icons/ContentBrowser/FileIcon.png");
	}

	void ContentBrowserPanel::OnImGuiRender()
	{
		UE_PROFILE_FUNCTION();
		ImGui::Begin("Content Browser");

		if (m_CurrentDirectory != std::filesystem::path(g_AssetPath))
		{
			if (ImGui::Button("<-"))
			{
				m_CurrentDirectory = m_CurrentDirectory.parent_path();
			}
		}

		static float padding = 0.0f;
		static float thumbnailSize = 52.0f;
		float cellSize = thumbnailSize + padding;

		float panelWidth = ImGui::GetContentRegionAvail().x;
		int columnCount = (int)(panelWidth / cellSize);
		if (columnCount < 1)
			columnCount = 1;

		ImGui::Columns(columnCount, 0, false);

		for (auto& directoryEntry : std::filesystem::directory_iterator(m_CurrentDirectory))
		{
			const auto& path = directoryEntry.path();
			auto relativePath = std::filesystem::relative(path, g_AssetPath);
			std::string filenameString = relativePath.filename().string();
			
			ImGui::PushID(filenameString.c_str());
			Ref<Texture2D> icon = directoryEntry.is_directory() ? m_DirectoryIcon : m_FileIcon;
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
			ImGui::ImageButton(filenameString.c_str(),(ImTextureID)icon->GetRendererID(), { thumbnailSize, thumbnailSize }, { 0, 1 }, { 1, 0 });

                        if (ImGui::BeginDragDropSource()) {
			  std::string itemPathStr = to_utf8(relativePath);
                          // const wchar_t *itemPath = relativePath.c_str();
			  const char* itemPath = itemPathStr.c_str();                          
			  ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath, itemPathStr.size() + 1);
				ImGui::EndDragDropSource();
			}

			ImGui::PopStyleColor();
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (directoryEntry.is_directory())
					m_CurrentDirectory /= path.filename();

			}
			ImGui::TextWrapped(filenameString.c_str());

			ImGui::NextColumn();

			ImGui::PopID();
		}

		ImGui::Columns(1);

		// ImGui::SliderFloat("Thumbnail Size", &thumbnailSize, 16, 512);
		// ImGui::SliderFloat("Padding", &padding, 0, 32);

		// TODO: status bar
		ImGui::End();
	}

}
