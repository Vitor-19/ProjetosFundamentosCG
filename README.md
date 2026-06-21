# Computação Gráfica - Híbrido

Repositório de exemplos de códigos em C++ utilizando OpenGL moderna (3.3+) criado para a Atividade Acadêmica Computação Gráfica do curso de graduação em Ciência da Computação - modalidade híbrida - da Unisinos. Ele é estruturado para facilitar a organização dos arquivos e a compilação dos projetos utilizando CMake.

## 📂 Estrutura do Repositório

```plaintext
📂 CGCCHibrido/
├── 📂 include/               # Cabeçalhos e bibliotecas de terceiros
│   ├── 📂 glad/              # Cabeçalhos da GLAD (OpenGL Loader)
│   │   ├── glad.h
│   │   ├── 📂 KHR/           # Diretório com cabeçalhos da Khronos (GLAD)
│   │       ├── khrplatform.h
├── 📂 common/                # Código reutilizável entre os projetos
│   ├── glad.c                # Implementação da GLAD
├── 📂 src/                   # Código-fonte dos exemplos e exercícios
│   ├── Hello3D.cpp           # Exemplo básico de renderização com OpenGL
│   ├── ...                   # Outros exemplos e exercícios futuros
├── 📂 build/                 # Diretório gerado pelo CMake (não incluído no repositório)
├── 📂 assets/                # diretório com modelos 3D, texturas, fontes etc
├── 📄 CMakeLists.txt         # Configuração do CMake para compilar os projetos
├── 📄 README.md              # Este arquivo, com a documentação do repositório
├── 📄 GettingStarted.md      # Tutorial detalhado sobre como compilar usando o CMake
```

Siga as instruções detalhadas em [GettingStarted.md](GettingStarted.md) para configurar e compilar o projeto.

## ⚠️ **IMPORTANTE: Baixar a GLAD Manualmente**
Para que o projeto funcione corretamente, é necessário **baixar a GLAD manualmente** utilizando o **GLAD Generator**.

### 🔗 **Acesse o web service do GLAD**:
👉 [GLAD Generator](https://glad.dav1d.de/)

### ⚙️ **Configuração necessária:**
- **API:** OpenGL  
- **Version:** 3.3+ (ou superior compatível com sua máquina)  
- **Profile:** Core  
- **Language:** C/C++  

### 📥 **Baixe e extraia os arquivos:**
Após a geração, extraia os arquivos baixados e coloque-os nos diretórios correspondentes:
- Copie **`glad.h`** para `include/glad/`
- Copie **`khrplatform.h`** para `include/glad/KHR/`
- Copie **`glad.c`** para `common/`

🚨 **Sem esses arquivos, a compilação falhará!** É necessário colocar esses arquivos nos diretórios corretos, conforme a orientação acima. 


# FINALIZAÇÃO GB:

**Nome:** João Vitor de Mello Lima

## 🛠 Setup e Execução

Utilizamos o CMake para a build. No terminal, dentro da pasta raiz do projeto:
    mkdir build && cd build
    cmake ..
    cmake --build .

Para executar um código, dentro da pasta build: .\nome_exe
    O codigo final é "Animation.cpp" e "Animation2.cpp", sendo o segundo uma versão do primeiro, mas com luzes fixas no mundo e pontos nos vertices dos objetos.

## 📥 Assets e Procedência:

Vozhd.obj/.mtl e model_baseColor.png - Gerados via Meshy AI, link: https://www.meshy.ai/pt-BR

Donut.obj/.mtl - Gerado de forma autoral no Blender

Pawn.obj/.mtl - Cedido por terceiros.

Suzanne,Cube, SuzanneSubdiv1 e demais assets - Disponibilizados pela disciplina

## 🚨 Referências:

- AKENINE-MÖLLER, T.; HAINES, E.; HOFFMAN, N. Real-Time Rendering. 4th ed.

- LENGYEL, Eric. Mathematics for 3D game programming and computer graphics. 3rd ed. Boston: Course Technology, 2012.

- Tutoriais consultados em - https://learnopengl.com/
