#ifndef STLL_OPENGL_INTERNALS_H
#define STLL_OPENGL_INTERNALS_H

#include "ogl_shader.h"

namespace STLL { namespace internal {

// base class that implements the OpenGL functionality
// all OpenGL classes are specialisations of this class
template <int V>
class openGL_internals
{
  public:
    // setup the openGL stuff, get buffers prepared here, and shaders, ...
    void setup(void) { }

    // free everything
    void cleanup(void) { }

    // setup the projection, this might be called only once and then many
    // outputs are performed
    void setupProjection(int width, int height) { }

    // when the texture has changed this will be called and you will need
    // to reuploade it, texture size might change as well
    void updateTexture(const uint8_t * data, int C) { }

    // this is a class you must declare for cached drawing put all variables
    // in here for your cache. Make sure that it can either properly copy
    // or only move and make sure you free all your resources when the objects
    // are freed
    class DrawCacheInternal_c { };

    // this class must contain information you need when doing a draw, e.g. buffers
    // for vertices or such
    class CreateInternal_c { };

    // this is called, when a cache is drawn, at first it is checked, whether
    // the cache that was used to create this drawcache is still useful and if
    // so this drawing function is called
    void drawCache(DrawCacheInternal_c & dc, SubPixelArrangement sp, int sx, int sy, int C) { }

    // when a proper drawing cache is created for a layout to render, this will be
    // called before starting the drawing operations, the draw functions must then
    // add to the cache when end cache preparation is called and you also need to
    // draw the cache
    void startCachePreparation(DrawCacheInternal_c &) { }
    void endCachePreparation(DrawCacheInternal_c & dc, CreateInternal_c &, SubPixelArrangement sp, int sx, int sy, uint32_t atlasId, int C) { }

    // this pair is used, when no cache is going to be made (e.g, the user provided
    // no cache object, or the layout is too complex and requires texture atlas flushes
    void startPreparation(int sx, int sy) { }
    void endPreparation(CreateInternal_c &, SubPixelArrangement sp, int sx, int sy, int C) { }

    // drawing functions for normal rectangles, smooth rectangles, and glyphs
    void drawRectangle(CreateInternal_c & vb, const CommandData_c & ii, const FontAtlasData_c & pos, Color_c c, int C) { }
    void drawSmoothRectangle(CreateInternal_c & vb, const CommandData_c & ii, const FontAtlasData_c & pos, Color_c c, int C) { }
    void drawSubpGlyph() { }
    void drawNormalGlyph(CreateInternal_c & vb, const CommandData_c & ii, const FontAtlasData_c & pos, Color_c c, int C) { }
};


template <>
class openGL_internals<1>
{
  private:
    // Helper function to draw one glyph or one sub pixel color of one glyph
    void drawGlyph(const CommandData_c & i, int subpcol, const FontAtlasData_c & pos, Color_c c, int C)
    {
      double w = pos.width-1;
      double wo = 0;

      if (subpcol > 0)
      {
        w /= 3.0;
        wo = subpcol-2;
      }

      glBegin(GL_QUADS);
      glColor3f(c.r()/255.0, c.g()/255.0, c.b()/255.0);
      glTexCoord2f(1.0*(pos.pos_x+wo)/C,             1.0*(pos.pos_y)/C);            glVertex3f(i.x/64.0+pos.left,   (i.y+32)/64-pos.top,            0);
      glTexCoord2f(1.0*(pos.pos_x+wo+pos.width-1)/C, 1.0*(pos.pos_y)/C);            glVertex3f(i.x/64.0+pos.left+w, (i.y+32)/64-pos.top,            0);
      glTexCoord2f(1.0*(pos.pos_x+wo+pos.width-1)/C, 1.0*(pos.pos_y+pos.rows-1)/C); glVertex3f(i.x/64.0+pos.left+w, (i.y+32)/64-pos.top+pos.rows-1, 0);
      glTexCoord2f(1.0*(pos.pos_x+wo)/C,             1.0*(pos.pos_y+pos.rows-1)/C); glVertex3f(i.x/64.0+pos.left,   (i.y+32)/64-pos.top+pos.rows-1, 0);
      glEnd();
    }

    void drawBuffers(GLuint displayList, SubPixelArrangement /*sp*/, int sx, int sy, float texscaler)
    {
      glMatrixMode(GL_TEXTURE);
      glScalef(1.0f/texscaler, 1.0f/texscaler, 1);

      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      glTranslated(sx/64.0, sy/64, 0);
      glCallList(displayList);
      glPopMatrix();

      glMatrixMode(GL_TEXTURE);
      glLoadIdentity();
    }


