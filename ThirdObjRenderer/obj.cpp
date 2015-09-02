//
//  obj.cpp
//  Sprite1
//
//  Created by Brian Jones on 3/14/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//
#ifdef __APPLE__
//#include <Foundation/Foundation.h>
#endif  // __APPLE__

#include <iostream>
#include <stdexcept>
#include <future>
#include <sstream>

#include "obj.hpp"

using namespace std;
using namespace knu;


const string MAT_STRING_REP = "(\\.mtl)";
const string OBJ_STRING_REP = "(.obj)";

const string MAT_COUNT = "(Count)";
const string MAT_NAME = "newmtl (\\w+)";
const string MAT_AMBIENT =	"Ka ([-+]?[0-9]*\\.?[0-9]*) ([-+]?[0-9]*\\.?[0-9]*) ([-+]?[0-9]*\\.?[0-9]*)";
const string MAT_DIFFUSE =	"Kd ([-+]?[0-9]*\\.?[0-9]*) ([-+]?[0-9]*\\.?[0-9]*) ([-+]?[0-9]*\\.?[0-9]*)";
const string MAT_SPECULAR = "Ks ([-+]?[0-9]*\\.?[0-9]*) ([-+]?[0-9]*\\.?[0-9]*) ([-+]?[0-9]*\\.?[0-9]*)";
const string MAT_SPECULAR_EXPONENT = "Ns ([-+]?[0-9]*\\.?[0-9]*)";
const string MAT_OPACITY = "d ([-+]?[0-9]*\\.?[0-9]*)";
const string MAT_DIFFUSE_TEXTURE = "map_Kd (\\w+\\.\\w+)";
const string MAT_AMBIENT_TEXTURE = "map_Ka";

const string OBJ_NEW_MESH = "o (\\w+)";
const string OBJ_VERTEX = "v ([-+]?[0-9]*\\.?[0-9]*) ([-+]?[0-9]*\\.?[0-9]*) ([-+]?[0-9]*\\.?[0-9]*)";
const string OBJ_TEXTURE = "vt ([-+]?[0-9]*\\.?[0-9]*) ([-+]?[0-9]*\\.?[0-9]*)";
const string OBJ_NORMAL = "vn ([-+]?[0-9]*\\.?[0-9]*) ([-+]?[0-9]*\\.?[0-9]*) ([-+]?[0-9]*\\.?[0-9]*)";

const string OBJ_USE_MATERIAL = "usemtl (\\w+)";
const string OBJ_FACE_0 = "f (\\d+) (\\d+) (\\d+)";         // 3 indices per line (triangle) | vertex only
const string OBJ_FACE_1 = "f (\\d+) (\\d+) (\\d+) (\\d+)";       // 4 indices per line (Quad) | vertex only
const string OBJ_FACE_2 = "f (\\d+)/(\\d+)/(\\d+) (\\d+)/(\\d+)/(\\d+) (\\d+)/(\\d+)/(\\d+)";    // vertex/texture/normal
const string OBJ_FACE_3 = "f (\\d+)//(\\d+) (\\d+)//(\\d+) (\\d+)//(\\d+)";		// vertex//normal x 3
const string OBJ_FACE_4 = "f (\\d+)/(\\d+) (\\d+)/(\\d+) (\\d+)/(\\d+)";		// vertex/texture


void Obj_Reader::register_handlers()
{
    mat_matches[MAT_COUNT] = &Obj_Reader::mat_count_handler;
    mat_matches[MAT_NAME] = &Obj_Reader::mat_name_handler;
    mat_matches[MAT_AMBIENT] = &Obj_Reader::mat_ambient_handler;
    mat_matches[MAT_DIFFUSE] = &Obj_Reader::mat_diffuse_handler;
    mat_matches[MAT_SPECULAR] = &Obj_Reader::mat_specular_handler;
    mat_matches[MAT_SPECULAR_EXPONENT] = &Obj_Reader::mat_specular_exponent_handler;
    mat_matches[MAT_OPACITY] = &Obj_Reader::mat_opacity_handler;
    mat_matches[MAT_DIFFUSE_TEXTURE] = &Obj_Reader::mat_diffuse_texture_handler;
    
    obj_matches[OBJ_NEW_MESH] = &Obj_Reader::obj_new_mesh_handler;
    obj_matches[OBJ_VERTEX] = &Obj_Reader::obj_vertex_handler;
    obj_matches[OBJ_TEXTURE] = &Obj_Reader::obj_texture_handler;
    obj_matches[OBJ_NORMAL] = &Obj_Reader::obj_normal_handler;
    obj_matches[OBJ_USE_MATERIAL] = &Obj_Reader::obj_add_mat_to_mesh_handler;
    obj_matches[OBJ_FACE_0] = &Obj_Reader::obj_face_handler;
    obj_matches[OBJ_FACE_1] = &Obj_Reader::obj_face_handler;
    obj_matches[OBJ_FACE_2] = &Obj_Reader::obj_face_handler;
	obj_matches[OBJ_FACE_3] = &Obj_Reader::obj_face_handler;
    obj_matches[OBJ_FACE_4] = &Obj_Reader::obj_face_handler;
}

