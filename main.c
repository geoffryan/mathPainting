#include <OpenGL/gl3.h>
#define __gl_h_
#include <OpenGL/glu.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <stdio.h>
#include <stdlib.h>

const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

const char * read_txt_file(const char *filename)
{
    FILE *f = fopen(filename, "r");
    if(f == NULL)
    {
        printf("Could not read %s\n", filename);
        return NULL;
    }

    // Go to end of file.
    int err = fseek(f, 0, SEEK_END);
    if(err)
    {
        fclose(f);
        return NULL;
    }
    
    // Get current read position, this is the length!
    long length = ftell(f);
   
    // Go back to beginning of file
    err = fseek(f, 0, SEEK_SET);
    if(err)
    {
        fclose(f);
        return NULL;
    }

    // Allocate space
    char *contents = (char *)malloc((length+1) * sizeof(char));
    if(contents == NULL)
    {
        printf("Could not allocate space for file %s\n", filename);
        fclose(f);
        return NULL;
    }

    // Read the thing!
    size_t actual_length = fread(contents, sizeof(char), length, f);
    contents[actual_length] = '\0';

    fclose(f);

    return contents;
}

void printShaderLog(GLuint shader)
{
    if(!glIsShader(shader))
    {
        printf("GL ID %lu is not a shader\n", (unsigned long) shader);
        return;
    }

    GLsizei infoLogLength;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
    GLchar infoLog[infoLogLength+1];
    glGetShaderInfoLog(shader, infoLogLength, NULL, infoLog);
    infoLog[infoLogLength] = '\0';
    printf("Shader %lu Log\n", (unsigned long) shader);
    printf("%s\n", infoLog);
}

void printProgramLog(GLuint program)
{
    if(!glIsProgram(program))
    {
        printf("GL ID %lu is not a program\n", (unsigned long) program);
        return;
    }

    GLsizei infoLogLength;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
    
    GLchar infoLog[infoLogLength+1];
    glGetProgramInfoLog(program, infoLogLength, NULL, infoLog);
    infoLog[infoLogLength] = '\0';
    
    printf("Program %lu Log\n", (unsigned long) program);
    printf("%s\n", infoLog);
}

GLuint compileShader(const char *shader_path, int shader_type)
{
    const char *shader_src = read_txt_file(shader_path);
    if(shader_src == NULL)
    {
        printf("Error reading shader file: %s\n", shader_path);
        return 0;
    }
    
    GLuint shaderID = glCreateShader(shader_type);

    glShaderSource(shaderID, 1, &shader_src, NULL);
    free((char *) shader_src);
    
    glCompileShader(shaderID);
    
    GLint status;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status);
    if(!status)
    {
        printf("Error compiling shader\n");
        printShaderLog(shaderID);
        glDeleteShader(shaderID);
        return 0;
    }

    return shaderID;
}

int loadGLProgram(GLuint *programID, const char *vrtx_shader_path,
                  const char *frag_shader_path)
{
    GLuint vrtxID = compileShader(vrtx_shader_path, GL_VERTEX_SHADER);
    if(vrtxID == 0)
    {
        printf("Error making vertex shader\n");
        return 1;
    }

    GLuint fragID = compileShader(frag_shader_path, GL_FRAGMENT_SHADER);
    if(fragID == 0)
    {
        printf("Error making vertex shader\n");
        glDeleteShader(vrtxID);
        return 1;
    }


    *programID = glCreateProgram();
    glAttachShader(*programID, vrtxID);
    glAttachShader(*programID, fragID);
    glLinkProgram(*programID);
    
    GLint status;
    glGetProgramiv(*programID, GL_LINK_STATUS, &status);
    if(!status)
    {
        printf("Error Linking Program\n");
        printProgramLog(*programID);
        glDetachShader(*programID, vrtxID);
        glDetachShader(*programID, fragID);
        glDeleteShader(vrtxID);
        glDeleteShader(fragID);
        return 1;
    }

    glDetachShader(*programID, vrtxID);
    glDetachShader(*programID, fragID);

    return 0;
}

