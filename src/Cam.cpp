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
#include <vector>   
#include <fstream>  
#include <sstream> 

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Carregar texturas
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// Enum para as direções de movimento da câmera
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Classe Câmera para encapsular atributos e as ações de Mover e Rotacionar (First Person)
class Camera {
public:
    glm::vec3 Position;
    glm::vec3 Front;
    glm::vec3 Up;
    glm::vec3 Right;
    glm::vec3 WorldUp;
    float Yaw;
    float Pitch;
    float MovementSpeed;
    float MouseSensitivity;
    float Zoom;

    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = -90.0f, float pitch = 0.0f) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(2.5f), MouseSensitivity(0.1f), Zoom(45.0f) {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors();
    }

    glm::mat4 GetViewMatrix() {
        return glm::lookAt(Position, Position + Front, Up);
    }

    void ProcessKeyboard(Camera_Movement direction, float deltaTime) {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD) Position += Front * velocity;
        if (direction == BACKWARD) Position -= Front * velocity;
        if (direction == LEFT) Position -= Right * velocity;
        if (direction == RIGHT) Position += Right * velocity;
    }

    void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true) {
        xoffset *= MouseSensitivity;
        yoffset *= MouseSensitivity;

        Yaw += xoffset;
        Pitch += yoffset;

        if (constrainPitch) {
            if (Pitch > 89.0f) Pitch = 89.0f;
            if (Pitch < -89.0f) Pitch = -89.0f;
        }
        updateCameraVectors();
    }

private:
    void updateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        Right = glm::normalize(glm::cross(Front, WorldUp));
        Up = glm::normalize(glm::cross(Right, Front));
    }
};

// Struct para armazenar propriedades individuais de cada modelo 3D
struct Object3D {
    GLuint VAO;
    int nVertices;
    glm::vec3 position;
    glm::vec3 scale;
    float rotX, rotY, rotZ; 
    string textureName; // Armazena o nome da textura
    GLuint textureID; // Var para armazenar o ID da textura

    // Propriedades de Material de Iluminação
    glm::vec3 Ka; // Coeficiente de reflexão ambiente
    glm::vec3 Kd; // Coeficiente de reflexão difusa
    glm::vec3 Ks; // Coeficiente de reflexão especular
    float Ns;     // Expoente de brilho 

    Object3D(GLuint vao, int nVerts, glm::vec3 pos, string texName, GLuint texID, glm::vec3 ka, glm::vec3 kd, glm::vec3 ks, float ns) {
        VAO = vao;
        nVertices = nVerts;
        position = pos;
        scale = glm::vec3(0.15f); 
        rotX = rotY = rotZ = 0.0f;
        textureName = texName;
        textureID = texID; 
        Ka = ka;
        Kd = kd;
        Ks = ks;
        Ns = ns;
    }
};

// Protótipo da função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Protótipos das funções de controle da câmera
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow *window);

// Protótipos das funções
int setupShader();
// Função agora recebe o caminho do arquivo OBJ por parâmetro e passa a string da textura por referência

GLuint setupGeometry(string path, int &nVertices, string &textureName, glm::vec3 &ka, glm::vec3 &kd, glm::vec3 &ks, float &ns); 

GLuint loadTexture(string filePath, int &width, int &height);

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 1000, HEIGHT = 1000;

// Instância global da câmera e variáveis para o mouse/tempo
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;
bool firstMouse = true;
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Código fonte do Vertex Shader (em GLSL): ainda hardcoded 
const GLchar* vertexShaderSource = "#version 450\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec3 color;\n"
"layout (location = 2) in vec2 texCoord;\n" 
"layout (location = 3) in vec3 normal;\n" // Lendo o atributo normal (vn) do VAO
"uniform mat4 model;\n"
"uniform mat4 view;\n"       // Matriz de visualização (câmera)
"uniform mat4 projection;\n" // Matriz de projeção (câmera)
"out vec4 finalColor;\n"
"out vec2 TexCoord;\n" 
"out vec3 FragPos;\n" // Enviando a posição do fragmento para o Fragment Shader
"out vec3 Normal;\n"  // Enviando a normal para o Fragment Shader
"void main()\n"
"{\n"
"gl_Position = projection * view * model * vec4(position, 1.0);\n" // Aplicação das matrizes projection e view
"FragPos = vec3(model * vec4(position, 1.0));\n" // Calculando a posição no mundo
"Normal = mat3(transpose(inverse(model))) * normal;\n" // Transformando a normal corretamente para não distorcer na escala
"finalColor = vec4(color, 1.0);\n"
"TexCoord = texCoord;\n"
"}\0";