Obj_Reader::Obj_Reader(string material_path, string obj_path)
{
    register_handlers();
    try
    {
		read_material_file(material_path);
		read_obj_file(obj_path);
		//auto mat_future = async(launch::async, &Obj_Reader::read_material_file, this, material_path);
		//auto obj_future = async(launch::async, &Obj_Reader::read_obj_file, this, obj_path);

		//mat_future.get();
		//obj_future.get();
    }catch(std::exception &ex)
    {
        std::cerr << ex.what() << std::endl;
        std::exit(-1);
    }
}

std::string Obj_Reader::read_next_line(std::ifstream &file)
{
    string line;
    getline(file, line);
    return line;
}

std::vector<std::string> Obj_Reader::split_string(std::string str)
{
	auto b = begin(str);
	auto e = end(str);

	vector<string> vs;
	decltype(b) temp;

	while (b != e)
	{
		temp = find(b, e, '\n');
		if (temp != e)
		{
			// temp++ so that the newline char is not included in the current string
			vs.push_back(string(b, temp++));
		}
		b = temp;
	}

	return vs;
}

void Obj_Reader::read_material_file(std::string material_path)
{
    regex r(MAT_STRING_REP);
    smatch match;
    
    if(!regex_search(material_path, match, r))
        throw runtime_error("Not a material file");
    
    ifstream file(material_path);
    
    if(!file)
        throw runtime_error("Could not open material file at " + material_path);
    
    while(!check_eof(file))
    {
        auto current_line = read_next_line(file);
        dispatch_to_mat_parser(current_line);
        
    }
}

void Obj_Reader::read_obj_file(std::string obj_path)
{
    regex r(OBJ_STRING_REP);
    smatch match;
    
    if(!regex_search(obj_path, match, r))
        throw runtime_error("Not a .obj file");
    
    ifstream file(obj_path);
    
    if(!file)
        throw runtime_error("Could not open obj file at" + obj_path);
	
	string str = string(istreambuf_iterator<char>{file}, istreambuf_iterator<char>{});
	auto vs = split_string(str);

	for (auto current_line : vs)
	{
		dispatch_to_obj_parser(current_line);
	}

    /*while(!check_eof(file))
    {
        auto current_line = read_next_line(file);
        dispatch_to_obj_parser(current_line);
    }*/
}

void Obj_Reader::dispatch_to_mat_parser(std::string line)
{
    auto iterator = find_if(begin(mat_matches), end(mat_matches), Mat_Compare_With_Line(line));
    if(iterator != end(mat_matches))
        (this->*iterator->second)(line);
    
}

void Obj_Reader::dispatch_to_obj_parser(std::string line)
{
    auto iterator = find_if(begin(obj_matches), end(obj_matches), Obj_Compare_With_Line(line));
    if(iterator != end(obj_matches))
        (this->*iterator->second)(line);
}

void Obj_Reader::mat_name_handler(std::string line)
{
    regex r(MAT_NAME);
    smatch match;
    regex_search(line, match, r);
    materials.push_back(Obj_Material());
    materials.back().mat_name = match[1];
}

void Obj_Reader::mat_count_handler(std::string line)
{
    stringstream ss(line);
    string identifier;
    int count = 0;
    ss >> identifier >> identifier >> identifier >> count;
    materials.reserve(count);
    
}

void Obj_Reader::mat_ambient_handler(std::string line)
{
    regex r(MAT_AMBIENT);
    smatch match;
    regex_search(line, match, r);
    
    materials.back().mat_ambient_color = knu::math::Vector4f(stof(match[1]), stof(match[2]), stof(match[3]), 1.0f);
}

void Obj_Reader::mat_diffuse_handler(std::string line)
{
    regex r(MAT_DIFFUSE);
    smatch match;
    regex_search(line, match, r);
    materials.back().mat_diffuse_color = knu::math::Vector4f(stof(match[1]), stof(match[2]), stof(match[3]), 1.0f);
}