  public:

    void setup(void) { }
    void cleanup(void) { }

    void setupProjection(int width, int height)
    {
      glViewport(0, 0, width, height);
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(0, width, height, 0, -1, 1);
      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();
    }

    void updateTexture(const uint8_t * data, int C)
    {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, C, C, 0, GL_ALPHA, GL_UNSIGNED_BYTE, data);
    }

    class DrawCacheInternal_c
    {
      public:
        GLuint vDisplayList;
        uint32_t atlasId;
        uint32_t scale;

        ~DrawCacheInternal_c(void)
        {
          glDeleteLists(vDisplayList, 1);
        }

        DrawCacheInternal_c(void) : vDisplayList(0), atlasId(0) {}

        DrawCacheInternal_c(const DrawCacheInternal_c &) = delete;

        DrawCacheInternal_c(DrawCacheInternal_c && orig)
        {
          vDisplayList = orig.vDisplayList; orig.vDisplayList = 0;
        }

        void operator=(DrawCacheInternal_c && orig)
        {
          vDisplayList = orig.vDisplayList; orig.vDisplayList = 0;
          atlasId = orig.atlasId; orig.atlasId = 0;
        }
    };

    void drawCache(DrawCacheInternal_c & dc, SubPixelArrangement sp, int sx, int sy, int C)
    {
      glEnable(GL_TEXTURE_2D);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      drawBuffers(dc.vDisplayList, sp, sx, sy, C/dc.scale);
    }

    class CreateInternal_c
    {
      public:
        CreateInternal_c(size_t /*s*/) { }
    };

    void startCachePreparation(DrawCacheInternal_c & dc)
    {
      if (dc.vDisplayList == 0)
      {
        dc.vDisplayList = glGenLists(1);
      }
      glNewList(dc.vDisplayList, GL_COMPILE);
    }

    void endCachePreparation(DrawCacheInternal_c & dc, CreateInternal_c &, SubPixelArrangement sp, int sx, int sy, uint32_t atlasId, int C)
    {
      glEndList();
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      drawBuffers(dc.vDisplayList, sp, sx, sy, 1);

      dc.atlasId = atlasId;
      dc.scale = C;
    }

    void startPreparation(int sx, int sy)
    {
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glPushMatrix();
      glTranslated(sx/64.0, sy/64, 0);

    }
    void endPreparation(CreateInternal_c &, SubPixelArrangement /*sp*/, int /*sx*/, int /*sy*/, int /*C*/)
    {
      glPopMatrix();
    }

    void drawRectangle(CreateInternal_c & /*vb*/, const CommandData_c & ii, const FontAtlasData_c & pos, Color_c c, int C)
    {
      glBegin(GL_QUADS);
      glColor3f(c.r()/255.0, c.g()/255.0, c.b()/255.0);
      int x = (ii.x+32)/64;
      int y = (ii.y+32)/64;
      int w = (ii.x+ii.w+32)/64;
      int h = (ii.y+ii.h+32)/64;
      glTexCoord2f(1.0*(pos.pos_x+5)/C, 1.0*(pos.pos_y+5)/C); glVertex3f(x, y, 0);
      glTexCoord2f(1.0*(pos.pos_x+5)/C, 1.0*(pos.pos_y+5)/C); glVertex3f(w, y, 0);
      glTexCoord2f(1.0*(pos.pos_x+5)/C, 1.0*(pos.pos_y+5)/C); glVertex3f(w, h, 0);
      glTexCoord2f(1.0*(pos.pos_x+5)/C, 1.0*(pos.pos_y+5)/C); glVertex3f(x, h, 0);
      glEnd();
    }

