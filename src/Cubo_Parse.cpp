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
#include <vector>   // Adicionado para armazenar os dados do OBJ
#include <fstream>  // Adicionado para leitura de arquivos
#include <sstream>  // Adicionado para processar as strings do OBJ

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
int setupGeometry(int &nVertices); // Adicionado parâmetro para retornar n de vértices

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
    // Atualizado o nome para refletir seu projeto
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Visualizador OBJ -- João Vitor de Mello Lima!", nullptr, nullptr);
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

    // Gerando um buffer para o objeto lido
    int nVertices;
    GLuint VAO = setupGeometry(nVertices);

    glUseProgram(shaderID);

    GLint modelLoc = glGetUniformLocation(shaderID, "model");

    glEnable(GL_DEPTH_TEST);

    // Array com as posições dos múltiplos objetos no espaço 3D
    glm::vec3 posicoesDosCubos[] = {
        glm::vec3( 0.0f,  0.0f,  0.0f), 
        glm::vec3( 0.6f,  0.6f, 0.0f), 
        glm::vec3(-0.6f, 0.6f, 0.0f), 
    };
    // Automatizar o num do for de criação
    int numCubos = std::size(posicoesDosCubos);
    // Loop da aplicação - "game loop"
    while (!glfwWindowShouldClose(window))
    {
        // Checa se houveram eventos de input e chama as funções de callback
        glfwPollEvents();

        // Limpa o buffer de cor e de profundidade
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f); //cor de fundo
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glLineWidth(2); // Reduzido para melhor visualização de malhas complexas
        glPointSize(5);

        float angle = (GLfloat)glfwGetTime();

        // Vincula o VAO
        glBindVertexArray(VAO);

        // Desenha todos os objetos
        for(unsigned int i = 0; i < numCubos; i++)
        {
            glm::mat4 model = glm::mat4(1.0f); 
            
            // Translação base (movimento da câmera/mundo via teclado)
            model = glm::translate(model, glm::vec3(posX, posY, posZ));
            
            // Translação específica 
            model = glm::translate(model, posicoesDosCubos[i]);

            // Escala geral
            model = glm::scale(model, glm::vec3(scale));

            // Rotação 
            float anguloDiferente = angle + (i * 0.5f); 
            if (rotateX)
                model = glm::rotate(model, anguloDiferente, glm::vec3(1.0f, 0.0f, 0.0f));
            else if (rotateY)
                model = glm::rotate(model, anguloDiferente, glm::vec3(0.0f, 1.0f, 0.0f));
            else if (rotateZ)
                model = glm::rotate(model, anguloDiferente, glm::vec3(0.0f, 0.0f, 1.0f));

            // Envia a matriz model atualizada para o Shader
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            // 1. Desenha as faces coloridas
            glDrawArrays(GL_TRIANGLES, 0, nVertices);

            // 2. Desenha os pontos em PRETO
            glDisableVertexAttribArray(1); // Desliga o buffer de cores
            glVertexAttrib3f(1, 0.0f, 0.0f, 0.0f); // Seta cor preta fixa
            glPointSize(10.0f);
            glDrawArrays(GL_POINTS, 0, nVertices);
            glEnableVertexAttribArray(1); // Religa para o próximo objeto
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

// Função para fazer o parse do arquivo .obj
vector<GLfloat> loadOBJ(string path)
{
    vector<glm::vec3> temp_vertices;
    vector<GLfloat> out_data;

    ifstream file(path);
    if (!file.is_open()) {
        cout << "Erro ao abrir o arquivo: " << path << endl;
        return out_data;
    }

    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string prefix;
        ss >> prefix;

        if (prefix == "v") {
            glm::vec3 v;
            ss >> v.x >> v.y >> v.z;
            temp_vertices.push_back(v);
        }
        else if (prefix == "f") {
            string v1, v2, v3;
            ss >> v1 >> v2 >> v3;

            // Extraindo os índices (v/vt/vn)
            int vIdx[3], nIdx[3];
            
            auto parseIndices = [](string s, int &v, int &n) {
                v = stoi(s.substr(0, s.find("/"))) - 1;
                n = stoi(s.substr(s.find_last_of("/") + 1)) - 1;
            };

            parseIndices(v1, vIdx[0], nIdx[0]);
            parseIndices(v2, vIdx[1], nIdx[1]);
            parseIndices(v3, vIdx[2], nIdx[2]);

            // índice da normal da primeira quina para definir a cor da face
            glm::vec3 faceColor;
            int faceID = nIdx[0] % 6; 

            if      (faceID == 0) faceColor = glm::vec3(1.0f, 0.2f, 0.2f); // Vermelho suave
            else if (faceID == 1) faceColor = glm::vec3(0.2f, 1.0f, 0.2f); // Verde suave
            else if (faceID == 2) faceColor = glm::vec3(0.2f, 0.2f, 1.0f); // Azul suave
            else if (faceID == 3) faceColor = glm::vec3(1.0f, 1.0f, 0.2f); // Amarelo
            else if (faceID == 4) faceColor = glm::vec3(1.0f, 0.2f, 1.0f); // Magenta
            else                  faceColor = glm::vec3(0.2f, 1.0f, 1.0f); // Ciano

            for (int i = 0; i < 3; i++) {
                out_data.push_back(temp_vertices[vIdx[i]].x);
                out_data.push_back(temp_vertices[vIdx[i]].y);
                out_data.push_back(temp_vertices[vIdx[i]].z);
                out_data.push_back(faceColor.r);
                out_data.push_back(faceColor.g);
                out_data.push_back(faceColor.b);
            }
        }
    }
    file.close();
    return out_data;
}
// Função para criar o VAO/VBO a partir do OBJ
int setupGeometry(int &nVertices)
{ 
    vector<GLfloat> vertices = loadOBJ("../assets/Modelos3D/Cube.obj"); 
    nVertices = vertices.size() / 6; // 6 elementos por vértice (x,y,z,r,g,b)

    GLuint VBO, VAO;

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

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