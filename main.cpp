#ifndef MEM_CPP
#define MEM_CPP

#include "header.h"
#include <SDL2/SDL.h>
#include <iomanip>

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

char buf[100] = "CPU Usage: 0.00%";
const int UPDATE_INTERVAL = 60;
int counter = 0;


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

    static bool animate = true;


    // Tabs for CPU, Fan, Thermal
    if (ImGui::BeginTabBar("Tabs"))
    {
        static bool showThermalInfo = false;
        static bool showFanInfo = false;
        bool is_thermal_active = false;
        bool is_fan_active = false;

        if (ImGui::BeginTabItem("CPU")) {

            
            static float fps = 30.0f; // Par défaut à 30 FPS
            static int values_offset = 0;
            static double lastTime = 0;

            if (showThermalInfo && is_fan_active == false) {
                // Désactivez les bordures pour la fenêtre "ThermalInfo"
                ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
                ImGui::BeginChild("ThermalInfo", ImVec2(200, 50), true);
                ImGui::Text("Temperature: %d°C", get_cpu_temperature());
                ImGui::EndChild();
                ImGui::PopStyleVar();
                ImGui::Separator();
                is_thermal_active = true;
            }

            if (showFanInfo && is_thermal_active == false) {
                // Désactivez les bordures pour la fenêtre "FanInfo"
                ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 0.0f);
                ImGui::BeginChild("FanInfo", ImVec2(200, 50), true);
                ImGui::Text("Fan status:");
                ImGui::Text("    status : %s", is_fan_enabled());
                ImGui::Text("    level  : %s", get_fan_level());
                ImGui::Text("    Speed  : %d", get_fan_speed());
                ImGui::EndChild();
                ImGui::PopStyleVar(); 
                ImGui::Separator();
                is_fan_active = true;
            }

            ImGui::Checkbox("Animate", &animate);

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
                if (animate) {
                    // Mise à jour du pourcentage du CPU et du tableau de valeurs
                    float currentPercentage = getCPUPercentage();
                    values[values_offset] = currentPercentage;
                    values_offset = (values_offset + 1) % VALUES_COUNT;

                    lastTime = currentTime;
                }
            }

            // Définition de la taille du graphique
            ImVec2 availableSize = ImGui::GetContentRegionAvail();
            ImVec2 graphSize = ImVec2(availableSize.x - 205 , availableSize.y - 20);


            // Sauvegarde de la position du curseur actuel
            ImVec2 currentCursorPos = ImGui::GetCursorPos();

            // Afficher le graphique
            char buf[50];
            snprintf(buf, sizeof(buf), "CPU");
            ImGui::PlotLines(buf, values, VALUES_COUNT, animate ? values_offset : 0, NULL, 0.0f, scaleMax, graphSize);

            // Calcul de la position pour le texte overlay
            ImVec2 overlayPos = ImVec2(currentCursorPos.x + graphSize.x * 0.5f - 70, currentCursorPos.y);
            ImGui::SetCursorPos(overlayPos);

            if (counter % UPDATE_INTERVAL == 0) {
                snprintf(buf, sizeof(buf), "CPU Usage: %.2f%%", values[values_offset]);
            }

            // Utilisez ImGui::Text pour allouer un espace fixe pour le texte.
            // Par exemple, si la longueur maximale du texte est de 20 caractères, réservez un espace pour cela.
            ImGui::Text("%-20s", buf);

            ImGui::EndTabItem();
        }

        if (ImGui::TabItemButton("Fan")) {
            showFanInfo = !showFanInfo;
        }

        if (ImGui::TabItemButton("Thermal")) {
            showThermalInfo = !showThermalInfo;
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

   // RAM
    ImGui::Text("Physical memory (RAM):");
    double usedMemoryGB = getPhysicalMemoryUsedInGB();
    double totalMemoryGB = get_total_ram_memory();
    float usedMemoryPercentage = static_cast<float>(usedMemoryGB / totalMemoryGB);

    // Formate le texte pour afficher seulement 2 décimales
    std::stringstream usedMemoryStream;
    usedMemoryStream << std::fixed << std::setprecision(2) << usedMemoryGB;
    std::string usedMemoryText = usedMemoryStream.str() + " GB";

    // Formate le totalMemoryGB pour afficher seulement 2 décimales
    std::stringstream totalMemoryStream;
    totalMemoryStream << std::fixed << std::setprecision(2) << totalMemoryGB;
    std::string totalMemoryText = totalMemoryStream.str() + " GB";

    // Affiche la barre de progression avec le texte formaté
    ImGui::ProgressBar(usedMemoryPercentage, ImVec2(-1, 0), usedMemoryText.c_str());

    // Affiche le texte "0 GB" à gauche
    ImGui::Text("0 GB");

    // Calcule la position X pour afficher le texte "totalMemoryGB GB" à droite
    float totalMemoryTextWidth = ImGui::CalcTextSize(totalMemoryText.c_str()).x;
    float contentRegionMaxX = ImGui::GetWindowContentRegionMax().x;
    float totalMemoryTextX = contentRegionMaxX - totalMemoryTextWidth;

    // Calcule la position Y pour afficher le texte sur la même ligne
    float cursorPosY = ImGui::GetCursorPosY();
    float textHeight = ImGui::GetTextLineHeightWithSpacing();
    float totalMemoryTextY = cursorPosY - textHeight; // Pour placer le texte sur la même ligne

    // Affiche le texte "totalMemoryGB GB" à droite sur la même ligne
    ImGui::SetCursorPosX(totalMemoryTextX);
    ImGui::SetCursorPosY(totalMemoryTextY);
    ImGui::Text(totalMemoryText.c_str());

    ImGui::Spacing();


    // SWAP
    ImGui::Text("Virtual Memory (SWAP) :");
    double usedSwapGB = getUsedSwapSpaceInGB();
    double totalSwapGB = getSwapSpaceInGB();
    float usedSwapPercentage = static_cast<float>(usedSwapGB / totalSwapGB);

    std::stringstream usedSwapStream;
    usedSwapStream << std::fixed << std::setprecision(2) << usedSwapGB;
    std::string usedSwapText = usedSwapStream.str() + " GB";


    std::stringstream totalSwapStream;
    totalSwapStream << std::fixed << std::setprecision(2) << totalSwapGB;
    std::string totalSwapText = totalSwapStream.str() + " GB";


    ImGui::ProgressBar(usedSwapPercentage, ImVec2(-1, 0), usedSwapText.c_str());

    ImGui::Text("0 GB");
   

    float totalSwapTextWidth = ImGui::CalcTextSize(totalSwapText.c_str()).x;
    float contentSRegionMaxX = ImGui::GetWindowContentRegionMax().x;
    float totalSwapTextX = contentSRegionMaxX - totalSwapTextWidth;

    // Calcule la position Y pour afficher le texte sur la même ligne
    float cursorSPosY = ImGui::GetCursorPosY();
    float textSHeight = ImGui::GetTextLineHeightWithSpacing();
    float totalSwapTextY = cursorSPosY - textSHeight; // Pour placer le texte sur la même ligne

    // Affiche le texte "totalMemoryGB GB" à droite sur la même ligne
    ImGui::SetCursorPosX(totalSwapTextX);
    ImGui::SetCursorPosY(totalSwapTextY);
    ImGui::Text(totalSwapText.c_str());

    ImGui::Spacing();

    // Disk
    std::string path = "/";
    ImGui::Text("Disk :");
    double usedDiskSpaceGB = getUsedDiskSpaceInGB(path);
    double diskSizeGB = getDiskSizeInGB(path);
    float usedDiskPercentage = static_cast<float>(usedDiskSpaceGB / diskSizeGB);

    std::stringstream usedDiskMemoryStream;
    std::stringstream totalDiskMemoryStream;
    usedDiskMemoryStream << std::fixed << std::setprecision(2) << usedDiskSpaceGB;
    totalDiskMemoryStream << std::fixed << std::setprecision(2) << diskSizeGB;
    std::string usedDiskText = usedDiskMemoryStream.str() + " GB";
    std::string totalDiskText = totalDiskMemoryStream.str() + " GB";

    ImGui::ProgressBar(usedDiskPercentage, ImVec2(-1, 0), (usedDiskText).c_str());
    ImGui::Text("0 GB");

    // Calcule la position X pour afficher le texte "totalMemoryGB GB" à droite
    float totalDiskTextWidth = ImGui::CalcTextSize(totalDiskText.c_str()).x;
    float contentDRegionMaxX = ImGui::GetWindowContentRegionMax().x;
    float totalDiskTextX = contentDRegionMaxX - totalDiskTextWidth;

    // Calcule la position Y pour afficher le texte sur la même ligne
    float cursorDPosY = ImGui::GetCursorPosY();
    float textDHeight = ImGui::GetTextLineHeightWithSpacing();
    float totalDiskTextY = cursorDPosY - textDHeight; // Pour placer le texte sur la même ligne

    // Affiche le texte "totalMemoryGB GB" à droite sur la même ligne
    ImGui::SetCursorPosX(totalDiskTextX);
    ImGui::SetCursorPosY(totalDiskTextY);
    ImGui::Text(totalDiskText.c_str());

    ImGui::Separator();

    if (ImGui::CollapsingHeader("Process Table"))
    {
        // The section is expanded when this code block is executed
       // Search Bar
        static char searchBuffer[256];
        ImGui::Text("Filter the process by name :");
        ImGui::InputText("filter", searchBuffer, sizeof(searchBuffer));

        // Process Table
        ImGui::BeginChild("ProcessList", ImVec2(0, 0), true);
        listProcesses(searchBuffer);
        ImGui::EndChild();
    }

    ImGui::End();
}