    void drawSmoothRectangle(CreateInternal_c & /*vb*/, const CommandData_c & ii, const FontAtlasData_c & pos, Color_c c, int C)
    {
      glBegin(GL_QUADS);
      glColor3f(c.r()/255.0, c.g()/255.0, c.b()/255.0);
      glTexCoord2f(1.0*(pos.pos_x)/C,             1.0*(pos.pos_y)/C);            glVertex3f(ii.x/64.0+pos.left,             (ii.y+32)/64-pos.top,            0);
      glTexCoord2f(1.0*(pos.pos_x+pos.width-1)/C, 1.0*(pos.pos_y)/C);            glVertex3f(ii.x/64.0+pos.left+pos.width-1, (ii.y+32)/64-pos.top,            0);
      glTexCoord2f(1.0*(pos.pos_x+pos.width-1)/C, 1.0*(pos.pos_y+pos.rows-1)/C); glVertex3f(ii.x/64.0+pos.left+pos.width-1, (ii.y+32)/64-pos.top+pos.rows-1, 0);
      glTexCoord2f(1.0*(pos.pos_x)/C,             1.0*(pos.pos_y+pos.rows-1)/C); glVertex3f(ii.x/64.0+pos.left,             (ii.y+32)/64-pos.top+pos.rows-1, 0);
      glEnd();
    }

    void drawNormalGlyph(CreateInternal_c & /*vb*/, const CommandData_c & ii, const FontAtlasData_c & pos, Color_c c, int C)
    {
      drawGlyph(ii, 0, pos, c, C);
    }

    void drawSubpGlyph(CreateInternal_c & /*vb*/, SubPixelArrangement sp, const CommandData_c & ii, const FontAtlasData_c & pos, Color_c c, int C)
    {
      switch (sp)
      {
        case SUBP_RGB:
        {
          glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE); drawGlyph(ii, 1, pos, c, C);
          glColorMask(GL_FALSE, GL_TRUE, GL_FALSE, GL_FALSE); drawGlyph(ii, 2, pos, c, C);
          glColorMask(GL_FALSE, GL_FALSE, GL_TRUE, GL_FALSE); drawGlyph(ii, 3, pos, c, C);
          glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        }
        break;

        case SUBP_BGR:
        {
          glColorMask(GL_FALSE, GL_FALSE, GL_TRUE, GL_FALSE); drawGlyph(ii, 1, pos, c, C);
          glColorMask(GL_FALSE, GL_TRUE, GL_FALSE, GL_FALSE); drawGlyph(ii, 2, pos, c, C);
          glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE); drawGlyph(ii, 3, pos, c, C);
          glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        }
        break;

        default:
        {
          drawGlyph(ii, 0, pos, c, C);
        }
        break;
      }
    }

};

template <>
class openGL_internals<2>
{
  private:

    internal::OGL_Program_c program;

    GLuint vertexBuffer;

    class vertex
    {
    public:
      GLfloat x, y;   // position
      GLfloat u, v;   // texture
      uint8_t r, g, b, a; // colour

      vertex (GLfloat _x, GLfloat _y, GLfloat _u, GLfloat _v, Color_c c) :
      x(_x), y(_y), u(_u), v(_v), r(c.r()), g(c.g()), b(c.b()), a(c.a()) {}
    };

