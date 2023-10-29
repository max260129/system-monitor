#include "header.h"
#include <SDL2/SDL.h>

/*
NOTE : You are free to change the code as you wish, the main objective is to make the
       application work and pass the audit.

       It will be provided the main function with the following functions :

       - `void systemWindow(const char *id, ImVec2 size, ImVec2 position)`
            This function will draw the system window on your screen
       - `void memoryProcessesWindow(const char *id, ImVec2 size, ImVec2 position)`
            This function will draw the memory and processes window on your screen
       - `void networkWindow(const char *id, ImVec2 size, ImVec2 position)`
            This function will draw the network window on your screen
*/

// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
//  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
//  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h> // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h> // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h> // Initialize with gladLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
#include <glad/gl.h> // Initialize with gladLoadGL(...) or gladLoaderLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
#define GLFW_INCLUDE_NONE      // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/Binding.h> // Initialize with glbinding::Binding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
#define GLFW_INCLUDE_NONE        // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/glbinding.h> // Initialize with glbinding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

float getCPUPercentage() {
    static long long lastTotalUser, lastTotalUserLow, lastTotalSys, lastTotalIdle;
    float percent;
    FILE* file = fopen("/proc/stat", "r");
    long long totalUser, totalUserLow, totalSys, totalIdle, total;

    fscanf(file, "cpu %lld %lld %lld %lld", &totalUser, &totalUserLow, &totalSys, &totalIdle);
    fclose(file);

    if (totalUser < lastTotalUser || totalUserLow < lastTotalUserLow ||
        totalSys < lastTotalSys || totalIdle < lastTotalIdle){
        // Overflow detection
        percent = -1.0;
    }
    else{
        total = (totalUser - lastTotalUser) + (totalUserLow - lastTotalUserLow) +
                (totalSys - lastTotalSys);
        percent = total;
        total += (totalIdle - lastTotalIdle);
        percent /= total;
        percent *= 100;
    }

    lastTotalUser = totalUser;
    lastTotalUserLow = totalUserLow;
    lastTotalSys = totalSys;
    lastTotalIdle = totalIdle;

    return percent;
}