void Obj_Reader::mat_specular_handler(std::string line)
{
    regex r(MAT_SPECULAR);
    smatch match;
    regex_search(line, match, r);
    materials.back().mat_specular_color = knu::math::Vector4f(stof(match[1]), stof(match[2]), stof(match[3]), 1.0f);
}

void Obj_Reader::mat_specular_exponent_handler(std::string line)
{
    regex r(MAT_SPECULAR_EXPONENT);
    smatch match;
    regex_search(line, match, r);
    materials.back().mat_specular_exponent = stof(match[1]);
}

void Obj_Reader::mat_opacity_handler(std::string line)
{
    regex r(MAT_OPACITY);
    smatch match;
    
    // Match instead of search in order to avoid conflict with MAT_DIFFUSE (kd #)
    if(regex_match(line, match, r))
    {
        materials.back().mat_opacity = stof(match[1]);
    }
	else	// check if it matches with MAT_DIFFUSE
	{
		
		regex r2(MAT_DIFFUSE);
		if(regex_match(line, match, r2))
		{
			mat_diffuse_handler(line);
		}
	}
}

void Obj_Reader::mat_diffuse_texture_handler(std::string line)
{
    regex r(MAT_DIFFUSE_TEXTURE);
    smatch match;
    auto pos = line.find_last_of("/");
    
    if(line.npos != pos)
    {
        ++pos;
        std::string texture_name(line.begin() + pos, line.end());
        materials.back().mat_diffuse_texture_name = texture_name;
    }else
    {
        regex_search(line, match, r);
        std::string textureName = match[1];
        materials.back().mat_diffuse_texture_name = textureName;
    }
}

void Obj_Reader::mat_ambient_texture_handler(std::string line)
{
    // TODO - THIS FUNCTION NEEDS TO BE TESTED
    regex r(MAT_AMBIENT_TEXTURE);
    smatch match;
    auto pos = line.find_last_of("/");
    
    if(line.npos != pos)
    {
        ++pos;
        std::string texture_name(line.begin() + pos, line.end());
        materials.back().mat_ambient_texture_name = texture_name;
    }else
    {
        regex_search(line, match, r);
        std::string textureName = match[1];
        materials.back().mat_ambient_texture_name = textureName;
    }
}

void Obj_Reader::obj_new_mesh_handler(std::string line)
{
    regex r(OBJ_NEW_MESH);
    smatch match;
    
    regex_search(line, match, r);
    meshes.push_back(Obj_Mesh());
    meshes.back().obj_name = match[1];
    
}

void Obj_Reader::obj_vertex_handler(std::string line)
{
    regex r(OBJ_VERTEX);
    smatch match;
    
    regex_search(line, match, r);
    vertices.push_back(knu::math::Vector3f(stof(match[1]), stof(match[2]), stof(match[3])));
}

void Obj_Reader::obj_texture_handler(std::string line)
{
    regex r(OBJ_TEXTURE);
    smatch match;
    
    regex_search(line, match, r);
    tex_coords.push_back(knu::math::Vector2f(stof(match[1]), stof(match[2])));
}

void Obj_Reader::obj_normal_handler(std::string line)
{
    regex r(OBJ_NORMAL);
    smatch match;
    
    regex_search(line, match, r);
    normals.push_back(knu::math::Vector3f(stof(match[1]), stof(match[2]), stof(match[3])));
}

void Obj_Reader::obj_add_mat_to_mesh_handler(std::string line)
{

    regex r1(OBJ_USE_MATERIAL);
    smatch match;
    regex_search(line, match, r1);

    std::string m_name = match[1];
    meshes.back().mat_name = m_name;
    
}