//Códifo fonte do Fragment Shader (em GLSL): adaptado para 3 luzes e atenuação
const GLchar* fragmentShaderSource = "#version 450\n"
"in vec4 finalColor;\n"
"in vec2 TexCoord;\n"
"in vec3 FragPos;\n" // Recebendo a posição do vértice interpolada
"in vec3 Normal;\n"  // Recebendo a normal interpolada
"uniform sampler2D texBuff;\n" 
"uniform bool useTexture;\n" 
"uniform vec3 viewPos;\n"     // Posição da câmera (observador)
"uniform vec3 materialKa;\n"  // Coeficiente Ambiente lido do MTL
"uniform vec3 materialKd;\n"  // Coeficiente Difuso lido do MTL
"uniform vec3 materialKs;\n"  // Coeficiente Especular lido do MTL
"uniform float materialNs;\n" // Expoente de brilho (Shininess) lido do MTL
"//Arrays para 3 luzes pontuais (Key, Fill, Back) e controle on/off\n"
"uniform vec3 lightPos[3];\n"
"uniform vec3 lightColor[3];\n"
"uniform bool lightEnabled[3];\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"   vec4 baseColor;\n"
"   if (useTexture) {\n" // Se for verdadeiro, desenha a imagem
"       baseColor = texture(texBuff, TexCoord);\n" 
"   } else {\n"          // Se for falso, desenha as cores das faces
"       baseColor = finalColor;\n" 
"   }\n"
"\n"
"   vec3 result = vec3(0.0);\n"
"   vec3 norm = normalize(Normal);\n"
"   vec3 viewDir = normalize(viewPos - FragPos);\n"
"\n"
"   // Cálculo de Iluminação para 3 pontos de luz\n"
"   for(int i = 0; i < 3; i++) {\n"
"       if(lightEnabled[i]) {\n"
"           // Fator de atenuação para simular a queda de luz baseada na distância\n"
"           float distance = length(lightPos[i] - FragPos);\n"
"           // Fatores padrões (constante=1.0, linear=0.09, quadratica=0.032)\n"
"           float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance));\n"
"\n"
"           // 1. Iluminação Ambiente (Calculada por luz, bem suave)\n"
"           vec3 ambient = materialKa * 0.1 * lightColor[i];\n"
"\n"
"           // 2. Iluminação Difusa\n"
"           vec3 lightDir = normalize(lightPos[i] - FragPos);\n"
"           float diff = max(dot(norm, lightDir), 0.0);\n"
"           vec3 diffuse = materialKd * diff * lightColor[i];\n"
"           diffuse *= attenuation; //Fator de atenuação aplicado na difusa\n"
"\n"
"           // 3. Iluminação Especular\n"
"           vec3 reflectDir = reflect(-lightDir, norm);\n"
"           float spec = pow(max(dot(viewDir, reflectDir), 0.0), max(materialNs, 0.1)); \n"
"           vec3 specular = materialKs * spec * lightColor[i];\n"
"           specular *= attenuation; // atenuação na especular \n"
"\n"
"           // Soma e acumula a contribuição desta fonte de luz ao pixel\n"
"           result += (ambient + diffuse) * vec3(baseColor) + specular;\n"
"       }\n"
"   }\n"
"   color = vec4(result, baseColor.a);\n"
"}\n\0";

