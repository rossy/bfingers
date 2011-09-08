#include <platform.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
#include "log.h"
#include "image.h"
#include "gar.h"
#include "bfingers.h"
#include "script.h"
#include "object.h"

lua_State *script_state = NULL;

static int lua_checkmetatable(lua_State *L, const char* tname)
{
	int ret;
	lua_getmetatable(L, -1);
	luaL_getmetatable(L, tname);
	ret = lua_equal(L, -1, -2);
	lua_pop(L, 2);
	return ret;
}

static int lua_trace(lua_State *L)
{
	size_t size;
	const char* str = luaL_checklstring(L, 1, &size);
	trace(str);
	return 0;
}

static int lua_gar_get(lua_State *L)
{
	size_t size;
	const char* str = luaL_checklstring(L, 1, &size);
	void* ret = gar_get(data_root, &size, str);
	if (ret)
	{
		void** ud = lua_newuserdata(L, sizeof(void*));
		*ud = ret;
		luaL_getmetatable(L, "Gar.data");
		lua_setmetatable(L, -2);
		lua_pushinteger(L, size);
		return 2;
	}
	else
		return 0;
}

static int lua_image_totexture(lua_State *L)
{
	if (!lua_istable(L, -1))
		goto error;
	lua_getfield(L, -1, "data");
	if (!lua_checkmetatable(L, "Gar.data"))
		goto error;
	void** data = lua_touserdata(L, -1);
	image_texture* tex = image_totexture(*data);
	if (tex)
	{
		image_texture** ud = lua_newuserdata(L, sizeof(image_texture*));
		*ud = tex;
		luaL_getmetatable(L, "Image.texture");
		lua_setmetatable(L, -2);
		return 1;
	}
	return 0;
	
	error:
	return luaL_argerror(L, 1, "bad data");
}

static int lua_image_drawtexture(lua_State *L)
{
	if (!bf_drawable)
	{
		lua_pushstring(L, "attempt to draw to screen outside of an onDraw event");
		return lua_error(L);
	}
	
	int narg = lua_gettop(L);
	image_texture** tex = luaL_checkudata(L, 1, "Image.texture");
	double x = luaL_checknumber(L, 2);
	double y = luaL_checknumber(L, 3);
	if (narg > 3)
	{
		double width = luaL_checknumber(L, 4);
		double height = luaL_checknumber(L, 5);
		if (narg > 5)
		{
			double rotx = luaL_checknumber(L, 6);
			double roty = luaL_checknumber(L, 7);
			double angle = luaL_checknumber(L, 8);
			image_drawrotate(*tex, x, y, width, height, rotx, roty, angle);
		}
		else
			image_drawscale(*tex, x, y, width, height);
	}
	else
		image_drawtexture(*tex, x, y);
	return 0;
}

static int lua_image_deletetexture(lua_State *L)
{
	image_texture** tex = luaL_checkudata(L, 1, "Image.texture");
	image_deletetexture(*tex);
	return 0;
}

static int lua_object_new(lua_State *L)
{
	object** ud = lua_newuserdata(L, sizeof(object*));
	*ud = object_create();
	luaL_getmetatable(L, "Object.object");
	lua_setmetatable(L, -2);
	return 1;
}

static int lua_object_makedrawable(lua_State *L)
{
	int narg = lua_gettop(L);
	object** obj = luaL_checkudata(L, 1, "Object.object");
	double x = luaL_checknumber(L, 2);
	double y = luaL_checknumber(L, 3);
	int z = luaL_checkinteger(L, 4);
	double r = luaL_checkinteger(L, 5);
	image_texture** texture = luaL_checkudata(L, 6, "Image.texture");
	double offsetx = (**texture).width / 2;
	double offsety = (**texture).height / 2;
	
	if (!*obj)
		return luaL_argerror(L, 1, "object already destroyed");
	
	if (z < 0 || z > 255)
		return luaL_argerror(L, 1, "bad z position");
	
	if (narg > 6)
	{
		offsetx = luaL_checknumber(L, 7);
		offsety = luaL_checknumber(L, 8);
	}
	
	object_makedrawable(*obj, x, y, z, r, *texture, offsetx, offsety);
	return 0;
}