int main( int argc, char* args[] )
{
	SDL_Window* window = NULL;
    SDL_GLContext context;

	//Initialize SDL
	if( SDL_Init( SDL_INIT_VIDEO ) < 0 )
	{
		printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
	    SDL_DestroyWindow( window );
        SDL_Quit();
        return 0;
	}

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                            SDL_GL_CONTEXT_PROFILE_CORE);

    //Create window
    window = SDL_CreateWindow("Painting With Maths",
                            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                            SCREEN_WIDTH, SCREEN_HEIGHT,
                            SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);

    if( window == NULL )
    {
        printf("Window could not be created! SDL_Error: %s\n",
               SDL_GetError() );
        SDL_DestroyWindow( window );
        SDL_Quit();
        return 0;
    }
    context = SDL_GL_CreateContext(window);

    if(context == NULL)
    {
        printf("OpenGL context could not be created! SDL_Error: %s\n",
               SDL_GetError() );
        SDL_DestroyWindow( window );
        SDL_Quit();
        return 0;
    }

    //SDL_GL_SetSwapInterval(1);

    GLuint programID;
    int status = loadGLProgram(&programID, "canvas.glsl", "paint.glsl");
    if(status)
    {
        printf("Couldn't load GL Program\n");
        glDeleteProgram(programID);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }

    glClearColor(0.5f, 0.8f, 0.4f, 1.0f); 

    int width, height;
    SDL_GetWindowSize(window, &width, &height);

    
    GLfloat xloc, yloc;
    if(width < height)
    {
        xloc = 1.0;
        yloc = ((GLfloat) width) / height;
    }
    else
    {
        xloc = ((GLfloat) height) / width;
        yloc = 1.0;
    }
    GLfloat vertexData[] = {-xloc, -yloc, xloc, -yloc,
                            xloc, yloc, -xloc, yloc};
    GLfloat uvData[] = {0.0f, 0.0f, 1.0f, 0.0f,
                            1.0f, 1.0f, 0.0f, 1.0f};

    GLuint vertexArray;
    glGenVertexArrays(1, &vertexArray);
    glBindVertexArray(vertexArray);

    GLuint vertexBuffer;
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, 8*sizeof(GLfloat),
                 vertexData, GL_STATIC_DRAW);

    GLuint uvBuffer;
    glGenBuffers(1, &uvBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glBufferData(GL_ARRAY_BUFFER, 8*sizeof(GLfloat),
                 uvData, GL_STATIC_DRAW);

    GLint vertexPositionLocation = glGetAttribLocation(programID,
                                                        "vertexPosition");
    GLint vertexUVLocation = glGetAttribLocation(programID,
                                                        "vertexUV");
    if(vertexPositionLocation < 0)
    {
        printf("Bad variable name\n");
        glDeleteProgram(programID);
        SDL_DestroyWindow( window );
        SDL_Quit();
        return 0;
    }


    /*
	SDL_Surface* screenSurface = NULL;
    screenSurface = SDL_GetWindowSurface( window );

    SDL_FillRect(screenSurface, NULL,
                 SDL_MapRGB( screenSurface->format, 0xFF, 0xFF, 0xFF ) );
    
    SDL_UpdateWindowSurface( window );
    */

    int quit = 0;

    //Enter event loop
    while(!quit)
    {
        SDL_Event e;
        while(SDL_PollEvent(&e))
        {
            if(e.type == SDL_QUIT)
            {
                quit = 1;
            }
            else if(e.type == SDL_KEYDOWN)
            {
                switch(e.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                        quit = 1;
                        break;
                    case SDLK_q:
                        quit = 1;
                        break;
                }
            }

            glClear(GL_COLOR_BUFFER_BIT);
            glUseProgram(programID);

            glEnableVertexAttribArray(vertexPositionLocation);
            glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
            glVertexAttribPointer(vertexPositionLocation, 2, GL_FLOAT,
                                  GL_FALSE, 0, NULL);

            glEnableVertexAttribArray(vertexUVLocation);
            glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
            glVertexAttribPointer(vertexUVLocation, 2, GL_FLOAT,
                                  GL_FALSE, 0, NULL);

            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

            glDisableVertexAttribArray(vertexPositionLocation);

            SDL_GL_SwapWindow(window);
        }
    }

	//Destroy window

    glDeleteProgram(programID);
	SDL_DestroyWindow( window );
	SDL_Quit();

	return 0;
}
