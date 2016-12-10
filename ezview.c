// Charles Beck
// CS 430
// Project 5: Image viewer
// This project will view an image and allow you to move the image, Scale the image
//	shear the image, and rotate the image with key commands
// 				Q to rotate counter-clockwise
//              W to rotate clockwise
//				A to scale the image up, S to scale the image down.
// 				Z to shear the image right, X to shear the image left 
//				Left arrow key to pan the image left. Right arrow key to pan the image right. Up arrow key to pan the image up.
//             	Down arrow key to pan the image down,
//				Spacebar to set the vertices to their original locations, escape to close the program.
#define GLFW_DLL 1
#define PI 3.14159265358979323846
#define WINDOW_WIDTH 500
#define WINDOW_HEIGHT 500

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <assert.h>

typedef struct Pixel
{
    unsigned char r, g, b;
} Pixel;


FILE* fh;
int width, height, cv; 
GLFWwindow* window;

typedef struct
{
    float position[3];
    float color[4];
    float textcoord[2];
} Vertex;

Vertex Vertices[] =
{
    {{1, -1, 0},  {1, 0, 0, 1}, {0.99999,0.99999}},
    {{1, 1, 0},   {0, 1, 0, 1}, {0.99999,0}},
    {{-1, 1, 0},  {0, 0, 1, 1}, {0,0}},
    {{-1, -1, 0}, {0, 0, 0, 1}, {0,0.99999}}
};

const GLubyte Indices[] =
{
    0, 1, 2,
    2, 3, 0
};

char* vertex_shader_src =
    "attribute vec4 Position;\n"
    "attribute vec4 SourceColor;\n"
    "\n"
    "attribute vec2 TexCoordIn;\n"
    "varying lowp vec2 TexCoordOut;\n"
    "\n"
    "varying lowp vec4 DestinationColor;\n"
    "\n"
    "void main(void) {\n"
    "    TexCoordOut = TexCoordIn;\n"
    "    DestinationColor = SourceColor;\n"
    "    gl_Position = Position;\n"
    "}\n";

char* fragment_shader_src =
    "varying lowp vec4 DestinationColor;\n"
    "\n"
    "varying lowp vec2 TexCoordOut;\n"
    "uniform sampler2D Texture;\n"
    "\n"
    "void main(void) {\n"
    "    gl_FragColor = texture2D(Texture, TexCoordOut);\n"
    "}\n";

GLint simple_shader(GLint shader_type, char* shader_src){

    GLint compile_success = 0;

    int shader_id = glCreateShader(shader_type);

    glShaderSource(shader_id, 1, &shader_src, 0);

    glCompileShader(shader_id);

    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &compile_success);

    if (compile_success == GL_FALSE)
    {
        GLchar message[256];
        glGetShaderInfoLog(shader_id, sizeof(message), 0, &message[0]);
        printf("glCompileShader Error: %s %s\n", shader_src, message);
        exit(1);
    }

    return shader_id;
}

int read_header(char input)  
{
    int i, line, endOfHeader, x;
    char a, b;
    char width_h[32], height_h[32], cv_h[32]; 
    char comment[64];                        

    a = fgetc(fh);
    b = fgetc(fh);
    if(a != 'P' || b != input)   
    {
        fprintf(stderr, "Error: Improper header filetype.\n", 005);
        exit(1);                   
    }
    a = fgetc(fh); 

    a = fgetc(fh); 
    if(a == '#')        
    {
        while(a != '\n')  
        {
            a = fgetc(fh);
        }
        a = fgetc(fh); 
    }


    i = 0;
    while(a != ' ') 
    {
        width_h[i] = a; 
        a = fgetc(fh);
        i++;
    }
    width = atoi(width_h);


    a = fgetc(fh);  
    i = 0;
    while(a != '\n')    
    {
        height_h[i] = a;    
        a = fgetc(fh);
        i++;
    }
    height = atoi(height_h); 

    a = fgetc(fh); 
    i = 0;
    while(a != '\n')    
    {
        cv_h[i] = a;
        a = fgetc(fh);
        i++;
    }
    cv = atoi(cv_h); 
    if(cv != 255)          
    {
        fprintf(stderr, "Error: Color value exceeds limit.\n", 007);
        exit(1);              
    }

    return 1;
}

