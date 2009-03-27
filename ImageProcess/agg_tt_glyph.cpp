/ agg_tt_glyph.cpp: implementation of the agg_tt_glyph class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "agg_tt_glyph.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_color_rgba.h"
#include "agg_renderer_base.h"
#include "agg_renderer_scanline.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_p.h"
#include "agg_conv_curve.h"
#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_scanline_u.h"
#include "agg_scanline_bin.h"
#include "agg_renderer_scanline.h"
#include "agg_renderer_primitives.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_conv_curve.h"
#include "agg_conv_contour.h"
#include "agg_pixfmt_rgb.h"
#include "agg_gamma_lut.h"
#include "agg_font_win32_tt.h"


namespace agg
{

    //=================================================================
    class tt_glyph
    {
        enum { buf_size = 16384-32 };
        
    public:
        ~tt_glyph();
        tt_glyph();

        // Set the created font and the "flip_y" flag.
        //------------------------
        void font(HDC dc, HFONT f) { m_dc = dc; m_font = f; }
        void flip_y(bool flip) { m_flip_y = flip; }
        
        bool glyph(unsigned chr, bool hinted = true);
        
        // The following functions can be called after glyph()
        // and return the respective values of the 
        // GLYPHMETRICS structure.
        //-------------------------
        int      origin_x() const { return m_origin_x; }
        int      origin_y() const { return m_origin_y; }
        unsigned width()    const { return m_width;    }
        unsigned height()   const { return m_height;   }
        int      inc_x()    const { return m_inc_x;    }
        int      inc_y()    const { return m_inc_y;    }
        
        // Set the starting point of the Glyph
        //-------------------------
        void start_point(double x, double y) 
        { 
            m_start_x = x; 
            m_start_y = y; 
        }
        
        // Vertex Source Interface
        //-------------------------
        void rewind(unsigned) 
        { 
            m_cur_vertex = m_vertices; 
            m_cur_flag = m_flags; 
        }

        unsigned vertex(double* x, double* y)
        {
            *x = m_start_x + *m_cur_vertex++;
            *y = m_start_y + *m_cur_vertex++;
            return *m_cur_flag++;
        }

    private:
        HDC           m_dc;
        HFONT         m_font;
        char*         m_gbuf;
        int8u*        m_flags;
        double*       m_vertices;
        unsigned      m_max_vertices;
        const int8u*  m_cur_flag; 
        const double* m_cur_vertex;
        double        m_start_x;
        double        m_start_y;
        MAT2          m_mat2;
        
        int      m_origin_x;
        int      m_origin_y;
        unsigned m_width;
        unsigned m_height;
        int      m_inc_x;
        int      m_inc_y;
        
        bool     m_flip_y;
    };




    tt_glyph::~tt_glyph()
    {
        delete [] m_vertices;
        delete [] m_flags;
        delete [] m_gbuf;
    }
    
    
    tt_glyph::tt_glyph() :
        m_dc(0),
        m_font(0),
        m_gbuf(new char [buf_size]),
        m_flags(new int8u [256]),
        m_vertices(new double[512]),
        m_max_vertices(256),
        m_cur_flag(m_flags),
        m_cur_vertex(m_vertices),
        m_start_x(0.0),
        m_start_y(0.0),
        m_flip_y(false)
    {
        m_vertices[0] = m_vertices[1] = 0.0;
        m_flags[0] = path_cmd_stop;
        memset(&m_mat2, 0, sizeof(m_mat2));
        m_mat2.eM11.value = 1;
        m_mat2.eM22.value = 1;
    }



    static inline double fx_to_dbl(const FIXED& p)
    {
        return double(p.value) + double(p.fract) * (1.0 / 65536.0);
    }
    
    static inline FIXED dbl_to_fx(double d)
    {
        int l;
        l = long(d * 65536.0);
        return *(FIXED*)&l;
    }