    void drawBuffers(SubPixelArrangement sp, size_t s, int sx, int sy, int C, float texscaler)
    {
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      glEnableVertexAttribArray(0); glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, x));
      glEnableVertexAttribArray(1); glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, u));
      glEnableVertexAttribArray(2); glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, r));

      program.setUniform("offset", sx/64.0, sy/64);
      program.setUniform("texscaler", texscaler);

      switch (sp)
      {
        default:
        case SUBP_NONE:
          program.setUniform("ashift", 0);
          glDrawArrays(GL_QUADS, 0, s);
          break;

        case SUBP_RGB:
          glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE); program.setUniform("ashift", -1.0f/C); glDrawArrays(GL_QUADS, 0, s);
          glColorMask(GL_FALSE, GL_TRUE, GL_FALSE, GL_FALSE); program.setUniform("ashift", -0.0f/C); glDrawArrays(GL_QUADS, 0, s);
          glColorMask(GL_FALSE, GL_FALSE, GL_TRUE, GL_FALSE); program.setUniform("ashift",  1.0f/C); glDrawArrays(GL_QUADS, 0, s);
          glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
          break;

        case SUBP_BGR:
          glColorMask(GL_FALSE, GL_FALSE, GL_TRUE, GL_FALSE); program.setUniform("ashift", -1.0f/C); glDrawArrays(GL_QUADS, 0, s);
          glColorMask(GL_FALSE, GL_TRUE, GL_FALSE, GL_FALSE); program.setUniform("ashift", -0.0f/C); glDrawArrays(GL_QUADS, 0, s);
          glColorMask(GL_TRUE, GL_FALSE, GL_FALSE, GL_FALSE); program.setUniform("ashift",  1.0f/C); glDrawArrays(GL_QUADS, 0, s);
          glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
          break;
      }
    }

  public:

    void setup(void)
    {
      program.attachShader(GL_FRAGMENT_SHADER, "110",
        "uniform sampler2D texture;"

        "void main()"
        "{"
        "  vec2 uv = gl_TexCoord[0].xy;"
        "  vec4 current = texture2D(texture, uv);"
        "  gl_FragColor = vec4(gl_Color.r, gl_Color.g, gl_Color.b, current.a);"
        "}"
      );

      program.attachShader(GL_VERTEX_SHADER, "110",
        "uniform float width;"
        "uniform float height;"
        "uniform float ashift;"
        "uniform float texscaler;"
        "uniform vec2 offset;"

        "attribute vec2 vertex;"
        "attribute vec4 color;"
        "attribute vec2 tex_coord;"
        "void main()"
        "{"
        "  gl_FrontColor = vec4(color.r/255.0, color.g/255.0, color.b/255.0, color.a/255.0);"
        "  gl_TexCoord[0].xy = vec2(tex_coord.x/texscaler+ashift, tex_coord.y/texscaler);"
        "  gl_Position = vec4((vertex.x-width+offset.x)/width, 1.0-(vertex.y+offset.y)/height, 0, 1.0);"
        "}"
      );

      program.bindAttributeLocation(0, "vertex");
      program.bindAttributeLocation(1, "tex_coord");
      program.bindAttributeLocation(2, "color");

      program.link();

      program.setUniform("texture", 0);
      program.setUniform("texscaler", 1.0f);

      glGenBuffers(1, &vertexBuffer);
    }

    void cleanup(void)
    {
      glDeleteBuffers(1, &vertexBuffer);
    }

    void setupProjection(int width, int height)
    {
      glViewport(0, 0, width, height);
      program.setUniform("width", width/2.0f);
      program.setUniform("height", height/2.0f);
    }

    void updateTexture(const uint8_t * data, int C)
    {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, C, C, 0, GL_ALPHA, GL_UNSIGNED_BYTE, data);
    }

    /** \brief class to store a layout in a cached format for even faster repaint */
    class DrawCacheInternal_c
    {
    public:

      GLuint vBuffer;
      size_t elements;
      uint32_t atlasId;
      uint32_t scale;

      ~DrawCacheInternal_c(void)
      {
        glDeleteBuffers(1, &vBuffer);
      }

      DrawCacheInternal_c(void) : vBuffer(0), elements(0), atlasId(0) {}

      DrawCacheInternal_c(const DrawCacheInternal_c &) = delete;

      DrawCacheInternal_c(DrawCacheInternal_c && orig)
      {
        vBuffer = orig.vBuffer; orig.vBuffer = 0;
        elements = orig.elements; orig.elements = 0;
        atlasId = orig.atlasId; orig.atlasId = 0;
      }

      void operator=(DrawCacheInternal_c && orig)
      {
        vBuffer = orig.vBuffer; orig.vBuffer = 0;
        elements = orig.elements; orig.elements = 0;
        atlasId = orig.atlasId; orig.atlasId = 0;
      }
    };


    class CreateInternal_c
    {
      public:
        std::vector<vertex> vb;

        CreateInternal_c(size_t s)
        {
          vb.reserve(s*4);
        }
    };

    void drawCache(DrawCacheInternal_c & dc, SubPixelArrangement sp, int sx, int sy, int C)
    {
      glBindBuffer(GL_ARRAY_BUFFER, dc.vBuffer);
      drawBuffers(sp, dc.elements, sx, sy, C, C/dc.scale);
    }

    void startCachePreparation(DrawCacheInternal_c & dc)
    {
      if (dc.vBuffer == 0) { glGenBuffers(1, &dc.vBuffer); }
      glBindBuffer(GL_ARRAY_BUFFER, dc.vBuffer);
    }
    void endCachePreparation(DrawCacheInternal_c & dc, CreateInternal_c & vb, SubPixelArrangement sp, int sx, int sy, uint32_t atlasId, int C)
    {
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertex)*vb.vb.size(), vb.vb.data(), GL_STATIC_DRAW);

      drawBuffers(sp, vb.vb.size(), sx, sy, C, 1);

      dc.atlasId = atlasId;
      dc.elements = vb.vb.size();
      dc.scale = C;
    }

    void startPreparation(int /*sx*/, int /*sy*/)
    {
      glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    }
    void endPreparation(CreateInternal_c & vb, SubPixelArrangement sp, int sx, int sy, int C)
    {
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertex)*vb.vb.size(), vb.vb.data(), GL_STREAM_DRAW);

      drawBuffers(sp, vb.vb.size(), sx, sy, C, 1);
    }

    void drawRectangle(CreateInternal_c & vb, const CommandData_c & ii, const FontAtlasData_c & pos, Color_c c, int C)
    {
      vb.vb.push_back(vertex((ii.x+32)/64,      (ii.y+32)/64,      1.0*(pos.pos_x+5)/C,           1.0*(pos.pos_y+5)/C,          c));
      vb.vb.push_back(vertex((ii.x+32+ii.w)/64, (ii.y+32)/64,      1.0*(pos.pos_x+pos.width-5)/C, 1.0*(pos.pos_y+5)/C,          c));
      vb.vb.push_back(vertex((ii.x+32+ii.w)/64, (ii.y+32+ii.h)/64, 1.0*(pos.pos_x+pos.width-5)/C, 1.0*(pos.pos_y+pos.rows-5)/C, c));
      vb.vb.push_back(vertex((ii.x+32)/64,      (ii.y+32+ii.h)/64, 1.0*(pos.pos_x+5)/C,           1.0*(pos.pos_y+pos.rows-5)/C, c));
    }

    void drawSmoothRectangle(CreateInternal_c & vb, const CommandData_c & ii, const FontAtlasData_c & pos, Color_c c, int C)
    {
      vb.vb.push_back(vertex((ii.x+32)/64+pos.left,           (ii.y+32)/64-pos.top,         1.0*(pos.pos_x)/C,           1.0*(pos.pos_y)/C,          c));
      vb.vb.push_back(vertex((ii.x+32)/64+pos.left+pos.width, (ii.y+32)/64-pos.top,         1.0*(pos.pos_x+pos.width)/C, 1.0*(pos.pos_y)/C,          c));
      vb.vb.push_back(vertex((ii.x+32)/64+pos.left+pos.width, (ii.y+32)/64-pos.top+pos.rows,1.0*(pos.pos_x+pos.width)/C, 1.0*(pos.pos_y+pos.rows)/C, c));
      vb.vb.push_back(vertex((ii.x+32)/64+pos.left,           (ii.y+32)/64-pos.top+pos.rows,1.0*(pos.pos_x)/C,           1.0*(pos.pos_y+pos.rows)/C, c));
    }

    void drawNormalGlyph(CreateInternal_c & vb, const CommandData_c & ii, const FontAtlasData_c & pos, Color_c c, int C)
    {
      vb.vb.push_back(vertex((ii.x)/64.0+pos.left,           (ii.y+32)/64-pos.top,         1.0*(pos.pos_x)/C,           1.0*(pos.pos_y)/C,          c));
      vb.vb.push_back(vertex((ii.x)/64.0+pos.left+pos.width, (ii.y+32)/64-pos.top,         1.0*(pos.pos_x+pos.width)/C, 1.0*(pos.pos_y)/C,          c));
      vb.vb.push_back(vertex((ii.x)/64.0+pos.left+pos.width, (ii.y+32)/64-pos.top+pos.rows,1.0*(pos.pos_x+pos.width)/C, 1.0*(pos.pos_y+pos.rows)/C, c));
      vb.vb.push_back(vertex((ii.x)/64.0+pos.left,           (ii.y+32)/64-pos.top+pos.rows,1.0*(pos.pos_x)/C,           1.0*(pos.pos_y+pos.rows)/C, c));
    }

    void drawSubpGlyph(CreateInternal_c & vb, SubPixelArrangement /*sp*/, const CommandData_c & ii, const FontAtlasData_c & pos, Color_c c, int C)
    {
      vb.vb.push_back(vertex((ii.x)/64.0+pos.left,               (ii.y+32)/64-pos.top,         1.0*(pos.pos_x)/C,           1.0*(pos.pos_y)/C,          c));
      vb.vb.push_back(vertex((ii.x)/64.0+pos.left+pos.width/3.0, (ii.y+32)/64-pos.top,         1.0*(pos.pos_x+pos.width)/C, 1.0*(pos.pos_y)/C,          c));
      vb.vb.push_back(vertex((ii.x)/64.0+pos.left+pos.width/3.0, (ii.y+32)/64-pos.top+pos.rows,1.0*(pos.pos_x+pos.width)/C, 1.0*(pos.pos_y+pos.rows)/C, c));
      vb.vb.push_back(vertex((ii.x)/64.0+pos.left,               (ii.y+32)/64-pos.top+pos.rows,1.0*(pos.pos_x)/C,           1.0*(pos.pos_y+pos.rows)/C, c));
    }
};