int readFile(Pixel* image)
{
    int i;
    unsigned char c;
    char number[5];
    for(i=0; i < width*height; i++) 
    {

        fgets(number, 10, fh); 
        c = (char)atoi(number);       
        if(c > cv)          
        {
            fprintf(stderr, "Error: Color value exceeds limit.\n", 006);
            exit(1);                  
        }
        image[i].r = c;

        fgets(number, 10, fh); 
        c = (char)atoi(number);      
        if(c > cv)          
        {
            fprintf(stderr, "Error: Color value exceeds limit.\n", 006);
            exit(1);                  
        }
        image[i].g = c;

        fgets(number, 10, fh); 
        c = (char)atoi(number);       
        if(c > cv)           
        {
            fprintf(stderr, "Error: Color value exceeds limit.\n", 006);
            exit(1);                
        }
        image[i].b = c;
    }
    return 1;
}

int simple_program()
{
    GLint link_success = 0;

    GLint program_id = glCreateProgram();
    GLint vertex_shader = simple_shader(GL_VERTEX_SHADER, vertex_shader_src);
    GLint fragment_shader = simple_shader(GL_FRAGMENT_SHADER, fragment_shader_src);

    glAttachShader(program_id, vertex_shader);
    glAttachShader(program_id, fragment_shader);

    glLinkProgram(program_id);

    glGetProgramiv(program_id, GL_LINK_STATUS, &link_success);

    if (link_success == GL_FALSE)
    {
        GLchar message[256];
        glGetProgramInfoLog(program_id, sizeof(message), 0, &message[0]);
        printf("glLinkProgram Error: %s\n", message);
        exit(1);
    }

    return program_id;
}


static void error_callback(int error, const char* description)
{
    fputs(description, stderr);
}

void pan(int direction)
{
    switch(direction)
    {
    case 0:
        printf("You pressed right arrow key.\n");
        Vertices[0].position[0] += 0.1;
        Vertices[1].position[0] += 0.1;
        Vertices[2].position[0] += 0.1;
        Vertices[3].position[0] += 0.1;
        break;
    case 1:
        printf("You pressed left arrow key.\n");
        Vertices[0].position[0] -= 0.1;
        Vertices[1].position[0] -= 0.1;
        Vertices[2].position[0] -= 0.1;
        Vertices[3].position[0] -= 0.1;
        break;
    case 2:
        printf("You pressed up arrow key.\n");
        Vertices[0].position[1] += 0.1;
        Vertices[1].position[1] += 0.1;
        Vertices[2].position[1] += 0.1;
        Vertices[3].position[1] += 0.1;
        break;
    case 3:
        printf("You pressed down arrow key.\n");
        Vertices[0].position[1] -= 0.1;
        Vertices[1].position[1] -= 0.1;
        Vertices[2].position[1] -= 0.1;
        Vertices[3].position[1] -= 0.1;
        break;
    default:
        printf("Something went wrong when trying to pan.\n");
        break;
    }
}

typedef float* V3;

static inline float v3_dot(V3 a, V3 b)
{
    return a[0]*b[0] + a[1]*b[1] + a[2]*b[2];
}

