#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <imgui/backends/imgui_impl_glfw.h>

#include "XEngineCore/Event.hpp"
#include "XEngineCore/Window.hpp"
#include "XEngineCore/Log.hpp"
#include "XEngineCore/Rendering/OpenGL/ShaderProgram.hpp"
#include "XEngineCore/Rendering/OpenGL/VertexBuffer.hpp"

namespace XEngine {

    static bool glfwInitialized = false;

    GLfloat points[] = {
        0.0f, 0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        -0.5f, -0.5f, 0.0f
    };

    GLfloat colors[] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f
    };

    const char* vertexShader =
        "#version 460\n"
        "layout(location = 0) in vec3 vertex_position;"
        "layout(location = 1) in vec3 vertex_color;"
        "out vec3 color;"
        "void main() {"
        "   color = vertex_color;"
        "   gl_Position = vec4(vertex_position, 1.0);"
        "}";

    const char* fragmentShader =
        "#version 460\n"
        "in vec3 color;"
        "out vec4 frag_color;"
        "void main() {"
        "   frag_color = vec4(color, 1.0);"
        "}";

    std::unique_ptr<ShaderProgram> shaderProgram;
    std::unique_ptr<VertexBuffer> pointsVBO;
    std::unique_ptr<VertexBuffer> colorsVBO;
    GLuint vao;

	Window::Window(std::string title, const unsigned int width, const unsigned int height)
		: w_data({std::move(title), width, height}) {
        //Initialize basis.
		int result = initialize();
        //Initialize ImGui.
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui_ImplOpenGL3_Init();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
	}
	Window::~Window() {
		shutdown();
	}

    int Window::initialize() {

        LOG_INFO("Creating instance of window '{0}', with size of {1}x{2}.", w_data.title, w_data.width, w_data.height);

        //Initialize the library.
        if (!glfwInitialized) {
            if (!glfwInit()) {
                LOG_CRIT("Error while trying to initialize GLFW (OpenGL).");
                return -100;
            }
            glfwInitialized = true;
        }

        //Create a windowed mode window and its OpenGL context.
        window = glfwCreateWindow(w_data.width, w_data.height, w_data.title.c_str(), nullptr, nullptr);
        if (!window) {
            LOG_CRIT("Error while trying to initialize window '{0}'.", w_data.title);
            glfwTerminate();
            return -101;
        }

        //Make the window's context current.
        glfwMakeContextCurrent(window);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            LOG_CRIT("Failed to initialize GLAD (OpenGL).");
            return -102;
        }

        //Handel events.
        glfwSetWindowUserPointer(window, &w_data);
        glfwSetWindowSizeCallback(window, [](GLFWwindow* pWindow, int width, int height) {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(pWindow));
            data.width = width;
            data.height = height;
            EventWindowResize event(width, height);
            data.eventCallbackFn(event);
        });
        glfwSetCursorPosCallback(window, [](GLFWwindow* pWindow, double x, double y) {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(pWindow));
            EventMouseMove event(x, y);
            data.eventCallbackFn(event);
        });
        glfwSetWindowCloseCallback(window, [](GLFWwindow* pWindow) {
            WindowData& data = *static_cast<WindowData*>(glfwGetWindowUserPointer(pWindow));
            EventWindowClose event;
            data.eventCallbackFn(event);
            LOG_INFO("Window '{0}' closed.", data.title);
        });

        glfwSetFramebufferSizeCallback(window, [](GLFWwindow* pWindow, int width, int height) {
            glViewport(0, 0, width, height);
        });

        //Instance all data to draw triangle.
        //Create shader program.
        shaderProgram = std::make_unique<ShaderProgram>(vertexShader, fragmentShader);
        if (!shaderProgram->isCompilied()) {
            LOG_CRIT("Error while compiling main shader.");
            return -110;
        }
        //Vertex buffers.
        pointsVBO = std::make_unique<VertexBuffer>(points, sizeof(points));
        colorsVBO = std::make_unique<VertexBuffer>(colors, sizeof(colors));
        //Vertex array object.
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);
        //Link position to buffer.
        glEnableVertexAttribArray(0);
        pointsVBO->bind();
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
        //Link color to buffer.
        glEnableVertexAttribArray(1);
        colorsVBO->bind();
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

        return 0;

	}
	void Window::shutdown() {
        glfwDestroyWindow(window);
        glfwTerminate();
	}
	void Window::update() {

        //Clear color buffer.
        glClearColor(bgColor[0], bgColor[1], bgColor[2], bgColor[3]);
        glClear(GL_COLOR_BUFFER_BIT);

        //Render triangle.
        shaderProgram->bind();
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        //Update InGui values.
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize.x = static_cast<float>(getWidth());
        io.DisplaySize.y = static_cast<float>(getHeight());
        //Create new frame for ImGui.
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        //Draw ImGui.
        
        //Demo window.
        ImGui::ShowDemoWindow();
        //XEngine Testing Window.
        ImGui::Begin("XEngine Testing Window");
        ImGui::Text("General Testing");
        ImGui::ColorEdit4("Background Color", bgColor);
        ImGui::End();

        //Render ImGui.
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        //Swap front and back buffers.
        glfwSwapBuffers(window);
        //Poll for and process events.
        glfwPollEvents();

	}

}