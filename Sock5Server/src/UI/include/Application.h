#pragma once
#include <winsock2.h>
#include <Windows.h>
#include <string>
#include <imgui.h>
#include <imgui_internal.h>

namespace Application {

	enum ClientItemColumnID {
		ClientItemColumnID_ID,
		ClientItemColumnID_IP,
		ClientItemColumnID_Name,
		ClientItemColumnID_ActiveSock
	};

	struct ClientItem {
		SOCKET Sock;
		std::string IP;
		std::string Name;

		static const ImGuiTableSortSpecs* s_current_sort_specs;
		// Compare function to be used by qsort()
		static int IMGUI_CDECL CompareWithSortSpecs(const void* lhs, const void* rhs) {
			const auto* a = (const ClientItem*)lhs;
			const auto* b = (const ClientItem*)rhs;
			for (int n = 0; n < s_current_sort_specs->SpecsCount; n++) {
				// Here we identify columns using the ColumnUserID value that we ourselves passed to TableSetupColumn()
				// We could also choose to identify columns based on their index (sort_spec->ColumnIndex), which is simpler!
				const ImGuiTableColumnSortSpecs* sort_spec = &s_current_sort_specs->Specs[n];
				int delta = 0;
				switch (sort_spec->ColumnUserID) {
				case ClientItemColumnID_ID:
					delta = (int)(a->Sock - b->Sock);
					break;
				case ClientItemColumnID_IP:
					delta = (strcmp(a->IP.c_str(), b->IP.c_str()));
					break;
				case ClientItemColumnID_Name:
					delta = (strcmp(a->Name.c_str(), b->Name.c_str()));
					break;
				default:
					IM_ASSERT(0);
					break;
				}
				if (delta > 0)
					return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? +1 : -1;
				if (delta < 0)
					return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? -1 : +1;
			}
			return (int)(a->Sock - b->Sock);
		}
	};

	extern ImVector<ClientItem> Clients;
	extern WORD C2Port, TunnelPort, Sock5Port;
	extern bool ServerThreadTerminated;
	extern bool C2ServerThreadTerminated;
	extern int ServerStatus;
	extern SOCKET Sock5Sock, TunnelSock, C2Sock, CurrentProxy;
	extern std::string DefaultFile;

	void Init(ImGuiIO& io);
	void UIRender();

	void PreDestroy();
	void Destroy();
}
