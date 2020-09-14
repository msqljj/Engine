#pragma once
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <Windows.h>

namespace Demo
{
	class VulkanShader
	{
	public:
		/**
		*	Class used for testing new shaders during development.
		*
		*	generateBatchFile() Generates a batch file that that will be used together with
		*	glslc.exe to generate spirV shaders from glsl text code.
		*
		*/
		VulkanShader();
		~VulkanShader();
		void generateSpirVShaders(const char* shaderNames);
		void setBatchFileName(const char* batchFileName);
		void setWorkingDirectory(const char* directory);
		void setFilenames(const char* vertShaderIn, const char* fragShaderIn, const char* vertShaderOut, const char* fragShaderOut);
		void generateVertexShaderTextFile(const char* shaderCode);
		void generateFragmentShaderTextFile(const char* shaderCode);
	private:
		std::filesystem::path cwd = std::filesystem::current_path() / "temp\\";
		std::string temp_folder = "temp\\";
		std::string batch_file_name = "shaderBatch.bat";
		std::string inVertShaderName = "shader.vert";
		std::string inFragShaderName = "shader.frag";
		std::string outVertShaderName = "vert.spv";
		std::string outFragShaderName = "frag.spv";
	};
}