    bool tt_glyph::glyph(unsigned chr, bool hinted)
    {
        m_vertices[0] = m_vertices[1] = 0.0;
        m_flags[0] = path_cmd_stop;
        rewind(0);
        
        if (m_font == 0) return false;
        
#ifndef GGO_UNHINTED         // For compatibility with old SDKs.
#define GGO_UNHINTED 0x0100
#endif
        
        int unhinted = hinted ? 0 : GGO_UNHINTED;
        
        GLYPHMETRICS gm;
        int total_size = GetGlyphOutline(m_dc,
                                         chr,
                                         GGO_NATIVE | unhinted,
                                         &gm,
                                         buf_size,
                                         (void*)m_gbuf,
                                         &m_mat2);

        if (total_size < 0) return false;
        
        m_origin_x = gm.gmptGlyphOrigin.x;
        m_origin_y = gm.gmptGlyphOrigin.y;
        m_width    = gm.gmBlackBoxX;
        m_height   = gm.gmBlackBoxY;
        m_inc_x    = gm.gmCellIncX;
        m_inc_y    = gm.gmCellIncY;
        
        if (m_max_vertices <= total_size / sizeof(POINTFX))
        {
            delete [] m_vertices;
            delete [] m_flags;
            m_max_vertices = total_size / sizeof(POINTFX) + 256;
            m_flags = new int8u [m_max_vertices];
            m_vertices = new double [m_max_vertices * 2];
        }

        const char* cur_glyph = m_gbuf;
        const char* end_glyph = m_gbuf + total_size;
        
        double* vertex_ptr = m_vertices;
        int8u*  flag_ptr   = m_flags;

        while(cur_glyph < end_glyph)
        {
            const TTPOLYGONHEADER* th = (TTPOLYGONHEADER*)cur_glyph;
            
            const char* end_poly = cur_glyph + th->cb;
            const char* cur_poly = cur_glyph + sizeof(TTPOLYGONHEADER);
            
            *vertex_ptr++ = fx_to_dbl(th->pfxStart.x);
            *vertex_ptr++ = m_flip_y ? 
                                -fx_to_dbl(th->pfxStart.y): 
                                 fx_to_dbl(th->pfxStart.y);
            *flag_ptr++   = path_cmd_move_to;
            
            while(cur_poly < end_poly)
            {
                const TTPOLYCURVE* pc = (const TTPOLYCURVE*)cur_poly;
                
                if (pc->wType == TT_PRIM_LINE)
                {
                    int i;
                    for (i = 0; i < pc->cpfx; i++)
                    {
                        *vertex_ptr++ = fx_to_dbl(pc->apfx[i].x);
                        *vertex_ptr++ = m_flip_y ? 
                                            -fx_to_dbl(pc->apfx[i].y): 
                                             fx_to_dbl(pc->apfx[i].y);
                        *flag_ptr++   = path_cmd_line_to;
                    }
                }
                
                if (pc->wType == TT_PRIM_QSPLINE)
                {
                    int u;
                    for (u = 0; u < pc->cpfx - 1; u++)  // Walk through points in spline
                    {
                        POINTFX pnt_b = pc->apfx[u];    // B is always the current point
                        POINTFX pnt_c = pc->apfx[u+1];
                        
                        if (u < pc->cpfx - 2)           // If not on last spline, compute C
                        {
                            // midpoint (x,y)
                            *(int*)&pnt_c.x = (*(int*)&pnt_b.x + *(int*)&pnt_c.x) / 2;
                            *(int*)&pnt_c.y = (*(int*)&pnt_b.y + *(int*)&pnt_c.y) / 2;
                        }
                        
                        *vertex_ptr++ = fx_to_dbl(pnt_b.x);
                        *vertex_ptr++ = m_flip_y ? 
                                            -fx_to_dbl(pnt_b.y): 
                                             fx_to_dbl(pnt_b.y);
                        *flag_ptr++   = path_cmd_curve3;
                        
                        *vertex_ptr++ = fx_to_dbl(pnt_c.x);
                        *vertex_ptr++ = m_flip_y ? 
                                            -fx_to_dbl(pnt_c.y): 
                                             fx_to_dbl(pnt_c.y);
                        *flag_ptr++   = path_cmd_curve3;
                    }
                }
                cur_poly += sizeof(WORD) * 2 + sizeof(POINTFX) * pc->cpfx;
            }
            cur_glyph += th->cb;
            *vertex_ptr++ = 0.0;
            *vertex_ptr++ = 0.0;
            *flag_ptr++   = path_cmd_end_poly | path_flags_close | path_flags_ccw;
        }
        
        *vertex_ptr++ = 0.0;
        *vertex_ptr++ = 0.0;
        *flag_ptr++   = path_cmd_stop;
        
        return true;
   }
   
}


