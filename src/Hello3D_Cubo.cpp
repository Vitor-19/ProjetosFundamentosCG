/* Hello Triangle - código adaptado de https://learnopengl.com/#!Getting-started/Hello-Triangle
 *
 * Adaptado por Rossana Baptista Queiroz
 * para as disciplinas de Processamento Gráfico/Computação Gráfica - Unisinos
 * Versão inicial: 7/4/2017
 * Última atualização em 07/03/2025
 */

#include <iostream>
#include <string>
#include <assert.h>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>


// Protótipo da função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Protótipos das funções
int setupShader();
int setupGeometry();

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 1000, HEIGHT = 1000;

// Código fonte do Vertex Shader (em GLSL): ainda hardcoded
const GLchar* vertexShaderSource = "#version 450\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec3 color;\n"
"uniform mat4 model;\n"
"out vec4 finalColor;\n"
"void main()\n"
"{\n"
//...pode ter mais linhas de código aqui!
"gl_Position = model * vec4(position, 1.0);\n"
"finalColor = vec4(color, 1.0);\n"
"}\0";

//Códifo fonte do Fragment Shader (em GLSL): ainda hardcoded
const GLchar* fragmentShaderSource = "#version 450\n"
"in vec4 finalColor;\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"color = finalColor;\n"
"}\n\0";

bool rotateX=false, rotateY=false, rotateZ=false;
// Var para controle de movimento
float posX = 0.0f, posY = 0.0f, posZ = 0.0f, scale = 0.2f;

// Função MAIN
int main()
{
    // Inicialização da GLFW
    glfwInit();

    // Criação da janela GLFW
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Ola 3D -- João Vitor de Mello Lima!", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // Fazendo o registro da função de callback para a janela GLFW
    glfwSetKeyCallback(window, key_callback);

    // GLAD: carrega todos os ponteiros d funções da OpenGL
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
    }

    // Obtendo as informações de versão
    const GLubyte* renderer = glGetString(GL_RENDERER); /* get renderer string */
    const GLubyte* version = glGetString(GL_VERSION); /* version as a string */
    cout << "Renderer: " << renderer << endl;
    cout << "OpenGL version supported " << version << endl;

    // Definindo as dimensões da viewport com as mesmas dimensões da janela da aplicação
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    // Compilando e buildando o programa de shader
    GLuint shaderID = setupShader();

    // Gerando um buffer simples, com a geometria de um triângulo (agora cubo)
    GLuint VAO = setupGeometry();

    glUseProgram(shaderID);

    GLint modelLoc = glGetUniformLocation(shaderID, "model");

    glEnable(GL_DEPTH_TEST);

    // Array com as posições dos múltiplos cubos no espaço 3D
    glm::vec3 posicoesDosCubos[] = {
        glm::vec3( 0.0f,  0.0f,  0.0f), // Cubo 1 
        glm::vec3( 0.6f,  0.6f, 0.0f), // Cubo 2
        glm::vec3(-0.6f, 0.6f, 0.0f), // Cubo 3
    };
	// Automatizar o num do for de criação de cubos
	int numCubos = std::size(posicoesDosCubos);
    // Loop da aplicação - "game loop"
    while (!glfwWindowShouldClose(window))
    {
        // Checa se houveram eventos de input e chama as funções de callback
        glfwPollEvents();

        // Limpa o buffer de cor e de profundidade
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); //cor de fundo
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glLineWidth(10);
        glPointSize(20);

        float angle = (GLfloat)glfwGetTime();

        // Vincula o VAO com a geometria do cubo uma única vez por frame
        glBindVertexArray(VAO);

        // Desenha todos os cubos
        for(unsigned int i = 0; i < numCubos; i++)
        {
            glm::mat4 model = glm::mat4(1.0f); 
            
            // Translação base (movimento da câmera/mundo via teclado)
            model = glm::translate(model, glm::vec3(posX, posY, posZ));
            
            // Translação específica para posicionar cada cubo no seu devido lugar
            model = glm::translate(model, posicoesDosCubos[i]);

            // Escala geral
            model = glm::scale(model, glm::vec3(scale));

            // Rotação (cada cubo com um ângulo ligeiramente diferente para efeito visual)
            float anguloDiferente = angle + (i * 0.5f); 
            if (rotateX)
                model = glm::rotate(model, anguloDiferente, glm::vec3(1.0f, 0.0f, 0.0f));
            else if (rotateY)
                model = glm::rotate(model, anguloDiferente, glm::vec3(0.0f, 1.0f, 0.0f));
            else if (rotateZ)
                model = glm::rotate(model, anguloDiferente, glm::vec3(0.0f, 0.0f, 1.0f));

            // Envia a matriz model atualizada para o Shader
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            // Desenha as faces do cubo
            glDrawArrays(GL_TRIANGLES, 0, 36);

            // Desenha os pontos de contorno
            glDrawArrays(GL_POINTS, 0, 18);
        }

        // Desvincula o VAO
        glBindVertexArray(0);

        // Troca os buffers da tela
        glfwSwapBuffers(window);
    }
    // Pede pra OpenGL desalocar os buffers
    glDeleteVertexArrays(1, &VAO);
    // Finaliza a execução da GLFW
    glfwTerminate();
    return 0;
}

