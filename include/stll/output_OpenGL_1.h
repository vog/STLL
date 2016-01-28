/*
 * STLL Simple Text Layouting Library
 *
 * STLL is the legal property of its developers, whose
 * names are listed in the COPYRIGHT file, which is included
 * within the source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#ifndef STLL_LAYOUTER_OPEN_GL
#define STLL_LAYOUTER_OPEN_GL

/** \file
 *  \brief OpenGL output driver
 */

#include "layouterFont.h"
#include "color.h"

#include "internal/glyphAtlas.h"
#include "internal/gamma.h"
#include "internal/openGL_internal.h"

namespace STLL {

/** \brief a class to output layouts using OpenGL
 *
 * To output layouts using this class, create an object of it and then
 * use the showLayout Function to output the layout.
 *
 * The class contains a glyph cache in form of an texture atlas. Once this
 * atlas if full, things available will be output and then the atlas will be cleared
 * and repopulated for the next section of the output. This will slow down output
 * considerably, so choose the size wisely. The atlas will be destroyed once the
 * class is destroyed. Things to consider:
 * - using sub pixel placement triples the space requirements for the glyphs
 * - blurring adds quite some amount of space around the glyphs, but as soon as you blurr the
 *   sub pixel placement will not be used as you will not see the difference anyways
 * - normal rectangles will not go into the cache, but blurres ones will
 * - so in short avoid blurring
 *
 * As OpenGL required function loaders and does not provide a header with all available
 * functionality you will need to include the proper OpenGL before including the header file
 * for this class.
 *
 * Gamma correct output is not handled by this class directly. You need to activate the sRGB
 * property for the target that this paints on
 *
 * \tparam C size of the texture cache. The cache is square C time C pixels.
 * \tparam G the gamma calculation function, if you use sRGB output... normally you don't need
 * to change this, keep the default
 */
template <int C, class G = internal::Gamma_c<>>
class showOpenGL : internal::openGL_internals<1>
{
  private:
    internal::GlyphAtlas_c cache;
    G gamma;

    GLuint glTextureId = 0;     // OpenGL texture id
    uint32_t uploadVersion = 0; // a counter changed each time the texture changes to know when to update
    uint32_t atlasId = 1;

  public:

    typedef DrawCacheInternal_c DrawCache_c;

    /** \brief constructor */
    showOpenGL(void) : cache(C, C)
    {
      glActiveTexture(GL_TEXTURE0);
      glGenTextures(1, &glTextureId);
      glBindTexture(GL_TEXTURE_2D, glTextureId);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexEnvi(GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_REPLACE);
      gamma.setGamma(22);

      setup();
    }

    ~showOpenGL(void)
    {
        glDeleteTextures(1, &glTextureId);
        cleanup();
    }

    /** \brief helper class used to draw images */
    class imageDrawerOpenGL_c
    {
      public:
        virtual void draw(int32_t x, int32_t y, uint32_t w, uint32_t h, const std::string & url) = 0;
    };


    /** \brief paint the layout
     *
     * \param l the layout to draw
     * \param sx x position on the target surface in 1/64th pixels
     * \param sy y position on the target surface in 1/64th pixels
     * \param sp which kind of sub-pixel positioning do you want?
     * \param images a pointer to an image drawer class that is used to draw the images, when you give
     *                a nullptr here, no images will be drawn
     * \return if true then the cache had to be cleared... which might hint at a problem with your cache
     * size
     */
    void showLayout(const TextLayout_c & l, int sx, int sy, SubPixelArrangement sp,
                    imageDrawerOpenGL_c * images = nullptr, DrawCache_c * dc = nullptr)
    {
      glActiveTexture(GL_TEXTURE0);
      glBindTexture( GL_TEXTURE_2D, glTextureId );
      glEnable(GL_BLEND);

      if (dc && dc->atlasId == atlasId)
      {
        drawCache(*dc, sp, sx, sy, C);
        return;
      }

      const auto & dat = l.getData();
      size_t i = 0;
      bool cleared = false;

      while (i < dat.size())
      {
        size_t j = i;

        // make sure that there is a small completely filled rectangle
        // used for drawing filled rectangles
        cache.getRect(640, 640, SUBP_NONE, 0);

        while (j < dat.size())
        {
          auto & ii = dat[j];

          bool found = true;

          switch (ii.command)
          {
            case CommandData_c::CMD_GLYPH:
              // when subpixel placement is on we always create all 3 required images
              found &= (bool)cache.getGlyph(ii.font, ii.glyphIndex, sp, ii.blurr);
              break;
            case CommandData_c::CMD_RECT:
              if (ii.blurr > 0)
                found &= (bool)cache.getRect(ii.w, ii.h, sp, ii.blurr);
              break;

            default:
              break;
          }

          if (!found) break;

          j++;
        }

        if (cache.getVersion() != uploadVersion)
        {
          uploadVersion = cache.getVersion();
          updateTexture(cache.getData(), C);
        }

        CreateInternal_c vb(dat.size());

        size_t k = i;

        if (dc && !cleared && j == dat.size())
        {
          startCachePreparation(*dc);
        }
        else
        {
          startPreparation(sx, sy);
        }

        while (k < j)
        {
          auto & ii = dat[k];

          switch (ii.command)
          {
            case CommandData_c::CMD_GLYPH:
              {
                auto pos = cache.getGlyph(ii.font, ii.glyphIndex, sp, ii.blurr).value();
                Color_c c = gamma.forward(ii.c);

                if ((sp == SUBP_RGB || sp == SUBP_BGR) && (ii.blurr <= cache.blurrmax))
                {
                  drawSubpGlyph(vb, sp, ii, pos, c, C);
                }
                else
                {
                  drawNormalGlyph(vb, ii, pos, c, C);
                }
              }
              break;

            case CommandData_c::CMD_RECT:
              if (ii.blurr == 0)
              {
                auto pos = cache.getRect(640, 640, SUBP_NONE, 0).value();
                Color_c c = gamma.forward(ii.c);
                drawRectangle(vb, ii, pos, c, C);
              }
              else
              {
                auto pos = cache.getRect(ii.w, ii.h, sp, ii.blurr).value();
                Color_c c = gamma.forward(ii.c);
                drawSmoothRectangle(vb, ii, pos, c, C);
              }
              break;

            case CommandData_c::CMD_IMAGE:
              if (images)
                images->draw(ii.x+sx, ii.y+sy, ii.w, ii.h, ii.imageURL);
              break;
          }
          k++;
        }

        if (dc && !cleared && j == dat.size())
        {
          endCachePreparation(*dc, vb, sp, sx, sy, atlasId, C);
          return;
        }
        else
        {
          endPreparation(vb, sp, sx, sy, C);
        }

        i = j;

        if (i < dat.size())
        {
          // atlas is not big enough, it needs to be cleared
          // and will be repopulated for the next batch of the layout
          cache.clear();
          atlasId++;
          cleared = true;
        }
      }

      return;
    }

    /** \brief helper function to setup the projection matrices for
     * the showLayout function. It will change the viewport and the
     * modelview and projection matrix to an orthogonal projection
     */
    void setupMatrixes(int width, int height)
    {
      setupProjection(width, height);
    }

    /** \brief get a pointer to the texture atlas with all the glyphs
     *
     * This is mainly helpful to check how full the texture atlas is
     */
    const uint8_t * getData(void) const { return cache.getData(); }

    void clear(void)
    {
      cache.clear();
      atlasId++;
    }
};

}

#endif