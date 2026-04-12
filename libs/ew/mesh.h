/*
*	Author: Eric Winebrenner
*/

#pragma once
#include <glm/glm.hpp>
#include <vector>

namespace ew {
	struct Vertex {
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec2 uv;
	};

	struct MeshData {
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
	};

	enum class DrawMode {
		TRIANGLES = 0,
		POINTS = 1,
		LINES = 2,
	};

	class Mesh {
	public:
		Mesh() {};
		Mesh(const MeshData& meshData, bool instanced = false);
		void load(const MeshData& meshData, bool instanced = false);
		/// Bind per-instance `glm::mat4` columns to attributes 3–6. Call once after creating the instance VBO.
		void bindInstanceBuffer(unsigned int instanceVbo) const;
		void draw(DrawMode drawMode = DrawMode::TRIANGLES, int count = 1)const;
		inline int getNumVertices()const { return m_numVertices; }
		inline int getNumIndices()const { return m_numIndices; }
	private:
		bool m_initialized = false;
		unsigned int m_vao = 0;
		unsigned int m_vbo = 0;
		unsigned int m_ebo = 0;
		unsigned int m_numVertices = 0;
		unsigned int m_numIndices = 0;
	};
}