// Função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (key == GLFW_KEY_X && action == GLFW_PRESS)
    {
        rotateX = true;
        rotateY = false;
        rotateZ = false;
    }

    if (key == GLFW_KEY_Y && action == GLFW_PRESS)
    {
        rotateX = false;
        rotateY = true;
        rotateZ = false;
    }

    if (key == GLFW_KEY_Z && action == GLFW_PRESS)
    {
        rotateX = false;
        rotateY = false;
        rotateZ = true;
    }
    //Movimento com WASD + IJ
    if (key == GLFW_KEY_W) posZ -= 0.1f;
    if (key == GLFW_KEY_S) posZ += 0.1f;
    if (key == GLFW_KEY_A) posX -= 0.1f;
    if (key == GLFW_KEY_D) posX += 0.1f;

    if (key == GLFW_KEY_I) posY += 0.1f;
    if (key == GLFW_KEY_J) posY -= 0.1f;

    //Escala com '-' e '+' ou 'n' e 'm'
    if (key == GLFW_KEY_KP_SUBTRACT || key == GLFW_KEY_N) scale -= 0.01f;
    if (key == GLFW_KEY_KP_ADD || key == GLFW_KEY_M) scale += 0.01f;
}

// Função para compilar os Shaders
int setupShader()
{
    // Vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    
    // Fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    
    // Linkando os shaders
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

// Função para criar o VAO/VBO do cubo
int setupGeometry()
{
    GLfloat vertices[] = {

    // ===== Face Frontal =====
    //x    y    z    r    g    b
    -0.5, -0.5,  0.5, 1,0,0,
     0.5, -0.5,  0.5, 1,0,0,
     0.5,  0.5,  0.5, 1,0,0,

    -0.5, -0.5,  0.5, 1,0,0,
     0.5,  0.5,  0.5, 1,0,0,
    -0.5,  0.5,  0.5, 1,0,0,

    // ===== Face Traseira =====
    -0.5, -0.5, -0.5, 0,1,0,
     0.5,  0.5, -0.5, 0,1,0,
     0.5, -0.5, -0.5, 0,1,0,

    -0.5, -0.5, -0.5, 0,1,0,
    -0.5,  0.5, -0.5, 0,1,0,
     0.5,  0.5, -0.5, 0,1,0,

    // ===== Face Lateral Esquerda =====
    -0.5, -0.5, -0.5, 0,0,1,
    -0.5, -0.5,  0.5, 0,0,1,
    -0.5,  0.5,  0.5, 0,0,1,

    -0.5, -0.5, -0.5, 0,0,1,
    -0.5,  0.5,  0.5, 0,0,1,
    -0.5,  0.5, -0.5, 0,0,1,

    // ===== Face Lateral Direita =====
     0.5, -0.5, -0.5, 1,1,0,
     0.5,  0.5,  0.5, 1,1,0,
     0.5, -0.5,  0.5, 1,1,0,

     0.5, -0.5, -0.5, 1,1,0,
     0.5,  0.5, -0.5, 1,1,0,
     0.5,  0.5,  0.5, 1,1,0,

    // ===== Face Superior =====
    -0.5,  0.5, -0.5, 1,0,1,
    -0.5,  0.5,  0.5, 1,0,1,
     0.5,  0.5,  0.5, 1,0,1,

    -0.5,  0.5, -0.5, 1,0,1,
     0.5,  0.5,  0.5, 1,0,1,
     0.5,  0.5, -0.5, 1,0,1,

    // ===== Face Inferior =====
    -0.5, -0.5, -0.5, 0,1,1,
     0.5, -0.5,  0.5, 0,1,1,
    -0.5, -0.5,  0.5, 0,1,1,

    -0.5, -0.5, -0.5, 0,1,1,
     0.5, -0.5, -0.5, 0,1,1,
     0.5, -0.5,  0.5, 0,1,1,
    };

    GLuint VBO, VAO;

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    
    //Atributo posição (x, y, z)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    //Atributo cor (r, g, b)
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
}