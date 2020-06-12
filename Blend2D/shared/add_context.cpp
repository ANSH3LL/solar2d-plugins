/*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
* [ MIT license: http://www.opensource.org/licenses/mit-license.php ]
*/

#include "blend2d.h"
#include "CoronaLua.h"
#include "common.h"
#include "utils.h"

#define CONTEXT_MNAME "blend2d.context"

BLContextCore * GetContext (lua_State * L, int arg, bool * intact_ptr)
{
	return Get<BLContextCore>(L, arg, CONTEXT_MNAME, intact_ptr);
}

BLCompOp GetCompOp (lua_State * L, int arg)
{
	const char * names[] = { "SRC_OVER", "SRC_COPY", "DIFFERENCE", nullptr };
	BLCompOp ops[] = { BL_COMP_OP_SRC_OVER, BL_COMP_OP_SRC_COPY, BL_COMP_OP_DIFFERENCE };
/*
	// BL_DEFINE_ENUM(BLCompOp) {
  //! Source-over [default].
  BL_COMP_OP_SRC_OVER = 0,
  //! Source-copy.
  BL_COMP_OP_SRC_COPY = 1,
  //! Source-in.
  BL_COMP_OP_SRC_IN = 2,
  //! Source-out.
  BL_COMP_OP_SRC_OUT = 3,
  //! Source-atop.
  BL_COMP_OP_SRC_ATOP = 4,
  //! Destination-over.
  BL_COMP_OP_DST_OVER = 5,
  //! Destination-copy [nop].
  BL_COMP_OP_DST_COPY = 6,
  //! Destination-in.
  BL_COMP_OP_DST_IN = 7,
  //! Destination-out.
  BL_COMP_OP_DST_OUT = 8,
  //! Destination-atop.
  BL_COMP_OP_DST_ATOP = 9,
  //! Xor.
  BL_COMP_OP_XOR = 10,
  //! Clear.
  BL_COMP_OP_CLEAR = 11,
  //! Plus.
  BL_COMP_OP_PLUS = 12,
  //! Minus.
  BL_COMP_OP_MINUS = 13,
  //! Modulate.
  BL_COMP_OP_MODULATE = 14,
  //! Multiply.
  BL_COMP_OP_MULTIPLY = 15,
  //! Screen.
  BL_COMP_OP_SCREEN = 16,
  //! Overlay.
  BL_COMP_OP_OVERLAY = 17,
  //! Darken.
  BL_COMP_OP_DARKEN = 18,
  //! Lighten.
  BL_COMP_OP_LIGHTEN = 19,
  //! Color dodge.
  BL_COMP_OP_COLOR_DODGE = 20,
  //! Color burn.
  BL_COMP_OP_COLOR_BURN = 21,
  //! Linear burn.
  BL_COMP_OP_LINEAR_BURN = 22,
  //! Linear light.
  BL_COMP_OP_LINEAR_LIGHT = 23,
  //! Pin light.
  BL_COMP_OP_PIN_LIGHT = 24,
  //! Hard-light.
  BL_COMP_OP_HARD_LIGHT = 25,
  //! Soft-light.
  BL_COMP_OP_SOFT_LIGHT = 26,
  //! Difference.
  BL_COMP_OP_DIFFERENCE = 27,
  //! Exclusion.
  BL_COMP_OP_EXCLUSION = 28,

  //! Count of composition & blending operators.
  BL_COMP_OP_COUNT = 29*/

	return ops[luaL_checkoption(L, arg, "SRC_OVER", names)];
}

static uint32_t GetCap (lua_State * L, int arg)
{
	const char * names[] = { "BUTT", "SQUARE", "ROUND", "ROUND_REV", "TRIANGLE", "TRIANGLE_REV", nullptr };
	BLStrokeCap caps[] = {BL_STROKE_CAP_BUTT, BL_STROKE_CAP_SQUARE, BL_STROKE_CAP_ROUND, BL_STROKE_CAP_ROUND_REV, BL_STROKE_CAP_TRIANGLE, BL_STROKE_CAP_TRIANGLE_REV };

	return caps[luaL_checkoption(L, arg, "BUTT", names)];
}