// Var para controle de objetos na cena
std::vector<Object3D> cena;
int objSelecionado = 0; 
// Array para rastrear se as teclas de luz estão ligadas ou desligadas
bool lightKeysOn[3] = {true, true, true}; 

// Função MAIN
int main()
{
    // Inicialização da GLFW
    glfwInit();

    // Criação da janela GLFW
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Visualizador OBJ -- João Vitor de Mello Lima!", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // Fazendo o registro da função de callback para a janela GLFW
    glfwSetKeyCallback(window, key_callback);

    // Capturando o cursor e registrando a função de callback do mouse
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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

    //Carregando as geometrias e instanciando os objetos na lista

    int nVerticesObj0, nVerticesObj1, nVerticesObj2;
    string texObj0, texObj1, texObj2; // Variáveis para receber os nomes das texturas
    
    // Variáveis para capturar as informações de materiais ao ler os objetos
    glm::vec3 ka0, kd0, ks0; float ns0;
    glm::vec3 ka1, kd1, ks1; float ns1;
    glm::vec3 ka2, kd2, ks2; float ns2;

    // vao's (agora passando as strings de textura e propriedades do materiaç)

    GLuint vao0 = setupGeometry("../assets/Modelos3D/Cube.obj", nVerticesObj0, texObj0, ka0, kd0, ks0, ns0);
    GLuint vao1 = setupGeometry("../assets/Modelos3D/Suzanne.obj", nVerticesObj1, texObj1, ka1, kd1, ks1, ns1);
    GLuint vao2 = setupGeometry("../assets/Modelos3D/Pawn.obj", nVerticesObj2, texObj2, ka2, kd2, ks2, ns2);

    // Carregando as imagens lidas dos arquivos MTL
    int imgWidth, imgHeight;
    GLuint texID0 = loadTexture("../assets/Modelos3D/" + texObj0, imgWidth, imgHeight);
    GLuint texID1 = loadTexture("../assets/Modelos3D/" + texObj1, imgWidth, imgHeight);
    GLuint texID2 = loadTexture("../assets/Modelos3D/" + texObj2, imgWidth, imgHeight);

    // Adicionando ao Vector
  
    cena.push_back(Object3D(vao0, nVerticesObj0, glm::vec3(-0.6f, 0.0f, 0.0f), texObj0, texID0, ka0, kd0, ks0, ns0)); 
    cena.push_back(Object3D(vao1, nVerticesObj1, glm::vec3(0.6f, 0.0f, 0.0f), texObj1, texID1, ka1, kd1, ks1, ns1));   
    cena.push_back(Object3D(vao2, nVerticesObj2, glm::vec3(0.0f, 0.6f, 0.0f), texObj2, texID2, ka2, kd2, ks2, ns2));
    
    // Print dos controles
    cout << "Controles:" << endl;
    cout << "[TAB] - Muda o obj selecionado" << endl;
    cout << "[WASD] - Move nos eixos X e Z | [I][J] - Move no eixo Y" << endl;
    cout << "[X][Y][Z] - Rotaciona nos eixos" << endl;
    cout << "[-] [+] ou [N] [M] - Altera a escala" << endl;
    cout << "[1][2][3] - Liga/Desliga a Key, Fill e Back Light respectivamente" << endl;
    // Adicionando as instruções da câmera aos prints
    cout << "[SETAS] - Move a camera em primeira pessoa" << endl;
    cout << "[MOUSE] - Rotaciona a visao da camera" << endl;

    glUseProgram(shaderID);
    GLint modelLoc = glGetUniformLocation(shaderID, "model");

    // Buscando a localização das uniforms da câmera no Shader
    GLint viewLoc = glGetUniformLocation(shaderID, "view");
    GLint projLoc = glGetUniformLocation(shaderID, "projection");

    GLint useTextureLoc = glGetUniformLocation(shaderID, "useTexture");

    glUniform1i(glGetUniformLocation(shaderID, "texBuff"), 0);
    
    // Carregando as posições dos uniforms da iluminação e câmera
    GLint viewPosLoc = glGetUniformLocation(shaderID, "viewPos");

    GLint matKaLoc = glGetUniformLocation(shaderID, "materialKa");
    GLint matKdLoc = glGetUniformLocation(shaderID, "materialKd");
    GLint matKsLoc = glGetUniformLocation(shaderID, "materialKs");
    GLint matNsLoc = glGetUniformLocation(shaderID, "materialNs");

    // Definindo a câmera estática
    glm::vec3 viewPos(0.0f, 0.0f, 3.0f);
    glUniform3fv(viewPosLoc, 1, glm::value_ptr(viewPos));

    glEnable(GL_DEPTH_TEST);

    // Loop da aplicação - "game loop"
    while (!glfwWindowShouldClose(window))
    {
        // Lógica para calcular o tempo do frame (deltaTime) visando movimento constante
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Verificando as setas do teclado para mover a câmera em cada frame
        processInput(window);

        glfwPollEvents();
        // Checa se houveram eventos de input e chama as funções de callback
        // Limpa o buffer de cor e de profundidade
        glClearColor(0.255f, 0.207f, 0.262f, 1.0f); // cor de fundo (RGBA)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Construindo e enviando a matriz de Projeção da câmera
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)width / (float)height, 0.1f, 100.0f);
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // Construindo e enviando a matriz de Visualização (View) da câmera
        glm::mat4 view = camera.GetViewMatrix();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        // Atualizando dinamicamente a posição da câmera no shader para a iluminação especular seguir a nova câmera
        glUniform3fv(viewPosLoc, 1, glm::value_ptr(camera.Position));

        // Lógica de Posicionamento da Iluminação de 3 Pontos (Key, Fill, Back)
        // O posicionamento baseia-se na posição e escala do objeto principal selecionado
        Object3D& mainObj = cena[objSelecionado]; 
        float scaleRef = mainObj.scale.x; // Pega a escala para ajustar a distância da luz automaticamente
        
        glm::vec3 lightPositions[3];
        glm::vec3 lightColors[3];
        
        // 1. Key Light (Principal): Intensa, na frente e para o lado (+X, +Y, +Z)
        lightPositions[0] = mainObj.position + glm::vec3(4.0f * scaleRef, 3.0f * scaleRef, 4.0f * scaleRef);
        lightColors[0] = glm::vec3(1.0f, 1.0f, 1.0f); // Branca, 100% intensidade
        
        // 2. Fill Light (Preenchimento): Menos intensa, lado oposto da Key Light para suavizar sombra (-X, +Y, +Z)
        lightPositions[1] = mainObj.position + glm::vec3(-4.0f * scaleRef, 1.5f * scaleRef, 3.0f * scaleRef);
        lightColors[1] = glm::vec3(0.4f, 0.4f, 0.4f); // 40% de intensidade
        
        // 3. Back Light (Fundo): Intensidade média, instalada atrás do objeto (+Y, -Z)
        lightPositions[2] = mainObj.position + glm::vec3(0.0f, 3.0f * scaleRef, -4.0f * scaleRef);
        lightColors[2] = glm::vec3(0.6f, 0.6f, 0.6f); // 60% de intensidade para fazer o contorno traseiro
        
        // Envia os dados atualizados das 3 luzes para o Shader
        for (int l = 0; l < 3; l++) {
            glUniform3fv(glGetUniformLocation(shaderID, ("lightPos[" + to_string(l) + "]").c_str()), 1, glm::value_ptr(lightPositions[l]));
            glUniform3fv(glGetUniformLocation(shaderID, ("lightColor[" + to_string(l) + "]").c_str()), 1, glm::value_ptr(lightColors[l]));
            glUniform1i(glGetUniformLocation(shaderID, ("lightEnabled[" + to_string(l) + "]").c_str()), lightKeysOn[l]);
        }

        glLineWidth(2);
        glPointSize(5);

        // Desenha todos os objetos da lista
        for(size_t i = 0; i < cena.size(); i++)
        {
            Object3D& obj = cena[i]; // Pega a referência do objeto atual

            // Vincula o VAO específico do objeto
            glBindVertexArray(obj.VAO);

            // Verifica se o objeto possui um nome de textura válido, se não usa as cores
            if (obj.textureName != "") {
                glUniform1i(useTextureLoc, 1); 
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, obj.textureID);
            } else {
                glUniform1i(useTextureLoc, 0); 
            }

            // Envia as propriedades de iluminação específicas deste objeto pro Fragment Shader
            glUniform3fv(matKaLoc, 1, glm::value_ptr(obj.Ka));
            glUniform3fv(matKdLoc, 1, glm::value_ptr(obj.Kd));
            glUniform3fv(matKsLoc, 1, glm::value_ptr(obj.Ks));
            glUniform1f(matNsLoc, obj.Ns);

            glm::mat4 model = glm::mat4(1.0f); 
            
            // Translação usando as coordenadas do próprio objeto
            model = glm::translate(model, obj.position);

            // Rotação manual controlada pelo teclado 
            model = glm::rotate(model, obj.rotX, glm::vec3(1.0f, 0.0f, 0.0f));
            model = glm::rotate(model, obj.rotY, glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::rotate(model, obj.rotZ, glm::vec3(0.0f, 0.0f, 1.0f));

            // Escala específica do objeto
            model = glm::scale(model, obj.scale);

            // Envia a matriz model atualizada para o Shader
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            // Desenha as faces coloridas
            glDrawArrays(GL_TRIANGLES, 0, obj.nVertices);

            // Desenha os pontos
            glDisableVertexAttribArray(1); 
            
            // O objeto selecionado fica com pontos azuis, os outros ficam com pontos pretos
            if (i == objSelecionado) {
                glVertexAttrib3f(1, 0.0f, 0.0f, 1.0f); // Azul
                glPointSize(12.0f); // Deixa um pouco maior os pontos do obj selecionado
            } else {
                glVertexAttrib3f(1, 0.0f, 0.0f, 0.0f); // Preto
                glPointSize(10.0f);
            }
            
            glDrawArrays(GL_POINTS, 0, obj.nVertices);
            glEnableVertexAttribArray(1); // Religa para o próximo objeto
        }

        // Desvincula o VAO
        glBindVertexArray(0);

        // Troca os buffers da tela
        glfwSwapBuffers(window);
    }
    
    // Pede pra OpenGL desalocar os buffers
    for(auto& obj : cena) {
        glDeleteVertexArrays(1, &obj.VAO);
    }

    // Finaliza a execução da GLFW
    glfwTerminate();
    return 0;
}