void rotateImage(int direction)
{
    float theta = 22.5*(PI/180); 
    float x, y;                 
    float rotationMatrixColA[3] = {cos(theta), sin(theta), 0}; 
    float rotationMatrixColB[3] = {-sin(theta), cos(theta), 0}; 


    switch(direction)
    {
    case 0:
        printf("You pressed W key.\n");
        x = v3_dot(Vertices[0].position, rotationMatrixColA); 
        y = v3_dot(Vertices[0].position, rotationMatrixColB); 
        Vertices[0].position[0] = x; 
        Vertices[0].position[1] = y; 

        x = v3_dot(Vertices[1].position, rotationMatrixColA);
        y = v3_dot(Vertices[1].position, rotationMatrixColB);
        Vertices[1].position[0] = x;
        Vertices[1].position[1] = y;

        x = v3_dot(Vertices[2].position, rotationMatrixColA);
        y = v3_dot(Vertices[2].position, rotationMatrixColB);
        Vertices[2].position[0] = x;
        Vertices[2].position[1] = y;

        x = v3_dot(Vertices[3].position, rotationMatrixColA);
        y = v3_dot(Vertices[3].position, rotationMatrixColB);
        Vertices[3].position[0] = x;
        Vertices[3].position[1] = y;

        break;
    case 1:
        printf("You pressed Q key.\n");
        theta = -theta; 
        rotationMatrixColA[0] = cos(theta); 
        rotationMatrixColA[1] = sin(theta);
        rotationMatrixColB[0] = -sin(theta);
        rotationMatrixColB[1] = cos(theta);

        x = v3_dot(Vertices[0].position, rotationMatrixColA); 
        y = v3_dot(Vertices[0].position, rotationMatrixColB);
        Vertices[0].position[0] = x;
        Vertices[0].position[1] = y;

        x = v3_dot(Vertices[1].position, rotationMatrixColA);
        y = v3_dot(Vertices[1].position, rotationMatrixColB);
        Vertices[1].position[0] = x;
        Vertices[1].position[1] = y;

        x = v3_dot(Vertices[2].position, rotationMatrixColA);
        y = v3_dot(Vertices[2].position, rotationMatrixColB);
        Vertices[2].position[0] = x;
        Vertices[2].position[1] = y;

        x = v3_dot(Vertices[3].position, rotationMatrixColA);
        y = v3_dot(Vertices[3].position, rotationMatrixColB);
        Vertices[3].position[0] = x;
        Vertices[3].position[1] = y;
        break;
    default:
        printf("Something went wrong when trying to rotate.\n");
        break;
    }
}

void shearImage(int direction)
{
    switch(direction)
    {
    case 0:
        printf("You pressed X key.\n");
        Vertices[2].position[0] += 0.1;
        Vertices[1].position[0] += 0.1;
        break;
    case 1:
        printf("You pressed Z key.\n");
        Vertices[2].position[0] -= 0.1;
        Vertices[1].position[0] -= 0.1;
        break;
    default:
        printf("Something went wrong when trying to shear.\n");
        break;
    }
}
void scaleImage(int direction)
{
    switch(direction)
    {
    case 0:
        printf("You pressed A key.\n");
        Vertices[0].position[0] *= 1.1;
        Vertices[0].position[1] *= 1.1;
        Vertices[1].position[0] *= 1.1;
        Vertices[1].position[1] *= 1.1;
        Vertices[2].position[0] *= 1.1;
        Vertices[2].position[1] *= 1.1;
        Vertices[3].position[0] *= 1.1;
        Vertices[3].position[1] *= 1.1;
        break;
    case 1:
        printf("You pressed S key.\n");
        Vertices[0].position[0] *= 1/1.1;
        Vertices[0].position[1] *= 1/1.1;
        Vertices[1].position[0] *= 1/1.1;
        Vertices[1].position[1] *= 1/1.1;
        Vertices[2].position[0] *= 1/1.1;
        Vertices[2].position[1] *= 1/1.1;
        Vertices[3].position[0] *= 1/1.1;
        Vertices[3].position[1] *= 1/1.1;
        break;
    default:
        printf("Something went wrong when trying to scale.\n");
        break;
    }
}



