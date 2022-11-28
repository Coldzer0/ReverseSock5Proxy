#define NOMINMAX
#include <filesystem>
#include "UI/GUI_backend.h"
#include "Application.h"
#include "ConfigIni.h"

bool Closed = false;
bool PreparingClose = false;
const char *GUI_CONFIG_FILE_NAME = "ServerGUICfg.ini";

void CloseApplication() {
  Closed = true;
}

void CancelCloseApplication() {
  Closed = false;
  PreparingClose = false;
}

void SaveDefaultConfig() {
  std::string defaultFileContent = R"(
    [Window]
    Maximized=0
 )";

  std::ofstream file;
  file.open(ConfigIni::CONFIG_FILE_NAME, std::ios_base::out);
  file << defaultFileContent;
  file.close();
}

void window_maximize_callback([[maybe_unused]] GLFWwindow *window, int maximized) {
  ConfigIni::SetInt("Window", "Maximized", maximized);
}

bool glfwSetWindowCenter(GLFWwindow *window) {
  if (!window)
    return false;

  int sx = 0, sy = 0;
  int px = 0, py = 0;
  int mx = 0, my = 0;
  int monitor_count = 0;
  int best_area = 0;
  int final_x = 0, final_y = 0;

  glfwGetWindowSize(window, &sx, &sy);
  glfwGetWindowPos(window, &px, &py);

  // Iterate through all monitors
  GLFWmonitor **m = glfwGetMonitors(&monitor_count);
  if (!m)
    return false;

  for (int j = 0; j < monitor_count; ++j) {

    glfwGetMonitorPos(m[j], &mx, &my);
    const GLFWvidmode *mode = glfwGetVideoMode(m[j]);
    if (!mode)
      continue;

    // Get intersection of two rectangles - screen and window
    int minX = std::max(mx, px);
    int minY = std::max(my, py);

    int maxX = std::min(mx + mode->width, px + sx);
    int maxY = std::min(my + mode->height, py + sy);

    // Calculate area of the intersection
    int area = std::max(maxX - minX, 0) * std::max(maxY - minY, 0);

    // If it's bigger than actual (window covers more space on this monitor)
    if (area > best_area) {
      // Calculate proper position in this monitor
      final_x = mx + (mode->width - sx) / 2;
      final_y = my + (mode->height - sy) / 2;

      best_area = area;
    }

  }

  // We found something
  if (best_area)
    glfwSetWindowPos(window, final_x, final_y);

    // Something is wrong - current window has NOT any intersection with any monitors. Move it to the default one.
  else {
    GLFWmonitor *primary = glfwGetPrimaryMonitor();
    if (primary) {
      const GLFWvidmode *desktop = glfwGetVideoMode(primary);

      if (desktop)
        glfwSetWindowPos(window, (desktop->width - sx) / 2, (desktop->height - sy) / 2);
      else
        return false;
    } else
      return false;
  }

  return true;
}

int InitServerGui() {

  if (!std::filesystem::exists(ConfigIni::CONFIG_FILE_NAME)) {
    SaveDefaultConfig();
  }

  ConfigIni::Init();


  // Setup GLFW
  glfwSetErrorCallback([](int error, const char *description) {
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
  });
  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialize GLFW.\n");
    return 1;
  }

  // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
  // GL ES 2.0 + GLSL 100
  const char* glsl_version = "#version 100";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
  // GL 3.2 + GLSL 150
  const char* glsl_version = "#version 150";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
  // GL 3.2 + GLSL 150
  const char *glsl_version = "#version 150";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

  // Start the Window Hidden so we position it first.
  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
//  glfwWindowHint(GLFW_DECORATED, GL_FALSE); // Hide title bar

  int width = 1280;// could declare them as "const int" if you like
  int height = 720;
  // Create window with graphics context
  GLFWwindow *window = glfwCreateWindow(width, height, "Coldroot Sock5 Server", nullptr, nullptr);
  if (window == nullptr) {
    fprintf(stderr, "Failed to create GLFW window.\n");
    return 1;
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // 1 = Enable vsync

  // Put the Window in the center of the screen.
  glfwSetWindowCenter(window);

  // Handle the Maximize Window State.
  glfwSetWindowMaximizeCallback(window, window_maximize_callback);
  if (ConfigIni::GetInt("Window", "Maximized", 0)) {
    glfwMaximizeWindow(window);
  }

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();

  ImGuiIO &io = ImGui::GetIO();
  (void) io;
  io.IniFilename = GUI_CONFIG_FILE_NAME;

  io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
#ifdef IMGUI_HAS_DOCK
  io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
  io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
#endif

  //io.ConfigViewportsNoAutoMerge = true;
  //io.ConfigViewportsNoTaskBarIcon = true;

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  //ImGui::StyleColorsClassic();

#ifdef IMGUI_HAS_DOCK
  // When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
  ImGuiStyle &style = ImGui::GetStyle();
  if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
    style.WindowRounding = 0.0f;
    style.Colors[ImGuiCol_WindowBg].w = 1.0f;
  }
#endif

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init(glsl_version);

  // Our Application Init. ex: Load Fonts, Style, Loading the Ports & Init Socks etc...
  Application::Init(io);

  // Our state
  ImVec4 clear_color = ImVec4(0.249f, 0.249f, 0.249f, 0.940f);

  // Show the Window.
  glfwShowWindow(window);

  // Main loop
  while (!Closed) {
    if (glfwWindowShouldClose(window) && !PreparingClose) {
      Application::PreDestroy();
      glfwSetWindowShouldClose(window, GLFW_FALSE);
      PreparingClose = true;
    }
    // Poll and handle events (inputs, window resize, etc.)
    glfwPollEvents();

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    //========== Our UI Rendering ============//

    Application::UIRender();

    //========================================//

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
#ifdef IMGUI_HAS_DOCK
    // Update and Render additional Platform Windows
    // (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
    //  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
    if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
      GLFWwindow *backup_current_context = glfwGetCurrentContext();
      ImGui::UpdatePlatformWindows();
      ImGui::RenderPlatformWindowsDefault();
      glfwMakeContextCurrent(backup_current_context);
    }
#endif
    glfwSwapBuffers(window);
    //glFinish();
  }

  // Application Cleanup
  Application::Destroy();

  // GUI Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}