// Função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    // Evita crashes caso a cena esteja vazia
    if (cena.empty()) return;

    // Seleção de objeto
    if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
        objSelecionado = (objSelecionado + 1) % cena.size();
        cout << "Objeto selecionado: " << objSelecionado << " | Textura: " << cena[objSelecionado].textureName << endl;
    }

    // Comando para habilitar e desabilitar luzes
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_1) lightKeysOn[0] = !lightKeysOn[0];
        if (key == GLFW_KEY_2) lightKeysOn[1] = !lightKeysOn[1];
        if (key == GLFW_KEY_3) lightKeysOn[2] = !lightKeysOn[2];
    }

    // Pega o objeto escolhido para fazer alterações
    Object3D& obj = cena[objSelecionado];

    // Agora a rotação está mais controlada, tendo que segurar as teclas para ela acontecer
    if (action == GLFW_PRESS || action == GLFW_REPEAT) 
    {
        // Rotação
        if (key == GLFW_KEY_X) obj.rotX += 0.1f;
        if (key == GLFW_KEY_Y) obj.rotY += 0.1f;
        if (key == GLFW_KEY_Z) obj.rotZ += 0.1f;

        // Translação com WASD e IJ
        if (key == GLFW_KEY_W) obj.position.z -= 0.1f; 
        if (key == GLFW_KEY_S) obj.position.z += 0.1f;
        if (key == GLFW_KEY_A) obj.position.x -= 0.1f;
        if (key == GLFW_KEY_D) obj.position.x += 0.1f;
        
        if (key == GLFW_KEY_I) obj.position.y += 0.1f; 
        if (key == GLFW_KEY_J) obj.position.y -= 0.1f;

        //Escala com '-' e '+' ou 'n' e 'm'
        if (key == GLFW_KEY_KP_SUBTRACT || key == GLFW_KEY_N) obj.scale -= glm::vec3(0.01f);
        if (key == GLFW_KEY_KP_ADD || key == GLFW_KEY_M) obj.scale += glm::vec3(0.01f);
    }
}

