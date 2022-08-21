#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

void GetFPS(int& frameCount, double& previousTime);

#define ASSERT(x) if (!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))

static void GLClearError() {
    while (glGetError() != GL_NO_ERROR);
}

static bool GLLogCall(const char* function, const char* file, int line) {
    while (GLenum error = glGetError()) {
        std::cout << "[OpenGL Error] (" << error << ") in file " << file << " on function " << function << " at line " << line << std::endl;
        return false;
    }

    return true;
}

struct ShaderProgramSource{
    std::string VertexSource;
    std::string FragmentSource;
};

static ShaderProgramSource ParseShader(const std::string& filepath) {
    std::ifstream stream(filepath);
    
    enum class ShaderType
    {
        NONE = -1,
        VERTEX = 0,
        FRAGMENT = 1
    };
    ShaderType type = ShaderType::NONE;
    std::string line;
    std::stringstream ss[2];
    while (getline(stream, line)) {
        if (line.find("#shader") != std::string::npos) {
            if (line.find("vertex") != std::string::npos)
                type = ShaderType::VERTEX;
            else if (line.find("fragment") != std::string::npos)
                type = ShaderType::FRAGMENT;
        }
        else {
            ss[(int)type] << line << std::endl;
        }
    }

    return {ss[0].str(), ss[1].str()};
}

static unsigned int CompileShader(unsigned int type, const std::string& source) {
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message  = (char*) alloca(length * sizeof(char));

        glGetShaderInfoLog(id, length, &length, message);

        std::cerr << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << std::endl;
        std::cerr << message << std::endl;
        glDeleteShader(id);

        return 0;
    }

    return id;
}

static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader) {
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

int main(void)
{

    GLFWwindow* window;


    /* Initialize the library */
    if (!glfwInit())
        return -1;


    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }


    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSwapInterval(0);

    if (glewInit() != GLEW_OK) {
        std::cerr << "[ERROR] Error initializing GLEW\n";
        return -1;
    }

    std::cout << glGetString(GL_VERSION) << "\n";

    float vertices[] = {
         0.0f,  0.0f,
         0.5f,  0.0f,
         0.5f,  0.5f,
         0.0f,  0.5f,
    };

    unsigned int indices[] = {
        0, 1, 2,
        0, 3, 2
    };

    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(float), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);

    unsigned int ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW);

    ShaderProgramSource source = ParseShader("res/shaders/basic.shader");
    unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);
    glUseProgram(shader);

    GLCall(int location = glGetUniformLocation(shader, "u_Color"));
    ASSERT(location != -1);
    GLCall(glUniform4f(location, 0.5f, 0.3f, 0.8f, 1.0f));
    
    float r = 0.0f;
    float r_increment = 0.05f;

    float g = 0.0f;
    float g_increment = 0.01f;

    float b = 1.0f;
    float b_increment = 0.3f;

    double previousTime = glfwGetTime();
    int frameCount = 0;
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        GetFPS(frameCount, previousTime);


        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        if (r > 1.0f)
            r_increment = -0.05f;
        else if (r < 0.0f)
            r_increment = 0.05f;

        if (g > 1.0f)
            g_increment = -0.01f;
        else if (g < 0.0f)
            g_increment = 0.01f;

        if (b > 1.0f)
            b_increment = -0.03f;
        else if (b < 0.0f)
            b_increment = 0.03f;
        
        r += r_increment;
        g += g_increment;
        b += b_increment;

        GLCall(glUniform4f(location, r, g, b, 1.0f));
        GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glDeleteProgram(shader);

    glfwTerminate();
    return 0;
}
void GetFPS(int &frameCount, double &previousTime) {
    // Measure speed
    double currentTime = glfwGetTime();
    frameCount++;
    // If a second has passed.
    if (currentTime - previousTime >= 1.0)
    {
        // Display the frame count here any way you want.
        std::cout << "FPS: " << frameCount << std::endl;

        frameCount = 0;
        previousTime = currentTime;
    }
}
