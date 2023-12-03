#include <ft2build.h>
#include FT_FREETYPE_H

#include <hb.h>
#include <hb-ft.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <map>
#include "GL.hpp"
#include "gl_compile_program.hpp"

//Inspired by https://learnopengl.com/In-Practice/Text-Rendering
class DrawText
{
public:
    struct Character
    {
        GLuint TextureID;
        glm::ivec2 Size;
        glm::ivec2 Bearing;
        GLuint Advance;
    };

	std::map<GLchar, Character> Characters;
	GLuint vao, vbo;

    void HB_FT_Init(const char* filename, int size);
    GLuint CreateTextShader();

    void Draw(GLuint shader, std::string text, GLfloat x, GLfloat y, glm::vec2 screen_size, glm::vec3 color = glm::vec3(1.0f), GLfloat scale = 1.0f);
};