// network, display information network information
void networkWindow(const char *id, ImVec2 size, ImVec2 position)
{
    ImGui::Begin(id);
    ImGui::SetWindowSize(id, size);
    ImGui::SetWindowPos(id, position);

    const char* lo_address = getIPv4Address("lo");
    const char* wlo1_address = getIPv4Address("wlp1s0");
    const char* docker0_address = getIPv4Address("docker0");

    // student TODO : add code here for the network information
    ImGui::Text("%s", getCurrentDateTimeStr());
    ImGui::Separator();
    ImGui::Text("ip4 network :");
    ImGui::Text("    lo : %s\n", lo_address);
    ImGui::Text("    wlo1 : %s\n", wlo1_address);
    ImGui::Text("    docker0 i: %s\n", docker0_address);
    ImGui::Separator();
    // À l'intérieur de votre boucle de rendu ImGui

    ImVec4 customBlue = ImVec4(0.2f, 0.4f, 0.6f, 1.0f);

    // Appliquez la couleur personnalisée
    ImGui::PushStyleColor(ImGuiCol_Header, customBlue);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(customBlue.x, customBlue.y, customBlue.z + 0.1f, customBlue.w));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, customBlue);

    if (ImGui::CollapsingHeader("Network table")) {
        
        if (ImGui::CollapsingHeader("RX")) {
            if (ImGui::BeginTable("table_rx", 9, ImGuiTableFlags_Borders)) {
                // En-têtes pour l'axe X
                ImGui::TableSetupColumn("interface");
                ImGui::TableSetupColumn("bytes");
                ImGui::TableSetupColumn("packets");
                ImGui::TableSetupColumn("errs");
                ImGui::TableSetupColumn("drop");
                ImGui::TableSetupColumn("fifo");
                ImGui::TableSetupColumn("frame");
                ImGui::TableSetupColumn("compressed");
                ImGui::TableSetupColumn("multicast");
                ImGui::TableHeadersRow();

                // Lignes pour l'axe Y
                const char* rows[] = { "lo", "wlp1s0", "docker0" };
                for (int rowIndex = 0; rowIndex < IM_ARRAYSIZE(rows); rowIndex++) {
                    ImGui::TableNextRow();
                    RX rowData = getRXData(rows[rowIndex]); // Récupérez les données pour l'interface spécifiée
                    for (int colIndex = 0; colIndex < 9; colIndex++) {
                        ImGui::TableSetColumnIndex(colIndex);
                        switch(colIndex) {
                            case 0:
                                ImGui::Text("%s", rows[rowIndex]);
                                break;
                            case 1:
                                ImGui::Text("%d", rowData.bytes);
                                break;
                            case 2:
                                ImGui::Text("%d", rowData.packets);
                                break;
                            case 3:
                                ImGui::Text("%d", rowData.errs);
                                break;
                            case 4:
                                ImGui::Text("%d", rowData.drop);
                                break;
                            case 5:
                                ImGui::Text("%d", rowData.fifo);
                                break;
                            case 6:
                                ImGui::Text("%d", rowData.colls);
                                break;
                            case 7:
                                ImGui::Text("%d", rowData.carrier);
                                break;
                            case 8:
                                ImGui::Text("%d", rowData.compressed);
                                break;
                        }
                    }
                }
                ImGui::EndTable();
            }
        }

        
        if (ImGui::CollapsingHeader("TX")) {
            if (ImGui::BeginTable("table_tx", 9, ImGuiTableFlags_Borders)) {
                // En-têtes pour l'axe X
                ImGui::TableSetupColumn("interface");
                ImGui::TableSetupColumn("bytes");
                ImGui::TableSetupColumn("packets");
                ImGui::TableSetupColumn("errs");
                ImGui::TableSetupColumn("drop");
                ImGui::TableSetupColumn("fifo");
                ImGui::TableSetupColumn("frame");
                ImGui::TableSetupColumn("compressed");
                ImGui::TableSetupColumn("multicast");
                ImGui::TableHeadersRow();

                // Lignes pour l'axe Y
                const char* rows[] = { "lo", "wlp1s0", "docker0" };
                for (int rowIndex = 0; rowIndex < IM_ARRAYSIZE(rows); rowIndex++) {
                    ImGui::TableNextRow();
                    TX rowData = getTXData(rows[rowIndex]); // Récupérez les données pour l'interface spécifiée
                    for (int colIndex = 0; colIndex < 9; colIndex++) {
                        ImGui::TableSetColumnIndex(colIndex);
                        switch(colIndex) {
                            case 0:
                                ImGui::Text("%s", rows[rowIndex]);
                                break;
                            case 1:
                                ImGui::Text("%d", rowData.bytes);
                                break;
                            case 2:
                                ImGui::Text("%d", rowData.packets);
                                break;
                            case 3:
                                ImGui::Text("%d", rowData.errs);
                                break;
                            case 4:
                                ImGui::Text("%d", rowData.drop);
                                break;
                            case 5:
                                ImGui::Text("%d", rowData.fifo);
                                break;
                            case 6:
                                ImGui::Text("%d", rowData.frame);
                                break;
                            case 7:
                                ImGui::Text("%d", rowData.compressed);
                                break;
                            case 8:
                                ImGui::Text("%d", rowData.multicast);
                                break;
                        }
                    }
                }
                ImGui::EndTable();
            }
        }

    }

    ImGui::PopStyleColor(3);

    if (ImGui::BeginTabBar("Tabs"))
    {
        if (ImGui::BeginTabItem("Receive(RX)")) {
            // RX "lo" Data
            ImGui::Text("lo :");
            RX rxData = getRXData("lo");
            double usedRXGB = static_cast<double>(rxData.bytes) / (1024.0 * 1024.0 * 1024.0); // Convert bytes to GB
            double totalRXGB = 2.0; // Total capacity for "lo" is 2 GB
            float usedRXPercentage = static_cast<float>(usedRXGB / totalRXGB);

            // Formate le texte pour afficher seulement 2 décimales
            std::stringstream usedRXStream;
            usedRXStream << std::fixed << std::setprecision(2) << usedRXGB;
            std::string usedRXText = usedRXStream.str() + " GB";

            // Affiche la barre de progression avec le texte formaté
            ImGui::ProgressBar(usedRXPercentage, ImVec2(500, 20), usedRXText.c_str());

            // Affiche le texte "0 GB" à gauche
            ImGui::Text("0 GB");

            // Calcule la position X pour afficher le texte "totalRXGB GB" à droite
            float totalRXTextWidth = ImGui::CalcTextSize((std::to_string(totalRXGB) + " GB").c_str()).x;
            float contentRegionMaxX = ImGui::GetWindowContentRegionMax().x;
            float totalRXTextX = contentRegionMaxX - totalRXTextWidth - 10; // Adjust with a small margin for aesthetics

            // Affiche le texte "totalRXGB GB" à droite
            ImGui::SetCursorPosX(totalRXTextX);
            ImGui::SameLine(); // This ensures that the text is drawn on the same line as the progress bar
            ImGui::Text(("                                                              2 GB"));

            ImGui::Spacing();
            ImGui::Spacing();

            ImGui::Text("wlp1s0 :");
            RX rxDataWlp1s0 = getRXData("wlp1s0");
            double usedRXGBWlp1s0 = static_cast<double>(rxDataWlp1s0.bytes) / (1024.0 * 1024.0 * 1024.0); // Convert bytes to GB
            double totalRXGBWlp1s0 = 2.0; // Total capacity for "wlp1s0" is 2 GB
            float usedRXPercentageWlp1s0 = static_cast<float>(usedRXGBWlp1s0 / totalRXGBWlp1s0);

            // Formate le texte pour afficher seulement 2 décimales
            std::stringstream usedRXStreamWlp1s0;
            usedRXStreamWlp1s0 << std::fixed << std::setprecision(2) << usedRXGBWlp1s0;
            std::string usedRXTextWlp1s0 = usedRXStreamWlp1s0.str() + " GB";

            // Affiche la barre de progression avec le texte formaté
            ImGui::ProgressBar(usedRXPercentageWlp1s0, ImVec2(500, 20), usedRXTextWlp1s0.c_str());

            // Se déplace à la ligne suivante
            ImGui::NewLine();

            // Affiche le texte "0 GB" à gauche
            ImGui::SameLine(0, 0); // 10 pixels d'espacement
            ImGui::Text("0 GB");

            // Calcule la position X pour afficher le texte "totalRXGBWlp1s0 GB" à droite
            float progressBarEndXWlp1s0 = ImGui::GetItemRectMax().x;
            float totalRXTextWidthWlp1s0 = ImGui::CalcTextSize((std::to_string(totalRXGBWlp1s0) + " GB").c_str()).x;
            float totalRXTextXWlp1s0 = progressBarEndXWlp1s0 - totalRXTextWidthWlp1s0; // place le texte juste à la fin de la barre de progression

            // Positionne le curseur pour le texte "2 GB" pour "wlp1s0"
            ImGui::SameLine(totalRXTextXWlp1s0);
            ImGui::Text("                                                                         2 GB");

            ImGui::Spacing();
            ImGui::Spacing();

            // ... [le code précédent pour wlp1s0 ici]

            ImGui::Text("docker0 :");
            RX rxDataDocker0 = getRXData("docker0");
            double usedRXGBDocker0 = static_cast<double>(rxDataDocker0.bytes) / (1024.0 * 1024.0 * 1024.0); // Convert bytes to GB
            double totalRXGBDocker0 = 2.0; // Total capacity for "docker0" is 2 GB
            float usedRXPercentageDocker0 = static_cast<float>(usedRXGBDocker0 / totalRXGBDocker0);

            // Formate le texte pour afficher seulement 2 décimales
            std::stringstream usedRXStreamDocker0;
            usedRXStreamDocker0 << std::fixed << std::setprecision(2) << usedRXGBDocker0;
            std::string usedRXTextDocker0 = usedRXStreamDocker0.str() + " GB";

            // Affiche la barre de progression avec le texte formaté
            ImGui::ProgressBar(usedRXPercentageDocker0, ImVec2(500, 20), usedRXTextDocker0.c_str());

            // Se déplace à la ligne suivante
            ImGui::NewLine();

            // Affiche le texte "0 GB" à gauche
            ImGui::SameLine(0, 0); // pas d'espacement
            ImGui::Text("0 GB");

            // Calcule la position X pour afficher le texte "totalRXGBDocker0 GB" à droite
            float progressBarEndXDocker0 = ImGui::GetItemRectMax().x;
            float totalRXTextWidthDocker0 = ImGui::CalcTextSize((std::to_string(totalRXGBDocker0) + " GB").c_str()).x;
            float totalRXTextXDocker0 = progressBarEndXDocker0 - totalRXTextWidthDocker0; // place le texte juste à la fin de la barre de progression

            // Positionne le curseur pour le texte "2 GB" pour "docker0"
            ImGui::SameLine(totalRXTextXDocker0);
            ImGui::Text("                                                                         2 GB");

            ImGui::Spacing();



            ImGui::EndTabItem();
        }


        if (ImGui::BeginTabItem("Transmit(TX)")) {
                        // TX "lo" Data
            ImGui::Text("lo :");
            TX txData = getTXData("lo");
            double usedTXGB = static_cast<double>(txData.bytes) / (1024.0 * 1024.0 * 1024.0); // Convert bytes to GB
            double totalTXGB = 2.0; // Total capacity for "lo" is 2 GB
            float usedTXPercentage = static_cast<float>(usedTXGB / totalTXGB);

            // Format the text to display only 2 decimals
            std::stringstream usedTXStream;
            usedTXStream << std::fixed << std::setprecision(2) << usedTXGB;
            std::string usedTXText = usedTXStream.str() + " GB";

            // Display the progress bar with the formatted text
            ImGui::ProgressBar(usedTXPercentage, ImVec2(500, 20), usedTXText.c_str());

            // Display the text "0 GB" on the left
            ImGui::Text("0 GB");

            // Calculate the X position to display the text "totalTXGB GB" on the right
            float totalTXTextWidth = ImGui::CalcTextSize((std::to_string(totalTXGB) + " GB").c_str()).x;
            float contentRegionMaxX = ImGui::GetWindowContentRegionMax().x;
            float totalTXTextX = contentRegionMaxX - totalTXTextWidth - 10; // Adjust with a small margin for aesthetics

            // Display the text "totalTXGB GB" on the right
            ImGui::SetCursorPosX(totalTXTextX);
            ImGui::SameLine();
            ImGui::Text("                                                              2 GB");

            ImGui::Spacing();
            ImGui::Spacing();

            // TX for "wlp1s0"
            ImGui::Text("wlp1s0 :");
            TX txDataWlp1s0 = getTXData("wlp1s0");
            double usedTXGBWlp1s0 = static_cast<double>(txDataWlp1s0.bytes) / (1024.0 * 1024.0 * 1024.0); // Convert bytes to GB
            double totalTXGBWlp1s0 = 2.0; // Total capacity for "wlp1s0" is 2 GB
            float usedTXPercentageWlp1s0 = static_cast<float>(usedTXGBWlp1s0 / totalTXGBWlp1s0);

            // Format the text to display only 2 decimals
            std::stringstream usedTXStreamWlp1s0;
            usedTXStreamWlp1s0 << std::fixed << std::setprecision(2) << usedTXGBWlp1s0;
            std::string usedTXTextWlp1s0 = usedTXStreamWlp1s0.str() + " GB";

            // Display the progress bar with the formatted text
            ImGui::ProgressBar(usedTXPercentageWlp1s0, ImVec2(500, 20), usedTXTextWlp1s0.c_str());

            // Move to the next line
            ImGui::NewLine();

            // Display the text "0 GB" on the left
            ImGui::SameLine(0, 0); 
            ImGui::Text("0 GB");

            // Calculate the X position to display the text "totalTXGBWlp1s0 GB" on the right
            float progressBarEndXWlp1s0 = ImGui::GetItemRectMax().x;
            float totalTXTextWidthWlp1s0 = ImGui::CalcTextSize((std::to_string(totalTXGBWlp1s0) + " GB").c_str()).x;
            float totalTXTextXWlp1s0 = progressBarEndXWlp1s0 - totalTXTextWidthWlp1s0;

            // Set the cursor position for the text "2 GB" for "wlp1s0"
            ImGui::SameLine(totalTXTextXWlp1s0);
            ImGui::Text("                                                                         2 GB");

            ImGui::Spacing();
            ImGui::Spacing();

            // TX for "docker0"
            ImGui::Text("docker0 :");
            TX txDataDocker0 = getTXData("docker0");
            double usedTXGBDocker0 = static_cast<double>(txDataDocker0.bytes) / (1024.0 * 1024.0 * 1024.0); // Convert bytes to GB
            double totalTXGBDocker0 = 2.0; // Total capacity for "docker0" is 2 GB
            float usedTXPercentageDocker0 = static_cast<float>(usedTXGBDocker0 / totalTXGBDocker0);

            // Format the text to display only 2 decimals
            std::stringstream usedTXStreamDocker0;
            usedTXStreamDocker0 << std::fixed << std::setprecision(2) << usedTXGBDocker0;
            std::string usedTXTextDocker0 = usedTXStreamDocker0.str() + " GB";

            // Display the progress bar with the formatted text
            ImGui::ProgressBar(usedTXPercentageDocker0, ImVec2(500, 20), usedTXTextDocker0.c_str());

            // Move to the next line
            ImGui::NewLine();

            // Display the text "0 GB" on the left
            ImGui::SameLine(0, 0); 
            ImGui::Text("0 GB");

            // Calculate the X position to display the text "totalTXGBDocker0 GB" on the right
            float progressBarEndXDocker0 = ImGui::GetItemRectMax().x;
            float totalTXTextWidthDocker0 = ImGui::CalcTextSize((std::to_string(totalTXGBDocker0) + " GB").c_str()).x;
            float totalTXTextXDocker0 = progressBarEndXDocker0 - totalTXTextWidthDocker0;

            // Set the cursor position for the text "2 GB" for "docker0"
            ImGui::SameLine(totalTXTextXDocker0);
            ImGui::Text("                                                                         2 GB");

            ImGui::Spacing();

            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }



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

#endif