// systemWindow, display information for the system monitorization
void systemWindow(const char *id, ImVec2 size, ImVec2 position)
{

    static double lastUpdateTime = 0;
    static char buffer[50] = {};

    double currentTime = ImGui::GetTime();
    if (currentTime - lastUpdateTime >= 1.0) {  // Mettre à jour toutes les secondes
        lastUpdateTime = currentTime;
        const char* processCount = NumberofWorking();
        strncpy(buffer, processCount, sizeof(buffer) - 1);
    }

    ImGui::Begin(id);
    ImGui::SetWindowSize(id, size);
    ImGui::SetWindowPos(id, position);

    // student TODO : add code here for the system window

    ImGui::Text("Operating System Used: %s", getOsName());
    ImGui::Text("Computer name: %s", getHostName());
    ImGui::Text("User logged in: %s", getUserName());
    ImGui::Text("CPU: %s", getCPUName());
    ImGui::Text("Number of working processes: %s", buffer);
    ImGui::Separator();


    // GRAPH
    const int VALUES_COUNT = 100;
    static float values[VALUES_COUNT] = { 0 };
    static int values_offset = 0;
    float currentPercentage = getCPUPercentage();
    values[values_offset] = currentPercentage;
    values_offset = (values_offset + 1) % VALUES_COUNT;

    // Tabs for CPU, Fan, Thermal
    if (ImGui::BeginTabBar("Tabs"))
    {
        if (ImGui::BeginTabItem("CPU")) {
            static float fps = 30.0f; // Par défaut à 30 FPS
            static int values_offset = 0;
            static double lastTime = 0;

            // Slider pour contrôler les FPS
            ImGui::SliderFloat("FPS", &fps, 1.0f, 144.0f);

            // Slider pour contrôler l'échelle max de l'axe Y
            static float scaleMax = 100.0f;
            ImGui::SliderFloat("Scale Max", &scaleMax, 0.0f, 100.0f);

            // Logique de mise à jour basée sur le FPS
            double currentTime = ImGui::GetTime();  // Utilise la fonction GetTime() d'ImGui pour obtenir le temps actuel
            double deltaTime = currentTime - lastTime;
            double interval = 1.0 / fps;  // Interval en secondes entre les mises à jour

            if (deltaTime >= interval) {
                // Mise à jour du pourcentage du CPU et du tableau de valeurs
                float currentPercentage = getCPUPercentage();
                values[values_offset] = currentPercentage;
                values_offset = (values_offset + 1) % VALUES_COUNT;

                lastTime = currentTime;
            }

            // Définition de la taille du graphique
            ImVec2 graphSize = ImVec2(400, 200); // Largeur de 400, hauteur de 200

            // Sauvegarde de la position du curseur actuel
            ImVec2 currentCursorPos = ImGui::GetCursorPos();

            // Afficher le graphique
            char buf[50];
            snprintf(buf, sizeof(buf), "CPU");
            ImGui::PlotLines(buf, values, VALUES_COUNT, values_offset, NULL, 0.0f, scaleMax, graphSize);

            // Calcul de la position pour le texte overlay
            ImVec2 overlayPos = ImVec2(currentCursorPos.x + graphSize.x * 0.5f - 70, currentCursorPos.y);
            ImGui::SetCursorPos(overlayPos);

            // Affichage du texte en superposition
            snprintf(buf, sizeof(buf), "CPU Usage: %.2f%%", values[values_offset]);
            ImGui::TextUnformatted(buf);


            ImGui::EndTabItem();
        }




        if (ImGui::BeginTabItem("Fan"))
        {
            // TODO: Content for Fan Tab
            ImGui::Text("This is the Fan Tab");
            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Thermal"))
        {
            // TODO: Content for Thermal Tab
            ImGui::Text("This is the Thermal Tab");
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    } 

    // End of example code

    ImGui::End();
}

// memoryProcessesWindow, display information for the memory and processes information
void memoryProcessesWindow(const char *id, ImVec2 size, ImVec2 position)
{
    ImGui::Begin(id);
    ImGui::SetWindowSize(id, size);
    ImGui::SetWindowPos(id, position);

    // student TODO : add code here for the memory and process information

    ImGui::End();
}

// network, display information network information
void networkWindow(const char *id, ImVec2 size, ImVec2 position)
{
    ImGui::Begin(id);
    ImGui::SetWindowSize(id, size);
    ImGui::SetWindowPos(id, position);

    // student TODO : add code here for the network information

    ImGui::End();
}

// Main code
int main(int, char **)
{
    // Setup SDL
    // (Some versions of SDL before <2.0.10 appears to have performance/stalling issues on a minority of Windows systems,
    // depending on whether SDL_INIT_GAMECONTROLLER is enabled or disabled.. updating to latest version of SDL is recommended!)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // GL 3.0 + GLSL 130
    const char *glsl_version = "#version 130";
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, 0);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    SDL_Window *window = SDL_CreateWindow("System-Monitor ML + DV", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    bool err = gladLoadGL() == 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
    bool err = gladLoadGL((GLADloadfunc)SDL_GL_GetProcAddress) == 0; // glad2 recommend using the windowing library loader instead of the (optionally) bundled one.
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
    bool err = false;
    glbinding::Binding::initialize();
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
    bool err = false;
    glbinding::initialize([](const char *name) { return (glbinding::ProcAddress)SDL_GL_GetProcAddress(name); });
#else
    bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    // render bindings
    ImGuiIO &io = ImGui::GetIO();

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // background color
    // note : you are free to change the style of the application
    ImVec4 clear_color = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(window);
        ImGui::NewFrame();

        {
            ImVec2 mainDisplay = io.DisplaySize;
            memoryProcessesWindow("== Memory and Processes ==",
                                  ImVec2((mainDisplay.x / 2) - 20, (mainDisplay.y / 2) + 30),
                                  ImVec2((mainDisplay.x / 2) + 10, 10));
            // --------------------------------------
            systemWindow("== System ==",
                         ImVec2((mainDisplay.x / 2) - 10, (mainDisplay.y / 2) + 30),
                         ImVec2(10, 10));
            // --------------------------------------
            networkWindow("== Network ==",
                          ImVec2(mainDisplay.x - 20, (mainDisplay.y / 2) - 60),
                          ImVec2(10, (mainDisplay.y / 2) + 50));
        }

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