static int NewContext (lua_State * L)
{

	BLContextCore * context = New<BLContextCore>(L);// [image, ]context

	if (IsImage(L, 1)) blContextInitAs(context, GetImage(L, 1), nullptr);
	else blContextInit(context);

	if (luaL_newmetatable(L, CONTEXT_MNAME)) // [image, ]context, mt
	{
		luaL_Reg context_funcs[] = {
			{
				"begin", [](lua_State * L)
				{
					blContextBegin(GetContext(L), GetImage(L, 2), nullptr);

					return 0;
				}
			}, {
				"destroy", [](lua_State * L)
				{
					BLContextCore * context = GetContext(L);

					blContextDestroy(context);
					Destroy(context);

					return 1;
				}
			}, {
				"end", [](lua_State * L)
				{
					blContextEnd(GetContext(L));

					return 0;
				}
			}, {
				"fillAll", [](lua_State * L)
				{
					blContextFillAll(GetContext(L));

					return 0;
				}
			}, {
				"fillGlyphRun", [](lua_State * L)
				{
					luaL_argcheck(L, lua_istable(L, 2), 2, "Point must be a table");
					lua_getfield(L, 2, "x"); // context, point, font, glyph_run, x
					lua_getfield(L, 2, "y"); // context, point, font, glyph_run, x, y
					
					BLPoint point = { luaL_checknumber(L, -2), luaL_checknumber(L, -1) };

					blContextFillGlyphRunD(GetContext(L), &point, GetFont(L, 3), *(const BLGlyphRun **)luaL_checkudata(L, 4, "TODO:GlyphRun"));

					return 0;
				}
			}, {
				"fillCircle", [](lua_State * L)
				{
					BLCircle circle;

					circle.reset(luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4));

					blContextFillGeometry(GetContext(L), BL_GEOMETRY_TYPE_CIRCLE, &circle);

					return 0;
				}
			}, {
				"fillPath", [](lua_State * L)
				{
					blContextFillPathD(GetContext(L), GetPath(L, 2));

					return 0;
				}
			}, {
				"fillRoundRect", [](lua_State * L)
				{
					BLRoundRect round_rect;

				/*
				  BL_INLINE BLResult fillRoundRect(const BLRoundRect& rr) noexcept { return fillGeometry(BL_GEOMETRY_TYPE_ROUND_RECT, &rr); }
  //! \overload
  BL_INLINE BLResult fillRoundRect(const BLRect& rect, double r) noexcept { return fillRoundRect(BLRoundRect(rect.x, rect.y, rect.w, rect.h, r)); }
  //! \overload
  BL_INLINE BLResult fillRoundRect(const BLRect& rect, double rx, double ry) noexcept { return fillRoundRect(BLRoundRect(rect.x, rect.y, rect.w, rect.h, rx, ry)); }
  //! \overload
  BL_INLINE BLResult fillRoundRect(double x, double y, double w, double h, double r) noexcept { return fillRoundRect(BLRoundRect(x, y, w, h, r)); }
  //! \overload
  BL_INLINE BLResult fillRoundRect(double x, double y, double w, double h, double rx, double ry) noexcept { return fillRoundRect(BLRoundRect(x, y, w, h, rx, ry)); }
				*/
					round_rect.reset(luaL_checknumber(L, 2), luaL_checknumber(L, 3), luaL_checknumber(L, 4), luaL_checknumber(L, 5), luaL_checknumber(L, 6));

					blContextFillGeometry(GetContext(L), BL_GEOMETRY_TYPE_ROUND_RECT, &round_rect);

					return 0;
				}
			}, {
				"fillUtf8Text", [](lua_State * L)
				{
					luaL_argcheck(L, lua_istable(L, 2), 2, "Point must be a table");
					lua_getfield(L, 2, "x"); // context, point, font, text, x
					lua_getfield(L, 2, "y"); // context, point, font, text, x, y

					BLPoint p = { luaL_checknumber(L, -2), luaL_checknumber(L, -1) };

					blContextFillTextD(GetContext(L), &p, GetFont(L, 3), luaL_checkstring(L, 4), SIZE_MAX, BL_TEXT_ENCODING_UTF8);

					return 0;
				}
			}, {
				"__gc", [](lua_State * L)
				{
					bool intact;

					BLContextCore * context = GetContext(L, 1, &intact);

					if (intact) blContextDestroy(context);

					return 0;
				}
			}, {
				"__index", Index
			}, {
				"rotate", [](lua_State * L)
				{
					if (lua_isnumber(L, 4))
					{
						double data[] = { luaL_checknumber(L, 2), luaL_checknumber(L, 3), lua_tonumber(L, 4) };

						blContextMatrixOp(GetContext(L), BL_MATRIX2D_OP_ROTATE_PT, data);
					}

					else
					{
						double data = luaL_checknumber(L, 2);

						blContextMatrixOp(GetContext(L), BL_MATRIX2D_OP_ROTATE, &data);
					}

					return 0;
				}
			}, {
				"setCompOp", [](lua_State * L)
				{
					blContextSetCompOp(GetContext(L), GetCompOp(L, 2));

					return 0;
				}
			}, {
				"setFillStyle", [](lua_State * L)
				{
					if (IsGradient(L, 2)) blContextSetFillStyleObject(GetContext(L), GetGradient(L, 2));
					else if (IsPattern(L, 2)) blContextSetFillStyleObject(GetContext(L), GetPattern(L, 2));
					else blContextSetFillStyleRgba32(GetContext(L), CheckUint32(L, 2));

					return 0;
				}
			}, {
				"setStrokeEndCap", [](lua_State * L)
				{
					blContextSetStrokeCap(GetContext(L), BL_STROKE_CAP_POSITION_END, GetCap(L, 2));

					return 0;
				}
			}, {
				"setStrokeStartCap", [](lua_State * L)
				{
					blContextSetStrokeCap(GetContext(L), BL_STROKE_CAP_POSITION_START, GetCap(L, 2));

					return 0;
				}
			}, {
				"setStrokeStyle", [](lua_State * L)
				{
					blContextSetStrokeStyleObject(GetContext(L), GetGradient(L, 2));

					return 0;
				}
			}, {
				"setStrokeWidth", [](lua_State * L)
				{
					blContextSetStrokeWidth(GetContext(L), luaL_checknumber(L, 2));

					return 0;
				}
			}, {
				"strokePath", [](lua_State * L)
				{
					blContextStrokePathD(GetContext(L), GetPath(L, 2));

					return 0;
				}
			},
			{ nullptr, nullptr }
		};

		luaL_register(L, NULL, context_funcs);
	}

	lua_setmetatable(L, -2);// [image, ]context

	return 1;
}

int add_context (lua_State * L)
{
	lua_newtable(L);// t
	lua_pushcfunction(L, NewContext);	// t, NewContext
	lua_setfield(L, -2, "New");	// t = { New = NewContext }

	return 1;
}