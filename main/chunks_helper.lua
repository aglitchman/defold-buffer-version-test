local chunks_ids = require("main.chunks.chunks")

local M = {}

function M.init()
    M.meshes = {}
    M.mesh_pool = {}
    M.empty_buffer = ext.make_empty_buffer()

    for i, chunk_id in ipairs(chunks_ids.chunks) do
        local mesh = {
            url = msg.url("/chunks/meshes#" .. chunk_id .. "_mesh"),
            vertices_res = go.get("/chunks/meshes#" .. chunk_id .. "_mesh", "vertices"),
            draws = false
        }
        table.insert(M.meshes, mesh)
        table.insert(M.mesh_pool, #M.meshes)

        resource.set_buffer(mesh.vertices_res, M.empty_buffer)
    end
end

function M.acquire_mesh(vertices_buf)
    if #M.mesh_pool == 0 then
        print("No meshes available")
        return 0
    end

    local handle = table.remove(M.mesh_pool)
    local mesh = M.meshes[handle]
    mesh.draws = true

    resource.set_buffer(mesh.vertices_res, vertices_buf)

    return handle
end

function M.release_mesh(handle)
    if handle == 0 then
        return
    end

    local mesh = M.meshes[handle]
    mesh.draws = false

    table.insert(M.mesh_pool, handle)
end

function M.update_meshes_consts(dt)
    for mesh_handle = 1, #M.meshes do
        local mesh = M.meshes[mesh_handle]

        if mesh.draws then
           
            if mesh.draws_dirty then
                mesh.draws_dirty = false

                local msg_type = ENABLE
                if not mesh.frustum_visible then
                    msg_type = DISABLE
                end

                if mesh.vertices_count > 0 then
                    msg.post(mesh.url, msg_type)
                end
                if mesh.water_vertices_count > 0 then
                    msg.post(mesh.water_url, msg_type)
                end

                update_fog_dist = true
            end

            mesh.fade_in = math.min(1, mesh.fade_in + dt * 2)
            camera_payload.w = mesh.fade_in

            if mesh.frustum_visible then
                if mesh.vertices_count > 0 then
                    if update_fog_dist then
                        go.set(mesh.url, fog_dist_hash, fog_dist_payload)
                    end
                    go.set(mesh.url, camera_hash, camera_payload)
                    go.set(mesh.url, fog_color_daylight_hash, fog_color_daylight_payload)
                    mu = mu + 1
                    mvt = mvt + mesh.vertices_count
                end

                if mesh.water_vertices_count > 0 then
                    if update_fog_dist then
                        go.set(mesh.water_url, fog_dist_hash, fog_dist_payload)
                    end
                    go.set(mesh.water_url, fog_color_daylight_hash, fog_color_daylight_payload)
                    go.set(mesh.water_url, camera_hash, camera_payload)
                    mu = mu + 1
                    mvt = mvt + mesh.water_vertices_count
                end
            end
        else
            if mesh.draws_dirty then
                mesh.draws_dirty = false
                mesh.fade_in = 0
                msg.post(mesh.url, DISABLE)
                msg.post(mesh.water_url, DISABLE)
            end
        end
    end

    M.meshes_updated = mu
    M.meshes_used = mu2
    M.meshes_total = mt * 2
    M.meshes_vertices_total = mvt
    M.force_update_all_consts = false

    rb_profile.leave()
end

return M