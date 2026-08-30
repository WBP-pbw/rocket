
#include <fstream>
#include <cstddef>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <tiny_obj_loader.h>
#include <iostream>

using namespace std;

//мышка
struct MouseState
{
    double lastX = 0.0;
    double lastY = 0.0;

    bool firstMouse = true;
    bool rotating = false;
    bool panning = false;
};

//камера
struct Camera
{
    glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);

    float distance = 10.0f;
    float yaw = 0.0f;
    float pitch = 0.0f;
};

// Нужно чтоб он был доступен везде
enum class EngineState {
    Off,
    On,
    Flight
};

struct RocketState
{
    float x, y, z;
    float velocity;
    bool stage1;
    bool stage2;
    EngineState engineStatus; // Создание класса двигатель внутри структуры
};

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
};

Camera camera;
MouseState mouse;

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (!mouse.rotating && !mouse.panning) {
        mouse.firstMouse = true;
        return;

    };

    if (mouse.firstMouse) {
        mouse.lastX = xpos;
        mouse.lastY = ypos;
        mouse.firstMouse = false;
    };
    float xOffset = static_cast<float>(xpos - mouse.lastX);
    float yOffset = static_cast<float>(mouse.lastY - ypos);

    mouse.lastX = xpos;
    mouse.lastY = ypos;
    if (mouse.rotating) {
        const float sensivity = 0.15f;

        camera.yaw += xOffset * sensivity;
        camera.pitch += yOffset * sensivity;
        camera.pitch = glm::clamp(camera.pitch, -89.0f, 89.0f);
    }
    if (mouse.panning) {
        float panSpeed = 0.01f * camera.distance;
        float yawRad = glm::radians(camera.yaw);
        float pitchRad = glm::radians(camera.pitch);

        glm::vec3 forward;
        forward.x = cos(pitchRad) * cos(yawRad);
        forward.y = sin(pitchRad);
        forward.z = cos(pitchRad) * sin(yawRad);
        
        glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
        glm::vec3 right = glm::normalize(glm::cross(forward, worldUp));
        glm::vec3 up = glm::normalize(glm::cross(right, forward));

        camera.target += right * xOffset * panSpeed;
        camera.target -= up * yOffset * panSpeed;
    }
}
// колесико
void scrollCallback(GLFWwindow* window, double xOffset, double yOffset) {
    camera.distance -= static_cast<float>(yOffset);

    if (camera.distance < 2.0f)
        camera.distance = 2.0f;
    if (camera.distance > 50.0f)
        camera.distance = 50.0f;
};

