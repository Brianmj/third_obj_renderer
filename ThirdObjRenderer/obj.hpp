//
//  obj.hpp
//  Sprite1
//
//  Created by Brian Jones on 3/14/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#ifndef OBJ_HPP
#define OBJ_HPP

#include <memory>
#include <vector>
#include <fstream>
#include <regex>
#include <unordered_map>
#include <knu/mathlibrary5.hpp>


namespace knu
{
    struct Obj_Material
    {
        std::string mat_name;
        knu::math::Vector4f mat_ambient_color;
        knu::math::Vector4f mat_diffuse_color;
        knu::math::Vector4f mat_specular_color;
        double mat_specular_exponent;
        double mat_opacity;
        std::string mat_diffuse_texture_name;
        std::string mat_ambient_texture_name;
        
    };
    
    struct Obj_Face
    {
        int v_index;
        int t_index;
        int n_index;
        
        explicit Obj_Face(int v_index_, int t_index_ = -1, int n_index_ = -1):
        v_index(v_index_), t_index(t_index_), n_index(n_index_) {}
    };
    
    struct Obj_Mesh
    {
        std::string obj_name;
        std::string mat_name;
        std::vector<Obj_Face> faces;
    };
    
    
    class Obj_Reader 
    {
        
        typedef void (Obj_Reader::* Parser)(std::string line);
        typedef std::unordered_map<std::string, Parser>::value_type String_Parser_Map;
        
        struct Mat_Compare_With_Line
        {
            Mat_Compare_With_Line(std::string line_):
            line(line_)
            {
                
            }
            
            bool operator()(const String_Parser_Map &p)
            {
                std::regex r (p.first);
                return (regex_search(line, r)) || (line.find(p.first) != std::string::npos);
            }
            
            std::string line;
        };
        
        struct Obj_Compare_With_Line
        {
            Obj_Compare_With_Line(std::string line_):
            line(line_)
            {
                
            }
            
            bool operator()(const String_Parser_Map &p)
            {
                std::regex r(p.first);
                return (std::regex_search(line, r)) || (line.find(p.first) != std::string::npos);
            }
            
            std::string line;
        };
        
        void register_handlers();
        
        std::string read_next_line(std::ifstream &file);
		std::vector<std::string> split_string(std::string str);
        void read_material_file(std::string material_path);
        void read_obj_file(std::string obj_path);
        void dispatch_to_mat_parser(std::string line);
        void dispatch_to_obj_parser(std::string line);
        
        
        void mat_count_handler(std::string line);
        void mat_name_handler(std::string line);
        void mat_ambient_handler(std::string line);
        void mat_diffuse_handler(std::string line);
        void mat_specular_handler(std::string line);
        void mat_specular_exponent_handler(std::string line);
        void mat_opacity_handler(std::string line);
        void mat_diffuse_texture_handler(std::string line);
        void mat_ambient_texture_handler(std::string line);
        
        void obj_new_mesh_handler(std::string line);
        void obj_vertex_handler(std::string line);
        void obj_texture_handler(std::string line);
        void obj_normal_handler(std::string line);
        void obj_add_mat_to_mesh_handler(std::string line);
        void obj_face_handler(std::string line);
        
        inline bool check_eof(std::ifstream &file) 
        {
            return file.eof();
        }
        
        std::unordered_map<std::string, Parser> obj_matches;
        std::unordered_map<std::string, Parser> mat_matches; 
        
    public:
        Obj_Reader(std::string material_path, std::string obj_path);
        std::vector<knu::math::Vector3f> vertices;
        std::vector<knu::math::Vector2f> tex_coords;
        std::vector<knu::math::Vector3f> normals;
        std::vector<Obj_Material> materials;
        std::vector<Obj_Mesh> meshes;
        
    };

    enum class ObjFormat
    {
        Vertex,    // vertex only
        VertexTexture,    // vertex texture
        VertexNormal,    // vertex normal
        VertexTextureNormal // vertex texture normal
    };
    
    struct Mesh
    {
		int vertex_count;
		ObjFormat model_format;
        std::string material;
        std::vector<knu::math::Vector3f> v;
        std::vector<knu::math::Vector2f> t;
        std::vector<knu::math::Vector3f> n;

		std::vector<float> interleaved_array();
    };
    
    class Obj
    {
        std::shared_ptr<knu::Obj_Reader> model_data;
        
    private:
        void make_obj(std::string mat_path, std::string obj_path);
        void make_mat();
        std::string get_path(std::string file_name);
        
    public:
        Obj();
        Obj(std::string material_name, std::string obj_name);
        void load_obj(std::string material_name, std::string obj_name);
        ObjFormat model_format;
		unsigned int model_format_size;
        std::vector<Mesh> meshes;
        std::unordered_map<std::string, knu::Obj_Material> str_mat_map;
        
        std::vector<knu::math::Vector3f> get_vertex_data() const
        {
            return meshes.front().v;
        }
        
        std::vector<knu::math::Vector2f> get_tex_coord_data() const
        {
            return meshes.front().t;
        }
        
        std::vector<knu::math::Vector3f> get_normal_data() const
        {
            return meshes.front().n;
        }

		Mesh first_mesh()
		{
			return meshes[0];
		}
        
    };
    
}

#endif
