#include <mesh.hpp>
#include <iostream>
#include <vector> // Required for std::vector
#include <string> // Required for std::string, std::stoi
#include <algorithm> // Required for std::min, std::max

// Temporary structure to hold face data (vertex and UV indices)
struct FaceData {
    int v_indices[3];
    int uv_indices[3];
    bool has_uvs;
};

bool Mesh::LoadObjFromFile(std::string filename, bool centerModel)
{
    std::ifstream f(filename);
    if (!f.is_open())
        return false;

    std::vector<Vec3> local_verts; // Cache for raw vertex positions
    std::vector<UV> local_uvs;     // Cache for raw UV coordinates
    std::vector<FaceData> face_data_list; // Cache for face definitions

    // Initialize AABB with large/small values
    aabb.min = {10000.0f, 10000.0f, 10000.0f};
    aabb.max = {-10000.0f, -10000.0f, -10000.0f};

    std::string line_str;
    while (std::getline(f, line_str))
    {
        std::stringstream s;
        s << line_str;
        char junk;

        if (line_str[0] == 'v' && line_str[1] == ' ')
        {
            Vec3 v;
            s >> junk >> v.x >> v.y >> v.z;
            local_verts.push_back(v);

            // Update AABB based on raw vertices
            aabb.min.x = std::min(aabb.min.x, v.x);
            aabb.min.y = std::min(aabb.min.y, v.y);
            aabb.min.z = std::min(aabb.min.z, v.z);
            aabb.max.x = std::max(aabb.max.x, v.x);
            aabb.max.y = std::max(aabb.max.y, v.y);
            aabb.max.z = std::max(aabb.max.z, v.z);
        }
        else if (line_str[0] == 'v' && line_str[1] == 't')
        {
            UV uv;
            std::string junkWord; // vt can be followed by more than one char
            s >> junkWord >> uv.u >> uv.v;
            local_uvs.push_back(uv);
        }
        else if (line_str[0] == 'f')
        {
            FaceData current_face;
            current_face.has_uvs = false;
            std::string face_tokens[3];
            s >> junk >> face_tokens[0] >> face_tokens[1] >> face_tokens[2];

            for (int i = 0; i < 3; ++i) {
                std::string token = face_tokens[i];
                std::size_t first_slash = token.find('/');
                
                current_face.v_indices[i] = std::stoi(token.substr(0, first_slash));

                if (first_slash != std::string::npos) {
                    // Check for UV coordinates
                    std::size_t second_slash = token.find('/', first_slash + 1);
                    if (second_slash != first_slash +1 && first_slash + 1 < token.length() && (second_slash == std::string::npos || second_slash > first_slash + 1) ) {
                         // Check if the character after the first slash is a digit
                        if (isdigit(token[first_slash+1])) {
                            current_face.uv_indices[i] = std::stoi(token.substr(first_slash + 1, second_slash - (first_slash + 1)));
                            current_face.has_uvs = true;
                        } else { // If no digit after first slash, no UV for this vertex
                             current_face.uv_indices[i] = 0; // Default or indicate no UV
                        }
                    } else {
                        current_face.uv_indices[i] = 0; // Default if no UV index part
                    }
                    // Normals would be after the second slash, ignored for now
                } else {
                     current_face.uv_indices[i] = 0; // No slashes, so no UVs
                }
            }
            face_data_list.push_back(current_face);
        }
    }
    f.close(); // Close the file stream once parsing is done.

    if (local_verts.empty()) {
      // std::cerr << "No vertices found in OBJ file: " << filename << std::endl;
      return false;
    }
    
    if (centerModel) { // Conditionally center the model
        // Calculate model center from the initial AABB
        Vec3 modelCenter = {
            (aabb.min.x + aabb.max.x) / 2.0f,
            (aabb.min.y + aabb.max.y) / 2.0f,
            (aabb.min.z + aabb.max.z) / 2.0f
        };
        // std::cout << "Calculated model center: (" << modelCenter.x << ", " << modelCenter.y << ", " << modelCenter.z << ")" << std::endl;

        // Adjust all local vertices to be relative to the modelCenter
        for (Vec3 &v : local_verts) {
            v.x -= modelCenter.x;
            v.y -= modelCenter.y;
            v.z -= modelCenter.z;
        }
        // std::cout << "Adjusted " << local_verts.size() << " vertices by model center." << std::endl;
    } else {
        // std::cout << "Model centering skipped for: " << filename << std::endl;
    }

    // Now create triangles using the (potentially adjusted) vertices and cached face data
    UV defaultUVs[3] = {{0, 0, 1}, {1, 0, 1}, {0, 1, 1}}; // Added w=1

    for (const auto& face_def : face_data_list) {
        Vec3 tri_verts[3];
        UV tri_uvs[3];

        for (int i = 0; i < 3; ++i) {
            if (face_def.v_indices[i] > 0 && face_def.v_indices[i] <= local_verts.size()) {
                tri_verts[i] = local_verts[face_def.v_indices[i] - 1];
            } else {
                // Handle error or default vertex
                // std::cerr << "Invalid vertex index: " << face_def.v_indices[i] << std::endl;
                tri_verts[i] = {0,0,0,1}; // Default
            }

            if (face_def.has_uvs && face_def.uv_indices[i] > 0 && face_def.uv_indices[i] <= local_uvs.size()) {
                tri_uvs[i] = local_uvs[face_def.uv_indices[i] - 1];
            } else {
                tri_uvs[i] = defaultUVs[i % 3]; // Cycle through default UVs
            }
        }
        tris.push_back({tri_verts[0], tri_verts[1], tri_verts[2], tri_uvs[0], tri_uvs[1], tri_uvs[2], {255, 255, 255}});
    }

    // Recalculate AABB based on the *adjusted* (centered) triangle vertices
    if (!tris.empty()) {
        aabb.min = tris[0].p[0];
        aabb.max = tris[0].p[0];
        for (const auto& tri : tris) {
            for (int i = 0; i < 3; i++) {
                aabb.min.x = std::min(aabb.min.x, tri.p[i].x);
                aabb.min.y = std::min(aabb.min.y, tri.p[i].y);
                aabb.min.z = std::min(aabb.min.z, tri.p[i].z);
                aabb.max.x = std::max(aabb.max.x, tri.p[i].x);
                aabb.max.y = std::max(aabb.max.y, tri.p[i].y);
                aabb.max.z = std::max(aabb.max.z, tri.p[i].z);
            }
        }
    } else if (local_verts.empty()){
        // If there were no vertices at all, AABB remains as initially set (large/small values)
        // or could be set to zero. For now, leave as is or consider resetting.
         aabb.min = {0,0,0};
         aabb.max = {0,0,0};
    }


    // std::cout << "Loaded " << tris.size() << " triangles" << (centerModel ? " (centered)" : " (original pivot)") << std::endl;
    // std::cout << (centerModel ? "Centered AABB: " : "Original AABB: ");
    // std::cout << "min(" << aabb.min.x << "," << aabb.min.y << "," << aabb.min.z << ") ";
    // std::cout << "max(" << aabb.max.x << "," << aabb.max.y << "," << aabb.max.z << ")" << std::endl;

    return true;
}