void Obj_Reader::obj_face_handler(std::string line)
{
    regex r0(OBJ_FACE_0);

    smatch match;
    
    if(regex_match(line, match, r0))
    {
        // 3 indices per face.
        // Only vertices x 3
        meshes.back().faces.push_back(Obj_Face(stoi(match[1]) -1, -1, -1));
        meshes.back().faces.push_back(Obj_Face(stoi(match[2]) -1, -1, -1));
        meshes.back().faces.push_back(Obj_Face(stoi(match[3]) -1, -1, -1));
        return;
    }
    // remember to subtract 1 from each indice as Wavefront obj files start at 0
    
    regex r1(OBJ_FACE_1);
    
    if(regex_match(line, match, r1))
    {
        // 4 indices per face.
        // Only vertices x 4
        meshes.back().faces.push_back(Obj_Face(stoi(match[1]) -1, -1, -1));
        meshes.back().faces.push_back(Obj_Face(stoi(match[2]) -1, -1, -1));
        meshes.back().faces.push_back(Obj_Face(stoi(match[3]) -1, -1, -1));
        meshes.back().faces.push_back(Obj_Face(stoi(match[4]) -1, -1, -1));
        return;
    }
    
    regex r2(OBJ_FACE_2);
    
    if(regex_match(line, match, r2))
    {
        // vertex/texture/normal x 3
        meshes.back().faces.push_back(Obj_Face(stoi(match[1]) - 1, stoi(match[2]) - 1, stoi(match[3]) - 1));
        meshes.back().faces.push_back(Obj_Face(stoi(match[4]) - 1, stoi(match[5]) - 1, stoi(match[6]) - 1));
        meshes.back().faces.push_back(Obj_Face(stoi(match[7]) - 1, stoi(match[8]) - 1, stoi(match[9]) - 1));
        return;
    }

	regex r3(OBJ_FACE_3);

	if(regex_match(line, match, r3))
	{
		// vertex//normal x 3
		meshes.back().faces.push_back(Obj_Face(stoi(match[1]) - 1, -1, stoi(match[2]) - 1));
		meshes.back().faces.push_back(Obj_Face(stoi(match[3]) - 1, -1, stoi(match[4]) - 1));
		meshes.back().faces.push_back(Obj_Face(stoi(match[5]) - 1, -1, stoi(match[6]) - 1));
		return;
	}
    
    regex r4(OBJ_FACE_4);
    
	if(regex_match(line, match, r4))
	{
		// vertex//normal x 3
		meshes.back().faces.push_back(Obj_Face(stoi(match[1]) - 1, stoi(match[2]) - 1, -1));
		meshes.back().faces.push_back(Obj_Face(stoi(match[3]) - 1, stoi(match[4]) - 1, -1));
		meshes.back().faces.push_back(Obj_Face(stoi(match[5]) - 1, stoi(match[6]) - 1, -1));
		return;
	}
}


Obj::Obj()
{
    
}

Obj::Obj(string material_name, string obj_name)
{
    load_obj(material_name, obj_name);
}

string Obj::get_path(std::string file_name)
{
    std::string file_path;
/*#ifdef __APPLE__
    int pos = file_name.find('.');
    if(pos == string::npos)
        throw (runtime_error("No file extension in: " + file_name));
    
    string name(begin(file_name), begin(file_name) + pos++);
    string extension(begin(file_name) + pos, end(file_name));
    
    NSString *file_name_ = [[NSBundle mainBundle] pathForResource:[NSString stringWithCString:name.c_str() encoding:NSUTF8StringEncoding] ofType:[NSString stringWithCString:extension.c_str() encoding:NSUTF8StringEncoding]];
    
    if(nil == file_name_)
        throw (runtime_error("Unable to find path for: " + file_name));
    
    file_path = file_name_.UTF8String;
    
#else   */                
    file_path = file_name;  // Windows machine ?
    
    return file_path;
}