template<class Rasterizer, class Renderer, class Scanline, class CharT>
void render_text(Rasterizer& ras, Renderer& ren, Scanline& sl, 
                 agg::tt_glyph& gl, double x, double y, const CharT* str,
                 bool hinted = true)
{
    // The minimal pipeline is the curve converter. Of course, there
    // any other transformations are applicapble, conv_stroke<>, for example.
    // If there are other thransformations, it probably makes sense to 
    // turn off the hints (hinted=false), i.e., to use unhinted glyphs.
    //--------------------------
    agg::conv_curve<agg::tt_glyph> curve(gl);
    while(*str)
    {
        gl.start_point(x, y);
        gl.glyph(*str++, hinted);
        ras.add_path(curve);
        agg::render_scanlines(ras, sl, ren);
        x += gl.inc_x();
        y += gl.inc_y();
    }
}

void DrawTextTTAgg(CMemAggGraphics &memdc, LPCTSTR szText)
{
	int		frame_width = memdc.Width();
	int		frame_height = memdc.Height();

    agg::rendering_buffer rbuf(memdc.GetBuff(), 
                               frame_width, 
                               frame_height, 
                               frame_width * 4);

    // Create the renderers, the rasterizer, and the scanline container
    //------------------------
    agg::pixfmt_rgba32 pixf(rbuf);
    agg::renderer_base<agg::pixfmt_rgba32> rbase(pixf);
    agg::renderer_scanline_aa_solid<agg::renderer_base<agg::pixfmt_rgba32> > ren(rbase);
    agg::rasterizer_scanline_aa<> ras;
    agg::scanline_p8 sl;

    // rbase.clear(agg::rgba8(255, 255, 255));


    // Font parameters
    //------------------------
    int  fontHeight      = 60;  // in Pixels in this case
    int  fontWidth       = 0;
    int  iAngle          = 0;
    bool bold            = true;
    bool italic          = true;
    const TCHAR * typeFace = _T("Times New Roman");


    // I'm not sure how to deal with those sneaky WinGDI functions correctly,
    // so, please correct me if there's something wrong. 
    // I'm not sure if I need to call ReleaseDC() for the screen.
    //----------------------------------
  
	LOGFONT lgfont = {100, 0, 0, 0, FW_HEAVY, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, 
		ANTIALIASED_QUALITY/*ANTIALIASED_QUALITY*/, DEFAULT_PITCH | FF_ROMAN, _T("Verdana")};
	HFONT font = CreateFontIndirect(&lgfont);
    /*HFONT font = ::CreateFont(fontHeight,           // height of font
                              fontWidth,            // average character width
                              iAngle,               // angle of escapement
                              iAngle,               // base-line orientation angle
                              bold ? 700 : 400,     // font weight
                              italic,               // italic attribute option
                              FALSE,                // underline attribute option
                              FALSE,                // strikeout attribute option
                              ANSI_CHARSET,         // character set identifier
                              OUT_DEFAULT_PRECIS,   // output precision
                              CLIP_DEFAULT_PRECIS,  // clipping precision
                              ANTIALIASED_QUALITY,  // output quality
                              FF_DONTCARE,          // pitch and family
                              typeFace);            // typeface name*/
    if(font)
    {
        HDC dc = ::GetDC(0);
        if(dc)
        {
            HGDIOBJ old_font = ::SelectObject(dc, font);
            agg::tt_glyph gl;
            gl.font(dc, font);
        
            ren.color(agg::rgba8(0,0,255));
            render_text(ras, ren, sl, gl, 10, 300, szText);
        
            ::SelectObject(dc, old_font);
            ::ReleaseDC(0, dc);
        }
        ::DeleteObject(font);
    }


}

class CAggDrawText
{
public:
    typedef agg::pixfmt_rgba32 pixfmt_type;
    typedef agg::renderer_base<pixfmt_type> base_ren_type;
    typedef agg::renderer_scanline_aa_solid<base_ren_type> renderer_solid;
    typedef agg::renderer_scanline_bin_solid<base_ren_type> renderer_bin;
    typedef agg::font_engine_win32_tt_int32 font_engine_type;
    typedef agg::font_cache_manager<font_engine_type> font_manager_type;

    // Pipeline to process the vectors glyph paths (curves + contour)
    typedef agg::conv_curve<font_manager_type::path_adaptor_type> conv_curve_type;
    typedef agg::conv_contour<conv_curve_type> conv_contour_type;