template <>
class openGL_internals<3>
{
  private:
    internal::OGL_Program_c program;

    GLuint vertexBuffer, vertexArray, vertexElement;

    class vertex
    {
    public:
      GLfloat x, y;   // position
      GLfloat u, v;   // texture
      uint8_t r, g, b, a; // colour
      GLbyte sp;

      vertex (GLfloat _x, GLfloat _y, GLfloat _u, GLfloat _v, Color_c c, uint8_t _sp) :
      x(_x), y(_y), u(_u), v(_v), r(c.r()), g(c.g()), b(c.b()), a(c.a()), sp(_sp) {}
    };

    void drawBuffers(SubPixelArrangement sp, size_t s, int sx, int sy, int C, float texscale)
    {
      glBlendFunc(GL_SRC1_COLOR, GL_ONE_MINUS_SRC1_COLOR);
      program.setUniform("offset", 1.0*sx/64.0, sy/64);
      program.setUniform("texscaler", texscale);
      switch (sp)
      {
        default:
        case SUBP_NONE:
          program.setUniform("texRshift", 0.0f, 0.0f);
          program.setUniform("texGshift", 0.0f, 0.0f);
          program.setUniform("texBshift", 0.0f, 0.0f);
          glDrawElements(GL_TRIANGLES, s, GL_UNSIGNED_INT, 0);
          break;

        case SUBP_RGB:
          program.setUniform("texRshift", 0.0f, 0.0f);
          program.setUniform("texGshift", 1.0f/C, 0.0f);
          program.setUniform("texBshift", 2.0f/C, 0.0f);
          glDrawElements(GL_TRIANGLES, s, GL_UNSIGNED_INT, 0);
          break;

        case SUBP_BGR:
          program.setUniform("texRshift", 2.0f/C, 0.0f);
          program.setUniform("texGshift", 1.0f/C, 0.0f);
          program.setUniform("texBshift", 0.0f/C, 0.0f);
          glDrawElements(GL_TRIANGLES, s, GL_UNSIGNED_INT, 0);
          break;
      }
    }