static int lua_object_destroy(lua_State *L)
{
	object** obj = luaL_checkudata(L, 1, "Object.object");
	object_destroy(*obj);
	*obj = NULL;
	return 0;
}

static void script_uninit()
{
	if (script_state)
		lua_close(script_state);
}

bool script_init()
{
	script_state = luaL_newstate();
	atexit(script_uninit);
	
	luaopen_base(script_state);
	luaopen_string(script_state);
	luaopen_table(script_state);
	luaopen_math(script_state);
	
	lua_pushnil(script_state);
	lua_setglobal(script_state, "collectgarbage");
	lua_pushnil(script_state);
	lua_setglobal(script_state, "dofile");
	lua_pushnil(script_state);
	lua_setglobal(script_state, "loadfile");
	lua_pushnil(script_state);
	lua_setglobal(script_state, "print");
	lua_pushnil(script_state);
	lua_setglobal(script_state, "module");
	lua_pushnil(script_state);
	lua_setglobal(script_state, "require");
	
	lua_register(script_state, "trace", lua_trace);
	
	luaL_newmetatable(script_state, "Gar.data");
	lua_pop(script_state, 1);
	
	luaL_newmetatable(script_state, "Image.texture");
	lua_newtable(script_state);
	lua_pushcfunction(script_state, lua_image_drawtexture);
	lua_setfield(script_state, -2, "draw");
	lua_setfield(script_state, -2, "__index");
	lua_pushcfunction(script_state, lua_image_deletetexture);
	lua_setfield(script_state, -2, "__gc");
	lua_pop(script_state, 1);
	
	luaL_newmetatable(script_state, "Object.object");
	lua_newtable(script_state);
	lua_pushcfunction(script_state, lua_object_destroy);
	lua_setfield(script_state, -2, "destroy");
	lua_pushcfunction(script_state, lua_object_makedrawable);
	lua_setfield(script_state, -2, "makeDrawable");
	lua_setfield(script_state, -2, "__index");
	lua_pushcfunction(script_state, lua_object_destroy);
	lua_setfield(script_state, -2, "__gc");
	lua_pop(script_state, 1);
	
	lua_newtable(script_state);
	lua_pushinteger(script_state, bf_width);
	lua_setfield(script_state, -2, "width");
	lua_pushinteger(script_state, bf_height);
	lua_setfield(script_state, -2, "height");
	lua_setfield(script_state, LUA_REGISTRYINDEX, "Display");
	lua_getfield(script_state, LUA_REGISTRYINDEX, "Display");
	lua_setglobal(script_state, "Display");
	
	lua_newtable(script_state);
	lua_pushliteral(script_state, "get");
	lua_pushcfunction(script_state, lua_gar_get);
	lua_rawset(script_state, -3);
	lua_setglobal(script_state, "Gar");
	
	lua_newtable(script_state);
	lua_pushliteral(script_state, "toTexture");
	lua_pushcfunction(script_state, lua_image_totexture);
	lua_rawset(script_state, -3);
	lua_setglobal(script_state, "Image");
	
	lua_newtable(script_state);
	lua_pushliteral(script_state, "new");
	lua_pushcfunction(script_state, lua_object_new);
	lua_rawset(script_state, -3);
	lua_setglobal(script_state, "Object");
	
	extern const unsigned char engine_init_lua[];	
	extern const unsigned long engine_init_lua_size;
	if (!script_runbuf(engine_init_lua, engine_init_lua_size))
	{
		lua_close(script_state);
		return false;
	}
	
	return true;
}

bool script_runbuf(const void* buf, size_t size)
{
	if (luaL_loadbuffer(script_state, buf, size, "data"))
	{
		tracef("script: Couldn't parse buffer, %s", lua_tostring(script_state, -1));
		lua_pop(script_state, 1);
		return false;
	}
	if (lua_pcall(script_state, 0, 1, 0))
	{
		tracef("script: Couldn't execute buffer, %s", lua_tostring(script_state, -1));
		lua_pop(script_state, 1);
		return false;
	}
	lua_pop(script_state, lua_gettop(script_state));
	return true;
}