void Obj::make_obj(string mat_path, string obj_path)
{
    model_data.reset(new knu::Obj_Reader(mat_path, obj_path));
    
    if((!model_data->normals.empty()) && (!model_data->tex_coords.empty()))
    {
        // Vertex, normals and texture coords are present
        model_format = ObjFormat::VertexTextureNormal;
		model_format_size = sizeof(knu::math::Vector3f) + sizeof(knu::math::Vector3f) + sizeof(knu::math::Vector2f);
        for(auto &m : model_data->meshes)
        {
            meshes.push_back(Mesh());
            meshes.back().material = m.mat_name;
			meshes.back().model_format = ObjFormat::VertexTextureNormal;
			meshes.back().vertex_count = m.faces.size();
            
            for (auto &f : m.faces)
            {
                meshes.back().v.push_back(knu::math::Vector3f());
                meshes.back().t.push_back(knu::math::Vector2f());
                meshes.back().n.push_back(knu::math::Vector3f());
                meshes.back().v.back() = knu::math::Vector3f(model_data->vertices[f.v_index].x, model_data->vertices[f.v_index].y,
                                                                  model_data->vertices[f.v_index].z);
                meshes.back().t.back() = knu::math::Vector2f(model_data->tex_coords[f.t_index].x, model_data->tex_coords[f.t_index].y);
                meshes.back().n.back() = knu::math::Vector3f(model_data->normals[f.n_index].x, model_data->normals[f.n_index].y,
                                                                  model_data->normals[f.n_index].z);
            }
            
        }
        
    }else if (model_data->normals.empty() && (!model_data->tex_coords.empty()))
    {
        // tex coords are present, but no normals
        model_format = ObjFormat::VertexTexture;
		model_format_size = sizeof(knu::math::Vector3f) + sizeof(knu::math::Vector2f);
        for(auto &m : model_data->meshes)
        {
            meshes.push_back(Mesh());
            meshes.back().material = m.mat_name;
			meshes.back().model_format = ObjFormat::VertexTexture;
			meshes.back().vertex_count = m.faces.size();
            
            for (auto &f : m.faces)
            {
                meshes.back().v.push_back(knu::math::Vector3f());
                meshes.back().t.push_back(knu::math::Vector2f());
                
                meshes.back().v.back() = knu::math::Vector3f(model_data->vertices[f.v_index].x, model_data->vertices[f.v_index].y,
                                                             model_data->vertices[f.v_index].z);
                meshes.back().t.back() = knu::math::Vector2f(model_data->tex_coords[f.t_index].x, model_data->tex_coords[f.t_index].y);
            }
            
        }

        
    }else if ((!model_data->normals.empty()) && model_data->tex_coords.empty())
    {
        // normals are present, but no tex coords
        model_format = ObjFormat::VertexNormal;
		model_format_size = sizeof(knu::math::Vector3f) + sizeof(knu::math::Vector3f);
        
        for(auto &m : model_data->meshes)
        {
            meshes.push_back(Mesh());
            meshes.back().material = m.mat_name;
			meshes.back().model_format = ObjFormat::VertexNormal;
			meshes.back().vertex_count = m.faces.size();
            
            for (auto &f : m.faces)
            {
                meshes.back().v.push_back(knu::math::Vector3f());
                meshes.back().n.push_back(knu::math::Vector3f());
                
                meshes.back().v.back() = knu::math::Vector3f(model_data->vertices[f.v_index].x, model_data->vertices[f.v_index].y,
                                                             model_data->vertices[f.v_index].z);
                meshes.back().n.back() = knu::math::Vector3f(model_data->normals[f.n_index].x, model_data->normals[f.n_index].y,
                                                             model_data->normals[f.n_index].z);
            }
        }

        
    }else if (model_data->normals.empty() && model_data->tex_coords.empty())
    {
        // no normals or tex coords
        model_format = ObjFormat::Vertex;
        model_format_size = sizeof(knu::math::Vector3f);

        for(auto &m : model_data->meshes)
        {
            meshes.push_back(Mesh());
            meshes.back().material = m.mat_name;
			meshes.back().model_format = ObjFormat::Vertex;
			meshes.back().vertex_count = m.faces.size();
            
            meshes.back().v.reserve(m.faces.size());
            for (auto &f : m.faces)
            {
                meshes.back().v.push_back(knu::math::Vector3f());
                meshes.back().v.back() = knu::math::Vector3f(model_data->vertices[f.v_index].x, model_data->vertices[f.v_index].y,
                                                             model_data->vertices[f.v_index].z);
            }
        }

    }
}

void Obj::make_mat()
{
    for (auto &m : model_data->materials)
    {
		str_mat_map[m.mat_name] = m;
    }
}

void Obj::load_obj(std::string material_name, std::string obj_name)
{
    auto mat_path = get_path(material_name);
    auto obj_path = get_path(obj_name);
    make_obj(mat_path, obj_path);
    make_mat();
	model_data.reset();
}

std::vector<float> Mesh::interleaved_array()
{
	std::vector<float> interleavedArray;

	switch (model_format)
	{
	case ObjFormat::Vertex:
	{
		for (auto const &vertex : v)
		{
			interleavedArray.emplace_back(vertex.x);
			interleavedArray.emplace_back(vertex.y);
			interleavedArray.emplace_back(vertex.z);
		}
	}break;

	case ObjFormat::VertexNormal:
	{
		for (int i = 0; i < vertex_count; ++i)
		{
			auto vertex = v[i];
			auto normal = n[i];

			interleavedArray.emplace_back(vertex.x);
			interleavedArray.emplace_back(vertex.y);
			interleavedArray.emplace_back(vertex.z);
			interleavedArray.emplace_back(normal.x);
			interleavedArray.emplace_back(normal.y);
			interleavedArray.emplace_back(normal.z);

		}
	}break;

	}

	return interleavedArray;
}