// Callback invocado pela GLFW quando o mouse se movimenta
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Invertido, pois as coordenadas Y vão de baixo para cima na tela

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// Função chamada a cada frame para processar input das setas (movimentação suave)
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
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

vector<GLfloat> loadOBJ(string path, string &textureName, glm::vec3 &ka, glm::vec3 &kd, glm::vec3 &ks, float &ns)
{
    vector<glm::vec3> temp_vertices;
    vector<glm::vec2> temp_uvs; 
    vector<glm::vec3> temp_normals; 
    vector<GLfloat> out_data;

    // Default caso o OBJ não tenha as info 
    ka = glm::vec3(0.8f);
    kd = glm::vec3(1.0f);
    ks = glm::vec3(0.5f);
    ns = 32.0f;

    string baseDir = path.substr(0, path.find_last_of('/') + 1);

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
        else if (prefix == "vt") { 
            glm::vec2 uv;
            ss >> uv.x >> uv.y;
            temp_uvs.push_back(uv);
        }
        else if (prefix == "vn") {
            glm::vec3 n;
            ss >> n.x >> n.y >> n.z;
            temp_normals.push_back(n);
        }
        else if (prefix == "mtllib") {
            string mtlFile;
            ss >> mtlFile;
            
            // Abre o arquivo MTL e busca o map_Kd e propriedades de Iluminação
            ifstream mtl(baseDir + mtlFile);
            if(mtl.is_open()) {
                string mtlLine;
                while(getline(mtl, mtlLine)) {
                    stringstream mtlSS(mtlLine);
                    string mtlPrefix;
                    mtlSS >> mtlPrefix;
                    if(mtlPrefix == "map_Kd") {
                        mtlSS >> textureName;
                    } 
                    // Recuperando os coeficientes de iluminação (Ka, Kd, Ks, Ns)

                    else if (mtlPrefix == "Ka") {
                        mtlSS >> ka.r >> ka.g >> ka.b;
                    }
                    else if (mtlPrefix == "Kd") {
                        mtlSS >> kd.r >> kd.g >> kd.b;
                    }
                    else if (mtlPrefix == "Ks") {
                        mtlSS >> ks.r >> ks.g >> ks.b;
                    }
                    else if (mtlPrefix == "Ns") {
                        mtlSS >> ns;
                    }
                }
                mtl.close();
            } else {
                cout << "Aviso: Nao foi possivel abrir o arquivo MTL: " << (baseDir + mtlFile) << endl;
            }
        }
        else if (prefix == "f") {
            string v1, v2, v3;
            ss >> v1 >> v2 >> v3;

            // Extraindo os índices (v/vt/vn)
            int vIdx[3], vtIdx[3], nIdx[3];
            
            auto parseIndices = [](string s, int &v, int &vt, int &n) {
                stringstream ssIdx(s);
                string item;
                int count = 0;
                vt = -1; 
                n = -1; 
                while (getline(ssIdx, item, '/')) {
                    if (!item.empty()) {
                        if (count == 0) v = stoi(item) - 1;
                        else if (count == 1) vt = stoi(item) - 1;
                        else if (count == 2) n = stoi(item) - 1;
                    }
                    count++;
                }
            };

            parseIndices(v1, vIdx[0], vtIdx[0], nIdx[0]);
            parseIndices(v2, vIdx[1], vtIdx[1], nIdx[1]);
            parseIndices(v3, vIdx[2], vtIdx[2], nIdx[2]);

            // índice da normal da primeira quina para definir a cor da face
            glm::vec3 faceColor;
            // Caso o nIdx[0] seja -1 (sem normal), evitamos falha matemática atribuindo 0
            int safeN = (nIdx[0] >= 0) ? nIdx[0] : 0;
            int faceID = safeN % 6; 

            if      (faceID == 0) faceColor = glm::vec3(1.0f, 0.2f, 0.2f); // Vermelho suave
            else if (faceID == 1) faceColor = glm::vec3(0.2f, 1.0f, 0.2f); // Verde suave
            else if (faceID == 2) faceColor = glm::vec3(0.2f, 0.2f, 1.0f); // Azul suave
            else if (faceID == 3) faceColor = glm::vec3(1.0f, 1.0f, 0.2f); // Amarelo
            else if (faceID == 4) faceColor = glm::vec3(1.0f, 0.2f, 1.0f); // Magenta
            else                  faceColor = glm::vec3(0.2f, 1.0f, 1.0f); // Ciano

            for (int i = 0; i < 3; i++) {
                // Posições
                out_data.push_back(temp_vertices[vIdx[i]].x);
                out_data.push_back(temp_vertices[vIdx[i]].y);
                out_data.push_back(temp_vertices[vIdx[i]].z);
                
                // Cores 
                out_data.push_back(faceColor.r);
                out_data.push_back(faceColor.g);
                out_data.push_back(faceColor.b);

                // UVs (Coordenadas de Textura)
                if (vtIdx[i] >= 0 && vtIdx[i] < temp_uvs.size()) {
                    out_data.push_back(temp_uvs[vtIdx[i]].x);
                    out_data.push_back(temp_uvs[vtIdx[i]].y);
                } else {
                    out_data.push_back(0.0f); // Default caso não haja vt
                    out_data.push_back(0.0f);
                }

                // Coordenadas da Normal do vértice (nx, ny, nz)
                if (nIdx[i] >= 0 && nIdx[i] < temp_normals.size()) {
                    out_data.push_back(temp_normals[nIdx[i]].x);
                    out_data.push_back(temp_normals[nIdx[i]].y);
                    out_data.push_back(temp_normals[nIdx[i]].z);
                } else {
                    out_data.push_back(0.0f); // Default caso não haja normal no arquivo
                    out_data.push_back(1.0f); // Aponta pra cima para não bugar Phong
                    out_data.push_back(0.0f);
                }
            }
        }
    }
    file.close();
    return out_data;
}

// Função para criar o VAO/VBO a partir do OBJ recebendo o PATH 
GLuint setupGeometry(string path, int &nVertices, string &textureName, glm::vec3 &ka, glm::vec3 &kd, glm::vec3 &ks, float &ns)
{ 
    vector<GLfloat> vertices = loadOBJ(path, textureName, ka, kd, ks, ns); 

    nVertices = vertices.size() / 11; 

    GLuint VBO, VAO;

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat), vertices.data(), GL_STATIC_DRAW);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    
    //Atributo posição (x, y, z) 
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    //Atributo cor (r, g, b) 
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    //Atributo textura (u, v)
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6*sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    // Atributo da Normal (nx, ny, nz) (location = 3)
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(8*sizeof(GLfloat)));
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
}

GLuint loadTexture(string filePath, int &width, int &height)
{
    GLuint texID; 

    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int nrChannels;

    // inverte a textura (Sem isso fica bugado)
    stbi_set_flip_vertically_on_load(true);

    unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);

    if (data)
    {
        if (nrChannels == 3) 
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        }
        else 
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        }
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture " << filePath << std::endl;
    }

    stbi_image_free(data);

    glBindTexture(GL_TEXTURE_2D, 0);

    return texID;
}