#include "shared.hpp"
#include "Engine.hpp"
#include "UI.hpp"

int main(void)
{
    /* Chane Dir Path */
    std::filesystem::current_path("./assets");

    /* Initialize the library */
    if (!glfwInit()) {
        std::cerr << "Failed to initialize the library" << '\n';
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(800, 800, "Hello World", NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to load windows" << std::endl;
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* Initialize the glad library */
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    Engine engine;

    /* Initialize ImGui */
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 430");

    std::cout << "==================================\n";
    std::cout << "Start Rendering" << '\n';
    /* Loop until the user closes the window */

    float lastTime = glfwGetTime(); // Last time
    float frameTimeCounter = 0.0f;
    unsigned int frames = 0; // Variable to store frames
    
    // [TODO] : Update the frame limit
    // [Refs] : https://stackoverflow.com/questions/57800608/how-to-render-at-a-fixed-fps-in-a-glfw-window
    // [Note] : A variable FPS limit is declared in shared.hpp

    while (!glfwWindowShouldClose(window))
    {
        float currentTime = glfwGetTime();
        float deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        frameTimeCounter += deltaTime;

        /* Main Update */
        glfwGetCursorPos(window, &mouseCursor.first, &mouseCursor.second);
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            onFocus = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        if (!ImGui::GetIO().WantCaptureMouse && glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
            onFocus = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }

        /* Update */
        engine.Update(deltaTime);

        /* Render */
        engine.Render();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();

        frames++;

        if (frameTimeCounter >= 1.0f) {
            std::cout << "\rFrames : " << frames << std::flush;
            frameTimeCounter = 0.0f, frames = 0;
        }
    }

    /* Terminate */
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}