	CAggDrawText(HDC dc) :
		m_feng(dc),
		m_fman(m_feng),
		m_old_height(0.0),
		m_curves(m_fman.path_adaptor()),
		m_contour(m_curves)
	{

		m_contour.auto_detect_orientation(false);

	}

    template<class Rasterizer, class Scanline, class RenSolid, class RenBin>
    unsigned draw_text(Rasterizer& ras, Scanline& sl, 
                       RenSolid& ren_solid, RenBin& ren_bin, 
					   int fontHeight, int x, int y, int width, int height, LPCTSTR text)
    {
		bool	text_flip = false;
		bool	bKerning = false;

        agg::glyph_rendering gren = agg::glyph_ren_native_mono;
		gren = agg::glyph_ren_native_gray8;
        /*switch(m_ren_type.cur_item())
        {
        case 0: gren = agg::glyph_ren_native_mono;  break;
        case 1: gren = agg::glyph_ren_native_gray8; break;
        case 2: gren = agg::glyph_ren_outline;      break;
        case 3: gren = agg::glyph_ren_agg_mono;     break;
        case 4: gren = agg::glyph_ren_agg_gray8;    break;
        }*/

        unsigned num_glyphs = 0;

        m_contour.width(-1.0 * fontHeight * 0.05);

        m_feng.hinting(true);
        m_feng.height(fontHeight);

        // Font width in Windows is strange. MSDN says, 
        // "specifies the average width", but there's no clue what
        // this "average width" means. It'd be logical to specify 
        // the width with regard to the font height, like it's done in 
        // FreeType. That is, width == height should mean the "natural", 
        // not distorted glyphs. In Windows you have to specify
        // the absolute width, which is very stupid and hard to use 
        // in practice.
        //-------------------------
        m_feng.width(0.0);
        m_feng.italic(false);
        m_feng.flip_y(text_flip);

        agg::trans_affine mtx;
        //mtx *= agg::trans_affine_skewing(-0.3, 0);
        mtx *= agg::trans_affine_rotation(agg::deg2rad(-4.0));
        m_feng.transform(mtx);

        if(m_feng.create_font("Arial", gren))
        {
            m_fman.precache(' ', 127);

            double x = 10.0;
            double y0 = height - fontHeight - 10.0;
            double y = y0;
            const TCHAR* p = text;

            while(*p)
            {
                const agg::glyph_cache* glyph = m_fman.glyph(*p);
                if(glyph)
                {
                    if(bKerning)
                    {
                        m_fman.add_kerning(&x, &y);
                    }

                    if(x >= width - fontHeight)
                    {
                        x = 10.0;
                        y0 -= fontHeight;
                        if(y0 <= 120) break;
                        y = y0;
                    }

                    m_fman.init_embedded_adaptors(glyph, x, y);

                    switch(glyph->data_type)
                    {
                    case agg::glyph_data_mono:
                        ren_bin.color(agg::rgba8(0, 0, 0));
                        agg::render_scanlines(m_fman.mono_adaptor(), 
                                              m_fman.mono_scanline(), 
                                              ren_bin);
                        break;

                    case agg::glyph_data_gray8:
                        ren_solid.color(agg::rgba8(0, 0, 0));
                        agg::render_scanlines(m_fman.gray8_adaptor(), 
                                              m_fman.gray8_scanline(), 
                                              ren_solid);
                        break;

                    case agg::glyph_data_outline:
                        ras.reset();
                        // if(fabs(m_weight.value()) <= 0.01)
                        {
                            // For the sake of efficiency skip the
                            // contour converter if the weight is about zero.
                            //-----------------------
                            ras.add_path(m_curves);
                        }
/*                        else
                        {
                            ras.add_path(m_contour);
                        }*/
                        ren_solid.color(agg::rgba8(0, 0, 0));
                        agg::render_scanlines(ras, sl, ren_solid);
                        break;
                    }

                    // increment pen position
                    x += glyph->advance_x;
                    y += glyph->advance_y;
                    ++num_glyphs;
                }
                ++p;
            }
        }
        return num_glyphs;
    }

    font_engine_type             m_feng;
    font_manager_type            m_fman;
    double                       m_old_height;

    conv_curve_type m_curves;
    conv_contour_type m_contour;


};


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

agg_tt_glyph::agg_tt_glyph()
{

}

agg_tt_glyph::~agg_tt_glyph()
{

}