int main() {
    // Инициализация GLFW
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW!" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Создание окна размером 800x600
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "Rocket", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create GLFW window!" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);

    // Делаем контекст окна текущим
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout <<"failed to inicilize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }
    glEnable(GL_DEPTH_TEST);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    std::string inputfile = "models\\main.obj";

    std::ifstream testFile(inputfile);

    if (!testFile.is_open())
    {
        std::cout << "FILE DOES NOT EXIST: "
            << inputfile
            << std::endl;
        return -1;
    }

    std::cout << "FILE EXISTS!" << std::endl;

    tinyobj::ObjReader reader;


    if (!reader.ParseFromFile(inputfile))
    {
        std::cout << "Failed to load OBJ:\n "
            << reader.Error()
            << std::endl;

        return -1;
    }
    
    const auto& attrib = reader.GetAttrib();
    const auto& shapes = reader.GetShapes();


    std::cout << "Shapes: " << shapes.size() << endl;
    if (shapes.empty()) {
        std::cout << "OBJ contains no shapes!" << std::endl;
        return -1;
    }
    for (size_t i = 0; i < shapes.size(); ++i) {
        std::cout << "Shape " << i
            << ": "
            << shapes[i].name
            << ", indices = "
            << shapes[i].mesh.indices.size()
            << '\n';
    }
   
    // Наш массив
    std::vector<Vertex> vertices;

    for (const auto& shape : shapes) {
        const auto& mesh = shape.mesh;
        size_t indexOffset = 0;

        for (size_t face = 0; face < mesh.num_face_vertices.size(); ++face) {
            int vertexCount = mesh.num_face_vertices[face];
            const auto& index0 = mesh.indices[indexOffset];

            for (int i = 1; i < vertexCount - 1; ++i) {
                const auto& index1 = mesh.indices[indexOffset + i];
                const auto& index2 = mesh.indices[indexOffset + i + 1];

                Vertex vertex0;
                Vertex vertex1;
                Vertex vertex2;

                // Первая вершина

                vertex0.position = glm::vec3(
                    attrib.vertices[index0.vertex_index * 3 + 0],
                    attrib.vertices[index0.vertex_index * 3 + 1],
                    attrib.vertices[index0.vertex_index * 3 + 2]
                );
                if (index0.normal_index >= 0) {
                    vertex0.normal = glm::vec3(
                        attrib.normals[index0.normal_index * 3 + 0],
                        attrib.normals[index0.normal_index * 3 + 1],
                        attrib.normals[index0.normal_index * 3 + 2]
                    );
                }
                else {
                    vertex0.normal = glm::vec3(0.0f, 1.0f, 0.0f);
                }

                // Вторая вершина

                vertex1.position = glm::vec3(
                    attrib.vertices[index1.vertex_index * 3 + 0],
                    attrib.vertices[index1.vertex_index * 3 + 1],
                    attrib.vertices[index1.vertex_index * 3 + 2]
                );
                if (index1.normal_index >= 0) {
                    vertex1.normal = glm::vec3(
                        attrib.normals[index1.normal_index * 3 + 0],
                        attrib.normals[index1.normal_index * 3 + 1],
                        attrib.normals[index1.normal_index * 3 + 2]
                    );
                }
                else {
                    vertex1.normal = glm::vec3(0.0f, 1.0f, 0.0f);
                }

                //третья вершина

                vertex2.position = glm::vec3(
                    attrib.vertices[index2.vertex_index * 3 + 0],
                    attrib.vertices[index2.vertex_index * 3 + 1],
                    attrib.vertices[index2.vertex_index * 3 + 2]
                );
                if (index2.normal_index >= 0) {
                    vertex2.normal = glm::vec3(
                        attrib.normals[index2.normal_index * 3 + 0],
                        attrib.normals[index2.normal_index * 3 + 1],
                        attrib.normals[index2.normal_index * 3 + 2]
                    );
                }
                else {
                    vertex2.normal = glm::vec3(0.0f, 1.0f, 0.0f);
                }
                vertices.push_back(vertex0);
                vertices.push_back(vertex1);
                vertices.push_back(vertex2);
            }

            indexOffset += vertexCount;
        }
    }


    // Статус для ракеты
    RocketState Rstate = { 0.0f, 10.0f, 0.0f, 0.0f, false, false, EngineState::On};

    //VAO, VBO
	unsigned int VAO; // Vertex Array Object
    glGenVertexArrays(1, &VAO);
    unsigned int VBO; // Vertex Buffer Object
    glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

	const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;
        out vec3 Normal;
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;

        void main()
        {
            Normal = mat3(transpose(inverse(model))) * aNormal;
            gl_Position = projection * view * model * vec4(aPos, 1.0);
        }
    )";
	const char* fragmentShaderSource = R"(
        #version 330 core
        in vec3 Normal;
        out vec4 FragColor;
        uniform vec3 lightDirection;
        void main() {
            vec3 normal = normalize(Normal);
            vec3 light = normalize(-lightDirection);
            float diffuse = max(dot(normal, light), 0.0);
            vec3 baseColor = vec3(0.7, 0.7, 0.7);
            vec3 finalColor = baseColor * (0.2 + 0.8 * diffuse);
            FragColor = vec4(finalColor, 1.0); 
        }
    )";

	unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);

	unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);

	unsigned int shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

    int modelLoc = glGetUniformLocation(shaderProgram, "model");
    int viewLoc = glGetUniformLocation(shaderProgram, "view");
    int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    int lightDirLoc = glGetUniformLocation(shaderProgram, "lightDirection");

    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);

    projection = glm::perspective(
        glm::radians(45.0f),
        1920.0f / 1080.0f,
        0.1f,
        100.0f
    );

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Цикл обработки событий и отрисовки
    while (!glfwWindowShouldClose(window)) {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glfwPollEvents();

        mouse.rotating = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
        mouse.panning = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
        if (!mouse.rotating && mouse.panning) {
            mouse.firstMouse = true;
        };

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glUseProgram(shaderProgram);

        float yawRad = glm::radians(camera.yaw);
        float pitchRad = glm::radians(camera.pitch);

        glm::vec3 cameraPosition;

        cameraPosition.x = camera.target.x + camera.distance * cos(pitchRad) * cos(yawRad);
        cameraPosition.y = camera.target.y + camera.distance * sin(pitchRad);
        cameraPosition.z = camera.target.z + camera.distance * cos(pitchRad) * sin(yawRad);

        view = glm::lookAt(
            cameraPosition, camera.target, glm::vec3(0.0f, 1.0f, 0.0f)
        );

        glUniformMatrix4fv(
            modelLoc,
            1,
            GL_FALSE,
            glm::value_ptr(model)
        );

        glUniformMatrix4fv(viewLoc,1,GL_FALSE,glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

		glBindVertexArray(VAO);
        glUniform3f(lightDirLoc, -0.5f, -1.0f, -0.5f);
		glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertices.size()));

        glfwSwapBuffers(window);
    }

    // удаление шейдера
    glDeleteProgram(shaderProgram);

    // Завершение работы
    glfwTerminate();
    return 0;
}