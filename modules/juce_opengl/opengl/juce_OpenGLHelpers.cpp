/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

BEGIN_JUCE_NAMESPACE

//==============================================================================
void OpenGLHelpers::resetErrorState()
{
    while (glGetError() != GL_NO_ERROR) {}
}

void* OpenGLHelpers::getExtensionFunction (const char* functionName)
{
   #if JUCE_WINDOWS
    return (void*) wglGetProcAddress (functionName);

   #elif JUCE_MAC
    static void* handle = dlopen (nullptr, RTLD_LAZY);
    return dlsym (handle, functionName);

   #elif JUCE_LINUX
    return (void*) glXGetProcAddress ((const GLubyte*) functionName);
   #endif
}

#if ! JUCE_OPENGL_ES
namespace
{
    bool isExtensionSupportedV3 (const char* extensionName)
    {
       #ifndef GL_NUM_EXTENSIONS
        enum { GL_NUM_EXTENSIONS = 0x821d };
       #endif

        JUCE_DECLARE_GL_EXTENSION_FUNCTION (glGetStringi, const GLubyte*, (GLenum, GLuint))

        if (glGetStringi == nullptr)
            glGetStringi = (type_glGetStringi) OpenGLHelpers::getExtensionFunction ("glGetStringi");

        if (glGetStringi != nullptr)
        {
            GLint numExtensions = 0;
            glGetIntegerv (GL_NUM_EXTENSIONS, &numExtensions);

            for (int i = 0; i < numExtensions; ++i)
                if (strcmp (extensionName, (const char*) glGetStringi (GL_EXTENSIONS, i)) == 0)
                    return true;
        }

        return false;
    }
}
#endif

bool OpenGLHelpers::isExtensionSupported (const char* const extensionName)
{
    jassert (extensionName != nullptr); // you must supply a genuine string for this.
    jassert (isContextActive()); // An OpenGL context will need to be active before calling this.

   #if ! JUCE_OPENGL_ES
    const GLubyte* version = glGetString (GL_VERSION);

    if (version != nullptr && version[0] >= '3')
    {
        return isExtensionSupportedV3 (extensionName);
    }
    else
   #endif
    {
        const char* extensions = (const char*) glGetString (GL_EXTENSIONS);
        jassert (extensions != nullptr); // Perhaps you didn't activate an OpenGL context before calling this?

        for (;;)
        {
            const char* found = strstr (extensions, extensionName);

            if (found == nullptr)
                break;

            extensions = found + strlen (extensionName);

            if (extensions[0] == ' ' || extensions[0] == 0)
                return true;
        }
    }

    return false;
}

