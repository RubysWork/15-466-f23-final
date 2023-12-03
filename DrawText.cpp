#include "DrawText.hpp"

// Inspired by https://learnopengl.com/In-Practice/Text-Rendering
void DrawText::HB_FT_Init(const char *filename, int size)
{
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Initialize FreeType
    FT_Library library;
    FT_Face face;
    FT_Init_FreeType(&library);
    FT_New_Face(library, filename, 0, &face);
    FT_Set_Char_Size(face, size * 64, 0, 72, 0);
    // FT_Set_Pixel_Sizes(face, 0, 48);

    // Harfbuzz from codepoint to glyph
    //  hb_buffer_t *buf = hb_buffer_create();
    //  hb_buffer_add_utf8(buf, characters, -1, 0, -1);

    // hb_buffer_set_direction(buf, HB_DIRECTION_LTR);
    // hb_buffer_set_script(buf, HB_SCRIPT_LATIN);
    // hb_buffer_set_language(buf, hb_language_from_string("en", -1));

    // //create hb_font from ft
    // hb_font_t *font = hb_ft_font_create(face, NULL);

    // hb_shape(font, buf, NULL, 0);

    // unsigned int glyph_count;
    // hb_glyph_info_t *glyph_info = hb_buffer_get_glyph_infos(buf, &glyph_count);
    // hb_glyph_position_t *glyph_pos = hb_buffer_get_glyph_positions(buf, &glyph_count);

    // Inspired by https://learnopengl.com/In-Practice/Text-Rendering
    //  Disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Load first 128 characters of ASCII set
    for (GLubyte c = 0; c < 128; c++)
    {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }
        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer);
        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            (GLuint)face->glyph->advance.x};
        Characters.insert(std::pair<GLchar, Character>(c, character));
    }
    // Unbind texture
    glBindTexture(GL_TEXTURE_2D, 0);
    // Destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(library);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // 6 points including 2 position and 2 texcoord data
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

GLuint DrawText::CreateTextShader()
{
    GLuint program = gl_compile_program(
        // vertex shader:
        "#version 330\n"
        "layout(location = 0) in vec4 vertex;\n"
        "out vec2 texCoord;\n"
        "uniform mat4 projection;\n"
        "void main() {\n"
        "	gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);\n"
        "	texCoord = vertex.zw;\n"
        "}\n",
        // fragment shader:
        "#version 330\n"
        "in vec2 texCoord;\n"
        "out vec4 fragColor;\n"
        "uniform sampler2D text;\n"
        "uniform vec3 text_color;\n"
        "void main() {\n"
        "	vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, texCoord).r);\n"
        "	fragColor = vec4(text_color, 1.0) * sampled;\n"
        "}\n");

    return program;
}

void DrawText::Draw(GLuint shader, std::string text, GLfloat x, GLfloat y, glm::vec2 screen_size, glm::vec3 color, GLfloat scale)
{
    glUseProgram(shader);
    glUniform3f(glGetUniformLocation(shader, "text_color"), color.x, color.y, color.z);
    glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(screen_size.x), 0.0f, static_cast<GLfloat>(screen_size.y));
    glUniformMatrix4fv(glGetUniformLocation(shader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(vao);

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = Characters[*c];

        GLfloat xpos = (GLfloat)((int)(x + ch.Bearing.x * scale) % 1010) + 100.0f;
        GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale - (GLfloat)((int)(x + ch.Bearing.x * scale) / 1010) * 40;

        GLfloat w = ch.Size.x * scale;
        GLfloat h = ch.Size.y * scale;
        // Update VBO for each character
        GLfloat vertices[6][4] = {
            {xpos, ypos + h, 0.0, 0.0},
            {xpos, ypos, 0.0, 1.0},
            {xpos + w, ypos, 1.0, 1.0},

            {xpos, ypos + h, 0.0, 0.0},
            {xpos + w, ypos, 1.0, 1.0},
            {xpos + w, ypos + h, 1.0, 0.0}};
        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices); // Be sure to use glBufferSubData and not glBufferData
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}
