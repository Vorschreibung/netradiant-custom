#include <string>
#include <string.h>
#include <dlfcn.h>
#include <libgen.h>

#include <luajit-2.1/lua.hpp>

#include "luaapi.h"

#include "luaengine.h"

// #include <lua.hpp>

/* #include <stdio.h> */

// #include <stdint.h>
// #include "lua.h"
// #include "lualib.h"
// #include "lauxlib.h"
// #include <math.h>
// #include "scenelib.h"

// static instance
// static scene::Instance* instance;

/* #include "./luaaa.hpp" */
/* using namespace luaaa; */

enum LadState
{
	LAD_STATE_UNINITIALIZED,
	LAD_STATE_INITIALIZED,
	LAD_STATE_ERROR,
};

static LadState lad_state = LAD_STATE_UNINITIALIZED;
static lua_State *L;

int add_library_path( lua_State* L ){
	char *library_path;
	{
		Dl_info info;
		if ( !dladdr( (void*)"lua_getlocal", &info ) ) {
			return 0;
		}
		// printf( "Loaded from path = %s\n", info.dli_fname );
		// library_path = dirname((char*)(info.dli_fname));
		library_path = strdup( info.dli_fname );
		library_path = dirname( library_path );
	}

	lua_getglobal( L, "package" );
	lua_getfield( L, -1, "path" ); // get field "path" from table at top of stack (-1)
	std::string cur_path = lua_tostring( L, -1 ); // grab path string from top of stack
	cur_path.append( ";" ); // do your path magic here
	cur_path.append( library_path );
	cur_path.append( "/?.lua" );
	lua_pop( L, 1 ); // get rid of the string on the stack we just pushed on line 5
	lua_pushstring( L, cur_path.c_str() ); // push the new one
	lua_setfield( L, -2, "path" ); // set the field "path" in table at -2 with value at top of stack
	lua_pop( L, 1 ); // get rid of package table from top of stack
	return 0; // all done!
}

// class LuaBrush
// {
// Brush &b;
// public:
// LuaBrush ();
// LuaBrush ( Brush &b_ ) : b( b_ ) {};
// virtual ~LuaBrush ();

// int size() { return b.size(); }

// private:
// /* data */
// };

int lad_init(){
	printf( "Initializing Lua Engine ...\n" );

	L = luaL_newstate();
	if ( !L ) {
		printf( "Failed to initialize lua\n" );
		return -1;
	}

	luaL_openlibs( L );  // load Lua libraries

	// add_library_path( L );

	if ( lad_init_luaapi( L ) ) {
		printf( "Failed to initialize lua api\n" );
		return -1;
	}

	printf( "Lua Engine successfully initialized\n" );
	return 0;
}

int lad_run(){
	// initialize lua on first run
	if ( lad_state == LAD_STATE_UNINITIALIZED ) {
		if ( lad_init() ) {
			lad_state = LAD_STATE_ERROR;
		}
		else {
			lad_state = LAD_STATE_INITIALIZED;
		}
	}

	if ( lad_state == LAD_STATE_ERROR ) {
		fprintf( stderr, "Lua Engine is in an error state. Ignoring invocation.\n" );
		return -1;
	}

	// const char* script = "print(c_sin(90))";
	int status = luaL_loadfile( L, "/home/raf/luadiant.lua" );  // load Lua script
	// status = luaL_loadfilex( L, "/home/raf/luadiant.lua", "rb" );  // load Lua script
	// status = luaL_loadstring(L, script);
	int ret = lua_pcall( L, 0, 0, 0 );
	if ( ret != 0 ) {
		fprintf( stderr, "ERROR: %s\n", lua_tostring( L, -1 ) );
		return 1;
	}

	return status;
}

void lad_run_menu_action(){
	lad_run();
}