void OpenGLHelpers::clear (const Colour& colour)
{
    glClearColor (colour.getFloatRed(), colour.getFloatGreen(),
                  colour.getFloatBlue(), colour.getFloatAlpha());

    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void OpenGLHelpers::setColour (const Colour& colour)
{
    glColor4f (colour.getFloatRed(), colour.getFloatGreen(),
               colour.getFloatBlue(), colour.getFloatAlpha());
}

void OpenGLHelpers::prepareFor2D (const int width, const int height)
{
    glMatrixMode (GL_PROJECTION);
    glLoadIdentity();

   #if JUCE_OPENGL_ES
    glOrthof (0.0f, (GLfloat) width, 0.0f, (GLfloat) height, 0.0f, 1.0f);
   #else
    glOrtho  (0.0, width, 0.0, height, 0, 1);
   #endif

    glViewport (0, 0, width, height);
}

void OpenGLHelpers::setPerspective (double fovy, double aspect, double zNear, double zFar)
{
    glLoadIdentity();

   #if JUCE_OPENGL_ES
    const GLfloat ymax = (GLfloat) (zNear * tan (fovy * double_Pi / 360.0));
    const GLfloat ymin = -ymax;

    glFrustumf (ymin * (GLfloat) aspect, ymax * (GLfloat) aspect, ymin, ymax, (GLfloat) zNear, (GLfloat) zFar);
   #else
    const double ymax = zNear * tan (fovy * double_Pi / 360.0);
    const double ymin = -ymax;

    glFrustum  (ymin * aspect, ymax * aspect, ymin, ymax, zNear, zFar);
   #endif
}

void OpenGLHelpers::applyTransform (const AffineTransform& t)
{
    const GLfloat m[] = { t.mat00, t.mat10, 0, 0,
                          t.mat01, t.mat11, 0, 0,
                          0,       0,       1, 0,
                          t.mat02, t.mat12, 0, 1 };
    glMultMatrixf (m);
}

void OpenGLHelpers::drawQuad2D (float x1, float y1,
                                float x2, float y2,
                                float x3, float y3,
                                float x4, float y4,
                                const Colour& colour)
{
    const GLfloat vertices[]      = { x1, y1, x2, y2, x4, y4, x3, y3 };
    const GLfloat textureCoords[] = { 0, 0, 1.0f, 0, 0, 1.0f, 1.0f, 1.0f };

    setColour (colour);

    glEnableClientState (GL_VERTEX_ARRAY);
    glVertexPointer (2, GL_FLOAT, 0, vertices);

    glEnableClientState (GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer (2, GL_FLOAT, 0, textureCoords);

    glDisableClientState (GL_COLOR_ARRAY);
    glDisableClientState (GL_NORMAL_ARRAY);

    glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
}

void OpenGLHelpers::drawQuad3D (float x1, float y1, float z1,
                                float x2, float y2, float z2,
                                float x3, float y3, float z3,
                                float x4, float y4, float z4,
                                const Colour& colour)
{
    const GLfloat vertices[]      = { x1, y1, z1, x2, y2, z2, x4, y4, z4, x3, y3, z3 };
    const GLfloat textureCoords[] = { 0, 0, 1.0f, 0, 0, 1.0f, 1.0f, 1.0f };

    setColour (colour);

    glEnableClientState (GL_VERTEX_ARRAY);
    glVertexPointer (3, GL_FLOAT, 0, vertices);

    glEnableClientState (GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer (2, GL_FLOAT, 0, textureCoords);

    glDisableClientState (GL_COLOR_ARRAY);
    glDisableClientState (GL_NORMAL_ARRAY);

    glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
}

//==============================================================================
namespace OpenGLGradientHelpers
{
    void drawTriangles (GLenum mode, const GLfloat* vertices, const GLfloat* textureCoords, const int numElements)
    {
        glEnable (GL_TEXTURE_2D);
        glEnableClientState (GL_VERTEX_ARRAY);
        glEnableClientState (GL_TEXTURE_COORD_ARRAY);
        glDisableClientState (GL_COLOR_ARRAY);
        glDisableClientState (GL_NORMAL_ARRAY);

        glVertexPointer (2, GL_FLOAT, 0, vertices);
        glTexCoordPointer (2, GL_FLOAT, 0, textureCoords);

        glColor4f (1.0f, 1.0f, 1.0f, 1.0f);
        glDrawArrays (mode, 0, numElements);
    }

    void fillWithLinearGradient (const Rectangle<int>& rect,
                                 const ColourGradient& grad,
                                 const AffineTransform& transform,
                                 const int textureSize)
    {
        const Point<float> p1 (grad.point1.transformedBy (transform));
        const Point<float> p2 (grad.point2.transformedBy (transform));
        const Point<float> p3 (Point<float> (grad.point1.getX() - (grad.point2.getY() - grad.point1.getY()) / textureSize,
                                             grad.point1.getY() + (grad.point2.getX() - grad.point1.getX()) / textureSize).transformedBy (transform));

        const AffineTransform textureTransform (AffineTransform::fromTargetPoints (p1.getX(), p1.getY(),  0.0f, 0.0f,
                                                                                   p2.getX(), p2.getY(),  1.0f, 0.0f,
                                                                                   p3.getX(), p3.getY(),  0.0f, 1.0f));

        const GLfloat l = (GLfloat) rect.getX();
        const GLfloat r = (GLfloat) rect.getRight();
        const GLfloat t = (GLfloat) rect.getY();
        const GLfloat b = (GLfloat) rect.getBottom();

        const GLfloat vertices[] = { l, t, r, t, l, b, r, b };
        GLfloat textureCoords[]  = { l, t, r, t, l, b, r, b };

        textureTransform.transformPoints (textureCoords[0], textureCoords[1], textureCoords[2], textureCoords[3]);
        textureTransform.transformPoints (textureCoords[4], textureCoords[5], textureCoords[6], textureCoords[7]);

        drawTriangles (GL_TRIANGLE_STRIP, vertices, textureCoords, 4);
    }

    void fillWithRadialGradient (const Rectangle<int>& rect,
                                 const ColourGradient& grad,
                                 const AffineTransform& transform)
    {
        const Point<float> centre (grad.point1.transformedBy (transform));

        const float screenRadius = centre.getDistanceFrom (rect.getCentre().toFloat())
                                    + Point<int> (rect.getWidth() / 2,
                                                  rect.getHeight() / 2).getDistanceFromOrigin()
                                    + 8.0f;

        const AffineTransform inverse (transform.inverted());
        const float sourceRadius = jmax (Point<float> (screenRadius, 0.0f).transformedBy (inverse).getDistanceFromOrigin(),
                                         Point<float> (0.0f, screenRadius).transformedBy (inverse).getDistanceFromOrigin());

        const int numDivisions = 90;
        GLfloat vertices      [4 + numDivisions * 2];
        GLfloat textureCoords [4 + numDivisions * 2];

        {
            GLfloat* t = textureCoords;
            *t++ = 0.0f;
            *t++ = 0.0f;

            const GLfloat texturePos = sourceRadius / grad.point1.getDistanceFrom (grad.point2);

            for (int i = numDivisions + 1; --i >= 0;)
            {
                *t++ = texturePos;
                *t++ = 0.0f;
            }
        }

        {
            GLfloat* v = vertices;
            *v++ = centre.getX();
            *v++ = centre.getY();

            const Point<float> first (grad.point1.translated (0, -sourceRadius)
                                                 .transformedBy (transform));
            *v++ = first.getX();
            *v++ = first.getY();

            for (int i = 1; i < numDivisions; ++i)
            {
                const float angle = i * (float_Pi * 2.0f / numDivisions);
                const Point<float> p (grad.point1.translated (std::sin (angle) * sourceRadius,
                                                              std::cos (angle) * -sourceRadius)
                                                 .transformedBy (transform));
                *v++ = p.getX();
                *v++ = p.getY();
            }

            *v++ = first.getX();
            *v++ = first.getY();
        }

        glEnable (GL_SCISSOR_TEST);
        glScissor (rect.getX(), rect.getY(), rect.getWidth(), rect.getHeight());
        drawTriangles (GL_TRIANGLE_FAN, vertices, textureCoords, numDivisions + 2);
        glDisable (GL_SCISSOR_TEST);
    }
}

void OpenGLHelpers::fillRectWithColourGradient (const Rectangle<int>& rect,
                                                const ColourGradient& gradient,
                                                const AffineTransform& transform)
{
    const int textureSize = 256;
    OpenGLTexture texture;

    HeapBlock<PixelARGB> lookup (textureSize);
    gradient.createLookupTable (lookup, textureSize);
    texture.load (lookup, textureSize, 1);
    texture.bind();

    if (gradient.point1 == gradient.point2)
    {
        fillRectWithColour (rect, gradient.getColourAtPosition (1.0));
    }
    else
    {
        if (gradient.isRadial)
            OpenGLGradientHelpers::fillWithRadialGradient (rect, gradient, transform);
        else
            OpenGLGradientHelpers::fillWithLinearGradient (rect, gradient, transform, textureSize);
    }
}

void OpenGLHelpers::fillRectWithColour (const Rectangle<int>& rect, const Colour& colour)
{
    glEnableClientState (GL_VERTEX_ARRAY);
    glDisableClientState (GL_TEXTURE_COORD_ARRAY);
    glDisableClientState (GL_COLOR_ARRAY);
    glDisableClientState (GL_NORMAL_ARRAY);
    setColour (colour);
    fillRect (rect);
}

void OpenGLHelpers::fillRect (const Rectangle<int>& rect)
{
    const GLfloat vertices[] = { (GLfloat) rect.getX(),     (GLfloat) rect.getY(),
                                 (GLfloat) rect.getRight(), (GLfloat) rect.getY(),
                                 (GLfloat) rect.getX(),     (GLfloat) rect.getBottom(),
                                 (GLfloat) rect.getRight(), (GLfloat) rect.getBottom() };

    glVertexPointer (2, GL_FLOAT, 0, vertices);
    glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
}

void OpenGLHelpers::fillRectWithTiledTexture (int textureWidth, int textureHeight,
                                              const Rectangle<int>& clip,
                                              const AffineTransform& transform,
                                              float alpha)
{
    glEnable (GL_TEXTURE_2D);
    glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glEnableClientState (GL_VERTEX_ARRAY);
    glEnableClientState (GL_TEXTURE_COORD_ARRAY);
    glDisableClientState (GL_COLOR_ARRAY);
    glDisableClientState (GL_NORMAL_ARRAY);
    glColor4f (1.0f, 1.0f, 1.0f, alpha);

    const GLfloat clipX = (GLfloat) clip.getX();
    const GLfloat clipY = (GLfloat) clip.getY();
    const GLfloat clipR = (GLfloat) clip.getRight();
    const GLfloat clipB = (GLfloat) clip.getBottom();

    const GLfloat vertices[]  = { clipX, clipY, clipR, clipY, clipX, clipB, clipR, clipB };
    GLfloat textureCoords[]   = { clipX, clipY, clipR, clipY, clipX, clipB, clipR, clipB };

    {
        const AffineTransform t (transform.inverted().scaled (1.0f / textureWidth,
                                                              1.0f / textureHeight));
        t.transformPoints (textureCoords[0], textureCoords[1], textureCoords[2], textureCoords[3]);
        t.transformPoints (textureCoords[4], textureCoords[5], textureCoords[6], textureCoords[7]);
    }

    glVertexPointer (2, GL_FLOAT, 0, vertices);
    glTexCoordPointer (2, GL_FLOAT, 0, textureCoords);

    glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
}

//==============================================================================
struct OpenGLEdgeTableRenderer
{
    OpenGLEdgeTableRenderer (float r_, float g_, float b_, const Point<int>& origin_) noexcept
        : origin (origin_), r (r_), g (g_), b (b_), lastAlpha (-1)
    {
    }

    void draw (const EdgeTable& et)
    {
        glDisableClientState (GL_TEXTURE_COORD_ARRAY);
        glEnableClientState (GL_VERTEX_ARRAY);
        glVertexPointer (2, GL_FLOAT, 0, vertices);

        et.iterate (*this);
    }

    void setEdgeTableYPos (const int y) noexcept
    {
        const int lineY = y + origin.getY();

        vertices[1] = (GLfloat) lineY;
        vertices[3] = (GLfloat) (lineY + 1);
        vertices[5] = (GLfloat) lineY;
        vertices[7] = (GLfloat) (lineY + 1);
    }

    void handleEdgeTablePixel (const int x, const int alphaLevel) noexcept
    {
        drawHorizontal (x, 1, alphaLevel);
    }

    void handleEdgeTablePixelFull (const int x) noexcept
    {
        drawHorizontal (x, 1, 255);
    }

    void handleEdgeTableLine (const int x, const int width, const int alphaLevel) noexcept
    {
        drawHorizontal (x, width, alphaLevel);
    }

    void handleEdgeTableLineFull (const int x, const int width) noexcept
    {
        drawHorizontal (x, width, 255);
    }

private:
    GLfloat vertices[8];
    const Point<int> origin;
    const float r, g, b;
    int lastAlpha;

    void drawHorizontal (int x, const int w, const int alphaLevel) noexcept
    {
        x += origin.getX();

        vertices[0] = (GLfloat) x;
        vertices[2] = (GLfloat) x;
        vertices[4] = (GLfloat) (x + w);
        vertices[6] = (GLfloat) (x + w);

        if (lastAlpha != alphaLevel)
        {
            lastAlpha = alphaLevel;
            glColor4f (r, g, b, alphaLevel / 255.0f);
        }

        glDrawArrays (GL_TRIANGLE_STRIP, 0, 4);
    }

    JUCE_DECLARE_NON_COPYABLE (OpenGLEdgeTableRenderer);
};

void OpenGLHelpers::fillEdgeTable (const EdgeTable& edgeTable,
                                   float red, float green, float blue,
                                   const Point<int>& offset)
{
    OpenGLEdgeTableRenderer etr (red, green, blue, offset);
    etr.draw (edgeTable);
}

//==============================================================================
// This breaks down a path into a series of horizontal strips of trapezoids..
class TriangulatedPath::TrapezoidedPath
{
public:
    TrapezoidedPath (const Path& p, const AffineTransform& transform)
        : firstSlice (nullptr),
          windingMask (p.isUsingNonZeroWinding() ? -1 : 1)
    {
        for (PathFlatteningIterator iter (p, transform); iter.next();)
            addLine (floatToInt (iter.x1), floatToInt (iter.y1),
                     floatToInt (iter.x2), floatToInt (iter.y2));
    }

    ~TrapezoidedPath()
    {
        for (HorizontalSlice* s = firstSlice; s != nullptr;)
        {
            const ScopedPointer<HorizontalSlice> deleter (s);
            s = s->next;
        }
    }

    template <class Consumer>
    void iterate (Consumer& consumer) const
    {
        for (HorizontalSlice* s = firstSlice; s != nullptr; s = s->next)
            s->iterate (consumer, windingMask);
    }

private:
    void addLine (int x1, int y1, int x2, int y2)
    {
        int winding = 1;

        if (y2 < y1)
        {
            std::swap (x1, x2);
            std::swap (y1, y2);
            winding = -1;
        }

        HorizontalSlice* last = nullptr;
        HorizontalSlice* s = firstSlice;

        while (y2 > y1)
        {
            if (s == nullptr)
            {
                insert (last, new HorizontalSlice (nullptr, x1, y1, x2, y2, winding));
                break;
            }

            if (s->y2 > y1)
            {
                if (y1 < s->y1)
                {
                    if (y2 <= s->y1)
                    {
                        insert (last, new HorizontalSlice (s, x1, y1, x2, y2, winding));
                        break;
                    }
                    else
                    {
                        const int newX = x1 + (int) ((s->y1 - y1) * (int64) (x2 - x1) / (y2 - y1));
                        HorizontalSlice* const newSlice = new HorizontalSlice (s, x1, y1, newX, s->y1, winding);
                        insert (last, newSlice);
                        last = newSlice;
                        x1 = newX;
                        y1 = s->y1;
                        continue;
                    }
                }
                else if (y1 > s->y1)
                {
                    s->split (y1);
                    s = s->next;
                    jassert (s != nullptr);
                }

                jassert (y1 == s->y1);

                if (y2 > s->y2)
                {
                    const int newY = s->y2;
                    const int newX = x1 + (int) ((newY - y1) * (int64) (x2 - x1) / (y2 - y1));
                    s->addLine (x1, newX, winding);
                    x1 = newX;
                    y1 = newY;
                }
                else
                {
                    if (y2 < s->y2)
                        s->split (y2);

                    jassert (y2 == s->y2);
                    s->addLine (x1, x2, winding);
                    break;
                }
            }

            last = s;
            s = s->next;
        }
    }

    struct HorizontalSlice
    {
        HorizontalSlice (const HorizontalSlice& other, HorizontalSlice* const next_, int y1_, int y2_)
            : next (next_), y1 (y1_), y2 (y2_), segments (other.segments)
        {
        }

        HorizontalSlice (HorizontalSlice* const next_, int x1, int y1_, int x2, int y2_, int winding)
            : next (next_), y1 (y1_), y2 (y2_)
        {
            jassert (next != this);
            jassert (y2 > y1);
            segments.ensureStorageAllocated (32);
            segments.add (LineSegment (x1, x2, winding));
        }

        void addLine (const int x1, const int x2, int winding)
        {
            const int dy = y2 - y1;

            for (int i = 0; i < segments.size(); ++i)
            {
                const LineSegment& l = segments.getReference (i);

                const int diff1 = l.x1 - x1;
                const int diff2 = l.x2 - x2;

                if ((diff1 < 0) == (diff2 > 0))
                {
                    const int dx1 = l.x2 - l.x1;
                    const int dx2 = x2 - x1;
                    const int dxDiff = dx2 - dx1;

                    if (dxDiff != 0)
                    {
                        const int intersectionY = (int) ((dy * (int64) diff1) / dxDiff);

                        if (intersectionY > 0 && intersectionY < dy)
                        {
                            const int intersectionX = x1 + (intersectionY * dx2) / dy;
                            split (intersectionY + y1);
                            next->addLine (intersectionX, x2, winding);
                            addLine (x1, intersectionX, winding);
                            return;
                        }
                    }
                }

                if (diff1 + diff2 > 0)
                {
                    segments.insert (i, LineSegment (x1, x2, winding));
                    return;
                }
            }

            segments.add (LineSegment (x1, x2, winding));
        }

        void split (const int newY)
        {
            jassert (newY > y1 && newY < y2);

            const int dy1 = newY - y1;
            const int dy2 = y2 - y1;
            next = new HorizontalSlice (*this, next, newY, y2);
            y2 = newY;

            LineSegment* const oldSegments = segments.getRawDataPointer();
            LineSegment* const newSegments = next->segments.getRawDataPointer();

            for (int i = 0; i < segments.size(); ++i)
            {
                LineSegment& l = oldSegments[i];
                const int newX = l.x1 + (int) (dy1 * (int64) (l.x2 - l.x1) / dy2);
                newSegments[i].x1 = newX;
                l.x2 = newX;
            }
        }

        template <class Consumer>
        void iterate (Consumer& consumer, const int windingMask)
        {
            jassert (segments.size() > 0);

            const float fy1 = intToFloat (y1);
            const float fy2 = intToFloat (y2);

            const LineSegment* s1 = segments.getRawDataPointer();
            const LineSegment* s2 = s1;
            int winding = s1->winding;

            for (int i = segments.size(); --i > 0;)
            {
                ++s2;
                winding += s2->winding;

                if ((winding & windingMask) == 0)
                {
                    const float ax1 = intToFloat (s1->x1);
                    const float ax2 = intToFloat (s1->x2);

                    if (s1->x1 == s2->x1)
                        consumer.addTriangle (ax1, fy1, ax2, fy2, intToFloat (s2->x2), fy2);
                    else if (s1->x2 == s2->x2)
                        consumer.addTriangle (ax1, fy1, intToFloat (s2->x1), fy1, ax2, fy2);
                    else
                        consumer.addTrapezoid (fy1, fy2, ax1, ax2, intToFloat (s2->x1), intToFloat (s2->x2));

                    s1 = s2 + 1;
                }
            }
        }

        HorizontalSlice* next;
        int y1, y2;

    private:
        struct LineSegment
        {
            inline LineSegment (int x1_, int x2_, int winding_) noexcept
                : x1 (x1_), x2 (x2_), winding (winding_) {}

            int x1, x2;
            int winding;
        };

        Array<LineSegment> segments;

        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HorizontalSlice);
    };

    HorizontalSlice* firstSlice;
    const int windingMask;

    inline void insert (HorizontalSlice* const last, HorizontalSlice* const newOne) noexcept
    {
        if (last == nullptr)
            firstSlice = newOne;
        else
            last->next = newOne;
    }

    enum { factor = 128 };
    static inline int floatToInt (const float n) noexcept     { return roundToInt (n * (float) factor); }
    static inline float intToFloat (const int n) noexcept     { return n * (1.0f / (float) factor); }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TrapezoidedPath);
};

//==============================================================================
struct TriangulatedPath::TriangleBlock
{
    TriangleBlock() noexcept
        : numVertices (0),
          triangles (maxVerticesPerBlock)
    {}

    void draw() const
    {
        glVertexPointer (2, GL_FLOAT, 0, triangles);
        glDrawArrays (GL_TRIANGLES, 0, numVertices / 2);
    }

    inline GLfloat* getNextTriangle() noexcept  { return triangles + numVertices; }
    void optimiseStorage()                      { triangles.realloc (numVertices); }

    // Some GL implementations can't take very large triangle lists, so store
    // the list as a series of blocks containing this max number of triangles.
    enum { maxVerticesPerBlock = 256 * 6 };

    unsigned int numVertices;
    HeapBlock<GLfloat> triangles;
};

TriangulatedPath::TriangulatedPath (const Path& path, const AffineTransform& transform)
{
    startNewBlock();
    TrapezoidedPath (path, transform).iterate (*this);
}

TriangulatedPath::~TriangulatedPath() {}

void TriangulatedPath::draw (const int oversamplingLevel) const
{
    glColor4f (1.0f, 1.0f, 1.0f, 1.0f / (oversamplingLevel * oversamplingLevel));

    glTranslatef (-0.5f, -0.5f, 0.0f);
    const float inc = 1.0f / oversamplingLevel;

    for (int y = oversamplingLevel; --y >= 0;)
    {
        for (int x = oversamplingLevel; --x >= 0;)
        {
            glTranslatef (inc, 0.0f, 0.0f);

            for (int i = 0; i < blocks.size(); ++i)
                blocks.getUnchecked(i)->draw();
        }

        glTranslatef (-1.0f, inc, 0.0f);
    }
}

void TriangulatedPath::optimiseStorage()
{
    currentBlock->optimiseStorage();
}

void TriangulatedPath::startNewBlock()
{
    currentBlock = new TriangleBlock();
    blocks.add (currentBlock);
}

void TriangulatedPath::addTriangle (GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2, GLfloat x3, GLfloat y3)
{
    if (currentBlock->numVertices >= TriangleBlock::maxVerticesPerBlock)
        startNewBlock();

    GLfloat* t = currentBlock->getNextTriangle();
    *t++ = x1; *t++ = y1; *t++ = x2; *t++ = y2; *t++ = x3; *t++ = y3;

    currentBlock->numVertices += 6;
}

void TriangulatedPath::addTrapezoid (GLfloat y1, GLfloat y2, GLfloat x1, GLfloat x2, GLfloat x3, GLfloat x4)
{
    if (currentBlock->numVertices >= TriangleBlock::maxVerticesPerBlock - 6)
        startNewBlock();

    GLfloat* t = currentBlock->getNextTriangle();
    *t++ = x1; *t++ = y1; *t++ = x2; *t++ = y2; *t++ = x3; *t++ = y1;
    *t++ = x4; *t++ = y2; *t++ = x2; *t++ = y2; *t++ = x3; *t++ = y1;

    currentBlock->numVertices += 12;
}

//==============================================================================
OpenGLTextureFromImage::OpenGLTextureFromImage (const Image& image)
    : width (image.getWidth()),
      height (image.getHeight())
{
    OpenGLFrameBufferImage* glImage = dynamic_cast <OpenGLFrameBufferImage*> (image.getSharedImage());

    if (glImage != nullptr)
    {
        textureID = glImage->frameBuffer.getTextureID();
    }
    else
    {
        if (OpenGLTexture::isValidSize (width, height))
        {
            texture = new OpenGLTexture();
            texture->load (image);
            textureID = texture->getTextureID();
        }
        else
        {
            frameBuffer = new OpenGLFrameBuffer();
            frameBuffer->initialise (image);
            textureID = frameBuffer->getTextureID();
        }
    }
}

OpenGLTextureFromImage::~OpenGLTextureFromImage() {}

//==============================================================================
OpenGLRenderingTarget::OpenGLRenderingTarget() {}
OpenGLRenderingTarget::~OpenGLRenderingTarget() {}

void OpenGLRenderingTarget::prepareFor2D()
{
    OpenGLHelpers::prepareFor2D (getRenderingTargetWidth(),
                                 getRenderingTargetHeight());
}


END_JUCE_NAMESPACE