void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
        pan(0);
    else if (key == GLFW_KEY_Q && action == GLFW_PRESS)
        rotateImage(1);
    else if (key == GLFW_KEY_W && action == GLFW_PRESS)
        rotateImage(0);
    else if (key == GLFW_KEY_A && action == GLFW_PRESS)
        scaleImage(0);
    else if (key == GLFW_KEY_S && action == GLFW_PRESS)
        scaleImage(1);
    else if (key == GLFW_KEY_Z && action == GLFW_PRESS)
        shearImage(0);
    else if (key == GLFW_KEY_X && action == GLFW_PRESS)
        shearImage(1);
	else if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
        pan(1);
    else if (key == GLFW_KEY_UP && action == GLFW_PRESS)
        pan(2);
    else if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
        pan(3);
    else if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) //escape exits the program
    {
        printf("closing...");
        glfwTerminate();
        exit(EXIT_SUCCESS);
    }
    else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) //resets the vertices
    {
        printf("Resetting vertices...\n");
        Vertices[0].position[0] = 1;
        Vertices[0].position[1] = -1;
        Vertices[1].position[0] = 1;
        Vertices[1].position[1] = 1;
        Vertices[2].position[0] = -1;
        Vertices[2].position[1] = 1;
        Vertices[3].position[0] = -1;
        Vertices[3].position[1] = -1;
    }
}

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        fprintf(stderr, "Error input: input_filename\n");
        exit(1); 
    }
    fh = fopen(argv[1], "r"); 
    if (fh == 0)
    {
        fprintf(stderr, "Error: Input file \"%s\" could not be opened.\n", argv[1]);
        exit(1); 
    }
    read_header('3'); //read the header of a P3 file
    Pixel* data = calloc(width*height, sizeof(Pixel*)); //allocate memory to hold all of the pixel data
    printf("width: %d\nheight: %d\n", width, height);
    readFile(&data[0]);
    printf("over \n");

    GLint program_id, position_slot, color_slot;
    GLuint vertex_buffer;
    GLuint index_buffer;

    glfwSetErrorCallback(error_callback);

    // Initialize GLFW library
    if (!glfwInit())
        return -1;

    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    // Create and open a window
    window = glfwCreateWindow(WINDOW_WIDTH,
                              WINDOW_HEIGHT,
                              "Mitchell Hewitt Project 5",
                              NULL,
                              NULL);

    if (!window)
    {
        glfwTerminate();
        printf("glfwCreateWindow Error\n");
        exit(1);
    }

    glfwMakeContextCurrent(window);


    // Create and bind texture
    GLuint myTexture;
    glGenTextures(1, &myTexture);
    glBindTexture(GL_TEXTURE_2D, myTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D,
                 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);


    program_id = simple_program();

    glUseProgram(program_id);
	// position
    position_slot = glGetAttribLocation(program_id, "Position");
    color_slot = glGetAttribLocation(program_id, "SourceColor");
    assert(position_slot != -1);
    assert(color_slot != -1);
    glEnableVertexAttribArray(position_slot);
    glEnableVertexAttribArray(color_slot);
	// textures
    GLint texCoordSlot = glGetAttribLocation(program_id, "TexCoordIn");
    assert(texCoordSlot != -1);
    glEnableVertexAttribArray(texCoordSlot);
    GLint textureUniform = glGetUniformLocation(program_id, "Texture");
    assert(textureUniform != -1);


    // Create Buffer
    glGenBuffers(1, &vertex_buffer);

    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

    while (!glfwWindowShouldClose(window))
    {

        glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

        glClearColor(255.0/255.0, 255.0/255.0, 255.0/255.0, 1.0); // set default to black
        glClear(GL_COLOR_BUFFER_BIT);

        glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

        glVertexAttribPointer(position_slot,
                              3,
                              GL_FLOAT,
                              GL_FALSE,
                              sizeof(Vertex),
                              0);

        glVertexAttribPointer(color_slot,
                              4,
                              GL_FLOAT,
                              GL_FALSE,
                              sizeof(Vertex),
                              (GLvoid*) (sizeof(float) * 3));

        glVertexAttribPointer(texCoordSlot,
                              2,
                              GL_FLOAT,
                              GL_FALSE,
                              sizeof(Vertex),
                              sizeof(float)*7);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, myTexture);
        glUniform1i(textureUniform, 0);
        glDrawElements(GL_TRIANGLES,
                       sizeof(Indices) / sizeof(GLubyte),
                       GL_UNSIGNED_BYTE, 0);

        glfwSwapBuffers(window);
        glfwSetKeyCallback(window, key_callback);
        glfwPollEvents();
    }

    //end 
    glfwDestroyWindow(window);
    glfwTerminate();
    exit(EXIT_SUCCESS);
}