#define LIB_NAME "Ext"
#define MODULE_NAME "ext"

#include <dmsdk/sdk.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

static void DbgBreak() { *((volatile unsigned *)0) = 0x0; }

static int buffers_created = 0;
static int buffers_freed = 0;

static dmBuffer::HBuffer GenTriBuf(float r, float g, float b, float t_x, float t_y, float scale, float angle) {
    static dmhash_t stream_position = dmHashString64("position");
    static dmhash_t stream_color = dmHashString64("color");
    const dmBuffer::StreamDeclaration streams_decl[] = {
        {stream_position, dmBuffer::VALUE_TYPE_FLOAT32, 3},
        {stream_color, dmBuffer::VALUE_TYPE_FLOAT32, 4},
    };
    dmBuffer::HBuffer hbuffer = 0;
    if (dmBuffer::Create(3, streams_decl, 2, &hbuffer) != dmBuffer::RESULT_OK) {
        dmLogFatal("Can't create buffer: %d, line %d; buffers created %d, freed %d.", r, __LINE__, buffers_created,
                   buffers_freed);
        DbgBreak();
    }
    // dmLogInfo("Created buffer #%u", hbuffer & 0xffff);

    buffers_created++;

    float *positions = 0;
    float *colors = 0;
    uint32_t count = 0;
    uint32_t components = 0;
    uint32_t stride = 0;
    if (dmBuffer::GetStream(hbuffer, stream_position, (void **)&positions, &count, &components, &stride) !=
        dmBuffer::RESULT_OK) {
        dmLogFatal("Can't get stream: %d, line %d, buffer #%u; buffers created %d, freed %d.", r, __LINE__,
                   hbuffer & 0xffff, buffers_created, buffers_freed);
        DbgBreak();
    }
    if (dmBuffer::GetStream(hbuffer, stream_color, (void **)&colors, NULL, NULL, NULL) != dmBuffer::RESULT_OK) {
        dmLogFatal("Can't get stream: %d, line %d, buffer #%u; buffers created %d, freed %d.", r, __LINE__,
                   hbuffer & 0xffff, buffers_created, buffers_freed);
        DbgBreak();
    }

    glm::mat4 matrix = glm::mat4(1.0);
    matrix = glm::translate(matrix, glm::vec3(t_x, t_y, 0.0f));
    matrix = glm::rotate(matrix, angle, glm::vec3(0.0f, 0.0f, 1.0f));
    matrix = glm::scale(matrix, glm::vec3(scale));

    float tri[] = {0.0f, 0.0f, 1.0f, 0.5f, 0.0f, 1.0f};
    float center_x = (tri[0] + tri[2] + tri[4]) / 3.0f;
    float center_y = (tri[1] + tri[3] + tri[5]) / 3.0f;
    int t = 0;

    for (uint32_t i = 0; i < count; ++i) {
        glm::vec4 t_pos = matrix * glm::vec4(glm::vec3(tri[t] - center_x, tri[t + 1] - center_y, 0.0f), 1.0f);
        t += 2;

        positions[0] = t_pos.x;
        positions[1] = t_pos.y;
        positions[2] = t_pos.z;

        colors[0] = r;
        colors[1] = g;
        colors[2] = b;
        colors[3] = 1.0f;

        positions += stride;
        colors += stride;
    }

    return hbuffer;
}

static int MakeTriangle(lua_State *L) {
    DM_LUA_STACK_CHECK(L, 1);

    float r = luaL_checknumber(L, 1);
    float g = luaL_checknumber(L, 2);
    float b = luaL_checknumber(L, 3);

    float t_x = luaL_checknumber(L, 4);
    float t_y = luaL_checknumber(L, 5);
    float scale = luaL_checknumber(L, 6);
    float angle = luaL_checknumber(L, 7);

    dmBuffer::HBuffer hbuffer = GenTriBuf(r, g, b, t_x, t_y, scale, angle);
    if (lua_toboolean(L, 8) == true) {
        dmScript::PushBuffer(L, {{hbuffer}, {dmScript::OWNER_LUA}});
    } else {
        dmScript::PushBuffer(L, {{hbuffer}, {dmScript::OWNER_C}});
    }

    return 1;
}

static int FreeBuffer(lua_State *L) {
    DM_LUA_STACK_CHECK(L, 0);

    dmBuffer::HBuffer hbuffer = dmScript::CheckBuffer(L, 1)->m_Buffer;
    // dmLogInfo("Destroy buffer #%u", hbuffer & 0xffff);
    dmBuffer::Destroy(hbuffer);

    buffers_freed++;

    return 0;
}

// Functions exposed to Lua
static const luaL_reg Module_methods[] = {{"make_triangle", MakeTriangle},
                                          {"free_buffer", FreeBuffer},
                                          /* Sentinel: */
                                          {NULL, NULL}};

static void LuaInit(lua_State *L) {
    int top = lua_gettop(L);

    // Register lua names
    luaL_register(L, MODULE_NAME, Module_methods);

    lua_pop(L, 1);
    assert(top == lua_gettop(L));
}

static dmExtension::Result InitializeExt(dmExtension::Params *params) {
    LuaInit(params->m_L);
    return dmExtension::RESULT_OK;
}

static dmExtension::Result FinalizeExt(dmExtension::Params *params) { return dmExtension::RESULT_OK; }

DM_DECLARE_EXTENSION(Ext, LIB_NAME, 0, 0, InitializeExt, 0, 0, FinalizeExt)