    void setupAttributes(void)
    {
      glEnableVertexAttribArray(0); glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, x));
      glEnableVertexAttribArray(1); glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, u));
      glEnableVertexAttribArray(2); glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, r));
      glEnableVertexAttribArray(3); glVertexAttribPointer(3, 1, GL_BYTE, GL_FALSE, sizeof(vertex), (void*)offsetof(vertex, sp));
    }

  public:
    void setup(void)
    {
      program.attachShader(GL_FRAGMENT_SHADER, "330 core",
        "uniform sampler2D texture;"
        "uniform vec2 texRshift;"
        "uniform vec2 texGshift;"
        "uniform vec2 texBshift;"

        "in vec2 TexCoord;"
        "in vec4 ourColor;"
        "in float sp;"

        "layout (location = 0, index = 0) out vec4 color;"
        "layout (location = 0, index = 1) out vec4 alpha;"

        "void main()"
        "{"
        "  vec4 r = texture2D(texture, TexCoord+sp*texRshift);"
        "  vec4 g = texture2D(texture, TexCoord+sp*texGshift);"
        "  vec4 b = texture2D(texture, TexCoord+sp*texBshift);"
        "  color = ourColor;"
        "  alpha = vec4(r.r, g.r, b.r, 1.0);"
        "}"
      );

      program.attachShader(GL_VERTEX_SHADER, "330 core",
        "uniform float width;"
        "uniform float height;"
        "uniform float texscaler;"
        "uniform vec2 offset;"

        "layout (location = 0) in vec3 vertex;"
        "layout (location = 1) in vec2 tex_coord;"
        "layout (location = 2) in vec4 color;"
        "layout (location = 3) in float subpixels;"

        "out vec2 TexCoord;"
        "out vec4 ourColor;"
        "out float sp;"

        "void main()"
        "{"
        "  ourColor = vec4(color.r/255.0, color.g/255.0, color.b/255.0, color.a/255.0);"
        "  TexCoord = vec2(tex_coord.x, tex_coord.y)/texscaler;"
        "  gl_Position = vec4((vertex.x-width+offset.x)/width, 1.0-(vertex.y+offset.y)/height, 0, 1.0);"
        "  sp = subpixels;"
        "}"
      );

      program.link();

      program.setUniform("texture", 0);
      program.setUniform("texscaler", 1.0f);

      glGenVertexArrays(1, &vertexArray); glBindVertexArray(vertexArray);
      glGenBuffers(1, &vertexBuffer);     glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
      glGenBuffers(1, &vertexElement);    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexElement);

      setupAttributes();
    }

    void cleanup(void)
    {
      glDeleteBuffers(1, &vertexBuffer);
      glDeleteBuffers(1, &vertexElement);
      glDeleteVertexArrays(1, &vertexArray);
    }

    void updateTexture(const uint8_t * data, int C)
    {
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, C, C, 0, GL_RED, GL_UNSIGNED_BYTE, data);
    }

    void setupProjection(int width, int height)
    {
      glViewport(0, 0, width, height);
      program.setUniform("width", width/2.0f);
      program.setUniform("height", height/2.0f);
    }

    /** \brief class to store a layout in a cached format for even faster repaint */
    class DrawCacheInternal_c
    {
      public:

        GLuint vArray, vBuffer, vElements;
        size_t elements;
        uint32_t atlasId;
        uint32_t scale;

        ~DrawCacheInternal_c(void)
        {
          glDeleteBuffers(1, &vBuffer);
          glDeleteBuffers(1, &vElements);
          glDeleteVertexArrays(1, &vArray);
        }

        DrawCacheInternal_c(void) : vArray(0), vBuffer(0), vElements(0), elements(0), atlasId(0) {}

        DrawCacheInternal_c(const DrawCacheInternal_c &) = delete;

        DrawCacheInternal_c(DrawCacheInternal_c && orig)
        {
          vArray = orig.vArray; orig.vArray = 0;
          vBuffer = orig.vBuffer; orig.vBuffer = 0;
          vElements = orig.vElements; orig.vElements = 0;
          elements = orig.elements; orig.elements = 0;
          atlasId = orig.atlasId; orig.atlasId = 0;
        }

        void operator=(DrawCacheInternal_c && orig)
        {
          vArray = orig.vArray; orig.vArray = 0;
          vBuffer = orig.vBuffer; orig.vBuffer = 0;
          vElements = orig.vElements; orig.vElements = 0;
          elements = orig.elements; orig.elements = 0;
          atlasId = orig.atlasId; orig.atlasId = 0;
        }
    };

    void drawCache(DrawCacheInternal_c & dc, SubPixelArrangement sp, int sx, int sy, int C)
    {
      glBindVertexArray(dc.vArray);
      drawBuffers(sp, dc.elements, sx, sy, C, C/dc.scale);
    }

    class CreateInternal_c
    {
      public:
        std::vector<vertex> vb;
        std::vector<GLuint> vbe;

        CreateInternal_c(size_t s)
        {
          vb.reserve(s*4);
          vbe.reserve(s*6);
        }
    };

    void startCachePreparation(DrawCacheInternal_c & dc)
    {
      if (dc.vArray == 0)    { glGenVertexArrays(1, &dc.vArray); } glBindVertexArray(dc.vArray);
      if (dc.vBuffer == 0)   { glGenBuffers(1, &dc.vBuffer);     } glBindBuffer(GL_ARRAY_BUFFER, dc.vBuffer);
      if (dc.vElements == 0) { glGenBuffers(1, &dc.vElements);   } glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dc.vElements);

      setupAttributes();
    }
    void endCachePreparation(DrawCacheInternal_c & dc, CreateInternal_c & vb, SubPixelArrangement sp, int sx, int sy, uint32_t atlasId, int C)
    {
      uploadAndDraw(vb, sp, sx, sy, C, GL_STATIC_DRAW);

      dc.atlasId = atlasId;
      dc.elements = vb.vbe.size();
      dc.scale = C;
    }

    void startPreparation(int /*sx*/, int /*sy*/)
    {
      glBindVertexArray(vertexArray);
    }
    void endPreparation(CreateInternal_c & vb, SubPixelArrangement sp, int sx, int sy, int C)
    {
      uploadAndDraw(vb, sp, sx, sy, C, GL_STREAM_DRAW);
    }

    void drawRectangle(CreateInternal_c & vb, const CommandData_c & ii, const FontAtlasData_c & pos, Color_c c, int C)
    {
      std::array<float, 8> data;
      data[0] = (ii.x+32)/64;         data[1] = (ii.x+32+ii.w)/64;
      data[2] = (ii.y+32)/64;         data[3] = (ii.y+32+ii.h)/64;
      data[4] = 1.0*(pos.pos_x+5)/C;  data[5] = 1.0*(pos.pos_x+pos.width-6)/C;
      data[6] = 1.0*(pos.pos_y+5)/C;  data[7] = 1.0*(pos.pos_y+pos.rows-6)/C;

      addQuad(vb, data, c, 0);
    }

    void drawSmoothRectangle(CreateInternal_c & vb, const CommandData_c & ii, const FontAtlasData_c & pos, Color_c c, int C)
    {
      std::array<float, 8> data;
      data[0] = (ii.x+32)/64+pos.left; data[1] = (ii.x+32)/64+pos.left+pos.width;
      data[2] = (ii.y+32)/64-pos.top;  data[3] = (ii.y+32)/64-pos.top+pos.rows;
      data[4] = 1.0*(pos.pos_x)/C;     data[5] = 1.0*(pos.pos_x+pos.width)/C;
      data[6] = 1.0*(pos.pos_y)/C;     data[7] = 1.0*(pos.pos_y+pos.rows)/C;

      addQuad(vb, data, c, 1);
    }

    void drawNormalGlyph(CreateInternal_c & vb, const CommandData_c & ii, const FontAtlasData_c & pos, Color_c c, int C)
    {
      std::array<float, 8> data;
      data[0] = (ii.x)/64.0+pos.left; data[1] = (ii.x)/64.0+pos.left+pos.width;
      data[2] = (ii.y+32)/64-pos.top; data[3] = (ii.y+32)/64-pos.top+pos.rows;
      data[4] = 1.0*(pos.pos_x)/C;    data[5] = 1.0*(pos.pos_x+pos.width)/C;
      data[6] = 1.0*(pos.pos_y)/C;    data[7] = 1.0*(pos.pos_y+pos.rows)/C;

      addQuad(vb, data, c, 0);
    }

    void drawSubpGlyph(CreateInternal_c & vb, SubPixelArrangement /*sp*/, const CommandData_c & ii, const FontAtlasData_c & pos, Color_c c, int C)
    {
      std::array<float, 8> data;
      data[0] = ii.x/64.0+pos.left;   data[1] = ii.x/64.0+pos.left+(pos.width-1)/3.0;
      data[2] = (ii.y+32)/64-pos.top; data[3] = (ii.y+32)/64-pos.top+pos.rows;
      data[4] = 1.0*(pos.pos_x)/C;    data[5] = 1.0*(pos.pos_x+pos.width-1)/C;
      data[6] = 1.0*(pos.pos_y)/C;    data[7] = 1.0*(pos.pos_y+pos.rows)/C;

      addQuad(vb, data, c, 1);
    }

  private:

    void uploadAndDraw(const CreateInternal_c & vb, SubPixelArrangement sp, int sx, int sy, int C, int drawMode)
    {
      glBufferData(GL_ARRAY_BUFFER, sizeof(vertex)*vb.vb.size(), vb.vb.data(), drawMode);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLint)*vb.vbe.size(), vb.vbe.data(), drawMode);
      drawBuffers(sp, vb.vbe.size(), sx, sy, C, 1);
    }

    void addQuad(CreateInternal_c & vb, std::array<float, 8> & data, Color_c c, int sp)
    {
      GLshort si = vb.vb.size();
      vb.vb.push_back(vertex(data[0], data[2], data[4], data[6], c, sp));
      vb.vb.push_back(vertex(data[1], data[2], data[5], data[6], c, sp));
      vb.vb.push_back(vertex(data[1], data[3], data[5], data[7], c, sp));
      vb.vb.push_back(vertex(data[0], data[3], data[4], data[7], c, sp));
      vb.vbe.push_back(si+0); vb.vbe.push_back(si+1); vb.vbe.push_back(si+2);
      vb.vbe.push_back(si+0); vb.vbe.push_back(si+2); vb.vbe.push_back(si+3);
    }

};

} }

#endif
